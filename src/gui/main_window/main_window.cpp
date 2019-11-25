#include "main_window/main_window.h"

#include "def.h"

#include "core/log.h"

#include "hdl_parser/hdl_parser_dispatcher.h"
#include "netlist/gate.h"
#include "netlist/gate_library/gate_library_manager.h"
#include "netlist/net.h"
#include "netlist/netlist.h"
#include "netlist/netlist_factory.h"
#include "netlist/persistent/netlist_serializer.h"

#include "gui/gui_def.h"
#include "gui/hal_plugin_access_manager/hal_plugin_access_manager.h"
#include "gui/main_window/about_dialog.h"

#include "gui/docking_system/dock_bar.h"
#include "gui/file_manager/file_manager.h"
#include "gui/gui_globals.h"
#include "gui/hal_action/hal_action.h"
#include "gui/hal_content_manager/hal_content_manager.h"
#include "gui/hal_logger/hal_logger_widget.h"
#include "gui/plugin_management/plugin_schedule_manager.h"
#include "gui/plugin_management/plugin_schedule_widget.h"
#include "gui/plugin_manager/plugin_manager_widget.h"
#include "gui/plugin_manager/plugin_model.h"
#include "gui/python/python_editor.h"
#include "gui/welcome_screen/welcome_screen.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QFuture>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QShortcut>
#include <QtConcurrent>

#include "graphics_effects/overlay_effect.h"
#include "notifications/notification.h"
#include "overlay/reminder_overlay.h"
#include "plugin_manager/plugin_manager_widget.h"

