#include "gui/logger/logger_widget.h"
#include "gui/channel_manager/channel_model.h"
#include "gui/channel_manager/channel_selector.h"
#include "gui/logger/filter_item.h"
#include "gui/logger/filter_tab_bar.h"
#include "gui/logger/logger_marshall.h"
#include "gui/toolbar/toolbar.h"
#include <QHeaderView>

namespace hal
{
    LoggerWidget::LoggerWidget(QWidget* parent) : ContentWidget("Log", parent)
    {
        mPlainTextEdit = new QPlainTextEdit(this);
        mPlainTextEdit->setReadOnly(true);
        mPlainTextEdit->setFrameStyle(QFrame::NoFrame);

        mPlainTextEditScrollbar = mPlainTextEdit->verticalScrollBar();
        scrollToBottom();
        mUserInteractedWithScrollbar = false;

        mTabBar      = new FilterTabBar();
        mLogMarshall = new LoggerMarshall(mPlainTextEdit);
        mContentLayout->addWidget(mPlainTextEdit);

        mPlainTextEdit->setContextMenuPolicy(Qt::CustomContextMenu);

        mSelector = new ChannelSelector();

        connect(mPlainTextEdit, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showTextEditContextMenu(const QPoint&)));
        connect(mTabBar, SIGNAL(currentChanged(int)), this, SLOT(filterItemClicked(int)));
        connect(mSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(handleCurrentChannelChanged(int)));
        connect(mPlainTextEditScrollbar, &QScrollBar::actionTriggered, this, &LoggerWidget::handleFirstUserInteraction);

        ChannelModel* model = ChannelModel::get_instance();
        connect(model, SIGNAL(updated(spdlog::level::level_enum, std::string, std::string)), this, SLOT(handleChannelUpdated(spdlog::level::level_enum, std::string, std::string)));
    }

    LoggerWidget::~LoggerWidget()
    {
        //cant set the parent correct, so just delete them in the constructor
        delete mTabBar;
        delete mSelector;
    }

    void LoggerWidget::setupToolbar(Toolbar* Toolbar)
    {
        //Toolbar->addWidget(mSelector);

        //selector will be deleted within the toolbars destructor
        ChannelSelector* selector = new ChannelSelector();
        connect(selector, SIGNAL(currentIndexChanged(int)), this, SLOT(handleCurrentChannelChanged(int)));
        Toolbar->addWidget(selector);
    }

    QPlainTextEdit* LoggerWidget::getPlainTextEdit()
    {
        return mPlainTextEdit;
    }

    FilterTabBar* LoggerWidget::getTabBar()
    {
        return mTabBar;
    }

    void LoggerWidget::showTextEditContextMenu(const QPoint& point)
    {
        Q_UNUSED(point)
    }

    void LoggerWidget::filterItemClicked(const int& index)
    {
        Q_UNUSED(index)
        //reloadLogContent();
    }

    void LoggerWidget::reloadLogContent()
    {
        //    hal_task_status_item *item = static_cast<hal_task_status_item*>(m_task_manager->get_tree_view()->currentIndex().internalPointer());
        //    FilterItem *filter = mTabBar->getCurrentFilter();

        //        mPlainTextEdit->clear();
        //        QWriteLocker item_locker(item->getLock());
        //        boost::circular_buffer<hal_task_log_entry*> *buffer = item->getBuffer();
        //        boost::circular_buffer<hal_task_log_entry*>::iterator itBegin = buffer->begin();
        //        boost::circular_buffer<hal_task_log_entry*>::iterator itEnd = buffer->end();
        //        for(; itBegin != itEnd; ++itBegin)
        //        {
        //            hal_task_log_entry *entry = *itBegin;
        //            mLogMarshall->appendLog(entry->mMsgType, QString::fromStdString(entry->mMsg), filter);
        //        }
    }    // item_locker scope

    //void LoggerWidget::handleCurrentChannelChanged(hal_channel_item* item)
    //{
    //    FilterItem* filter = mTabBar->getCurrentFilter();

    //    mPlainTextEdit->clear();
    //    QWriteLocker item_locker(item->getLock());
    //    boost::circular_buffer<hal_ChannelEntry*>* buffer           = item->getBuffer();
    //    boost::circular_buffer<hal_ChannelEntry*>::iterator itBegin = buffer->begin();
    //    boost::circular_buffer<hal_ChannelEntry*>::iterator itEnd   = buffer->end();
    //    for (; itBegin != itEnd; ++itBegin)
    //    {
    //        hal_ChannelEntry* entry = *itBegin;
    //        mLogMarshall->appendLog(entry->mMsgType, QString::fromStdString(entry->mMsg), filter);
    //    }
    //}    // item_locker scope

    void LoggerWidget::handleCurrentChannelUpdated(spdlog::level::level_enum t, const QString& msg)
    {
        FilterItem* filter = mTabBar->getCurrentFilter();
        mLogMarshall->appendLog(t, msg, filter);
    }

    void LoggerWidget::handleChannelUpdated(spdlog::level::level_enum t, const std::string& logger_name, const std::string& msg)
    {
        if (mCurrentChannel == "")
        {
            handleCurrentChannelChanged(0);
        }
        if (logger_name != mCurrentChannel)
            return;

        FilterItem* filter = mTabBar->getCurrentFilter();
        mLogMarshall->appendLog(t, QString::fromStdString(msg), filter);
    }

    void LoggerWidget::handleCurrentChannelChanged(int index)
    {
        ChannelModel* model = ChannelModel::get_instance();
        ChannelItem* item   = static_cast<ChannelItem*>((model->index(index, 0, QModelIndex())).internalPointer());

        mCurrentChannel = item->name().toStdString();

        FilterItem* filter = mTabBar->getCurrentFilter();

        mPlainTextEdit->clear();
        QWriteLocker item_locker(item->getLock());
        const boost::circular_buffer<ChannelEntry*>* buffer           = item->getBuffer();
        boost::circular_buffer<ChannelEntry*>::const_iterator itBegin = buffer->begin();
        boost::circular_buffer<ChannelEntry*>::const_iterator itEnd   = buffer->end();
        for (; itBegin != itEnd; ++itBegin)
        {
            ChannelEntry* entry = *itBegin;
            mLogMarshall->appendLog(entry->mMsgType, QString::fromStdString(entry->mMsg), filter);
        }
    }

    void LoggerWidget::handleFirstUserInteraction(int value)
    {
        Q_UNUSED(value);

        if (!mUserInteractedWithScrollbar)
            mUserInteractedWithScrollbar = true;
    }

    void LoggerWidget::resizeEvent(QResizeEvent* event)
    {
        Q_UNUSED(event)

        if (!mUserInteractedWithScrollbar)
            scrollToBottom();
    }

    void LoggerWidget::scrollToBottom()
    {
        mPlainTextEditScrollbar->setValue(mPlainTextEditScrollbar->maximum());
    }
}
