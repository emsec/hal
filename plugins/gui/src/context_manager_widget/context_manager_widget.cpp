#include "gui/context_manager_widget/context_manager_widget.h"

#include "gui/gui_globals.h"

#include "gui/graph_tab_widget/graph_tab_widget.h"
#include "gui/graph_widget/contexts/graph_context.h"
//#include "gui/graph_widget/graph_widget.h"

#include "gui/input_dialog/input_dialog.h"
#include "gui/validator/empty_string_validator.h"
#include "gui/validator/unique_string_validator.h"

//#include "hal_core/utilities/log.h"
#include "gui/gui_utils/graphics.h"
//#include "hal_core/netlist/gate.h"
//#include "hal_core/netlist/module.h"
//#include "hal_core/netlist/netlist.h"
#include "gui/toolbar/toolbar.h"
#include <QAction>
#include <QDebug>
//#include <QListWidgetItem>
#include <QMenu>
#include <QResizeEvent>
#include <QSize>
//#include <QStringList>
//#include <QStringListModel>
#include <QVBoxLayout>
#include <QHeaderView>

namespace hal
{
    ContextManagerWidget::ContextManagerWidget(GraphTabWidget* tab_view, QWidget* parent)
        : ContentWidget("Views", parent), m_new_view_action(new QAction(this)), m_rename_action(new QAction(this)), m_duplicate_action(new QAction(this)),
          m_delete_action(new QAction(this)), m_open_action(new QAction(this)), m_search_action(new QAction(this))
    {
        //needed to load the properties
        ensurePolished();
        m_tab_view = tab_view;

        m_open_action->setIcon(gui_utility::get_styled_svg_icon(m_open_icon_style, m_open_icon_path));
        m_new_view_action->setIcon(gui_utility::get_styled_svg_icon(m_new_view_icon_style, m_new_view_icon_path));
        m_rename_action->setIcon(gui_utility::get_styled_svg_icon(m_rename_icon_style, m_rename_icon_path));
        m_duplicate_action->setIcon(gui_utility::get_styled_svg_icon(m_duplicate_icon_style, m_duplicate_icon_path));
        m_delete_action->setIcon(gui_utility::get_styled_svg_icon(m_delete_icon_style, m_delete_icon_path));
        m_search_action->setIcon(gui_utility::get_styled_svg_icon(m_search_icon_style, m_search_icon_path));

        m_open_action->setToolTip("Open");
        m_new_view_action->setToolTip("New");
        m_rename_action->setToolTip("Rename");
        m_duplicate_action->setToolTip("Duplicate");
        m_delete_action->setToolTip("Delete");
        m_search_action->setToolTip("Search");

        m_open_action->setText("Open View");
        m_new_view_action->setText("Create New View");
        m_rename_action->setText("Rename View");
        m_duplicate_action->setText("Duplicate View");
        m_delete_action->setText("Delete View");

        //m_open_action->setEnabled(false);
        //m_rename_action->setEnabled(false);
        //m_duplicate_action->setEnabled(false);
        //m_delete_action->setEnabled(false);

        m_context_table_model = g_graph_context_manager->get_context_table_model();

        m_context_table_proxy_model = new ContextTableProxyModel();
        m_context_table_proxy_model->setSourceModel(m_context_table_model);
        m_context_table_proxy_model->setSortRole(Qt::UserRole);

        m_context_table_view = new QTableView(this);
        m_context_table_view->setModel(m_context_table_proxy_model);
        m_context_table_view->setSortingEnabled(true);
        m_context_table_view->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_context_table_view->setSelectionMode(QAbstractItemView::SingleSelection); // ERROR ???
        m_context_table_view->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
        m_context_table_view->sortByColumn(1, Qt::SortOrder::DescendingOrder);
        m_context_table_view->verticalHeader()->hide();
        m_context_table_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        m_context_table_view->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        m_context_table_view->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);


        m_content_layout->addWidget(m_context_table_view);
        m_content_layout->addWidget(&m_searchbar);

        m_searchbar.hide();

