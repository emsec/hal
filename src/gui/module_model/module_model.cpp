#include "module_model/module_model.h"

#include "module_model/module_item.h"

#include "gui/gui_globals.h"
#include "gui/gui_utils/graphics.h"
//#include "gui/ModuleModel/ModuleItem.h"

#include "netlist/gate.h"
#include "netlist/net.h"

namespace hal
{
    ModuleModel::ModuleModel(QObject* parent) : QAbstractItemModel(parent), m_top_ModuleItem(nullptr)
    {
    }

    QModelIndex ModuleModel::index(int row, int column, const QModelIndex& parent) const
    {
        // BEHAVIOR FOR ILLEGAL INDICES IS UNDEFINED
        // SEE QT DOCUMENTATION

        if (!parent.isValid())
        {
            if (row == 0 && column == 0 && m_top_ModuleItem)
                return createIndex(0, 0, m_top_ModuleItem);
            else
                return QModelIndex();
        }

        if (column != 0 || parent.column() != 0)
            return QModelIndex();

        ModuleItem* parent_item = get_item(parent);

        ModuleItem* child_item = static_cast<ModuleItem*>(parent_item)->child(row);
        assert(child_item);

        return createIndex(row, column, child_item);

        // NECESSARY ???
        //    if (column != 0)
        //        return QModelIndex();

        //    // PROBABLY REDUNDANT
        //    if (parent.isValid() && parent.column() != 0)
        //        return QModelIndex();

        //    ModuleItem* parent_item = get_item(parent);
        //    ModuleItem* child_item = parent_item->child(row);

        //    if (child_item)
        //        return createIndex(row, column, child_item);
        //    else
        //        return QModelIndex();
    }

    QModelIndex ModuleModel::parent(const QModelIndex& index) const
    {
        if (!index.isValid())
            return QModelIndex();

        ModuleItem* item = get_item(index);

        if (item == m_top_ModuleItem)
            return QModelIndex();

        ModuleItem* parent_item = item->parent();
        return createIndex(parent_item->row(), 0, parent_item);

        //    if (!index.isValid())
        //        return QModelIndex();

        //    ModuleItem* child_item  = get_item(index);
        //    ModuleItem* parent_item = child_item->parent();

        //    if (parent_item == m_root_item)
        //        return QModelIndex();

        //    return createIndex(parent_item->row(), 0, parent_item);
    }

    int ModuleModel::rowCount(const QModelIndex& parent) const
    {
        if (!parent.isValid())    // ??
            return 1;

        if (parent.column() != 0)
            return 0;

        ModuleItem* parent_item = get_item(parent);

        return parent_item->childCount();
    }

    int ModuleModel::columnCount(const QModelIndex& parent) const
    {
        Q_UNUSED(parent)

        return 1;
    }

    QVariant ModuleModel::data(const QModelIndex& index, int role) const
    {
        if (!index.isValid())
            return QVariant();

        ModuleItem* item = static_cast<ModuleItem*>(index.internalPointer());

        if (!item)
            return QVariant();

        switch (role)
        {
            case Qt::DecorationRole:
            {
                if (index.column() == 0)
                {
                    QString run_icon_style = "all->" + item->color().name();
                    QString run_icon_path  = ":/icons/filled-circle";

                    return gui_utility::get_styled_svg_icon(run_icon_style, run_icon_path);
                }
                break;
            }
            case Qt::DisplayRole:
            {
                return item->data(index.column());
            }
            case Qt::ForegroundRole:
            {
                if (item->highlighted())
                    return QColor(QColor(255, 221, 0));    // USE STYLESHEETS
                else
                    return QColor(QColor(255, 255, 255));    // USE STYLESHEETS
            }
            default:
                return QVariant();
        }
        return QVariant();
    }

    Qt::ItemFlags ModuleModel::flags(const QModelIndex& index) const
    {
        if (!index.isValid())
            return 0;

        return QAbstractItemModel::flags(index);
    }

    QVariant ModuleModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        Q_UNUSED(section)
        Q_UNUSED(orientation)
        Q_UNUSED(role)