main_window::main_window(QWidget* parent) : QWidget(parent), m_schedule_widget(new plugin_schedule_widget()), m_action_schedule(new hal_action(this)), m_action_content(new hal_action(this))
{
    ensurePolished();    // ADD REPOLISH METHOD
    connect(file_manager::get_instance(), &file_manager::file_opened, this, &main_window::handle_file_opened);

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    m_menu_bar = new QMenuBar();
    m_menu_bar->setNativeMenuBar(true);
    m_menu_bar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_layout->addWidget(m_menu_bar);

    m_tool_bar_layout = new QHBoxLayout();
    m_tool_bar_layout->setContentsMargins(0, 0, 0, 0);
    m_tool_bar_layout->setSpacing(0);
    m_layout->addLayout(m_tool_bar_layout);

    m_left_tool_bar = new QToolBar();
    m_left_tool_bar->setObjectName("toolbar");
    m_left_tool_bar->setMovable(false);
    m_left_tool_bar->setFloatable(false);
    m_left_tool_bar->setIconSize(QSize(18, 18));
    m_left_tool_bar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_tool_bar_layout->addWidget(m_left_tool_bar);

    m_right_tool_bar = new QToolBar();
    m_right_tool_bar->setObjectName("toolbar");
    m_right_tool_bar->setMovable(false);
    m_right_tool_bar->setFloatable(false);
    m_right_tool_bar->setIconSize(QSize(18, 18));
    m_right_tool_bar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_right_tool_bar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_tool_bar_layout->addWidget(m_right_tool_bar);

    m_stacked_widget = new QStackedWidget();
    m_layout->addWidget(m_stacked_widget);

    m_stacked_widget->addWidget(m_schedule_widget);

    m_settings = new main_settings_widget();
    m_stacked_widget->addWidget(m_settings);

    m_layout_area = new content_layout_area();
    m_stacked_widget->addWidget(m_layout_area);

    //    m_container = new QWidget();
    //    m_stacked_widget->addWidget(m_container);

    //    m_container_layout = new QVBoxLayout(m_container);
    //    m_container_layout->setContentsMargins(0, 0, 0, 0);
    //    m_container_layout->setSpacing(0);

    //    m_tool_bar = new QToolBar();
    //    m_tool_bar->setMovable(false);
    //    m_tool_bar->setFloatable(false);
    //    m_tool_bar->setIconSize(QSize(18, 18));
    //    m_tool_bar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    //    m_container_layout->addWidget(m_tool_bar);

    //    m_layout_area = new hal_content_layout_area();
    //    m_container_layout->addWidget(m_layout_area);

    //welcome screen + other screens (?)

    m_welcome_screen = new welcome_screen();
    m_stacked_widget->addWidget(m_welcome_screen);
    m_stacked_widget->setCurrentWidget(m_welcome_screen);

    setLocale(QLocale(QLocale::English, QLocale::UnitedStates));

    m_action_new          = new hal_action(this);
    m_action_open         = new hal_action(this);
    m_action_save         = new hal_action(this);
    m_action_about        = new hal_action(this);
    m_action_run_schedule = new hal_action(this);
    //m_action_content      = new hal_action(this);
    m_action_settings = new hal_action(this);
    m_action_close    = new hal_action(this);
    m_action_content  = new hal_action(this);

    //    //m_open_icon_style = "all->#fcfcb0";
    //    //m_open_icon_style = "all->#f2e4a4";
    //    m_open_icon_style = "all->#3192C5";
    //    m_open_icon_path  = ":/icons/folder";

    //    m_save_icon_style = "all->#3192C5";
    //    m_save_icon_path  = ":/icons/folder-down";

    //    //m_schedule_icon_style = "all->#08d320";
    //    //m_schedule_icon_style = "all->#05c61c";
    //    m_schedule_icon_style = "all->#2BAD4A";
    //    m_schedule_icon_path  = ":/icons/list";

    //    //m_run_icon_style = "all->#05c61c";
    //    m_run_icon_style = "all->#2BAD4A";
    //    m_run_icon_path  = ":/icons/run";

    //    m_content_icon_style = "all->#AFB1B3";
    //    m_content_icon_path  = ":/icons/content";

    //    m_settings_icon_style = "all->#AFB1B3";
    //    m_settings_icon_path  = ":/icons/settings";

    m_action_new->setIcon(gui_utility::get_styled_svg_icon(m_new_file_icon_style, m_new_file_icon_path));
    //m_action_new->setIcon(gui_utility::get_styled_svg_icon(m_open_icon_style, m_open_icon_path));
    m_action_open->setIcon(gui_utility::get_styled_svg_icon(m_open_icon_style, m_open_icon_path));
    m_action_save->setIcon(gui_utility::get_styled_svg_icon(m_save_icon_style, m_save_icon_path));
    m_action_schedule->setIcon(gui_utility::get_styled_svg_icon(m_schedule_icon_style, m_schedule_icon_path));
    m_action_run_schedule->setIcon(gui_utility::get_styled_svg_icon(m_run_icon_style, m_run_icon_path));
    m_action_content->setIcon(gui_utility::get_styled_svg_icon(m_content_icon_style, m_content_icon_path));
    m_action_settings->setIcon(gui_utility::get_styled_svg_icon(m_settings_icon_style, m_settings_icon_path));

    m_menu_file = new QMenu(m_menu_bar);
    m_menu_edit = new QMenu(m_menu_bar);
    m_menu_help = new QMenu(m_menu_bar);

    m_menu_bar->addAction(m_menu_file->menuAction());
    m_menu_bar->addAction(m_menu_edit->menuAction());
    m_menu_bar->addAction(m_menu_help->menuAction());
    m_menu_file->addAction(m_action_new);
    m_menu_file->addAction(m_action_open);
    //m_menu_file->addAction(m_action_close);
    m_menu_file->addAction(m_action_save);
    m_menu_edit->addAction(m_action_settings);
    m_menu_help->addAction(m_action_about);
    m_left_tool_bar->addAction(m_action_new);
    m_left_tool_bar->addAction(m_action_open);
    m_left_tool_bar->addAction(m_action_save);
    //    m_left_tool_bar->addSeparator();
    m_left_tool_bar->addAction(m_action_schedule);
    m_left_tool_bar->addAction(m_action_run_schedule);
    m_left_tool_bar->addAction(m_action_content);
    //    m_left_tool_bar->addSeparator();
    //    m_right_tool_bar->addSeparator();
    m_right_tool_bar->addAction(m_action_settings);

    g_keybind_manager.bind(m_action_new, "keybinds/project_create_file");
    g_keybind_manager.bind(m_action_open, "keybinds/project_open_file");
    g_keybind_manager.bind(m_action_save, "keybinds/project_save_file");
    g_keybind_manager.bind(m_action_run_schedule, "keybinds/schedule_run");

    setWindowTitle("HAL");
    m_action_new->setText("New Netlist");
    m_action_open->setText("Open");
    m_action_save->setText("Save");
    m_action_about->setText("About");
    m_action_schedule->setText("Edit Schedule");
    m_action_run_schedule->setText("Run Schedule");
    m_action_content->setText("Content (Disabled)");
    m_action_settings->setText("Settings");
    m_action_close->setText("Close Document");
    m_menu_file->setTitle("File");
    m_menu_edit->setTitle("Edit");
    m_menu_help->setTitle("Help");

    m_about_dialog = new about_dialog(this);
    m_plugin_model = new plugin_model(this);

    g_python_context = std::make_unique<python_context>();

    g_content_manager = new hal_content_manager(this);

    connect(m_action_new, &hal_action::triggered, this, &main_window::handle_action_new);
    connect(m_action_open, &hal_action::triggered, this, &main_window::handle_action_open);
    connect(m_action_about, &hal_action::triggered, m_about_dialog, &about_dialog::exec);
    connect(m_action_schedule, &hal_action::triggered, this, &main_window::toggle_schedule);
    connect(m_action_settings, &hal_action::triggered, this, &main_window::toggle_settings);
    connect(m_settings, &main_settings_widget::close, this, &main_window::close_settings);
    connect(m_action_save, &hal_action::triggered, this, &main_window::handle_save_triggered);
    //debug
    connect(m_action_close, &hal_action::triggered, this, &main_window::handle_action_closed);

    connect(m_action_run_schedule, &hal_action::triggered, plugin_schedule_manager::get_instance(), &plugin_schedule_manager::run_schedule);

    connect(this, &main_window::save_triggered, g_content_manager, &hal_content_manager::handle_save_triggered);

    restore_state();

    //    plugin_manager_widget* widget = new plugin_manager_widget(nullptr);
    //    widget->set_plugin_model(m_plugin_model);
    //    widget->show();

    //setGraphicsEffect(new overlay_effect());

    //reminder_overlay* o = new reminder_overlay(this);
    //Q_UNUSED(o)
}

