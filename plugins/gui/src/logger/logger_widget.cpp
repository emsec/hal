#include "gui/logger/logger_widget.h"
#include "gui/channel_manager/channel_model.h"
#include "gui/channel_manager/channel_selector.h"
#include "gui/logger/logger_marshall.h"
#include "gui/toolbar/toolbar.h"
#include "gui/logger/logger_settings.h"
#include "gui/settings/settings_manager.h"
#include "gui/searchbar/searchbar.h"

#include <QHeaderView>
#include <QVBoxLayout>
#include <QSignalMapper>
#include <QPushButton>
#include <QLabel>
#include <QSpacerItem>
#include <QLineEdit>
#include <QAction>
#include <QDebug>

#include <string>
#include <QRegularExpression>
#include <iostream>

namespace hal
{
    LoggerWidget::LoggerWidget(QWidget* parent) : ContentWidget("Log", parent)
    {
        // Active QPropertys
        ensurePolished();

        mPlainTextEdit = new QPlainTextEdit(this);
        mPlainTextEdit->setReadOnly(true);
        mPlainTextEdit->setFrameStyle(QFrame::NoFrame);

        mPlainTextEditScrollbar = mPlainTextEdit->verticalScrollBar();
        scrollToBottom();
        mUserInteractedWithScrollbar = false;

        mLogMarshall = new LoggerMarshall(mPlainTextEdit);
        mContentLayout->addWidget(mPlainTextEdit);

        mPlainTextEdit->setContextMenuPolicy(Qt::CustomContextMenu);
        // Get last severity settings
        restoreSettings();

        mSearchbar = new Searchbar(mPlainTextEdit);
        mSearchbar->hide();
        mContentLayout->addWidget(mSearchbar);
        mSearchOptions = 8;

        connect(mPlainTextEditScrollbar, &QScrollBar::actionTriggered, this, &LoggerWidget::handleFirstUserInteraction);

        ChannelModel* model = ChannelModel::instance();
        connect(model, SIGNAL(updated(spdlog::level::level_enum, std::string, std::string)), this, SLOT(handleChannelUpdated(spdlog::level::level_enum, std::string, std::string)));

        mSearchAction = new QAction(this);
        mSearchAction->setIcon(gui_utility::getStyledSvgIcon(mSearchIconStyle, mSearchIconPath));

        mChannelLabel = new QLabel(this);
        mChannelLabel->setText("all");

        mSelector = new ChannelSelector(this);
        mSelector->setEditable(true);
    }

    LoggerWidget::~LoggerWidget()
    {

    }

