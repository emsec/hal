#include "gui/logger/logger_widget.h"
#include "gui/channel_manager/channel_model.h"
#include "gui/channel_manager/channel_selector.h"
#include "gui/severity_manager/severity_selector.h"
#include "gui/logger/logger_marshall.h"
#include "gui/toolbar/toolbar.h"

#include <QHeaderView>
#include <QVBoxLayout>
#include <QSignalMapper>

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

        //mSelector = new ChannelSelector();
        mInfoSeverity = true;
        mWarningSeverity = true;
        mErrorSeverity = true;

        //connect(mSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(handleCurrentChannelChanged(int)));
        connect(mPlainTextEditScrollbar, &QScrollBar::actionTriggered, this, &LoggerWidget::handleFirstUserInteraction);

        ChannelModel* model = ChannelModel::get_instance();
        connect(model, SIGNAL(updated(spdlog::level::level_enum, std::string, std::string)), this, SLOT(handleChannelUpdated(spdlog::level::level_enum, std::string, std::string)));
    }

    LoggerWidget::~LoggerWidget()
    {
        //cant set the parent correct, so just delete them in the constructor

        //delete mSelector;
    }

    void LoggerWidget::setupToolbar(Toolbar* Toolbar)
    {
        //Toolbar->addWidget(mSelector);

        //selector will be deleted within the toolbars destructor
        ChannelSelector* selector = new ChannelSelector(this);
        connect(selector, SIGNAL(currentIndexChanged(int)), this, SLOT(handleCurrentFilterChanged(int)));
        Toolbar->addWidget(selector);


        mInfoSelector = new SeveritySelector(this);
        mInfoSelector->setChecked(true);
        connect(mInfoSelector, SIGNAL(stateChanged(int)), this, SLOT(handleCurrentFilterChanged(int)));
        mInfoSelector->setText("Info");
        Toolbar->addWidget(mInfoSelector);

        mWarningSelector = new SeveritySelector(this);
        mWarningSelector->setChecked(true);
        connect(mWarningSelector, SIGNAL(stateChanged(int)), this, SLOT(handleCurrentFilterChanged(int)));
        mWarningSelector->setText("Warning");
        Toolbar->addWidget(mWarningSelector);

        mErrorSelector = new SeveritySelector(this);
        mErrorSelector->setChecked(true);
        connect(mErrorSelector, SIGNAL(stateChanged(int)), this, SLOT(handleCurrentFilterChanged(int)));
        mErrorSelector->setText("Error");
        Toolbar->addWidget(mErrorSelector);
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

        if (filter)
        {
            mLogMarshall->appendLog(t, QString::fromStdString(msg));
        }
    }

    void LoggerWidget::handleCurrentFilterChanged(int p)
    {
        if (sender() == mInfoSelector)
        {
            mInfoSeverity = (p == 2);
        }
        else if (sender() == mWarningSelector)
        {
            mWarningSeverity = (p == 2);
        }
        else if (sender() == mErrorSelector)
        {
            mErrorSeverity = (p == 2);
        }
        else
        {
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

            if (filter)
            {
            mLogMarshall->appendLog(entry->mMsgType, QString::fromStdString(entry->mMsg));
            }
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
