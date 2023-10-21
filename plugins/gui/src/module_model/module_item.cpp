#include "gui/module_model/module_item.h"

#include "hal_core/netlist/module.h"

#include "gui/gui_globals.h"
#include "gui/gui_utils/graphics.h"

namespace hal
{

ModuleItem::ModuleItem(const u32 id, const TreeItemType type) :
        mParent(nullptr),
        mId(id),
        mType(type),
        mHighlighted(false)
    {
        switch(type)
        {
        case TreeItemType::Module:
            mName = QString::fromStdString(gNetlist->get_module_by_id(id)->get_name());
            break;
        case TreeItemType::Gate:
            mName = QString::fromStdString(gNetlist->get_gate_by_id(id)->get_name());
            break;
        case TreeItemType::Net:
            mName = QString::fromStdString(gNetlist->get_net_by_id(id)->get_name());
            break;
        }
    }

    void ModuleItem::insertChild(int row, ModuleItem* child)
    {
        mChildItems.insert(row, child);
    }

    void ModuleItem::removeChild(ModuleItem* child)
    {
        mChildItems.removeOne(child);
    }

    void ModuleItem::appendChild(ModuleItem* child)
    {
        mChildItems.append(child);
    }

    void ModuleItem::appendExistingChildIfAny(const QMap<u32,ModuleItem*>& moduleMap)
    {
        if(mType != TreeItemType::Module) // only module can have children
            return;

        Module* m = gNetlist->get_module_by_id(mId);
        Q_ASSERT(m);
        for (Module* subm : m->get_submodules())
        {
            auto it = moduleMap.find(subm->get_id());
            if (it != moduleMap.constEnd())
            {
                ModuleItem* childItem = it.value();
                appendChild(childItem);
                childItem->setParent(this);
            }
        }
    }

    void ModuleItem::prependChild(ModuleItem* child)
    {
        // PROBABLY OBSOLETE
        mChildItems.prepend(child);
    }

    ModuleItem* ModuleItem::parent()
    {
        return mParent;
    }

    ModuleItem* ModuleItem::child(int row)
    {
        return mChildItems.value(row);
    }

    const ModuleItem* ModuleItem::constParent() const
    {
        return mParent;
    }

    const ModuleItem* ModuleItem::constChild(int row) const
    {
        return mChildItems.value(row);
    }

    int ModuleItem::childCount() const
    {
        return mChildItems.count();
    }

    int ModuleItem::row() const
    {
        if (mParent)
            return mParent->mChildItems.indexOf(const_cast<ModuleItem*>(this));
        else
            return 0;
    }

    QVariant ModuleItem::getData(int column) const
    {
        // DEBUG CODE, USE STYLED DELEGATES OR SOMETHING
        if(column == 0)
            return mName;
        else if (column == 1)
            return mId;
        else if(column == 2)
        {
            switch(mType)
            {
                case TreeItemType::Module:
                {
                    Module* module = gNetlist->get_module_by_id(mId);
                    if(!module) 
                        return QVariant();
                    return QString::fromStdString(module->get_type());
                }
                case TreeItemType::Gate:
                    Gate* gate = gNetlist->get_gate_by_id(mId);
                    if(!gate) 
                        return QVariant();
                    return QString::fromStdString(gate->get_type()->get_name());
            }
        }
        return QVariant();
    }

    void ModuleItem::setData(QList<QVariant> data)
    {
        setName(data[0].toString());
        switch(mType)
        {
            case TreeItemType::Module:
            {
                Module* module = gNetlist->get_module_by_id(mId);
                if(!module)
                    return;
                module->set_type(data[3].toString().toStdString());
            }
            case TreeItemType::Gate:
                return;
        }
    }

    void ModuleItem::setDataAtIndex(int index, QVariant &data)
    {
        if(index == 0) {
            setName(data.toString());
            return;
        }
        else if (index == 1)
            return;
        else if(index == 2)
        {
            switch(mType)
            {
                case TreeItemType::Module:
                {
                    Module* module = gNetlist->get_module_by_id(mId);
                    if(!module)
                        return;
                    module->set_type(data.toString().toStdString());
                }
                case TreeItemType::Gate:
                    return;
            }
        }
    }

    QString ModuleItem::name() const
    {
        return mName;
    }

    u32 ModuleItem::id() const
    {
        return mId;
    }

    bool ModuleItem::highlighted() const
    {
        return mHighlighted;
    }

    ModuleItem::TreeItemType ModuleItem::getType() const{
        return mType;
    }

    void ModuleItem::setParent(ModuleItem* parent)
    {
        mParent = parent;
    }

    void ModuleItem::setName(const QString& name)
    {
        mName = name;
    }

    void ModuleItem::setHighlighted(const bool highlighted)
    {
        mHighlighted = highlighted;
    }

    int ModuleItem::getColumnCount() const
    {
        return 3;
    }

    void ModuleItem::appendData(QVariant data) {}
}
