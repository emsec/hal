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

namespace hal
{
    ContextManagerWidget::ContextManagerWidget(GraphTabWidget* tab_view, QWidget* parent)
        : ContentWidget("View Manager", parent), m_list_widget(new QListWidget()), m_new_view_action(new QAction(this)), m_rename_action(new QAction(this)), m_duplicate_action(new QAction(this)),
          m_delete_action(new QAction(this)), m_timestamp_action(new QAction(this))
    {
        //needed to load the properties
        ensurePolished();
        m_tab_view = tab_view;

        m_list_widget->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

        m_content_layout->addWidget(m_list_widget);

        connect(m_list_widget, &QListWidget::customContextMenuRequested, this, &ContextManagerWidget::handle_context_menu_request);
        connect(m_list_widget, &QListWidget::itemDoubleClicked, this, &ContextManagerWidget::handle_item_double_clicked);
        connect(m_list_widget, &QListWidget::itemSelectionChanged, this, &ContextManagerWidget::handle_selection_changed);

        connect(&g_graph_context_manager, &GraphContextManager::context_created, this, &ContextManagerWidget::handle_context_created);
        connect(&g_graph_context_manager, &GraphContextManager::context_renamed, this, &ContextManagerWidget::handle_context_renamed);
        connect(&g_graph_context_manager, &GraphContextManager::deleting_context, this, &ContextManagerWidget::handle_context_removed);

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
        m_timestamp_action->setIcon(gui_utility::get_styled_svg_icon(m_timestamp_icon_style, m_timestamp_icon_path));
        m_timestamp_action->setToolTip("Toggle Timestamps");


        m_rename_action->setEnabled(false);
        m_duplicate_action->setEnabled(false);
        m_delete_action->setEnabled(false);

        connect(m_new_view_action, &QAction::triggered, this, &ContextManagerWidget::handle_create_context_clicked);
        connect(m_rename_action, &QAction::triggered, this, &ContextManagerWidget::handle_rename_context_clicked);
        connect(m_duplicate_action, &QAction::triggered, this, &ContextManagerWidget::handle_duplicate_context_clicked);
        connect(m_delete_action, &QAction::triggered, this, &ContextManagerWidget::handle_delete_context_clicked);
        connect(m_timestamp_action, &QAction::triggered, this, &ContextManagerWidget::handle_toggle_timestamps_clicked);
    }

    void ContextManagerWidget::resizeEvent(QResizeEvent* event)
    {
        m_list_widget->setFixedWidth(event->size().width());
    }

    void ContextManagerWidget::handle_context_menu_request(const QPoint& point)
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
        connect(&create_action, &QAction::triggered, this, &ContextManagerWidget::handle_create_context_clicked);