    void LoggerWidget::setupToolbar(Toolbar* toolbar)
    {
        // Action button to activate Searchbar
        toolbar->addAction(mSearchAction);

        // Show current channel
        toolbar->addWidget(mChannelLabel);

        // ChannelSelector
        connect(mSelector->lineEdit(), SIGNAL(editingFinished()), this, SLOT(handleCustomChannel()));
        connect(mSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(handleCurrentFilterChanged(int)));
//        mSelector->setStyleSheet("QCombobox {background-color : rgb(32, 43, 63); border-radius: 2px;}");
        mSelector->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
        toolbar->addWidget(mSelector);

        // Severity buttons
        mDebugButton = new QPushButton("Debug", this);
        mDebugButton->setCheckable(true);
        mDebugButton->setChecked(mDebugSeverity);
//        mDebugButton->setStyleSheet("QPushButton { border : none; background-color : rgb(26, 26, 26) } QPushButton:hover { background-color : rgb(46, 46, 46) } QPushButton:checked:hover { background-color : rgb(66, 66, 66) } QPushButton:checked { background-color : rgb(89, 89, 89); color : rgb(204, 204, 204)} ");
        toolbar->addWidget(mDebugButton);

        mInfoButton = new QPushButton("Info", this);
        mInfoButton->setCheckable(true);
        mInfoButton->setChecked(mInfoSeverity);
//        mInfoButton->setStyleSheet("QPushButton { border : none; background-color : rgb(26, 26, 26) } QPushButton:hover { background-color : rgb(46, 46, 46) } QPushButton:checked:hover { background-color : rgb(66, 66, 66) } QPushButton:checked { background-color : rgb(89, 89, 89); color : rgb(204, 204, 204)} ");
        toolbar->addWidget(mInfoButton);

        mWarningButton = new QPushButton("Warning", this);
        mWarningButton->setCheckable(true);
        mWarningButton->setChecked(mWarningSeverity);
//        mWarningButton->setStyleSheet("QPushButton { border : none; background-color : rgb(26, 26, 26) } QPushButton:hover { background-color : rgb(46, 46, 46) } QPushButton:checked:hover { background-color : rgb(66, 66, 66) } QPushButton:checked { background-color : rgb(89, 89, 89); color : rgb(204, 204, 204)} ");
        toolbar->addWidget(mWarningButton);

        mErrorButton = new QPushButton("Error", this);
        mErrorButton->setCheckable(true);
        mErrorButton->setChecked(mErrorSeverity);
//        mErrorButton->setStyleSheet("QPushButton { border : none; background-color : rgb(26, 26, 26) } QPushButton:hover { background-color : rgb(46, 46, 46) } QPushButton:checked:hover { background-color : rgb(66, 66, 66) } QPushButton:checked { background-color : rgb(89, 89, 89); color : rgb(204, 204, 204)} ");
        toolbar->addWidget(mErrorButton);

        toolbar->addSpacer();

        mMuteButton = new QPushButton("Mute all", this);
//        mMuteButton->setStyleSheet("QPushButton { border : none; background-color : rgb(26, 26, 26) } QPushButton:hover { background-color : rgb(46, 46, 46) }  QPushButton:pressed { background-color : rgb(89, 89, 89); color : rgb(204, 204, 204)} ");
        toolbar->addWidget(mMuteButton);

        mVerboseButton = new QPushButton("Show all", this);
//        mVerboseButton->setStyleSheet("QPushButton { border : none; background-color : rgb(26, 26, 26) } QPushButton:hover { background-color : rgb(46, 46, 46) }  QPushButton:pressed { background-color : rgb(89, 89, 89); color : rgb(204, 204, 204)} ");
        toolbar->addWidget(mVerboseButton);

        connect(mSearchAction, SIGNAL(triggered()), this, SLOT(toggleSearchbar()));
        connect(mSearchbar, SIGNAL(triggerNewSearch(QString, int)), this, SLOT(handleSearchChanged(QString, int)));
        connect(mDebugButton, SIGNAL(toggled(bool)), this, SLOT(handleSeverityChanged(bool)));
        connect(mInfoButton, SIGNAL(toggled(bool)), this, SLOT(handleSeverityChanged(bool)));
        connect(mWarningButton, SIGNAL(toggled(bool)), this, SLOT(handleSeverityChanged(bool)));
        connect(mErrorButton, SIGNAL(toggled(bool)), this, SLOT(handleSeverityChanged(bool)));
        connect(mMuteButton, SIGNAL(clicked(bool)), this, SLOT(handleSeverityChanged(bool)));
        connect(mVerboseButton, SIGNAL(clicked(bool)), this, SLOT(handleSeverityChanged(bool)));

        toolbar->repolish();
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

    bool LoggerWidget::isMatching(QString searchString, QString stringToCheck)
    {
        if(!mSearchOptions.isExactMatch() && !mSearchOptions.isRegularExpression()){
            //check if stringToCheck contains the searchString
            return stringToCheck.contains(searchString, mSearchOptions.isCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive);
        }
        else if(mSearchOptions.isExactMatch()){
            //check if the stringToCheck is the same as the searchString   - also checks CaseSensitivity

            return 0 == QString::compare(searchString, stringToCheck, mSearchOptions.isCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive);
        }
        else if(mSearchOptions.isRegularExpression()){
            //checks if the stringToCheck matches the regEx given by searchString
            auto regEx = QRegularExpression(searchString, mSearchOptions.isCaseSensitive() ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);
            return regEx.match(stringToCheck).hasMatch();
        }
        return false;
    }

    void LoggerWidget::handleCurrentFilterChanged(int p)
    {
        ChannelModel* model = ChannelModel::instance();

        // Channel has been changed or initially set
        if ((sender() == mSelector) || p == 0) {
            mCurrentChannelIndex = p;
        }

        // Set "all" Channel
        if (p == -1) {
            mCurrentChannelIndex = model->rowCount() - 1;
        }

        if (mCurrentChannelIndex == -2) {
            return;
        }

        ChannelItem* item   = static_cast<ChannelItem*>((model->index(mCurrentChannelIndex, 0, QModelIndex())).internalPointer());
        mCurrentChannel = item->name().toStdString();
        mChannelLabel->setText(QString::fromStdString(mCurrentChannel));
        mSelector->setCurrentText(QString::fromStdString(mCurrentChannel));

        mPlainTextEdit->clear();
        QWriteLocker item_locker(item->getLock());

        // Iterate through every log entry
        for (ChannelEntry* entry : *(item->getEntries()))
        {
            bool filter = false;
            // If entry msg matches with search string
            //QRegularExpression re(mSearchFilter);
            if (isMatching(mSearchFilter, QString::fromStdString(entry->mMsg)))
            {
                // If entry severity matches the choosen severitys
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
            }

            // If log entry should be shown
            if (filter)
            {
                mLogMarshall->appendLog(entry->mMsgType, QString::fromStdString(entry->mMsg));
            }
        }
    }

    void LoggerWidget::handleSeverityChanged(bool state)
    {

        // change Severitys
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
            // "all" channel
            handleCurrentFilterChanged(-1);
        }

        // Make changes
        handleCurrentFilterChanged(1);

        // Save severity settings for next session
        saveSettings();
    }

    void LoggerWidget::handleSearchChanged(QString filter, int searchOptions)
    {
        mSearchFilter = filter;
        std::cout << mSearchFilter.QString::toStdString() << std::endl;
        mSearchOptions = SearchOptions(searchOptions);
        handleCurrentFilterChanged(1);
    }

    void LoggerWidget::handleFirstUserInteraction(int value)
    {
        Q_UNUSED(value);

        if (!mUserInteractedWithScrollbar)
            mUserInteractedWithScrollbar = true;
    }

    void LoggerWidget::handleCustomChannel()
    {
        QString txt = mSelector->lineEdit()->text();
        if (txt.isEmpty()) return;
        std::string channel_name = txt.toStdString();
        ChannelModel* model = ChannelModel::instance();
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

    void LoggerWidget::toggleSearchbar()
    {
        if(mSearchbar->isHidden())
        {
            mSearchbar->show();
            mSearchbar->setFocus();
        }
        else
        {
            mSearchbar->hide();
            setFocus();
        }
    }

    QString LoggerWidget::searchIconPath() const
    {
        return mSearchIconPath;
    }

    QString LoggerWidget::searchIconStyle() const
    {
        return mSearchIconStyle;
    }

    void LoggerWidget::setSearchIconPath(const QString& path)
    {
        mSearchIconPath = path;
    }

    void LoggerWidget::setSearchIconStyle(const QString& style)
    {
        mSearchIconStyle = style;
    }
}
