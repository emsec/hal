#include "hal_content_manager/hal_content_manager.h"

#include "context_manager_widget/context_manager_widget.h"
#include "file_manager/file_manager.h"
#include "graph_tab_widget/graph_tab_widget.h"
#include "graph_widget/graph_context_manager.h"
#include "graph_widget/graph_widget.h"
#include "gui/content_layout_area/content_layout_area.h"
#include "gui/content_widget/content_widget.h"
#include "gui/docking_system/tab_widget.h"
#include "gui/graph_widget/graph_graphics_view.h"
#include "gui/graph_widget/graph_widget.h"
#include "gui/gui_utils/graphics.h"
#include "gui/hal_graphics/hal_graphics_view.h"
#include "gui/hal_logger/hal_logger_widget.h"
#include "gui/module_widget/module_widget.h"
#include "gui/python/python_console_widget.h"
#include "gui/python/python_editor.h"
#include "gui/selection_details_widget/selection_details_widget.h"
#include "gui_globals.h"
#include "main_window/main_window.h"
#include "netlist/netlist.h"
#include "netlist/persistent/netlist_serializer.h"
#include "vhdl_editor/vhdl_editor.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QOpenGLWidget>
namespace hal{
hal_content_manager::hal_content_manager(main_window* parent) : QObject(parent), m_main_window(parent)
{
    // has to be created this early in order to receive deserialization by the core signals
    m_python_widget = new python_editor();

    connect(file_manager::get_instance(), &file_manager::file_opened, this, &hal_content_manager::handle_open_document);
    connect(file_manager::get_instance(), &file_manager::file_closed, this, &hal_content_manager::handle_close_document);
    connect(file_manager::get_instance(), &file_manager::file_changed, this, &hal_content_manager::handle_filsystem_doc_changed);
}

hal_content_manager::~hal_content_manager()
{
}

void hal_content_manager::hack_delete_content()
{
    for (auto content : m_content)
        delete content;
}

python_editor* hal_content_manager::get_python_editor_widget()
{
    return m_python_widget;
}

graph_tab_widget* hal_content_manager::get_graph_tab_widget()
{
    return m_graph_tab_wid;
}

context_manager_widget* hal_content_manager::get_context_manager_widget()
{
    return m_context_manager_wid;
}

void hal_content_manager::handle_open_document(const QString& file_name)
{
    m_graph_tab_wid = new graph_tab_widget(nullptr);
    //    vhdl_editor* code_edit = new vhdl_editor();
    //    m_graph_tab_wid->addTab(code_edit, "Source");
    m_main_window->add_content(m_graph_tab_wid, 2, content_anchor::center);

    module_widget* m = new module_widget();
    m_main_window->add_content(m, 0, content_anchor::left);
    m->open();

    m_context_manager_wid = new context_manager_widget(m_graph_tab_wid);
    m_main_window->add_content(m_context_manager_wid, 1, content_anchor::left);
    m_context_manager_wid->open();

    QTimer::singleShot(50, [this]() { this->m_context_manager_wid->handle_create_context_clicked(); });

    selection_details_widget* details = new selection_details_widget();
    m_main_window->add_content(details, 0, content_anchor::bottom);

    hal_logger_widget* logger_widget = new hal_logger_widget();
    m_main_window->add_content(logger_widget, 1, content_anchor::bottom);

    details->open();
    logger_widget->open();

    //m_content.append(code_edit);
    //m_content.append(navigation);
    m_content.append(details);
    m_content.append(logger_widget);

    //-------------------------Test Buttons---------------------------

    content_widget* blue = new content_widget("blue");
    blue->setStyleSheet("* {background-color: #2B3856;}");
    content_widget* venomgreen = new content_widget("venomgreen");
    venomgreen->setStyleSheet("* {background-color: #728C00;}");
    content_widget* jade = new content_widget("jade");
    jade->setStyleSheet("* {background-color: #C3FDB8;}");

    //    m_main_window->add_content(blue, content_anchor::left);
    //    m_main_window->add_content(venomgreen, content_anchor::left);
    //    m_main_window->add_content(jade, content_anchor::left);

    //    hal_netlistics_view *view = new hal_netlistics_view();
    //    view->setScene(graph_scene);
    //    view->setDragMode(QGraphicsView::ScrollHandDrag);
    //    view->show();

    plugin_model* model                  = new plugin_model(this);
    plugin_manager_widget* plugin_widget = new plugin_manager_widget();
    plugin_widget->set_plugin_model(model);

    //    m_main_window->add_content(plugin_widget, content_anchor::bottom);

    connect(model, &plugin_model::run_plugin, m_main_window, &main_window::run_plugin_triggered);

    m_window_title = "HAL - " + QString::fromStdString(std::filesystem::path(file_name.toStdString()).stem().string());
    m_main_window->setWindowTitle(m_window_title);

    m_main_window->add_content(m_python_widget, 3, content_anchor::right);
    //m_python_widget->open();

    python_console_widget* python_console = new python_console_widget();
    m_main_window->add_content(python_console, 5, content_anchor::bottom);
    python_console->open();
    m_netlist_watcher = new netlist_watcher(this);
}

void hal_content_manager::handle_close_document()
{
    //(if possible) store state first, then remove all subwindows from main window
    m_window_title = "HAL";
    m_main_window->setWindowTitle(m_window_title);
    m_main_window->on_action_close_document_triggered();
    //delete all windows here
    for (auto content : m_content)
    {
        m_content.removeOne(content);
        delete content;
    }

    file_manager::get_instance()->close_file();
}

void hal_content_manager::handle_filsystem_doc_changed(const QString& file_name)
{
    Q_UNUSED(file_name)
}

void hal_content_manager::handle_save_triggered()
{
    Q_EMIT save_triggered();
}
}