        if (clicked_index.isValid())
        {
            context_menu.addAction(&open_action);
            context_menu.addAction(&rename_action);
            context_menu.addAction(&delete_action);
            context_menu.addAction(&duplicate_action);

            connect(&open_action, &QAction::triggered, this, &ContextManagerWidget::handle_open_context_clicked);
            connect(&rename_action, &QAction::triggered, this, &ContextManagerWidget::handle_rename_context_clicked);
            connect(&duplicate_action, &QAction::triggered, this, &ContextManagerWidget::handle_duplicate_context_clicked);
            connect(&delete_action, &QAction::triggered, this, &ContextManagerWidget::handle_delete_context_clicked);
        }
        //show context menu
        context_menu.exec(m_list_widget->viewport()->mapToGlobal(point));
    }

    void ContextManagerWidget::handle_create_context_clicked()
    {
        GraphContext* new_context = nullptr;

        new_context = g_graph_context_manager.create_new_context(QString::fromStdString(g_netlist->get_top_module()->get_name()));
        new_context->add({g_netlist->get_top_module()->get_id()}, {});

        m_tab_view->show_context(new_context);
    }

    void ContextManagerWidget::select_view_context(GraphContext* context)
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

    void ContextManagerWidget::handle_open_context_clicked()
    {
        GraphContext* clicked_context = m_assigned_pointers[m_list_widget->currentItem()];
        m_tab_view->show_context(clicked_context);
    }

    void ContextManagerWidget::handle_rename_context_clicked()
    {
        GraphContext* clicked_context = m_assigned_pointers[m_list_widget->currentItem()];

        QStringList used_context_names;
        for (const auto& ctx : g_graph_context_manager.get_contexts())
        {
            used_context_names.append(ctx->name());
        }

        UniqueStringValidator unique_validator(used_context_names);
        EmptyStringValidator empty_validator;

        InputDialog ipd;
        ipd.set_window_title("Rename View");
        ipd.set_info_text("Please select a new and unique name for the view");
        ipd.set_input_text(clicked_context->name());
        ipd.add_validator(&unique_validator);
        ipd.add_validator(&empty_validator);

        if (ipd.exec() == QDialog::Accepted)
            g_graph_context_manager.rename_graph_context(clicked_context, ipd.text_value());
    }

    void ContextManagerWidget::handle_duplicate_context_clicked()
    {
        GraphContext* clicked_context = m_assigned_pointers[m_list_widget->currentItem()];
        GraphContext* new_context     = g_graph_context_manager.create_new_context(clicked_context->name());
        new_context->add(clicked_context->modules(), clicked_context->gates());
    }

    void ContextManagerWidget::handle_delete_context_clicked()
    {
        GraphContext* clicked_context = m_assigned_pointers[m_list_widget->currentItem()];
        g_graph_context_manager.delete_graph_context(clicked_context);
    }

    void ContextManagerWidget::handle_toggle_timestamps_clicked()
    {
        m_show_timestamps = !m_show_timestamps;

        for(int i = 0; i < m_list_widget->count(); i++)
        {
            auto item = m_list_widget->item(i);
            auto context = m_assigned_pointers[item];

            QString context_name = context->name();

            if(m_show_timestamps)
                context_name += " (" + context->get_timestamp().toString(Qt::SystemLocaleShortDate) + ")";

            item->setText(context_name);
        }
    }


    void ContextManagerWidget::setup_toolbar(Toolbar* Toolbar)
    {
        Toolbar->addAction(m_new_view_action);
        Toolbar->addAction(m_duplicate_action);
        Toolbar->addAction(m_rename_action);
        Toolbar->addAction(m_delete_action);
        Toolbar->addAction(m_timestamp_action);
    }

    void ContextManagerWidget::handle_item_double_clicked(QListWidgetItem* clicked)
    {
        Q_UNUSED(clicked);
        GraphContext* clicked_context = m_assigned_pointers[m_list_widget->currentItem()];
        m_tab_view->show_context(clicked_context);
    }

    void ContextManagerWidget::handle_selection_changed()
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

    void ContextManagerWidget::handle_context_created(GraphContext* context)
    {
        QString context_name = context->name();

        if(m_show_timestamps)
            context_name += " (" + context->get_timestamp().toString(Qt::SystemLocaleShortDate) + ")";

        m_list_widget->addItem(context_name);
        auto new_item                 = m_list_widget->item(m_list_widget->count() - 1);
        m_assigned_pointers[new_item] = context;
        select_view_context(context);
    }

    void ContextManagerWidget::handle_context_renamed(GraphContext* context)
    {
        for (int i = 0; i < m_list_widget->count(); ++i)
        {
            if (m_assigned_pointers[m_list_widget->item(i)] == context)
            {
                QString context_name = context->name();

                if(m_show_timestamps)
                    context_name += " (" + context->get_timestamp().toString(Qt::SystemLocaleShortDate) + ")";

                m_list_widget->item(i)->setText(context_name);
                return;
            }
        }
    }

    void ContextManagerWidget::handle_context_removed(GraphContext* context)
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

    QString ContextManagerWidget::new_view_icon_path() const
    {
        return m_new_view_icon_path;
    }

    QString ContextManagerWidget::new_view_icon_style() const
    {
        return m_new_view_icon_style;
    }

    QString ContextManagerWidget::rename_icon_path() const
    {
        return m_rename_icon_path;
    }

    QString ContextManagerWidget::rename_icon_style() const
    {
        return m_rename_icon_style;
    }

    QString ContextManagerWidget::duplicate_icon_path() const
    {
        return m_duplicate_icon_path;
    }

    QString ContextManagerWidget::duplicate_icon_style() const
    {
        return m_duplicate_icon_style;
    }

    QString ContextManagerWidget::delete_icon_path() const
    {
        return m_delete_icon_path;
    }

    QString ContextManagerWidget::delete_icon_style() const
    {
        return m_delete_icon_style;
    }

    QString ContextManagerWidget::timestamp_icon_path() const
    {
        return m_timestamp_icon_path;
    }

    QString ContextManagerWidget::timestamp_icon_style() const
    {
        return m_timestamp_icon_style;
    }

    void ContextManagerWidget::set_new_view_icon_path(const QString& path)
    {
        m_new_view_icon_path = path;
    }

    void ContextManagerWidget::set_new_view_icon_style(const QString& style)
    {
        m_new_view_icon_style = style;
    }

    void ContextManagerWidget::set_rename_icon_path(const QString& path)
    {
        m_rename_icon_path = path;
    }

    void ContextManagerWidget::set_rename_icon_style(const QString& style)
    {
        m_rename_icon_style = style;
    }

    void ContextManagerWidget::set_duplicate_icon_path(const QString& path)
    {
        m_duplicate_icon_path = path;
    }

    void ContextManagerWidget::set_duplicate_icon_style(const QString& style)
    {
        m_duplicate_icon_style = style;
    }

    void ContextManagerWidget::set_delete_icon_path(const QString& path)
    {
        m_delete_icon_path = path;
    }

    void ContextManagerWidget::set_delete_icon_style(const QString& style)
    {
        m_delete_icon_style = style;
    }

    void ContextManagerWidget::set_timestamp_icon_path(const QString& path)
    {
        m_timestamp_icon_path = path;
    }

    void ContextManagerWidget::set_timestamp_icon_style(const QString& style)
    {
        m_timestamp_icon_style = style;
    }
}
