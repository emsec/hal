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

#include <QObject>
#include <QVector>

class QAction;

namespace hal
{
    class MainSettingsWidget;
    class PluginScheduleWidget;
    class WelcomeScreen;
    class Window;
    class WindowToolbar;

    class WindowManager final : public QObject
    {
        Q_OBJECT

    public:
        explicit WindowManager(QObject* parent = nullptr);

        void setup();

        void addWindow();
        void removeWindow(Window* window);
        void setMainWindow(Window* window);

        void lockAll();
        void unlockAll();

        void saveLayout(const QString& name);
        void restoreLayout(const QString& name);

        void handleWindowCloseRequest(Window* window);

        void repolish();

    public Q_SLOTS:
        void handleOverlayClicked();

    private Q_SLOTS:
        void handleActionOpen();
        void handleActionClose();
        void handleActionSave();
        void handleActionSchedule();
        void handleActionRunSchedule();
        void handleActionContent();
        void handleActionSettings();
        void handleActionAbout();

    private:
        Window* mMainWindow;
        QVector<Window*> mWindows;

        WindowToolbar* mToolbar;

        bool mStaticWindows;
        bool mSharedMinimize;
        bool mSwitchMainOnFocus;

        QAction* mActionOpenFile;
        QAction* mActionCloseFile;
        QAction* mActionSave;
        QAction* mActionSchedule;
        QAction* mActionRunSchedule;
        QAction* mActionContent;
        QAction* mActionSettings;
        QAction* mActionAbout;

        WelcomeScreen* mWelcomeScreen;
        PluginScheduleWidget* mPluginScheduleWidget;
        MainSettingsWidget* mMainSettingsWidget;
    };
}
