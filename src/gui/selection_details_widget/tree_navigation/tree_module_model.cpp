#include "selection_details_widget/tree_navigation/tree_module_model.h"
#include "selection_details_widget/tree_navigation/tree_module_item.h"
#include "gui_globals.h"
#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/module.h"
#include "gui_utils/graphics.h"
#include <QDebug>

namespace hal
{
    tree_ModuleModel::tree_ModuleModel(QObject* parent) : QAbstractItemModel(parent)
    {
        QVector<QVariant> rootHeaderData;
        rootHeaderData << "Name"
                       << "ID"
                       << "Type";
        m_root_item = new tree_ModuleItem(rootHeaderData);
        load_data_settings();
        setup_model_data();
    }

    tree_ModuleModel::~tree_ModuleModel()
    {
        delete m_root_item;
    }

    QVariant tree_ModuleModel::data(const QModelIndex& index, int role) const
    {
        if (!index.isValid())
            return QVariant();

        // UserRole is mapped to "is a structure element?"
        if (role == Qt::UserRole)
            return get_item(index)->get_type() == tree_ModuleItem::item_type::structure;

        if (get_item(index)->get_type() == tree_ModuleItem::item_type::structure && index.column() == 0)
        {
            if (role == Qt::FontRole)
                return m_structured_font;

    //        if(get_item(index) == m_gates_item && role == Qt::DecorationRole)
    //            return m_design_icon;
        }

        if (role != Qt::DisplayRole)
            return QVariant();

        tree_ModuleItem* item = get_item(index);
        return item->data(index.column());
    }

    QVariant tree_ModuleModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
            return m_root_item->data(section);

