#include "gui/logger/logger_widget.h"
#include "gui/channel_manager/channel_model.h"
#include "gui/channel_manager/channel_selector.h"
#include "gui/logger/logger_marshall.h"
#include "gui/toolbar/toolbar.h"

#include <QHeaderView>
#include <QVBoxLayout>

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

        mLogMarshall = new LoggerMarshall(mPlainTextEdit);
        mContentLayout->addWidget(mPlainTextEdit);

        mPlainTextEdit->setContextMenuPolicy(Qt::CustomContextMenu);

        mSelector = new ChannelSelector();

        connect(mSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(handleCurrentChannelChanged(int)));
        connect(mPlainTextEditScrollbar, &QScrollBar::actionTriggered, this, &LoggerWidget::handleFirstUserInteraction);

        ChannelModel* model = ChannelModel::get_instance();
        connect(model, SIGNAL(updated(spdlog::level::level_enum, std::string, std::string)), this, SLOT(handleChannelUpdated(spdlog::level::level_enum, std::string, std::string)));
    }

    LoggerWidget::~LoggerWidget()
    {
        //cant set the parent correct, so just delete them in the constructor
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

    void LoggerWidget::handleCurrentChannelUpdated(spdlog::level::level_enum t, const QString& msg)
    {
        mLogMarshall->appendLog(t, msg);
    }

    void LoggerWidget::handleChannelUpdated(spdlog::level::level_enum t, const std::string& logger_name, const std::string& msg)
    {
        if (mCurrentChannel == "")
        {
            handleCurrentChannelChanged(0);
        }
        if (logger_name != mCurrentChannel)
            return;

        mLogMarshall->appendLog(t, QString::fromStdString(msg));
    }

    void LoggerWidget::handleCurrentChannelChanged(int index)
    {
        ChannelModel* model = ChannelModel::get_instance();
        ChannelItem* item   = static_cast<ChannelItem*>((model->index(index, 0, QModelIndex())).internalPointer());

        mCurrentChannel = item->name().toStdString();

        mPlainTextEdit->clear();
        QWriteLocker item_locker(item->getLock());
        for (ChannelEntry* entry : *(item->getList()))
        {
            mLogMarshall->appendLog(entry->mMsgType, QString::fromStdString(entry->mMsg));
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
