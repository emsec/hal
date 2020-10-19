#include "gui/content_manager/content_manager.h"

#include "gui/context_manager_widget/context_manager_widget.h"
#include "gui/grouping/grouping_manager_widget.h"
#include "gui/file_manager/file_manager.h"
#include "gui/graph_tab_widget/graph_tab_widget.h"
#include "gui/graph_widget/graph_context_manager.h"
#include "gui/graph_widget/graph_widget.h"
#include "gui/content_layout_area/content_layout_area.h"
#include "gui/content_widget/content_widget.h"
#include "gui/docking_system/tab_widget.h"
#include "gui/graph_widget/graph_graphics_view.h"
#include "gui/graph_widget/graph_widget.h"
#include "gui/gui_utils/graphics.h"
#include "gui/graphics/graphics_view.h"
#include "gui/logger/logger_widget.h"
#include "gui/module_widget/module_widget.h"
#include "gui/python/python_console_widget.h"
#include "gui/python/python_editor.h"
#include "gui/selection_details_widget/selection_details_widget.h"
#include "gui/gui_globals.h"
#include "gui/main_window/main_window.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/persistent/netlist_serializer.h"
#include "gui/vhdl_editor/vhdl_editor.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QOpenGLWidget>

namespace hal
{
    ContentManager::ContentManager(MainWindow* parent) : QObject(parent), m_MainWindow(parent)
    {
        // has to be created this early in order to receive deserialization by the core signals
        m_python_widget = new PythonEditor();

        connect(FileManager::get_instance(), &FileManager::file_opened, this, &ContentManager::handle_open_document);
        connect(FileManager::get_instance(), &FileManager::file_closed, this, &ContentManager::handle_close_document);
        connect(FileManager::get_instance(), &FileManager::file_changed, this, &ContentManager::handle_filsystem_doc_changed);
    }

    ContentManager::~ContentManager()
    {
    }

    void ContentManager::hack_delete_content()
    {
        for (auto content : m_content)
            delete content;
    }

    PythonEditor* ContentManager::get_python_editor_widget()
    {
        return m_python_widget;
    }

    GraphTabWidget* ContentManager::get_graph_tab_widget()
    {
        return m_graph_tab_wid;
    }

    SelectionDetailsWidget* ContentManager::getSelectionDetailsWidget()
    {
        return mSelectionDetailsWidget;
    }

    GroupingManagerWidget* ContentManager::getGroupingManagerWidget()
    {
        return mGroupingManagerWidget;
    }

    ContextManagerWidget* ContentManager::get_context_manager_widget()
    {
        return m_context_manager_wid;
    }

    void ContentManager::handle_open_document(const QString& file_name)
    {
        m_graph_tab_wid = new GraphTabWidget(nullptr);
        //    VhdlEditor* code_edit = new VhdlEditor();
        //    m_graph_tab_wid->addTab(code_edit, "Source");
        m_MainWindow->add_content(m_graph_tab_wid, 2, content_anchor::center);

        ModuleWidget* m = new ModuleWidget();
        m_MainWindow->add_content(m, 0, content_anchor::left);
        m->open();

        m_context_manager_wid = new ContextManagerWidget(m_graph_tab_wid);
        m_MainWindow->add_content(m_context_manager_wid, 1, content_anchor::left);
        m_context_manager_wid->open();

        mGroupingManagerWidget = new GroupingManagerWidget(m_graph_tab_wid);
        m_MainWindow->add_content(mGroupingManagerWidget, 2, content_anchor::left);
        mGroupingManagerWidget->open();

        //we should probably document somewhere why we need this timer and why we have some sort of racing condition(?) here?
        //QTimer::singleShot(50, [this]() { this->m_context_manager_wid->handle_create_context_clicked(); });

        //executes same code as found in 'create_context_clicked' from the context manager widget but allows to keep its method private
        QTimer::singleShot(50, [this]() {
            GraphContext* new_context = nullptr;
            new_context = g_graph_context_manager->create_new_context(QString::fromStdString(g_netlist->get_top_module()->get_name()));
            new_context->add({g_netlist->get_top_module()->get_id()}, {});

            m_context_manager_wid->select_view_context(new_context);
        });

        //why does this segfault without a timer?
        //GraphContext* new_context = nullptr;
        //new_context = g_graph_context_manager->create_new_context(QString::fromStdString(g_netlist->get_top_module()->get_name()));
        //new_context->add({g_netlist->get_top_module()->get_id()}, {});
        //m_context_manager_wid->select_view_context(new_context);

        mSelectionDetailsWidget = new SelectionDetailsWidget();
        m_MainWindow->add_content(mSelectionDetailsWidget, 0, content_anchor::bottom);

        LoggerWidget* logger_widget = new LoggerWidget();
        m_MainWindow->add_content(logger_widget, 1, content_anchor::bottom);

        mSelectionDetailsWidget->open();
        logger_widget->open();

        //m_content.append(code_edit);
        //m_content.append(navigation);
        m_content.append(mSelectionDetailsWidget);
        m_content.append(logger_widget);

        //-------------------------Test Buttons---------------------------

        /*
        ContentWidget* blue = new ContentWidget("blue");
        blue->setObjectName("blue");
        blue->setStyleSheet("* {background-color: #2B3856;}");
        ContentWidget* venomgreen = new ContentWidget("venomgreen");
        venomgreen->setObjectName("venomgreen");
        venomgreen->setStyleSheet("* {background-color: #728C00;}");
        ContentWidget* jade = new ContentWidget("jade");
        jade->setObjectName("jade";
        jade->setStyleSheet("* {background-color: #C3FDB8;}");
*/

        //    m_MainWindow->add_content(blue, content_anchor::left);
        //    m_MainWindow->add_content(venomgreen, content_anchor::left);
        //    m_MainWindow->add_content(jade, content_anchor::left);

        //    hal_netlistics_view *view = new hal_netlistics_view();
        //    view->setScene(graph_scene);
        //    view->setDragMode(QGraphicsView::ScrollHandDrag);
        //    view->show();

        PluginModel* model                  = new PluginModel(this);
        PluginManagerWidget* plugin_widget = new PluginManagerWidget();
        plugin_widget->set_plugin_model(model);

        //    m_MainWindow->add_content(plugin_widget, content_anchor::bottom);

        connect(model, &PluginModel::run_plugin, m_MainWindow, &MainWindow::run_plugin_triggered);

        m_window_title = "HAL - " + QString::fromStdString(std::filesystem::path(file_name.toStdString()).stem().string());
        m_MainWindow->setWindowTitle(m_window_title);

        m_MainWindow->add_content(m_python_widget, 3, content_anchor::right);
        //m_python_widget->open();

        PythonConsoleWidget* PythonConsole = new PythonConsoleWidget();
        m_MainWindow->add_content(PythonConsole, 5, content_anchor::bottom);
        PythonConsole->open();
        m_NetlistWatcher = new NetlistWatcher(this);
    }

    void ContentManager::handle_close_document()
    {
        //(if possible) store state first, then remove all subwindows from main window
        m_window_title = "HAL";
        m_MainWindow->setWindowTitle(m_window_title);
        m_MainWindow->on_action_close_document_triggered();
        //delete all windows here
        for (auto content : m_content)
        {
            m_content.removeOne(content);
            delete content;
        }

        FileManager::get_instance()->close_file();
    }

    void ContentManager::handle_filsystem_doc_changed(const QString& file_name)
    {
        Q_UNUSED(file_name)
    }

    void ContentManager::handle_save_triggered()
    {
        Q_EMIT save_triggered();
    }
}
