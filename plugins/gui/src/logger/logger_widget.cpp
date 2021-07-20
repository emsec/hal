#include "gui/logger/logger_widget.h"
#include "gui/channel_manager/channel_model.h"
#include "gui/channel_manager/channel_selector.h"
#include "gui/logger/logger_marshall.h"
#include "gui/toolbar/toolbar.h"

#include <QHeaderView>
#include <QVBoxLayout>
#include <QSignalMapper>
#include <QPushButton>
#include <QLabel>

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

        mInfoSeverity = true;
        mWarningSeverity = true;
        mErrorSeverity = true;

        connect(mPlainTextEditScrollbar, &QScrollBar::actionTriggered, this, &LoggerWidget::handleFirstUserInteraction);

        ChannelModel* model = ChannelModel::get_instance();
        connect(model, SIGNAL(updated(spdlog::level::level_enum, std::string, std::string)), this, SLOT(handleChannelUpdated(spdlog::level::level_enum, std::string, std::string)));
    }

    LoggerWidget::~LoggerWidget()
    {

    }

    void LoggerWidget::setupToolbar(Toolbar* Toolbar)
    {

        //selector will be deleted within the toolbars destructor
        selector = new ChannelSelector(this);
        connect(selector, SIGNAL(currentIndexChanged(int)), this, SLOT(handleCurrentFilterChanged(int)));
        selector->setStyleSheet("QCombobox {background-color : rgb(32, 43, 63); border-radius: 2px;}");
        Toolbar->addWidget(selector);

        mDebugButton = new QPushButton("Debug", this);
        mDebugButton->setCheckable(true);
        mDebugButton->setStyleSheet("QPushButton { border : none; background-color : rgb(26, 26, 26) } QPushButton:checked { background-color : rgb(89, 89, 89); color : rgb(204, 204, 204)} ");
        connect(mDebugButton, SIGNAL(toggled(bool)), this, SLOT(handleSeverityChanged(bool)));
        Toolbar->addWidget(mDebugButton);

        mInfoButton = new QPushButton("Info", this);
        mInfoButton->setCheckable(true);
        mInfoButton->setChecked(true);
        mInfoButton->setStyleSheet("QPushButton { border : none; background-color : rgb(26, 26, 26) } QPushButton:checked { background-color : rgb(89, 89, 89); color : rgb(204, 204, 204)} ");
        connect(mInfoButton, SIGNAL(toggled(bool)), this, SLOT(handleSeverityChanged(bool)));
        Toolbar->addWidget(mInfoButton);

        mWarningButton = new QPushButton("Warning", this);
        mWarningButton->setCheckable(true);
        mWarningButton->setChecked(true);
        mWarningButton->setStyleSheet("QPushButton { border : none; background-color : rgb(26, 26, 26) } QPushButton:checked { background-color : rgb(89, 89, 89); color : rgb(204, 204, 204)} ");
        connect(mWarningButton, SIGNAL(toggled(bool)), this, SLOT(handleSeverityChanged(bool)));
        Toolbar->addWidget(mWarningButton);

        mErrorButton = new QPushButton("Error", this);
        mErrorButton->setCheckable(true);
        mErrorButton->setChecked(true);
        mErrorButton->setStyleSheet("QPushButton { border : none; background-color : rgb(26, 26, 26) } QPushButton:checked { background-color : rgb(89, 89, 89); color : rgb(204, 204, 204)} ");
        connect(mErrorButton, SIGNAL(toggled(bool)), this, SLOT(handleSeverityChanged(bool)));
        Toolbar->addWidget(mErrorButton);
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
            handleCurrentFilterChanged(0);
        }
        if (logger_name != mCurrentChannel)
            return;


        bool filter = false;
        if ((t == spdlog::level::level_enum::info) && mInfoSeverity) {
            filter = true;
        }
        else if ((t == spdlog::level::level_enum::warn) && mWarningSeverity) {
            filter = true;
        }
        else if ((t == spdlog::level::level_enum::err) && mErrorSeverity) {
            filter = true;
        }
        else if ((t == spdlog::level::level_enum::debug) && mDebugSeverity) {
            filter = true;
        }
        else if (t == spdlog::level::level_enum::critical) {
            filter = true;
        }

        if (filter)
        {
            mLogMarshall->appendLog(t, QString::fromStdString(msg));
        }
    }

    void LoggerWidget::handleCurrentFilterChanged(int p)
    {
        if ((sender() == selector) || p == 0) {
            mCurrentChannelIndex = p;
        }


        ChannelModel* model = ChannelModel::get_instance();
        ChannelItem* item   = static_cast<ChannelItem*>((model->index(mCurrentChannelIndex, 0, QModelIndex())).internalPointer());

        mCurrentChannel = item->name().toStdString();

        mPlainTextEdit->clear();
        QWriteLocker item_locker(item->getLock());
        for (ChannelEntry* entry : *(item->getList()))
        {
            bool filter = false;
            if ((entry->mMsgType == spdlog::level::level_enum::info) && mInfoSeverity) {
                filter = true;
            }
            else if ((entry->mMsgType == spdlog::level::level_enum::warn) && mWarningSeverity) {
                filter = true;
            }
            else if ((entry->mMsgType == spdlog::level::level_enum::err) && mErrorSeverity) {
                filter = true;
            }
            else if ((entry->mMsgType == spdlog::level::level_enum::debug) && mDebugSeverity) {
                filter = true;
            }
            else if (entry->mMsgType == spdlog::level::level_enum::critical) {
                filter = true;
            }

            if (filter)
            {
            mLogMarshall->appendLog(entry->mMsgType, QString::fromStdString(entry->mMsg));
            }
        }
    }

    void LoggerWidget::handleSeverityChanged(bool state)
    {
        if (sender() == mDebugButton)
        {
            mDebugSeverity = state;
        }
        else if (sender() == mInfoButton)
        {
            mInfoSeverity = state;
        }
        else if (sender() == mWarningButton)
        {
            mWarningSeverity = state;
        }
        else if (sender() == mErrorButton)
        {
            mErrorSeverity = state;
        }

        handleCurrentFilterChanged(1);

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
