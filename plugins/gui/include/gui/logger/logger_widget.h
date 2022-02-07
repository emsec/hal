//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once

#include "gui/channel_manager/channel_item.h"
#include "gui/content_widget/content_widget.h"
#include "hal_core/utilities/log.h"
#include "gui/gui_utils/graphics.h"

#include <QLabel>
#include <QMenu>
#include <QPlainTextEdit>
#include <QString>
#include <QToolButton>
#include <QWidget>
#include <QtCore/qreadwritelock.h>
#include <QScrollBar>
#include <QPushButton>

namespace hal
{
    class LoggerMarshall;
    class ChannelSelector;
    class Searchbar;

    /**
     * @ingroup logging
     * @brief Displays the logs in the gui.
     *
     * The LoggerWidget is the content widget that displays the current hal log.
     */
    class LoggerWidget : public ContentWidget
    {
        Q_OBJECT
        Q_PROPERTY(QString searchIconPath READ searchIconPath WRITE setSearchIconPath)
        Q_PROPERTY(QString searchIconStyle READ searchIconStyle WRITE setSearchIconStyle)

    public:
        /**
         * Constructor.
         *
         * @param parent - The parent widget
         */
        LoggerWidget(QWidget* parent = nullptr);
        ~LoggerWidget() override;

        /**
         * Initializes the toolbar of the LoggerWidget. It creates the ChannelSelector for the logger channels within
         * the toolbar.
         *
         * @param Toolbar - The toolbar to initialize
         */
        virtual void setupToolbar(Toolbar* Toolbar) override;

        /**
         * Gets the underlying (read only) QPlainTextEdit object this LoggerWidget uses to display the log.
         *
         * @returns the underlying QPlainTextEdit object
         */
        QPlainTextEdit* getPlainTextEdit();

        /**
         * Overrides the QWidget resizeEvent. After resizing the logger should be scrolled to the bottom, but only
         * if the user hasn't interacted with the scrollbar before.
         *
         * @param event
         */
        void resizeEvent(QResizeEvent* event) override;

        /**
         * Q_PROPERTY READ function for the "search"-icon path.
         *
         * @return The "search" icon path.
         */
        QString searchIconPath() const;

        /**
         * Q_PROPERTY READ function for the "search active"-icon style.
         *
         * @return The "search activate" icon style.
         */
        QString searchIconStyle() const;

        /**
         * Q_PROPERTY WRITE function for the "search"-icon path.
         *
         * @param path - The new path.
         */
        void setSearchIconPath(const QString &path);

        /**
         * Q_PROPERTY WRITE function for the "search"-icon style.
         *
         * @param style - The new style.
         */
        void setSearchIconStyle(const QString &style);

    public Q_SLOTS:
        /**
         * Appends the log message to the appropriate filter.
         *
         * @param t - The enum level.
         * @param msg - The new log message.
         */
        void handleCurrentChannelUpdated(spdlog::level::level_enum t, QString const& msg);

        /**
         * Q_SLOT to handle that the log manager has received a new message.
         *
         * @param t - The type of the message
         * @param logger_name - The channel of the message
         * @param msg - The message itself
         */
        void handleChannelUpdated(spdlog::level::level_enum t, const std::string& logger_name, std::string const& msg);

        /**
         * Q_SLOT to handle that the currently selected filter has been changed (e.g. by choosing another one in the
         * the ChannelSelector combobox or by toggling a severity button).
         *
         * @param p - New channel index or new state of a severity button
         */
        void handleCurrentFilterChanged(int p);

        /**
         * Q_SLOT to handle that the severity filter has been changed (e.g. by toggling a severity).
         *
         * @param state - New state of a severity button
         */
        void handleSeverityChanged(bool state);

        /**
         * Q_SLOT to handle that a new search filter has been set.
         *
         * @param filter - Search string to filter
         */
        void handleSearchChanged(QString filter);

        /**
         * Q_SLOT to handle interactions with the scrollbar. After the first scrollbar interaction the scrollbar wont
         * be locked at the bottom anymore (e.g. after a resize event).
         *
         * @param value - The new scroll position of the scrollbar (unused)
         */
        void handleFirstUserInteraction(int value);

        /**
         * Q_SLOT to handle that a new channel has been added to the ChannelSelector combobox.
         */
        void handleCustomChannel();

        /**
         * Toggles the visibiliy of the searchbar.
         */
        void toggleSearchbar();

    private:
        Searchbar* mSearchbar;
        QAction* mSearchAction;

        QString mSearchIconPath;
        QString mSearchIconStyle;

        void saveSettings();
        void restoreSettings();

        void scrollToBottom();

        QPlainTextEdit* mPlainTextEdit;

        ChannelSelector* mSelector;
        QLabel* mChannelLabel;

        QPushButton* mMuteButton;
        QPushButton* mVerboseButton;
        QPushButton* mDebugButton;
        QPushButton* mInfoButton;
        QPushButton* mWarningButton;
        QPushButton* mErrorButton;

        bool mInfoSeverity;
        bool mWarningSeverity;
        bool mErrorSeverity;
        bool mDebugSeverity;

        QString mSearchFilter;

        LoggerMarshall* mLogMarshall;
        int mCurrentChannelIndex;
        std::string mCurrentChannel;
        QReadWriteLock mLock;
        QScrollBar* mPlainTextEditScrollbar;
        bool mUserInteractedWithScrollbar;
    };
}
