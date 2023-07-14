#include "gui/module_model/module_model.h"

#include "gui/module_model/module_item.h"

#include "gui/gui_globals.h"
#include "gui/gui_utils/graphics.h"
//#include "gui/ModuleModel/ModuleItem.h"

#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"

namespace hal
{
    ModuleModel::ModuleModel(QObject* parent) : QAbstractItemModel(parent), mTopModuleItem(nullptr)
    {
    }

    QModelIndex ModuleModel::index(int row, int column, const QModelIndex& parent) const
    {
        // BEHAVIOR FOR ILLEGAL INDICES IS UNDEFINED
        // SEE QT DOCUMENTATION
        if (!hasIndex(row, column, parent))
            return QModelIndex();

        if (!parent.isValid())
        {
            if (row == 0 && column >= 0 && column < 3 && mTopModuleItem)
                return createIndex(0, column, mTopModuleItem);
            else
                return QModelIndex();
        }

        if (column < 0 || column >= 3 || parent.column() < 0 || parent.column() >= 3)
            return QModelIndex();

        ModuleItem* parent_item = getItem(parent);

        ModuleItem* child_item = static_cast<ModuleItem*>(parent_item)->child(row);
        assert(child_item);

        return createIndex(row, column, child_item);

        // NECESSARY ???
        //    if (column != 0)
        //        return QModelIndex();

        //    // PROBABLY REDUNDANT
        //    if (parent.isValid() && parent.column() != 0)
        //        return QModelIndex();

        //    ModuleItem* parent_item = getItem(parent);
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

        ModuleItem* item = getItem(index);

        if (item == mTopModuleItem)
            return QModelIndex();

        ModuleItem* parent_item = item->parent();
        return createIndex(parent_item->row(), 0, parent_item);

        //    if (!index.isValid())
        //        return QModelIndex();

        //    ModuleItem* child_item  = getItem(index);
        //    ModuleItem* parent_item = child_item->parent();

        //    if (parent_item == m_root_item)
        //        return QModelIndex();

        //    return createIndex(parent_item->row(), 0, parent_item);
    }

    int ModuleModel::rowCount(const QModelIndex& parent) const
    {
        if (!parent.isValid())    // ??
            return 1;

        //if (parent.column() != 0)
        //    return 0;

        ModuleItem* parent_item = getItem(parent);

        return parent_item->childCount();
    }

    int ModuleModel::columnCount(const QModelIndex& parent) const
    {
        Q_UNUSED(parent)

        return 3;
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
                    QString runIconStyle = "all->" + item->color().name();
                    //QString runIconPath  = ":/icons/filled-circle";
                    QString moduleIconPath = ":/icons/ne_module";
                    QString gateIconPath = ":/icons/ne_gate";
                    QString netIconPath = ":/icons/ne_net";

                    switch(item->getType()){
                        case ModuleItem::TreeItemType::Module:
                            return gui_utility::getStyledSvgIcon(runIconStyle, moduleIconPath);
                        case ModuleItem::TreeItemType::Gate:
                            runIconStyle = "all->" + item->parent()->color().name();
                            return gui_utility::getStyledSvgIcon(runIconStyle, gateIconPath);
                        case ModuleItem::TreeItemType::Net:
                            runIconStyle = "all->" + item->parent()->color().name();
                            return gui_utility::getStyledSvgIcon(runIconStyle, netIconPath);
                    }
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
        /*Q_UNUSED(section)
        Q_UNUSED(orientation)
        Q_UNUSED(role)

        return QVariant();*/
        const char* horizontalHeader[] = { "Name", "ID", "Type"};
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section < columnCount())
            return QString(horizontalHeader[section]);

