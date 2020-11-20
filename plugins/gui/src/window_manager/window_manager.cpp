#include "gui/window_manager/window_manager.h"

#include "gui/dialogs/warning_dialog.h"
#include "gui/overlay/dialog_overlay.h"
#include "gui/overlay/overlay.h"
#include "gui/plugin_management/plugin_schedule_widget.h"
#include "gui/settings/main_settings_widget.h"
#include "gui/style/style.h"
#include "gui/style/shared_properties_qss_adapter.h"
#include "gui/welcome_screen/welcome_screen.h"
#include "gui/window_manager/window.h"
#include "gui/window_manager/window_toolbar.h"

#include <QAction>
#include <QShortcut>
#include <QDebug>

#include <cassert>

namespace hal
{
    WindowManager::WindowManager(QObject* parent) : QObject(parent),
        mMainWindow           (nullptr),
        mToolbar               (new WindowToolbar(nullptr)),
        mActionOpenFile      (new QAction("Open File", this)),
        mActionCloseFile     (new QAction("Close File", this)),
        mActionSave           (new QAction("Save", this)),
        mActionSchedule       (new QAction("Schedule", this)),
        mActionRunSchedule   (new QAction("Run Schedule", this)),
        mActionContent        (new QAction("Content", this)),
        mActionSettings       (new QAction("Settings", this)),
        mActionAbout          (new QAction("About", this)),
        mWelcomeScreen        (new WelcomeScreen()),
        mPluginScheduleWidget(new PluginScheduleWidget()),
        mMainSettingsWidget  (new MainSettingsWidget())
    {
        connect(mActionOpenFile,    &QAction::triggered, this, &WindowManager::handleActionOpen);
        connect(mActionCloseFile,   &QAction::triggered, this, &WindowManager::handleActionClose);
        connect(mActionSave,         &QAction::triggered, this, &WindowManager::handleActionSave);
        connect(mActionSchedule,     &QAction::triggered, this, &WindowManager::handleActionSchedule);
        connect(mActionRunSchedule, &QAction::triggered, this, &WindowManager::handleActionRunSchedule);
        connect(mActionContent,      &QAction::triggered, this, &WindowManager::handleActionContent);
        connect(mActionSettings,     &QAction::triggered, this, &WindowManager::handleActionSettings);
        connect(mActionAbout,        &QAction::triggered, this, &WindowManager::handleActionAbout);

        repolish();

        mToolbar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

        mToolbar->addAction(mActionOpenFile);
        mToolbar->addAction(mActionSave);
        mToolbar->addAction(mActionSchedule);
        mToolbar->addAction(mActionRunSchedule);
        mToolbar->addAction(mActionContent);
        mToolbar->addSpacer();
        mToolbar->addAction(mActionSettings);

        // LOAD ALL LAYOUTS
        // RESTORE SELECTED LAYOUT OR USE DEFAULT LAYOUT

        // DEBUG CODE
        addWindow();
        addWindow();

        // CHECK IF SHORTCUTS WORK AS EXPECTED
        mActionOpenFile->setShortcut(QKeySequence("Ctrl+O"));
        mActionSave->setShortcut(QKeySequence("Ctrl+S"));
        mActionRunSchedule->setShortcut(QKeySequence("Ctrl+Shift+R"));

        mMainWindow->showSpecialScreen(mWelcomeScreen);

        // THIS WORKS, COMPARE TO HARDCODED EVENT LISTENER
        QShortcut* shortcut = new QShortcut(QKeySequence("F1"), mWindows.at(0));
        shortcut->setContext(Qt::ApplicationShortcut);
        connect(shortcut, &QShortcut::activated, this, &WindowManager::handleActionClose);
    }

    void WindowManager::addWindow()
    {
        Window* window = new Window(nullptr);
        mWindows.append(window);

        if (!mMainWindow)
            setMainWindow(window);

        window->show();
    }

    void WindowManager::removeWindow(Window* window)
    {
        assert(window);

        if (mWindows.removeOne(window))
        {
            if (window == mMainWindow)
            {
                if (!mWindows.empty())
                    setMainWindow(mWindows[0]);
                else
                    mMainWindow = nullptr;
            }
            window->deleteLater();
        }
    }

    void WindowManager::setMainWindow(Window* window)
    {
        assert(window);

        if (mMainWindow)
            mMainWindow->showToolbarExtension(); // USE SETTING HERE

        mMainWindow = window;
        mMainWindow->showToolbar(mToolbar);
        mMainWindow->hideToolbarExtension();
    }

    void WindowManager::lockAll()
    {
        for (Window*& window : mWindows)
        {
            //Overlay* overlay = new Overlay();
            DialogOverlay* overlay = new DialogOverlay();
            WarningDialog* dialog = new WarningDialog();
            overlay->setDialog(dialog);
            dialog->fadeIn();
            window->lock(overlay);
        }
    }

    void WindowManager::unlockAll()
    {
        for (Window*& window : mWindows)
            window->unlock();
    }

    void WindowManager::handleWindowCloseRequest(Window* window)
    {
        Q_UNUSED(window);
        if (mStaticWindows)
        {
            // ASK FOR CONFIRMATION / FORWARD TO WINDOW MANAGER WIDGET
        }
        else
        {
            // STORE CONTENT AND CLOSE WINDOW
        }
    }

    void WindowManager::repolish()
    {
        const SharedPropertiesQssAdapter* a = SharedPropertiesQssAdapter::instance();

        mActionOpenFile   ->setIcon(style::getStyledSvgIcon(a->mOpenIconStyle, a->mOpenIconPath));
        //mActionCloseFile  ->setIcon(style::getStyledSvgIcon(a->m_close_icon_style, a->m_close_icon_path));
        mActionSave        ->setIcon(style::getStyledSvgIcon(a->mSaveIconStyle, a->mSaveIconPath));
        mActionSchedule    ->setIcon(style::getStyledSvgIcon(a->mScheduleIconStyle, a->mScheduleIconPath));
        mActionRunSchedule->setIcon(style::getStyledSvgIcon(a->mRunIconStyle, a->mRunIconPath));
        mActionContent     ->setIcon(style::getStyledSvgIcon(a->mContentIconStyle, a->mContentIconPath));
        mActionSettings    ->setIcon(style::getStyledSvgIcon(a->mSettingsIconStyle, a->mSettingsIconPath));
        //mActionAbout       ->setIcon(style::getStyledSvgIcon(a->m_about_icon_style, a->m_about_icon_path));

        for (Window*& window : mWindows)
            window->repolish();
    }

    void WindowManager::handleOverlayClicked()
    {
        unlockAll();
    }

    void WindowManager::handleActionOpen()
    {
        qDebug() << "handle action open called";
        lockAll();
    }

    void WindowManager::handleActionClose()
    {
        qDebug() << "handle action close called";
        unlockAll();
    }

    void WindowManager::handleActionSave()
    {

    }

    void WindowManager::handleActionSchedule()
    {

    }

    void WindowManager::handleActionRunSchedule()
    {

    }

    void WindowManager::handleActionContent()
    {

    }

    void WindowManager::handleActionSettings()
    {

    }

    void WindowManager::handleActionAbout()
    {

    }
}
