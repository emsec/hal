#include "gui/logger/logger_widget.h"
#include "gui/channel_manager/channel_model.h"
#include "gui/channel_manager/channel_selector.h"
#include "gui/logger/logger_marshall.h"
#include "gui/toolbar/toolbar.h"
#include "gui/logger/logger_settings.h"
#include "gui/settings/settings_manager.h"

#include <QHeaderView>
#include <QVBoxLayout>
#include <QSignalMapper>
#include <QPushButton>
#include <QLabel>
#include <QSpacerItem>
#include <QLineEdit>

#include <iostream>


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

        restoreSettings();

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
        selector->setEditable(true);
        connect(selector->lineEdit(), SIGNAL(editingFinished()), this, SLOT(handleCustomChannel()));
        connect(selector, SIGNAL(currentIndexChanged(int)), this, SLOT(handleCurrentFilterChanged(int)));
        selector->setStyleSheet("QCombobox {background-color : rgb(32, 43, 63); border-radius: 2px;}");
        Toolbar->addWidget(selector);

        mDebugButton = new QPushButton("Debug", this);
        mDebugButton->setCheckable(true);
        mDebugButton->setChecked(mDebugSeverity);
        mDebugButton->setStyleSheet("QPushButton { border : none; background-color : rgb(26, 26, 26) } QPushButton:hover { background-color : rgb(46, 46, 46) } QPushButton:checked:hover { background-color : rgb(66, 66, 66) } QPushButton:checked { background-color : rgb(89, 89, 89); color : rgb(204, 204, 204)} ");
        connect(mDebugButton, SIGNAL(toggled(bool)), this, SLOT(handleSeverityChanged(bool)));
        Toolbar->addWidget(mDebugButton);

        mInfoButton = new QPushButton("Info", this);
        mInfoButton->setCheckable(true);
        mInfoButton->setChecked(mInfoSeverity);
        mInfoButton->setStyleSheet("QPushButton { border : none; background-color : rgb(26, 26, 26) } QPushButton:hover { background-color : rgb(46, 46, 46) } QPushButton:checked:hover { background-color : rgb(66, 66, 66) } QPushButton:checked { background-color : rgb(89, 89, 89); color : rgb(204, 204, 204)} ");
        connect(mInfoButton, SIGNAL(toggled(bool)), this, SLOT(handleSeverityChanged(bool)));
        Toolbar->addWidget(mInfoButton);

        mWarningButton = new QPushButton("Warning", this);
        mWarningButton->setCheckable(true);
        mWarningButton->setChecked(mWarningSeverity);
        mWarningButton->setStyleSheet("QPushButton { border : none; background-color : rgb(26, 26, 26) } QPushButton:hover { background-color : rgb(46, 46, 46) } QPushButton:checked:hover { background-color : rgb(66, 66, 66) } QPushButton:checked { background-color : rgb(89, 89, 89); color : rgb(204, 204, 204)} ");
        connect(mWarningButton, SIGNAL(toggled(bool)), this, SLOT(handleSeverityChanged(bool)));
        Toolbar->addWidget(mWarningButton);

        mErrorButton = new QPushButton("Error", this);
        mErrorButton->setCheckable(true);
        mErrorButton->setChecked(mErrorSeverity);
        mErrorButton->setStyleSheet("QPushButton { border : none; background-color : rgb(26, 26, 26) } QPushButton:hover { background-color : rgb(46, 46, 46) } QPushButton:checked:hover { background-color : rgb(66, 66, 66) } QPushButton:checked { background-color : rgb(89, 89, 89); color : rgb(204, 204, 204)} ");
        connect(mErrorButton, SIGNAL(toggled(bool)), this, SLOT(handleSeverityChanged(bool)));
        Toolbar->addWidget(mErrorButton);

        Toolbar->addSpacer();

        mMuteButton = new QPushButton("Mute all", this);
        mMuteButton->setStyleSheet("QPushButton { border : none; background-color : rgb(26, 26, 26) } QPushButton:hover { background-color : rgb(46, 46, 46) }  QPushButton:pressed { background-color : rgb(89, 89, 89); color : rgb(204, 204, 204)} ");
        connect(mMuteButton, SIGNAL(clicked(bool)), this, SLOT(handleSeverityChanged(bool)));
        Toolbar->addWidget(mMuteButton);

        mVerboseButton = new QPushButton("Show all", this);
        mVerboseButton->setStyleSheet("QPushButton { border : none; background-color : rgb(26, 26, 26) } QPushButton:hover { background-color : rgb(46, 46, 46) }  QPushButton:pressed { background-color : rgb(89, 89, 89); color : rgb(204, 204, 204)} ");
        connect(mVerboseButton, SIGNAL(clicked(bool)), this, SLOT(handleSeverityChanged(bool)));
        Toolbar->addWidget(mVerboseButton);
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
        ChannelModel* model = ChannelModel::get_instance();
        if ((sender() == selector) || p == 0) {
            mCurrentChannelIndex = p;
        }
        // set ALL Channel
        if (p == -1) {
            mCurrentChannelIndex = model->rowCount() - 1;
            selector->setCurrentIndex(mCurrentChannelIndex);
        }

        ChannelItem* item   = static_cast<ChannelItem*>((model->index(mCurrentChannelIndex, 0, QModelIndex())).internalPointer());
        mCurrentChannel = item->name().toStdString();
        selector->setCurrentText(QString::fromStdString(mCurrentChannel));

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
        else if (sender() == mMuteButton)
        {
            mDebugButton->setChecked(false);
            mInfoButton->setChecked(false);
            mWarningButton->setChecked(false);
            mErrorButton->setChecked(false);
        }
        else if (sender() == mVerboseButton)
        {
            mDebugButton->setChecked(true);
            mInfoButton->setChecked(true);
            mWarningButton->setChecked(true);
            mErrorButton->setChecked(true);
            handleCurrentFilterChanged(-1);
        }

        handleCurrentFilterChanged(1);
        saveSettings();
    }

    void LoggerWidget::handleFirstUserInteraction(int value)
    {
        Q_UNUSED(value);

        if (!mUserInteractedWithScrollbar)
            mUserInteractedWithScrollbar = true;
    }

    void LoggerWidget::handleCustomChannel()
    {
        std::string channel_name = (selector->currentText()).toStdString();
        ChannelModel* model = ChannelModel::get_instance();
        std::cout << channel_name << std::endl;
        model->handleLogmanagerCallback(spdlog::level::level_enum::debug , channel_name, channel_name + " has manually been added to channellist");
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

    void LoggerWidget::saveSettings()
    {
        LoggerSettings settings = { mDebugSeverity, mInfoSeverity, mWarningSeverity, mErrorSeverity };
        SettingsManager::instance()->saveLoggerSettings(settings);
    }

    void LoggerWidget::restoreSettings()
    {
        LoggerSettings settings = SettingsManager::instance()->loggerSettings();
        mDebugSeverity = settings.debugSeverity;
        mInfoSeverity = settings.infoSeverity;
        mWarningSeverity = settings.warningSeverity;
        mErrorSeverity = settings.errorSeverity;
    }
}