QString main_window::new_file_icon_path() const
{
    return m_new_file_icon_path;
}

QString main_window::new_file_icon_style() const
{
    return m_new_file_icon_style;
}

QString main_window::open_icon_path() const
{
    return m_open_icon_path;
}

QString main_window::open_icon_style() const
{
    return m_open_icon_style;
}

QString main_window::save_icon_path() const
{
    return m_save_icon_path;
}

QString main_window::save_icon_style() const
{
    return m_save_icon_style;
}

QString main_window::schedule_icon_path() const
{
    return m_schedule_icon_path;
}

QString main_window::schedule_icon_style() const
{
    return m_schedule_icon_style;
}

QString main_window::run_icon_path() const
{
    return m_run_icon_path;
}

QString main_window::run_icon_style() const
{
    return m_run_icon_style;
}

QString main_window::content_icon_path() const
{
    return m_content_icon_path;
}

QString main_window::content_icon_style() const
{
    return m_content_icon_style;
}

QString main_window::settings_icon_path() const
{
    return m_settings_icon_path;
}

QString main_window::settings_icon_style() const
{
    return m_settings_icon_style;
}

void main_window::set_new_file_icon_path(const QString& path)
{
    m_new_file_icon_path = path;
}

void main_window::set_new_file_icon_style(const QString& style)
{
    m_new_file_icon_style = style;
}

void main_window::set_open_icon_path(const QString& path)
{
    m_open_icon_path = path;
}