        return QVariant();
    }

    ModuleItem* ModuleModel::getItem(const QModelIndex& index) const
    {
        if (index.isValid())
            return static_cast<ModuleItem*>(index.internalPointer());
        else
            return nullptr;
    }

    QModelIndex ModuleModel::getIndex(const ModuleItem* const item) const
    {
        assert(item);

        QVector<int> row_numbers;
        const ModuleItem* current_item = item;

        while (current_item != mTopModuleItem)
        {
            row_numbers.append(current_item->row());
            current_item = current_item->constParent();
        }

        QModelIndex model_index = index(0, 0, QModelIndex());

        for (QVector<int>::const_reverse_iterator i = row_numbers.crbegin(); i != row_numbers.crend(); ++i)
            model_index = index(*i, 0, model_index);

        return model_index;
    }

    void ModuleModel::init()
    {
        setModuleColor(1, QColor(96, 110, 112));
        ModuleItem* item = new ModuleItem(1);
        mModuleMap.insert(1, item);

        beginInsertRows(index(0, 0, QModelIndex()), 0, 0);
        mTopModuleItem = item;
        endInsertRows();

        // This is broken because it can attempt to insert a child before its parent
        // which will cause an assertion failure and then crash

        // std::set<Module*> s = gNetlist->get_modules();
        // s.erase(gNetlist->get_top_module());
        // for (Module* m : s)
        //     addModule(m->get_id(), m->get_parent_module()->get_id());

        // This works

        // recursively insert modules
        Module* m = gNetlist->get_top_module();
        addRecursively(m->get_submodules());
        // add remaining gates and modules
        for(auto g : m->get_gates())
            addGate(g->get_id(), 1);
        for(auto n: m->get_internal_nets())
            addNet(n->get_id(), 1);
    }

    void ModuleModel::clear()
    {
        beginResetModel();

        mTopModuleItem = nullptr;

        for (ModuleItem* m : mModuleMap)
            delete m;
        for (ModuleItem* g : mGateMap)
            delete g;
        for (ModuleItem* n : mNetMap)
            delete n;

        mModuleMap.clear();
        mGateMap.clear();
        mNetMap.clear();
        mModuleColors.clear();

        endResetModel();
    }

    void ModuleModel::addModule(u32 id, u32 parent_module)
    {
        assert(gNetlist->get_module_by_id(id));
        assert(gNetlist->get_module_by_id(parent_module));
        assert(!mModuleMap.contains(id));
        assert(mModuleMap.contains(parent_module));

        ModuleItem* item   = new ModuleItem(id);
        item->appendExistingChildIfAny(mModuleMap);
        ModuleItem* parent = mModuleMap.value(parent_module);

        item->setParent(parent);
        mModuleMap.insert(id, item);

        QModelIndex index = getIndex(parent);

        int row = parent->childCount();
        mIsModifying = true;
        beginInsertRows(index, row, row);
        parent->insertChild(row, item);
        mIsModifying = false;
        endInsertRows();
    }

    void ModuleModel::addGate(u32 id, u32 parent_module)
    {
        assert(gNetlist->get_gate_by_id(id));
        assert(gNetlist->get_module_by_id(parent_module));
        assert(!mGateMap.contains(id));
        assert(mModuleMap.contains(parent_module));
        ModuleItem* item   = new ModuleItem(id, ModuleItem::TreeItemType::Gate);
        //item->appendExistingChildIfAny(mModuleMap);
        ModuleItem* parent = mModuleMap.value(parent_module);

        item->setParent(parent);
        mGateMap.insert(id, item);

        QModelIndex index = getIndex(parent);

        int row = parent->childCount();
        mIsModifying = true;
        beginInsertRows(index, row, row);
        parent->insertChild(row, item);
        mIsModifying = false;
        endInsertRows();
    }


    void ModuleModel::addNet(u32 id, u32 parent_module)
    {
        assert(gNetlist->get_net_by_id(id));
        assert(gNetlist->get_module_by_id(parent_module));
        assert(!mNetMap.contains(id));
        assert(mModuleMap.contains(parent_module));

        ModuleItem* item   = new ModuleItem(id, ModuleItem::TreeItemType::Net);
        //item->appendExistingChildIfAny(mModuleMap);
        ModuleItem* parent = mModuleMap.value(parent_module);

        item->setParent(parent);
        mNetMap.insert(id, item);

        QModelIndex index = getIndex(parent);

        int row = parent->childCount();
        mIsModifying = true;
        beginInsertRows(index, row, row);
        parent->insertChild(row, item);
        mIsModifying = false;
        endInsertRows();
    }

    void ModuleModel::addRecursively(const std::vector<Module*>& modules)
    {
        for (auto &m : modules)
        {
            addModule(m->get_id(), m->get_parent_module()->get_id());
            addRecursively(m->get_submodules());

            for(auto &g : m->get_gates())
                addGate(g->get_id(), m->get_id());
            for(auto &n : m->get_internal_nets())
                addNet(n->get_id(), m->get_id());
        }
    }

    void ModuleModel::remove_module(const u32 id)
    {
        assert(id != 1);
        assert(gNetlist->get_module_by_id(id));
        assert(mModuleMap.contains(id));

        ModuleItem* item   = mModuleMap.value(id);
        ModuleItem* parent = item->parent();
        assert(item);
        assert(parent);

        QModelIndex index = getIndex(parent);

        int row = item->row();

        mIsModifying = true;
        beginRemoveRows(index, row, row);
        parent->removeChild(item);
        mIsModifying = false;
        endRemoveRows();

        mModuleMap.remove(id);
        delete item;
    }

    void ModuleModel::remove_gate(const u32 id)
    {
        assert(gNetlist->get_gate_by_id(id));
        assert(mGateMap.contains(id));

        ModuleItem* item   = mGateMap.value(id);
        ModuleItem* parent = item->parent();
        assert(item);
        assert(parent);

        QModelIndex index = getIndex(parent);

        int row = item->row();

        mIsModifying = true;
        beginRemoveRows(index, row, row);
        parent->removeChild(item);
        mIsModifying = false;
        endRemoveRows();

        mGateMap.remove(id);
        delete item;
    }

    void ModuleModel::remove_net(const u32 id)
    {
        assert(gNetlist->get_net_by_id(id));
        assert(mModuleMap.contains(id));

        ModuleItem* item   = mNetMap.value(id);
        ModuleItem* parent = item->parent();
        assert(item);
        assert(parent);

        QModelIndex index = getIndex(parent);

        int row = item->row();

        mIsModifying = true;
        beginRemoveRows(index, row, row);
        parent->removeChild(item);
        mIsModifying = false;
        endRemoveRows();

        mNetMap.remove(id);
        delete item;
    }

    void ModuleModel::updateModule(u32 id)    // SPLIT ???
    {
        assert(gNetlist->get_module_by_id(id));
        assert(mModuleMap.contains(id));

        ModuleItem* item = mModuleMap.value(id);
        assert(item);

        item->setName(QString::fromStdString(gNetlist->get_module_by_id(id)->get_name()));    // REMOVE & ADD AGAIN
        item->setColor(gNetlistRelay->getModuleColor(id));

        QModelIndex index = getIndex(item);
        Q_EMIT dataChanged(index, index);
    }

    ModuleItem* ModuleModel::getItem(u32 id, ModuleItem::TreeItemType type) const
    {
        return mModuleItemMaps[(int)type]->value(id);
    }

    QColor ModuleModel::moduleColor(u32 id) const
    {
        return mModuleColors.value(id);
    }

    QColor ModuleModel::setModuleColor(u32 id, const QColor& col)
    {
        QColor retval = mModuleColors.value(id);
        mModuleColors[id] = col;
        return retval;
    }

    QColor ModuleModel::setRandomColor(u32 id)
    {
        QColor retval = mModuleColors.value(id);
        mModuleColors.insert(id,gui_utility::getRandomColor());
        return retval;
    }

    void ModuleModel::removeColor(u32 id)
    {
        mModuleColors.remove(id);
    }

    bool ModuleModel::isModifying()
    {
        return mIsModifying;
    }
}
