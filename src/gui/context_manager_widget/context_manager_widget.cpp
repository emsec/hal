#include "context_manager_widget/context_manager_widget.h"

#include "gui_globals.h"

#include "gui/graph_tab_widget/graph_tab_widget.h"
#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/graph_widget/graph_widget.h"

#include "input_dialog/input_dialog.h"
#include "validator/empty_string_validator.h"
#include "validator/unique_string_validator.h"

#include "core/log.h"
#include "gui_utils/graphics.h"
#include "netlist/gate.h"
#include "netlist/module.h"
#include "netlist/netlist.h"
#include "toolbar/toolbar.h"
#include <QAction>
#include <QDebug>
#include <QListWidgetItem>
#include <QMenu>
#include <QResizeEvent>
#include <QSize>
#include <QStringList>
#include <QStringListModel>
#include <QVBoxLayout>
#include <QDateTime>

context_manager_widget::context_manager_widget(graph_tab_widget* tab_view, QWidget* parent)
    : content_widget("View Manager", parent), m_list_widget(new QListWidget()), m_new_view_action(new QAction(this)), m_rename_action(new QAction(this)), m_duplicate_action(new QAction(this)),
      m_delete_action(new QAction(this))
{
    //needed to load the properties
    ensurePolished();
    m_tab_view = tab_view;

    m_list_widget->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

    m_content_layout->addWidget(m_list_widget);

    connect(m_list_widget, &QListWidget::customContextMenuRequested, this, &context_manager_widget::handle_context_menu_request);
    connect(m_list_widget, &QListWidget::itemDoubleClicked, this, &context_manager_widget::handle_item_double_clicked);
    connect(m_list_widget, &QListWidget::itemSelectionChanged, this, &context_manager_widget::handle_selection_changed);

    connect(&g_graph_context_manager, &graph_context_manager::context_created, this, &context_manager_widget::handle_context_created);
    connect(&g_graph_context_manager, &graph_context_manager::context_renamed, this, &context_manager_widget::handle_context_renamed);
    connect(&g_graph_context_manager, &graph_context_manager::deleting_context, this, &context_manager_widget::handle_context_removed);

    //load top context (top module) into list
    for (const auto& ctx : g_graph_context_manager.get_contexts())
    {
        handle_context_created(ctx);
    }
    // if (m_list_widget->count() == 0)
    // {
    //     handle_create_context_clicked();
    // }

    m_new_view_action->setIcon(gui_utility::get_styled_svg_icon(m_new_view_icon_style, m_new_view_icon_path));
    m_new_view_action->setToolTip("New View");
    m_rename_action->setIcon(gui_utility::get_styled_svg_icon(m_rename_icon_style, m_rename_icon_path));
    m_rename_action->setToolTip("Rename");
    m_duplicate_action->setIcon(gui_utility::get_styled_svg_icon(m_duplicate_icon_style, m_duplicate_icon_path));
    m_duplicate_action->setToolTip("Duplicate");
    m_delete_action->setIcon(gui_utility::get_styled_svg_icon(m_delete_icon_style, m_delete_icon_path));
    m_delete_action->setToolTip("Delete");

    m_rename_action->setEnabled(false);
    m_duplicate_action->setEnabled(false);
    m_delete_action->setEnabled(false);

    connect(m_new_view_action, &QAction::triggered, this, &context_manager_widget::handle_create_context_clicked);
    connect(m_rename_action, &QAction::triggered, this, &context_manager_widget::handle_rename_context_clicked);
    connect(m_duplicate_action, &QAction::triggered, this, &context_manager_widget::handle_duplicate_context_clicked);
    connect(m_delete_action, &QAction::triggered, this, &context_manager_widget::handle_delete_context_clicked);
}

void context_manager_widget::resizeEvent(QResizeEvent* event)
{
    m_list_widget->setFixedWidth(event->size().width());
}

void context_manager_widget::handle_context_menu_request(const QPoint& point)
{
    //check if right click / context menu request occured on position of an list item
    QModelIndex clicked_index = m_list_widget->indexAt(point);
    //setup context menu
    QMenu context_menu;

    QAction create_action("Create New View", &context_menu);
    QAction open_action("Open View", &context_menu);
    QAction rename_action("Rename View", &context_menu);
    QAction duplicate_action("Duplicate View", &context_menu);
    QAction delete_action("Remove View", &context_menu);

    context_menu.addAction(&create_action);
    connect(&create_action, &QAction::triggered, this, &context_manager_widget::handle_create_context_clicked);

    if (clicked_index.isValid())
    {
        context_menu.addAction(&open_action);
        context_menu.addAction(&rename_action);
        context_menu.addAction(&delete_action);
        context_menu.addAction(&duplicate_action);

        connect(&open_action, &QAction::triggered, this, &context_manager_widget::handle_open_context_clicked);
        connect(&rename_action, &QAction::triggered, this, &context_manager_widget::handle_rename_context_clicked);
        connect(&duplicate_action, &QAction::triggered, this, &context_manager_widget::handle_duplicate_context_clicked);
        connect(&delete_action, &QAction::triggered, this, &context_manager_widget::handle_delete_context_clicked);
    }
    //show context menu
    context_menu.exec(m_list_widget->viewport()->mapToGlobal(point));
}

void context_manager_widget::handle_create_context_clicked()
{
    graph_context* new_context = nullptr;

    new_context = g_graph_context_manager.create_new_context(QString::fromStdString(g_netlist->get_top_module()->get_name()) + " (" + QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate) + ")");
    new_context->add({g_netlist->get_top_module()->get_id()}, {});

    m_tab_view->show_context(new_context);
}