void main_window::set_open_icon_style(const QString& style)
{
    m_open_icon_style = style;
}

void main_window::set_save_icon_path(const QString& path)
{
    m_save_icon_path = path;
}

void main_window::set_save_icon_style(const QString& style)
{
    m_save_icon_style = style;
}

void main_window::set_schedule_icon_path(const QString& path)
{
    m_schedule_icon_path = path;
}

void main_window::set_schedule_icon_style(const QString& style)
{
    m_schedule_icon_style = style;
}

void main_window::set_run_icon_path(const QString& path)
{
    m_run_icon_path = path;
}

void main_window::set_run_icon_style(const QString& style)
{
    m_run_icon_style = style;
}

void main_window::set_content_icon_path(const QString& path)
{
    m_content_icon_path = path;
}

void main_window::set_content_icon_style(const QString& style)
{
    m_content_icon_style = style;
}

void main_window::set_settings_icon_path(const QString& path)
{
    m_settings_icon_path = path;
}

void main_window::set_settings_icon_style(const QString& style)
{
    m_settings_icon_style = style;
}

extern void run_main(const QString file_name, const QList<QString> plugins);

void main_window::run_plugin_triggered(const QString& name)
{
    if (!file_manager::get_instance()->file_open())
    {
        return;
    }

    //    QString document = m_document_manager->get_input_file_name();
    //    QList<QString> plugins;
    //    plugins.append(name);
    //    QFuture<void> future = QtConcurrent::run(run_main, document, plugins);

    auto args            = hal_plugin_access_manager::request_arguments(name.toStdString());
    QFuture<void> future = QtConcurrent::run(hal_plugin_access_manager::run_plugin, name.toStdString(), &args);
}

// GENERALIZE TOGGLE METHODS
void main_window::toggle_schedule()
{
    if (m_stacked_widget->currentWidget() == m_schedule_widget)
    {
        if (file_manager::get_instance()->file_open())
            m_stacked_widget->setCurrentWidget(m_layout_area);
        else
            m_stacked_widget->setCurrentWidget(m_welcome_screen);
    }
    else
        m_stacked_widget->setCurrentWidget(m_schedule_widget);
}

void main_window::on_action_close_document_triggered()
{
    //m_layout_area->remove_content();
}

void main_window::toggle_settings()
{
    if (m_stacked_widget->currentWidget() == m_settings)
    {
        close_settings();
    }
    else
        m_stacked_widget->setCurrentWidget(m_settings);
}

void main_window::close_settings()
{
    if (file_manager::get_instance()->file_open())
        m_stacked_widget->setCurrentWidget(m_layout_area);
    else
        m_stacked_widget->setCurrentWidget(m_welcome_screen);
}

void main_window::handle_action_new()
{
    if (g_netlist != nullptr)
    {
        QMessageBox msgBox;
        msgBox.setText("Error");
        msgBox.setInformativeText("You are already working on a file. Restart HAL to switch to a different file.");
        msgBox.setStyleSheet("QLabel{min-width: 600px;}");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
        return;
    }

    QString title = "Create New Netlist";
    QString text  = "Please select a gate library";

    QStringList items;
    for (const auto& it : gate_library_manager::get_gate_libraries())
    {
        items.append(QString::fromStdString(it.first));
    }
    bool ok          = false;
    QString selected = QInputDialog::getItem(this, title, text, items, 0, false, &ok);

    if (ok)
    {
        g_netlist = netlist_factory::create_netlist(selected.toStdString());
        Q_EMIT file_manager::get_instance()->file_opened("new netlist");
    }
}

