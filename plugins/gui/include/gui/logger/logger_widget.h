//  MIT License
//
//  Copyright (c) 2019 Ruhr-University Bochum, Germany, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
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

namespace hal
{
    class FilterTabBar;
    class LoggerMarshall;
    struct FilterItem;
    class ChannelSelector;

    class LoggerWidget : public ContentWidget
    {
        Q_OBJECT

    public:
        LoggerWidget(QWidget* parent = nullptr);
        ~LoggerWidget() override;

        virtual void setupToolbar(Toolbar* Toolbar) override;

        QPlainTextEdit* getPlainTextEdit();

        FilterTabBar* getTabBar();

        void resizeEvent(QResizeEvent* event) override;

    Q_SIGNALS:
        void queueLogUpdate(spdlog::level::level_enum t, QString const& msg, FilterItem* filter);

    public Q_SLOTS:
        void handleCurrentChannelUpdated(spdlog::level::level_enum t, QString const& msg);
        void handleChannelUpdated(spdlog::level::level_enum t, const std::string& logger_name, std::string const& msg);
        void handleCurrentChannelChanged(int index);
        void showTextEditContextMenu(const QPoint& point);
        void filterItemClicked(const int& index);
        void handleFirstUserInteraction(int value);

    private:
        void scrollToBottom();

        FilterTabBar* mTabBar;
        QPlainTextEdit* mPlainTextEdit;
        ChannelSelector* mSelector;
        LoggerMarshall* mLogMarshall;
        std::string mCurrentChannel;
        QReadWriteLock mLock;
        QScrollBar* mPlainTextEditScrollbar;
        bool mUserInteractedWithScrollbar;
    };
}
