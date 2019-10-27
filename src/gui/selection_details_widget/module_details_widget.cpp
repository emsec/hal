#include "selection_details_widget/module_details_widget.h"
#include "gui_globals.h"
#include "netlist/gate.h"
#include "netlist/module.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QShortcut>
#include <QRegExp>
#include <QDebug>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>

#include "selection_details_widget/tree_navigation/tree_module_item.h"

module_details_widget::module_details_widget(QWidget* parent) : QWidget(parent), m_treeview(new QTreeView(this)), m_tree_module_model(new tree_module_model(this)),
    m_tree_module_proxy_model(new tree_module_proxy_model(this)), m_ignore_selection_change(false)
{
    m_content_layout = new QVBoxLayout(this);
    m_content_layout->setContentsMargins(0, 0, 0, 0);
    m_content_layout->setSpacing(0);
    m_content_layout->setAlignment(Qt::AlignTop);

    m_treeview->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tree_module_proxy_model->setSourceModel(m_tree_module_model);
    m_tree_module_proxy_model->setFilterKeyColumn(-1);
    m_treeview->setModel(m_tree_module_proxy_model);
    m_treeview->setExpanded(m_tree_module_proxy_model->index(0, 0, m_treeview->rootIndex()), true);

    m_general_table = new QTableWidget(2,2, this);
    m_general_table->horizontalHeader()->setStretchLastSection(true);
    m_general_table->horizontalHeader()->hide();
    m_general_table->verticalHeader()->hide();

    QFont font("Iosevka");
    font.setBold(true);
    font.setPixelSize(13);

    QTableWidgetItem* name_item = new QTableWidgetItem("Name: ");
    name_item->setFont(font);
    name_item->setFlags(Qt::ItemIsEnabled);
    m_general_table->setItem(0,0, name_item);

    m_name_item = new QTableWidgetItem();
    m_name_item->setFlags(Qt::ItemIsEnabled);
    m_general_table->setItem(0, 1, m_name_item);

    QTableWidgetItem* id_item = new QTableWidgetItem("ID: ");
    id_item->setFont(font);
    id_item->setFlags(Qt::ItemIsEnabled);
    m_general_table->setItem(1, 0, id_item);

    m_id_item = new QTableWidgetItem();
    m_id_item->setFlags(Qt::ItemIsEnabled);
    m_general_table->setItem(1, 1, m_id_item);

    m_general_table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    m_general_table->verticalHeader()->setDefaultSectionSize(16);
    m_general_table->resizeColumnToContents(0);
    m_general_table->setShowGrid(false);
    m_general_table->setFocusPolicy(Qt::NoFocus);
    m_general_table->setFrameStyle(QFrame::NoFrame);
    m_general_table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_general_table->setFixedHeight(m_general_table->verticalHeader()->length());

    m_content_layout->addWidget(m_general_table);
    m_content_layout->addWidget(m_treeview);

//    QShortcut* search_shortcut = new QShortcut(QKeySequence("Ctrl+f"), this);
//    connect(search_shortcut, &QShortcut::activated, this, &module_details_widget::toggle_searchbar);

//    QShortcut* resize_shortcut = new QShortcut(QKeySequence("Ctrl+b"), this);
//    connect(resize_shortcut, &QShortcut::activated, this, &module_details_widget::toggle_resize_columns);

    connect(&g_netlist_relay, &netlist_relay::module_event, this, &module_details_widget::handle_module_event);

    //g_selection_relay.register_sender(this, "module_details_widget");
    connect(m_treeview, &QTreeView::doubleClicked, this, &module_details_widget::handle_tree_double_clicked);
    connect(&g_selection_relay, &selection_relay::selection_changed, this, &module_details_widget::handle_selection_changed);
}

void module_details_widget::handle_module_event(module_event_handler::event ev, std::shared_ptr<module> module, u32 associated_data)
{
    Q_UNUSED(ev)
    Q_UNUSED(associated_data)
    if (m_current_id == module->get_id())
    {
        update(module->get_id());
    }
}

void module_details_widget::handle_selection_changed(void *sender)
{
    if (sender == this)
    {
        return;
    }

    //neccessary to elimante a bug where this widget erases the module_selection
    if(!g_selection_relay.m_selected_modules.isEmpty())
        return;

    QList<u32> gate_ids, net_ids;

    for (auto i : g_selection_relay.m_selected_gates)
    {
        gate_ids.append(i);
    }
    for (auto i : g_selection_relay.m_selected_nets)
    {
        net_ids.append(i);
    }

    QModelIndexList selected_indexes = m_tree_module_model->get_corresponding_indexes(gate_ids, net_ids);
    QItemSelection selection;
    for (const auto& index : selected_indexes)
        selection.select(m_tree_module_proxy_model->mapFromSource(index), m_tree_module_proxy_model->mapFromSource(index));


    if(!selection.isEmpty() || (!m_treeview->selectionModel()->selectedIndexes().isEmpty() && selection.isEmpty()))
        m_ignore_selection_change = true;

    m_treeview->selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
}

void module_details_widget::handle_searchbar_text_edited(const QString &text)
{
    QRegExp* regex = new QRegExp(text);
    if (regex->isValid())
        m_tree_module_proxy_model->setFilterRegExp(*regex);
}

void module_details_widget::handle_tree_double_clicked(const QModelIndex &index)
{
    if(!index.isValid())
        return;

    auto item = static_cast<tree_module_item*>(m_tree_module_proxy_model->mapToSource(index).internalPointer());

    //this line neccessary to call g_selection_relay.clear() without problems
    if(item->get_type() == tree_module_item::item_type::structure)
        return;

    g_selection_relay.clear();
    auto id = item->data(tree_module_model::ID_COLUMN).toInt();

    switch(item->get_type())
    {
        case tree_module_item::item_type::gate:
            g_selection_relay.m_selected_gates.insert(id);
            g_selection_relay.selection_changed(this);
            return;
        case tree_module_item::item_type::net:
            g_selection_relay.m_selected_nets.insert(id);
            g_selection_relay.selection_changed(this);
            return;
        default:
            break;
    }

}

void module_details_widget::toggle_resize_columns()
{
    for(int i = 0; i < m_tree_module_model->columnCount(); i++)
        m_treeview->resizeColumnToContents(i);
}

void module_details_widget::update(u32 module_id)
{
    m_current_id = module_id;
    m_tree_module_model->update(module_id);
    toggle_resize_columns();

    if(!m_treeview->selectionModel()->selectedIndexes().isEmpty())
        m_ignore_selection_change = true;
    m_treeview->clearSelection();

    m_id_item->setText(QString::number(module_id));
    m_name_item->setText(QString::fromStdString(g_netlist->get_module_by_id(module_id)->get_name()));
}