void main_window::handle_action_open()
{
    if (g_netlist != nullptr)
    {
        QMessageBox msgBox;
        msgBox.setText("Error");
        msgBox.setInformativeText("You are already working on a file. Restart HAL to switch to a different file.");
        msgBox.setStyleSheet("QLabel{min-width: 600px;}");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
        return;
    }

    QString title = "Open File";
    QString text  = "All Files(*.vhd *.vhdl *.v *.hal);;VHDL Files (*.vhd *.vhdl);;Verilog Files (*.v);;HAL Progress Files (*.hal)";

    // Non native dialogs does not work on macOS. Therefore do net set DontUseNativeDialog!
    QString file_name = QFileDialog::getOpenFileName(nullptr, title, QDir::currentPath(), text, nullptr);

    if (!file_name.isNull())
    {
        file_manager::get_instance()->open_file(file_name);
    }
}

void main_window::handle_file_opened(const QString& file_name)
{
    Q_UNUSED(file_name)
    if (m_stacked_widget->currentWidget() == m_welcome_screen)
    {
        m_stacked_widget->setCurrentWidget(m_layout_area);
        m_welcome_screen->close();
    }
    g_python_context->update_netlist();
}

void main_window::handle_save_triggered()
{
    if (g_netlist)
    {
        hal::path path = file_manager::get_instance()->file_name().toStdString();

        if (path.empty())
        {
            QString title = "Save File";
            QString text  = "HAL Progress Files (*.hal)";

            // Non native dialogs does not work on macOS. Therefore do net set DontUseNativeDialog!
            QString file_name = QFileDialog::getSaveFileName(nullptr, title, QDir::currentPath(), text, nullptr);
            if (!file_name.isNull())
            {
                path = file_name.toStdString();
            }
            else
            {
                return;
            }
        }

        path.replace_extension(".hal");
        netlist_serializer::serialize_to_file(g_netlist, path);

        g_file_status_manager.flush_unsaved_changes();
        file_manager::get_instance()->watch_file(QString::fromStdString(path.string()));

        Q_EMIT save_triggered();
    }
}

void main_window::handle_action_closed()
{
}

void main_window::on_action_quit_triggered()
{
    close();
}

void main_window::closeEvent(QCloseEvent* event)
{
    //check for unsaved changes and show confirmation dialog
    if (g_file_status_manager.modified_files_existing())
    {
        QMessageBox msgBox;
        msgBox.setStyleSheet("QLabel{min-width: 600px;}");
        auto cancelButton = msgBox.addButton("Cancel", QMessageBox::RejectRole);
        msgBox.addButton("Close Anyway", QMessageBox::ApplyRole);
        msgBox.setDefaultButton(cancelButton);
        msgBox.setInformativeText("Are you sure you want to close the application ?");

        msgBox.setText("There are unsaved modifications.");
        QString detailed_text = "The following modifications have not been saved yet:\n";
        for (const auto& s : g_file_status_manager.get_unsaved_change_descriptors())
            detailed_text.append("   ->  " + s + "\n");
        msgBox.setDetailedText(detailed_text);

        for (const auto& button : msgBox.buttons())
        {
            // if (button->text() == "Show Details...")
            if (msgBox.buttonRole(button) == QMessageBox::ActionRole)
            {
                button->click();
                break;
            }
        }

        msgBox.exec();

        if (msgBox.clickedButton() == cancelButton)
        {
            event->ignore();
            return;
        }
    }

    file_manager::get_instance()->close_file();

    save_state();
    event->accept();
    // hack, remove later
    g_content_manager->hack_delete_content();
    qApp->quit();
}

void main_window::restore_state()
{
    QPoint pos = g_settings_manager.get("main_window/position", QPoint(0, 0)).toPoint();
    move(pos);
    QRect rect = QApplication::desktop()->screenGeometry();
    QSize size = g_settings_manager.get("main_window/size", QSize(rect.width(), rect.height())).toSize();
    resize(size);
    //restore state of all subwindows
    m_layout_area->init_splitter_size(size);
}

void main_window::save_state()
{
    g_settings_manager.update("main_window/position", pos());
    g_settings_manager.update("main_window/size", size());
    //save state of all subwindows and everything else that might need to be restored on the next program start
    g_settings_manager.sync();
}

void main_window::add_content(content_widget* widget, int index, content_anchor anchor)
{
    m_layout_area->add_content(widget, index, anchor);
}