        return QVariant();
    }

    QModelIndex tree_ModuleModel::index(int row, int column, const QModelIndex& parent) const
    {
        if (parent.isValid() && parent.column() != 0)
            return QModelIndex();

        tree_ModuleItem* parentItem = get_item(parent);
        tree_ModuleItem* childItem  = parentItem->get_child(row);

        if (childItem)
            return createIndex(row, column, childItem);
        else
            return QModelIndex();
    }

    QModelIndex tree_ModuleModel::parent(const QModelIndex& index) const
    {
        if (!index.isValid())
            return QModelIndex();

        tree_ModuleItem* childItem  = get_item(index);
        tree_ModuleItem* parentItem = childItem->get_parent();

        if (parentItem == m_root_item)
            return QModelIndex();

        return createIndex(parentItem->get_row_number(), 0, parentItem);
    }

    Qt::ItemFlags tree_ModuleModel::flags(const QModelIndex& index) const
    {
        if (!index.isValid())
            return 0;

        return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
    }

    int tree_ModuleModel::rowCount(const QModelIndex& parent) const
    {
        tree_ModuleItem* item = get_item(parent);
        return item->get_child_count();
    }

    int tree_ModuleModel::columnCount(const QModelIndex& parent) const
    {
        Q_UNUSED(parent)
        return m_root_item->get_column_count();
    }

    QModelIndexList tree_ModuleModel::get_corresponding_indexes(const QList<u32>& gate_ids, const QList<u32>& net_ids)
    {
        QModelIndexList list;
        for (int i = 0; i < m_gates_item->get_child_count(); i++)
        {
            tree_ModuleItem* current_item = m_gates_item->get_child(i);
            if (gate_ids.contains(current_item->data(1).toInt()))
                list.append(get_modelindexes_for_row(current_item));
        }
        for (int i = 2; i < m_nets_item->get_child_count(); i++)    //first two items are the global input/output items
        {
            tree_ModuleItem* current_item = m_nets_item->get_child(i);
            if (net_ids.contains(current_item->data(1).toInt()))
                list.append(get_modelindexes_for_row(current_item));
        }
        return list;
    //    for (int i = 0; i < m_global_output_nets_item->get_child_count(); i++)
    //    {
    //        tree_ModuleItem* current_item = m_global_output_nets_item->get_child(i);
    //        if (net_ids.contains(current_item->data(1).toInt()))
    //            list.append(get_modelindexes_for_row(current_item));
    //    }
    //    for (int i = 0; i < m_global_input_nets_item->get_child_count(); i++)
    //    {
    //        tree_ModuleItem* current_item = m_global_input_nets_item->get_child(i);
    //        if (net_ids.contains(current_item->data(1).toInt()))
    //            list.append(get_modelindexes_for_row(current_item));
    //    }
    //    for (int i = 0; i < m_modules_item->get_child_count(); i++)
    //    {
    //        tree_ModuleItem* current_item = m_modules_item->get_child(i);
    //        if (module_ids.contains(current_item->data(1).toInt()))
    //            list.append(get_modelindexes_for_row(current_item));
    //    }
    //    return list;
    }

    void tree_ModuleModel::update(u32 module_id)
    {
        int gates_child_count = m_gates_item->get_child_count();
        int nets_child_count = m_nets_item->get_child_count();
        for(int i = 0; i < gates_child_count; i++)
        {
            auto current_child = m_gates_item->get_child(0);
            remove_item(current_child);
        }
        for(int i = 0; i < nets_child_count; i++)
        {
            auto current_child = m_nets_item->get_child(0);
            remove_item(current_child);
        }

        auto _module = g_netlist->get_module_by_id(module_id);
        auto gates = _module->get_gates();
        auto nets = _module->get_internal_nets();

        m_gates_item->set_data(NAME_COLUMN, "Gates (" + QString::number(gates.size()) + ")");
        m_nets_item->set_data(NAME_COLUMN, "Nets (" + QString::number(nets.size()) + ")");

        for(const std::shared_ptr<Gate> &_g : gates)
        {
            tree_ModuleItem* item = new tree_ModuleItem(QVector<QVariant>() << QString::fromStdString(_g->get_name()) << _g->get_id() << QString::fromStdString(_g->get_type()->get_name()), tree_ModuleItem::item_type::gate, m_gates_item);
            insert_item(m_gates_item, m_gates_item->get_child_count(), item);
        }

        for(const std::shared_ptr<Net> &_n : nets)
        {
            tree_ModuleItem* item = new tree_ModuleItem(QVector<QVariant>() << QString::fromStdString(_n->get_name()) << _n->get_id() << "", tree_ModuleItem::item_type::net, m_nets_item);
            insert_item(m_nets_item, m_nets_item->get_child_count(), item);
        }
    }

    void tree_ModuleModel::setup_model_data()
    {
        m_gates_item = new tree_ModuleItem(QVector<QVariant>() << "Gates" << "" << "", tree_ModuleItem::item_type::structure, m_root_item);
        m_nets_item = new tree_ModuleItem(QVector<QVariant>() << "Nets" << "" << "", tree_ModuleItem::item_type::structure, m_root_item);
        m_root_item->insert_child(0, m_gates_item);
        m_root_item->insert_child(1, m_nets_item);
    }

    tree_ModuleItem* tree_ModuleModel::get_item(const QModelIndex& index) const
    {
        if (index.isValid())
        {
            tree_ModuleItem* item = static_cast<tree_ModuleItem*>(index.internalPointer());
            if (item)
                return item;
        }
        return m_root_item;
    }

    QModelIndex tree_ModuleModel::get_modelindex(tree_ModuleItem* item)
    {
        if (item == m_root_item)
            return QModelIndex();

        int row    = item->get_row_number();
        int column = 0;
        return createIndex(row, column, item);
    }

    QList<QModelIndex> tree_ModuleModel::get_modelindexes_for_row(tree_ModuleItem* item)
    {
        QList<QModelIndex> row_indexes;
        if (item == m_root_item)
            return row_indexes;

        int row = item->get_row_number();
        for (int column = 0; column < columnCount(); column++)
            row_indexes.append(createIndex(row, column, item));
        return row_indexes;
    }

    void tree_ModuleModel::insert_item(tree_ModuleItem* parent, int position, tree_ModuleItem* item)
    {
        const QModelIndex parent_index = get_modelindex(parent);
        int firstRow                   = position;
        int lastRow                    = position;

        beginInsertRows(parent_index, firstRow, lastRow);
        parent->insert_child(position, item);
        endInsertRows();
    }

    void tree_ModuleModel::remove_item(tree_ModuleItem* item)
    {
        tree_ModuleItem* parent  = item->get_parent();
        const QModelIndex parentIndex = get_modelindex(parent);
        int pos                       = item->get_row_number();
        int firstRow                  = pos;
        int lastRow                   = pos;
        beginRemoveRows(parentIndex, firstRow, lastRow);
        parent->remove_children(pos, 1);
        endRemoveRows();
    }

    void tree_ModuleModel::load_data_settings()
    {
        m_structured_font = QFont();
        m_structured_font.setBold(true);
        m_structured_font.setPixelSize(15);
        m_design_icon = gui_utility::get_styled_svg_icon("all->#888888", ":/icons/open");
    }
}
