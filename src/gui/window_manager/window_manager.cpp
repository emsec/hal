#include "window_manager/window_manager.h"

#include "gui/dialogs/warning_dialog.h"
#include "overlay/dialog_overlay.h"
#include "overlay/overlay.h"
#include "plugin_management/plugin_schedule_widget.h"
#include "settings/main_settings_widget.h"
#include "style/style.h"
#include "style/shared_properties_qss_adapter.h"
#include "welcome_screen/welcome_screen.h"
#include "window_manager/window.h"
#include "window_manager/window_toolbar.h"

#include <QAction>
#include <QShortcut>

#include <QDebug>

#include <cassert>

namespace hal
{
    WindowManager::WindowManager(QObject* parent) : QObject(parent),
        m_main_window           (nullptr),
        m_toolbar               (new WindowToolbar(nullptr)),
        m_action_open_file      (new QAction("Open File", this)),
        m_action_close_file     (new QAction("Close File", this)),
        m_action_save           (new QAction("Save", this)),
        m_action_schedule       (new QAction("Schedule", this)),
        m_action_run_schedule   (new QAction("Run Schedule", this)),
        m_action_content        (new QAction("Content", this)),
        m_action_settings       (new QAction("Settings", this)),
        m_action_about          (new QAction("About", this)),
        m_welcome_screen        (new WelcomeScreen()),
        m_plugin_schedule_widget(new PluginScheduleWidget()),
        m_main_settings_widget  (new MainSettingsWidget())
    {
        connect(m_action_open_file,    &QAction::triggered, this, &WindowManager::handle_action_open);
        connect(m_action_close_file,   &QAction::triggered, this, &WindowManager::handle_action_close);
        connect(m_action_save,         &QAction::triggered, this, &WindowManager::handle_action_save);
        connect(m_action_schedule,     &QAction::triggered, this, &WindowManager::handle_action_schedule);
        connect(m_action_run_schedule, &QAction::triggered, this, &WindowManager::handle_action_run_schedule);
        connect(m_action_content,      &QAction::triggered, this, &WindowManager::handle_action_content);
        connect(m_action_settings,     &QAction::triggered, this, &WindowManager::handle_action_settings);
        connect(m_action_about,        &QAction::triggered, this, &WindowManager::handle_action_about);

        repolish();

        m_toolbar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

        m_toolbar->addAction(m_action_open_file);
        m_toolbar->addAction(m_action_save);
        m_toolbar->addAction(m_action_schedule);
        m_toolbar->addAction(m_action_run_schedule);
        m_toolbar->addAction(m_action_content);
        m_toolbar->add_spacer();
        m_toolbar->addAction(m_action_settings);

        // LOAD ALL LAYOUTS
        // RESTORE SELECTED LAYOUT OR USE DEFAULT LAYOUT

        // DEBUG CODE
        add_window();
        add_window();

        // CHECK IF SHORTCUTS WORK AS EXPECTED
        m_action_open_file->setShortcut(QKeySequence("Ctrl+O"));
        m_action_save->setShortcut(QKeySequence("Ctrl+S"));
        m_action_run_schedule->setShortcut(QKeySequence("Ctrl+Shift+R"));

        m_main_window->show_special_screen(m_welcome_screen);

        // THIS WORKS, COMPARE TO HARDCODED EVENT LISTENER
        QShortcut* shortcut = new QShortcut(QKeySequence("F1"), m_windows.at(0));
        shortcut->setContext(Qt::ApplicationShortcut);
        connect(shortcut, &QShortcut::activated, this, &WindowManager::handle_action_close);
    }

    void WindowManager::add_window()
    {
        Window* window = new Window(nullptr);
        m_windows.append(window);

        if (!m_main_window)
            set_main_window(window);

        window->show();
    }

    void WindowManager::remove_window(Window* window)
    {
        assert(window);

        if (m_windows.removeOne(window))
        {
            if (window == m_main_window)
            {
                if (!m_windows.empty())
                    set_main_window(m_windows[0]);
                else
                    m_main_window = nullptr;
            }
            window->deleteLater();
        }
    }

    void WindowManager::set_main_window(Window* window)
    {
        assert(window);

        if (m_main_window)
            m_main_window->show_toolbar_extension(); // USE SETTING HERE

        m_main_window = window;
        m_main_window->show_toolbar(m_toolbar);
        m_main_window->hide_toolbar_extension();
    }

    void WindowManager::lock_all()
    {
        for (Window*& window : m_windows)
        {
            //Overlay* overlay = new Overlay();
            DialogOverlay* overlay = new DialogOverlay();
            WarningDialog* dialog = new WarningDialog();
            overlay->set_dialog(dialog);
            dialog->fade_in();
            window->lock(overlay);
        }
    }

    void WindowManager::unlock_all()
    {
        for (Window*& window : m_windows)
            window->unlock();
    }

    void WindowManager::handle_window_close_request(Window* window)
    {
        Q_UNUSED(window);
        if (m_static_windows)
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

        m_action_open_file   ->setIcon(style::get_styled_svg_icon(a->m_open_icon_style, a->m_open_icon_path));
        //m_action_close_file  ->setIcon(style::get_styled_svg_icon(a->m_close_icon_style, a->m_close_icon_path));
        m_action_save        ->setIcon(style::get_styled_svg_icon(a->m_save_icon_style, a->m_save_icon_path));
        m_action_schedule    ->setIcon(style::get_styled_svg_icon(a->m_schedule_icon_style, a->m_schedule_icon_path));
        m_action_run_schedule->setIcon(style::get_styled_svg_icon(a->m_run_icon_style, a->m_run_icon_path));
        m_action_content     ->setIcon(style::get_styled_svg_icon(a->m_content_icon_style, a->m_content_icon_path));
        m_action_settings    ->setIcon(style::get_styled_svg_icon(a->m_settings_icon_style, a->m_settings_icon_path));
        //m_action_about       ->setIcon(style::get_styled_svg_icon(a->m_about_icon_style, a->m_about_icon_path));

        for (Window*& window : m_windows)
            window->repolish();
    }

    void WindowManager::handle_overlay_clicked()
    {
        unlock_all();
    }

    void WindowManager::handle_action_open()
    {
        qDebug() << "handle action open called";
        lock_all();
    }

    void WindowManager::handle_action_close()
    {
        qDebug() << "handle action close called";
        unlock_all();
    }

    void WindowManager::handle_action_save()
    {

    }

    void WindowManager::handle_action_schedule()
    {

    }

    void WindowManager::handle_action_run_schedule()
    {

    }

    void WindowManager::handle_action_content()
    {

    }

    void WindowManager::handle_action_settings()
    {

    }

    void WindowManager::handle_action_about()
    {

    }
}
