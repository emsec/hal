#include "hal_content_manager/hal_content_manager.h"
#include "main_window/main_window.h"
#include "vhdl_editor/vhdl_editor.h"

#include "netlist/netlist.h"
#include "netlist/persistent/netlist_serializer.h"

#include "file_manager/file_manager.h"
#include "gui/content_layout_area/content_layout_area.h"
#include "gui/content_widget/content_widget.h"
#include "gui/docking_system/tab_widget.h"
#include "gui/graph_layouter/graph_layouter_view.h"
#include "gui/graph_layouter/gui_graph_gate.h"
#include "gui/graph_layouter/old_graph_layouter.h"
#include "gui/graph_manager/hal_graph_widget.h"
#include "gui/graph_navigation_widget/graph_navigation_widget.h"
#include "gui/graph_widget/graph_scene_manager.h"
#include "gui/graph_widget/graph_widget.h"
#include "gui/gui_utility.h"
#include "gui/hal_graphics/hal_graphics_view.h"
#include "gui/hal_logger/hal_logger_widget.h"
#include "gui/python/python_console_widget.h"
#include "gui/python/python_editor.h"
#include "gui/selection_details_widget/selection_details_widget.h"
#include "gui_globals.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QOpenGLWidget>

hal_content_manager::hal_content_manager(file_manager* file_manager, main_window* parent) : QObject(parent), m_main_window(parent), m_file_manager(file_manager)
{
    m_python_widget = new python_editor();
}

hal_content_manager::~hal_content_manager()
{
    //gui crashes when closing if this line is uncommented, need to check
    //delete m_console;
}

void hal_content_manager::hack_delete_content()
{
    for (auto content : m_content)
        delete content;
}

void hal_content_manager::handle_open_document(const QString& file_name)
{
    vhdl_editor* code_edit = new vhdl_editor();
    m_main_window->add_content(code_edit, 0, content_anchor::center);

    /*QGraphicsScene* */ m_graph_scene = new QGraphicsScene(this);
    layouter                           = new old_graph_layouter(m_graph_scene, g_netlist);
    layouter->layout_graph();

    //graph_layouter_view* layouter_view = new graph_layouter_view(graph_scene, layouter, g_netlist);
    m_layouter_view = new graph_layouter_view(m_graph_scene, layouter, g_netlist);

    //hal_graph_widget* gw1              = new hal_graph_widget(layouter_view);
    hal_graph_widget* gw1 = new hal_graph_widget(m_layouter_view);
    m_main_window->add_content(gw1, 1, content_anchor::center);
    gw1->open();

    connect(gw1, &hal_graph_widget::relayout_button_clicked, this, &hal_content_manager::handle_relayout_button_clicked);
    //        graph_widget* gw = new graph_widget();
    //        m_main_window->add_content(gw, 2, content_anchor::center);

    //    graph_widget* gw3 = new graph_widget();
    //    m_main_window->add_content(gw3, 3, content_anchor::center);

    graph_navigation_widget* navigation = new graph_navigation_widget();
    m_main_window->add_content(navigation, 0, content_anchor::left);

    selection_details_widget* details = new selection_details_widget();
    m_main_window->add_content(details, 0, content_anchor::bottom);

    hal_logger_widget* logger_widget = new hal_logger_widget();
    m_main_window->add_content(logger_widget, 1, content_anchor::bottom);

    //    m_console->init();
    //    m_main_window->add_content(m_console, 2, content_anchor::bottom);

    navigation->open();
    details->open();
    logger_widget->open();

    //    m_content.append(gw);

    m_content2.append(code_edit);
    m_content2.append(navigation);
    m_content2.append(details);
    m_content2.append(logger_widget);

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

    QString tmp("HAL - " + QString::fromStdString(hal::path(file_name.toStdString()).stem().string()));
    m_main_window->setWindowTitle(tmp);

    //m_python_widget = new python_editor();
    m_main_window->add_content(m_python_widget, 3, content_anchor::right);
    m_python_widget->open();

    m_python_console_widget = new python_console_widget();
    m_main_window->add_content(m_python_console_widget, 5, content_anchor::bottom);
    m_python_console_widget->open();

    m_netlist_watcher = new netlist_watcher(this);
}

void hal_content_manager::handle_close_document()
{
    //TODO
    //(if possible) store state first, then remove all subwindows from main window
    m_main_window->setWindowTitle("HAL");
    m_main_window->on_action_close_document_triggered();
    //delete all windows here
    for (auto content : m_content2)
    {
        m_content2.removeOne(content);
        delete content;
    }

    m_file_manager->close_file();
}

void hal_content_manager::handle_filsystem_doc_changed(const QString& file_name)
{
    Q_UNUSED(file_name)
}

void hal_content_manager::handle_relayout_button_clicked()
{
    layouter->relayout_graph();
    m_layouter_view->handle_graph_relayouted();
    m_graph_scene->update();
}

bool hal_content_manager::has_python_editor_unsaved_changes()
{
    if(m_file_manager->is_document_open())
        return m_python_widget->has_unsaved_tabs();
    else
        return false;
}

bool hal_content_manager::has_netlist_unsaved_changes()
{
    if(m_file_manager->is_document_open())
        return m_netlist_watcher->has_netlist_unsaved_changes();
    else
        return false;
}

void hal_content_manager::mark_netlist_saved()
{
    if(m_file_manager->is_document_open())
        m_netlist_watcher->set_netlist_unsaved_changes(false);
}

QStringList hal_content_manager::get_names_of_unsaved_python_tabs()
{
    if(m_file_manager->is_document_open())
        return m_python_widget->get_names_of_unsaved_tabs();
    else
        return QStringList();
}
