#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include <QObject>
#include <QVector>

class QAction;

namespace hal{
class hal_window;
class main_settings_widget;
class plugin_schedule_widget;
class welcome_screen;
class work_space; // TEMP NAME ?

class window_manager : public QObject
{
    Q_OBJECT

public:
    explicit window_manager(QObject* parent = nullptr);

    void setup();

    void add_window();
    void remove_window(hal_window* window);
    void set_main_window(hal_window* window);

    void lock_all();
    void unlock_all();

    void save_layout(const QString& name);
    void restore_layout(const QString& name);

    void handle_window_close_request(hal_window* window);

    void repolish();

public Q_SLOTS:
    void handle_overlay_clicked();

private Q_SLOTS:
    void handle_action_open();
    void handle_action_close();
    void handle_action_save();
    void handle_action_schedule();
    void handle_action_run_schedule();
    void handle_action_content();
    void handle_action_settings();
    void handle_action_about();

private:
    hal_window* m_main_window;

    QVector<hal_window*> m_windows;

    bool m_static_windows;
    bool m_shared_minimize;
    bool m_switch_main_on_focus;

    QAction* m_action_open_file;
    QAction* m_action_close_file;
    QAction* m_action_save;
    QAction* m_action_schedule;
    QAction* m_action_run_schedule;
    QAction* m_action_content;
    QAction* m_action_settings;
    QAction* m_action_about;

    welcome_screen* m_welcome_screen;
    plugin_schedule_widget* m_plugin_schedule_widget;
    main_settings_widget* m_main_settings_widget;
};
}

#endif // WINDOW_MANAGER_H
