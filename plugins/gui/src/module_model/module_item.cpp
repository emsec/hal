#include "gui/module_model/module_item.h"
#include "gui/module_model/module_model.h"

#include "hal_core/netlist/module.h"

#include "gui/gui_globals.h"
#include "gui/gui_utils/graphics.h"

namespace hal
{

ModuleItem::ModuleItem(const u32 id, const TreeItemType type, ModuleModel *model) :
        BaseTreeItem(),
        mId(id),
        mItemType(type),
        mHighlighted(false),
        mModuleModel(model)
    {
        switch(type)
        {
        case TreeItemType::Module:
        {
            const Module* m = gNetlist->get_module_by_id(id);
            Q_ASSERT(m);
            mName       = QString::fromStdString(m->get_name());
            mModuleType = QString::fromStdString(m->get_type());
            break;
        }
        case TreeItemType::Gate:
        {
            const Gate* g = gNetlist->get_gate_by_id(id);
            Q_ASSERT(g);
            mName       = QString::fromStdString(g->get_name());
            mModuleType = QString::fromStdString(g->get_type()->get_name());
            break;
        }
        case TreeItemType::Net:
        {
            const Net* n = gNetlist->get_net_by_id(id);
            Q_ASSERT(n);
            mName = QString::fromStdString(n->get_name());
            break;
        }
        }
        mModuleModel->mModuleItemMaps[(int)mItemType]->insertMulti(id,this);
    }

    ModuleItem::~ModuleItem()
    {
        auto it = mModuleModel->mModuleItemMaps[(int)mItemType]->lowerBound(mId);
        while (it != mModuleModel->mModuleItemMaps[(int)mItemType]->upperBound(mId))
        {
            if (it.value() == this)
                it = mModuleModel->mModuleItemMaps[(int)mItemType]->erase(it);
            else
                ++it;
        }
    }

    void ModuleItem::appendExistingChildIfAny(const QMap<u32,ModuleItem*>& moduleMap)
    {
        if(mItemType != TreeItemType::Module) // only module can have children
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

    void ModuleItem::setModuleType(const QString &moduleType)
    {
        if (mItemType != TreeItemType::Module) return;
        Module* module = gNetlist->get_module_by_id(mId);
        if (!module) return;
        module->set_type(moduleType.toStdString());
        mModuleType = moduleType;
    }

    QVariant ModuleItem::getData(int column) const
    {
        // DEBUG CODE, USE STYLED DELEGATES OR SOMETHING
        switch (column) {
        case 0:
            return mName;
        case 1:
            return mId;
        case 2:
            return mModuleType;
        }
        return QVariant();
    }

    void ModuleItem::setData(QList<QVariant> data)
    {
        setName(data[0].toString());
        if (mItemType == TreeItemType::Module)
            setModuleType(data.at(2).toString());
    }

    void ModuleItem::setDataAtColumn(int column, QVariant &data)
    {
        switch (column) {
        case 0:
            setName(data.toString());
            return;
        case 1:
            return;
        case 2:
            setModuleType(data.toString());
            return;
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

    bool ModuleItem::isToplevelItem() const
    {
        if (dynamic_cast<RootTreeItem*>(mParent)) return true;
        return false;
    }

    ModuleItem::TreeItemType ModuleItem::getType() const{
        return mItemType;
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

    void ModuleItem::appendData(QVariant data)
    {
        Q_UNUSED(data);
    }
}