void context_manager_widget::select_view_context(graph_context* context)
{
    for (int i = 0; i < m_list_widget->count(); ++i)
    {
        if (m_assigned_pointers[m_list_widget->item(i)] == context)
        {
            m_list_widget->setCurrentRow(i);
            return;
        }
    }
}

void context_manager_widget::handle_open_context_clicked()
{
    graph_context* clicked_context = m_assigned_pointers[m_list_widget->currentItem()];
    m_tab_view->show_context(clicked_context);
}

void context_manager_widget::handle_rename_context_clicked()
{
    graph_context* clicked_context = m_assigned_pointers[m_list_widget->currentItem()];

    QStringList used_context_names;
    for (const auto& ctx : g_graph_context_manager.get_contexts())
    {
        used_context_names.append(ctx->name());
    }

    unique_string_validator unique_validator(used_context_names);
    empty_string_validator empty_validator;

    input_dialog ipd;
    ipd.set_window_title("Rename View");
    ipd.set_info_text("Please select a new and unique name for the view");
    ipd.set_input_text(clicked_context->name());
    ipd.add_validator(&unique_validator);
    ipd.add_validator(&empty_validator);

    if (ipd.exec() == QDialog::Accepted)
        g_graph_context_manager.rename_graph_context(clicked_context, ipd.text_value());
}

void context_manager_widget::handle_duplicate_context_clicked()
{
    graph_context* clicked_context = m_assigned_pointers[m_list_widget->currentItem()];
    graph_context* new_context     = g_graph_context_manager.create_new_context(clicked_context->name());
    new_context->add(clicked_context->modules(), clicked_context->gates());
}

void context_manager_widget::handle_delete_context_clicked()
{
    graph_context* clicked_context = m_assigned_pointers[m_list_widget->currentItem()];
    g_graph_context_manager.delete_graph_context(clicked_context);
}

void context_manager_widget::setup_toolbar(toolbar* toolbar)
{
    toolbar->addAction(m_new_view_action);
    toolbar->addAction(m_duplicate_action);
    toolbar->addAction(m_rename_action);
    toolbar->addAction(m_delete_action);
}

void context_manager_widget::handle_item_double_clicked(QListWidgetItem* clicked)
{
    Q_UNUSED(clicked);
    graph_context* clicked_context = m_assigned_pointers[m_list_widget->currentItem()];
    m_tab_view->show_context(clicked_context);
}

void context_manager_widget::handle_selection_changed()
{
    if (m_list_widget->selectedItems().isEmpty())
    {
        m_rename_action->setEnabled(false);
        m_duplicate_action->setEnabled(false);
        m_delete_action->setEnabled(false);
    }
    else
    {
        m_rename_action->setEnabled(true);
        m_duplicate_action->setEnabled(true);
        m_delete_action->setEnabled(true);
    }
}

void context_manager_widget::handle_context_created(graph_context* context)
{
    m_list_widget->addItem(context->name());
    auto new_item                 = m_list_widget->item(m_list_widget->count() - 1);
    m_assigned_pointers[new_item] = context;
    select_view_context(context);
}

void context_manager_widget::handle_context_renamed(graph_context* context)
{
    for (int i = 0; i < m_list_widget->count(); ++i)
    {
        if (m_assigned_pointers[m_list_widget->item(i)] == context)
        {
            m_list_widget->item(i)->setText(context->name());
            return;
        }
    }
}

void context_manager_widget::handle_context_removed(graph_context* context)
{
    for (int i = 0; i < m_list_widget->count(); ++i)
    {
        if (m_assigned_pointers[m_list_widget->item(i)] == context)
        {
            m_assigned_pointers.erase(m_list_widget->item(i));
            delete m_list_widget->takeItem(i);    // has to be deleted manually
            return;
        }
    }
}

// ##########################################################
// ##########################################################
// ##########################################################

QString context_manager_widget::new_view_icon_path() const
{
    return m_new_view_icon_path;
}

QString context_manager_widget::new_view_icon_style() const
{
    return m_new_view_icon_style;
}

QString context_manager_widget::rename_icon_path() const
{
    return m_rename_icon_path;
}

QString context_manager_widget::rename_icon_style() const
{
    return m_rename_icon_style;
}

QString context_manager_widget::duplicate_icon_path() const
{
    return m_duplicate_icon_path;
}

QString context_manager_widget::duplicate_icon_style() const
{
    return m_duplicate_icon_style;
}

QString context_manager_widget::delete_icon_path() const
{
    return m_delete_icon_path;
}

QString context_manager_widget::delete_icon_style() const
{
    return m_delete_icon_style;
}

void context_manager_widget::set_new_view_icon_path(const QString& path)
{
    m_new_view_icon_path = path;
}

void context_manager_widget::set_new_view_icon_style(const QString& style)
{
    m_new_view_icon_style = style;
}

void context_manager_widget::set_rename_icon_path(const QString& path)
{
    m_rename_icon_path = path;
}

void context_manager_widget::set_rename_icon_style(const QString& style)
{
    m_rename_icon_style = style;
}

void context_manager_widget::set_duplicate_icon_path(const QString& path)
{
    m_duplicate_icon_path = path;
}

void context_manager_widget::set_duplicate_icon_style(const QString& style)
{
    m_duplicate_icon_style = style;
}

void context_manager_widget::set_delete_icon_path(const QString& path)
{
    m_delete_icon_path = path;
}

void context_manager_widget::set_delete_icon_style(const QString& style)
{
    m_delete_icon_style = style;
}