        return QVariant();
    }

    ModuleItem* ModuleModel::get_item(const QModelIndex& index) const
    {
        if (index.isValid())
            return static_cast<ModuleItem*>(index.internalPointer());
        else
            return nullptr;
    }

    QModelIndex ModuleModel::get_index(const ModuleItem* const item) const
    {
        assert(item);

        QVector<int> row_numbers;
        const ModuleItem* current_item = item;

        while (current_item != m_top_ModuleItem)
        {
            row_numbers.append(current_item->row());
            current_item = current_item->const_parent();
        }

        QModelIndex model_index = index(0, 0, QModelIndex());

        for (QVector<int>::const_reverse_iterator i = row_numbers.crbegin(); i != row_numbers.crend(); ++i)
            model_index = index(*i, 0, model_index);

        return model_index;
    }

    void ModuleModel::init()
    {
        ModuleItem* item = new ModuleItem(1);

        m_ModuleItems.insert(1, item);

        beginInsertRows(index(0, 0, QModelIndex()), 0, 0);
        m_top_ModuleItem = item;
        endInsertRows();

        // This is broken because it can attempt to insert a child before its parent
        // which will cause an assertion failure and then crash

        // std::set<std::shared_ptr<Module>> s = g_netlist->get_modules();
        // s.erase(g_netlist->get_top_module());
        // for (std::shared_ptr<Module> m : s)
        //     add_module(m->get_id(), m->get_parent_module()->get_id());

        // This works

        // recursively insert modules
        std::shared_ptr<Module> m = g_netlist->get_top_module();
        add_recursively(m->get_submodules());
    }

    void ModuleModel::clear()
    {
        beginResetModel();

        m_top_ModuleItem = nullptr;

        for (ModuleItem* m : m_ModuleItems)
            delete m;

        endResetModel();
    }

    void ModuleModel::add_module(const u32 id, const u32 parent_module)
    {
        assert(g_netlist->get_module_by_id(id));
        assert(g_netlist->get_module_by_id(parent_module));
        assert(!m_ModuleItems.contains(id));
        assert(m_ModuleItems.contains(parent_module));

        ModuleItem* item   = new ModuleItem(id);
        ModuleItem* parent = m_ModuleItems.value(parent_module);

        item->set_parent(parent);
        m_ModuleItems.insert(id, item);

        QModelIndex index = get_index(parent);

        int row = parent->childCount();
        m_is_modifying = true;
        beginInsertRows(index, row, row);
        parent->insert_child(row, item);
        m_is_modifying = false;
        endInsertRows();
    }

    void ModuleModel::add_recursively(std::set<std::shared_ptr<Module>> modules)
    {
        for (auto &m : modules)
        {
            add_module(m->get_id(), m->get_parent_module()->get_id());
            add_recursively(m->get_submodules());
        }
    }

    void ModuleModel::remove_module(const u32 id)
    {
        assert(id != 1);
        assert(g_netlist->get_module_by_id(id));
        assert(m_ModuleItems.contains(id));

        ModuleItem* item   = m_ModuleItems.value(id);
        ModuleItem* parent = item->parent();
        assert(item);
        assert(parent);

        QModelIndex index = get_index(parent);

        int row = item->row();

        m_is_modifying = true;
        beginRemoveRows(index, row, row);
        parent->remove_child(item);
        m_is_modifying = false;
        endRemoveRows();

        m_ModuleItems.remove(id);
        delete item;
    }

    void ModuleModel::update_module(const u32 id)    // SPLIT ???
    {
        assert(g_netlist->get_module_by_id(id));
        assert(m_ModuleItems.contains(id));

        ModuleItem* item = m_ModuleItems.value(id);
        assert(item);

        item->set_name(QString::fromStdString(g_netlist->get_module_by_id(id)->get_name()));    // REMOVE & ADD AGAIN
        item->set_color(g_NetlistRelay.get_module_color(id));

        QModelIndex index = get_index(item);
        Q_EMIT dataChanged(index, index);
    }

    ModuleItem* ModuleModel::get_item(const u32 module_id) const
    {
        return m_ModuleItems.value(module_id);
    }

    bool ModuleModel::is_modifying()
    {
        return m_is_modifying;
    }
}
