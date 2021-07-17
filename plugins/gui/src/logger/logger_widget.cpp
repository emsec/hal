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
        infoSeverity = true;
        warningSeverity = true;
        errorSeverity = true;

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
        connect(selector, SIGNAL(currentIndexChanged(int)), this, SLOT(handleCurrentChannelChanged(int)));
        Toolbar->addWidget(selector);


        infoSelector = new SeveritySelector(this);
        infoSelector->setChecked(true);
        connect(infoSelector, SIGNAL(stateChanged(int)), this, SLOT(handleCurrentChannelChanged(int)));
        infoSelector->setText("Info");
        Toolbar->addWidget(infoSelector);

        warningSelector = new SeveritySelector(this);
        warningSelector->setChecked(true);
        connect(warningSelector, SIGNAL(stateChanged(int)), this, SLOT(handleCurrentChannelChanged(int)));
        warningSelector->setText("Warning");
        Toolbar->addWidget(warningSelector);

        errorSelector = new SeveritySelector(this);
        errorSelector->setChecked(true);
        connect(errorSelector, SIGNAL(stateChanged(int)), this, SLOT(handleCurrentChannelChanged(int)));
        errorSelector->setText("Error");
        Toolbar->addWidget(errorSelector);
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

        bool filter = false;
        if ((t == spdlog::level::level_enum::info) && infoSeverity) {
            filter = true;
        }
        else if ((t == spdlog::level::level_enum::warn) && warningSeverity) {
            filter = true;
        }
        else if ((t == spdlog::level::level_enum::err) && errorSeverity) {
            filter = true;
        }

        if (filter)
        {
            mLogMarshall->appendLog(t, QString::fromStdString(msg));
        }
    }

    void LoggerWidget::handleCurrentChannelChanged(int p)
    {
        if (sender() == infoSelector)
        {
            if (p == 2)
            {
                infoSeverity = true;
            }
            else
            {
                infoSeverity = false;
            }
        }
        else if (sender() == warningSelector)
        {
            if (p == 2)
            {
                warningSeverity = true;
            }
            else
            {
                warningSeverity = false;
            }
        }
        else if (sender() == errorSelector)
        {
            if (p == 2)
            {
                errorSeverity = true;
            }
            else
            {
                errorSeverity = false;
            }
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
            if ((entry->mMsgType == spdlog::level::level_enum::info) && infoSeverity) {
                filter = true;
            }
            else if ((entry->mMsgType == spdlog::level::level_enum::warn) && warningSeverity) {
                filter = true;
            }
            else if ((entry->mMsgType == spdlog::level::level_enum::err) && errorSeverity) {
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