        connect(m_open_action, &QAction::triggered, this, &ContextManagerWidget::handle_open_context_clicked);
        connect(m_new_view_action, &QAction::triggered, this, &ContextManagerWidget::handle_create_context_clicked);
        connect(m_rename_action, &QAction::triggered, this, &ContextManagerWidget::handle_rename_context_clicked);
        connect(m_duplicate_action, &QAction::triggered, this, &ContextManagerWidget::handle_duplicate_context_clicked);
        connect(m_delete_action, &QAction::triggered, this, &ContextManagerWidget::handle_delete_context_clicked);
        connect(m_search_action, &QAction::triggered, this, &ContextManagerWidget::toggle_searchbar);

        connect(m_context_table_view, &QTableView::customContextMenuRequested, this, &ContextManagerWidget::handle_context_menu_request);
        connect(m_context_table_view, &QTableView::doubleClicked, this, &ContextManagerWidget::handle_open_context_clicked);
        connect(m_context_table_view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ContextManagerWidget::handle_selection_changed);

        connect(&m_searchbar, &Searchbar::text_edited, m_context_table_proxy_model, &ContextTableProxyModel::handle_filter_text_changed);
        connect(&m_searchbar, &Searchbar::text_edited, this, &ContextManagerWidget::handle_filter_text_changed);
    }

    void ContextManagerWidget::handle_create_context_clicked()
    {
        GraphContext* new_context = nullptr;
        new_context = g_graph_context_manager->create_new_context(QString::fromStdString(g_netlist->get_top_module()->get_name()));
        new_context->add({g_netlist->get_top_module()->get_id()}, {});
    }

    void ContextManagerWidget::handle_open_context_clicked()
    {
        GraphContext* clicked_context = get_current_context();
        m_tab_view->show_context(clicked_context);
    }

    void ContextManagerWidget::handle_rename_context_clicked()
    {
        GraphContext* clicked_context = get_current_context();

        QStringList used_context_names;
        for (const auto& context : g_graph_context_manager->get_contexts())
            used_context_names.append(context->name());

        UniqueStringValidator unique_validator(used_context_names);
        EmptyStringValidator empty_validator;

        InputDialog ipd;
        ipd.set_window_title("Rename View");
        ipd.set_info_text("Please select a new unique name for the view.");
        ipd.set_input_text(clicked_context->name());
        ipd.add_validator(&unique_validator);
        ipd.add_validator(&empty_validator);

        if (ipd.exec() == QDialog::Accepted)
            g_graph_context_manager->rename_graph_context(clicked_context, ipd.text_value());
    }

    void ContextManagerWidget::handle_duplicate_context_clicked()
    {
        GraphContext* clicked_context = get_current_context();
        GraphContext* new_context     = g_graph_context_manager->create_new_context(clicked_context->name() + " (Copy)");
        new_context->add(clicked_context->modules(), clicked_context->gates());
    }

    void ContextManagerWidget::handle_delete_context_clicked()
    {
        GraphContext* clicked_context = get_current_context();
        g_graph_context_manager->delete_graph_context(clicked_context);
    }

    void ContextManagerWidget::handle_selection_changed(const QItemSelection &selected, const QItemSelection &deselected)
    {
        Q_UNUSED(deselected);

        if(selected.indexes().isEmpty())
            set_toolbar_buttons_enabled(false);
        else
            set_toolbar_buttons_enabled(true);
    }

    void ContextManagerWidget::handle_context_menu_request(const QPoint& point)
    {
        const QModelIndex clicked_index = m_context_table_view->indexAt(point);

        QMenu context_menu;

        context_menu.addAction(m_new_view_action);

        if (clicked_index.isValid())
        {
            context_menu.addAction(m_open_action);
            context_menu.addAction(m_duplicate_action);
            context_menu.addAction(m_rename_action);
            context_menu.addAction(m_delete_action);
        }

        context_menu.exec(m_context_table_view->viewport()->mapToGlobal(point));
    }

    void ContextManagerWidget::handle_filter_text_changed(const QString& filter_text)
    {
        if(filter_text.isEmpty())
            m_search_action->setIcon(gui_utility::get_styled_svg_icon(m_search_icon_style, m_search_icon_path));
        else
            m_search_action->setIcon(gui_utility::get_styled_svg_icon("all->#30ac4f", m_search_icon_path)); //color test, integrate into stylsheet later
    }

    void ContextManagerWidget::select_view_context(GraphContext* context)
    {
        const QModelIndex source_model_index = m_context_table_model->get_index(context);
        const QModelIndex proxy_model_index = m_context_table_proxy_model->mapFromSource(source_model_index);

        if(proxy_model_index.isValid())
            m_context_table_view->setCurrentIndex(proxy_model_index);
        else
            m_context_table_view->clearSelection();
    }

    GraphContext* ContextManagerWidget::get_current_context()
    {
        QModelIndex proxy_model_index = m_context_table_view->currentIndex();
        QModelIndex source_model_index = m_context_table_proxy_model->mapToSource(proxy_model_index);

        return m_context_table_model->get_context(source_model_index);
    }

    void ContextManagerWidget::setup_toolbar(Toolbar* toolbar)
    {
        toolbar->addAction(m_new_view_action);
        toolbar->addAction(m_open_action);
        toolbar->addAction(m_duplicate_action);
        toolbar->addAction(m_rename_action);
        toolbar->addAction(m_delete_action);
        toolbar->addAction(m_search_action);
    }

    void ContextManagerWidget::set_toolbar_buttons_enabled(bool enabled)
    {
        m_open_action->setEnabled(enabled);
        m_rename_action->setEnabled(enabled);
        m_duplicate_action->setEnabled(enabled);
        m_delete_action->setEnabled(enabled);

        if(enabled)
        {
            m_open_action->setIcon(gui_utility::get_styled_svg_icon(m_open_icon_style, m_open_icon_path));
            m_rename_action->setIcon(gui_utility::get_styled_svg_icon(m_rename_icon_style, m_rename_icon_path));
            m_duplicate_action->setIcon(gui_utility::get_styled_svg_icon(m_duplicate_icon_style, m_duplicate_icon_path));
            m_delete_action->setIcon(gui_utility::get_styled_svg_icon(m_delete_icon_style, m_delete_icon_path));
        }
        else
        {
            m_open_action->setIcon(gui_utility::get_styled_svg_icon(m_disabled_icon_style, m_open_icon_path));
            m_rename_action->setIcon(gui_utility::get_styled_svg_icon(m_disabled_icon_style, m_rename_icon_path));
            m_duplicate_action->setIcon(gui_utility::get_styled_svg_icon(m_disabled_icon_style, m_duplicate_icon_path));
            m_delete_action->setIcon(gui_utility::get_styled_svg_icon(m_disabled_icon_style, m_delete_icon_path));
        }
    }

    QList<QShortcut*> ContextManagerWidget::create_shortcuts()
    {
        QShortcut* search_shortcut = g_keybind_manager->make_shortcut(this, "keybinds/searchbar_toggle");
        connect(search_shortcut, &QShortcut::activated, this, &ContextManagerWidget::toggle_searchbar);

        QList<QShortcut*> list;
        list.append(search_shortcut);

        return list;
    }

    void ContextManagerWidget::toggle_searchbar()
    {
        if (m_searchbar.isHidden())
        {
            m_searchbar.show();
            m_searchbar.setFocus();
        }
        else
        {
            m_searchbar.hide();
        }
    }

    QString ContextManagerWidget::disabled_icon_style() const
    {
        return m_disabled_icon_style;
    }

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

    QString ContextManagerWidget::open_icon_path() const
    {
        return m_open_icon_path;
    }

    QString ContextManagerWidget::open_icon_style() const
    {
        return m_open_icon_style;
    }

    QString ContextManagerWidget::search_icon_path() const
    {
        return m_search_icon_path;
    }

    QString ContextManagerWidget::search_icon_style() const
    {
        return m_search_icon_style;
    }

    void ContextManagerWidget::set_disabled_icon_style(const QString& style)
    {
        m_disabled_icon_style = style;
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

    void ContextManagerWidget::set_open_icon_path(const QString& path)
    {
        m_open_icon_path = path;
    }

    void ContextManagerWidget::set_open_icon_style(const QString& style)
    {
        m_open_icon_style = style;
    }

    void ContextManagerWidget::set_search_icon_path(const QString& path)
    {
        m_search_icon_path = path;
    }

    void ContextManagerWidget::set_search_icon_style(const QString& style)
    {
        m_search_icon_style = style;
    }
}
