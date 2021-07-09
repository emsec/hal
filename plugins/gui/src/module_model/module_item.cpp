#include "gui/module_model/module_item.h"

#include "hal_core/netlist/module.h"

#include "gui/gui_globals.h"
#include "gui/gui_utils/graphics.h"

namespace hal
{
    ModuleItem::ModuleItem(const u32 id) :
        mParent(nullptr),
        mId(id),
        mName(QString::fromStdString(gNetlist->get_module_by_id(id)->get_name())),
        mColor(gNetlistRelay->getModuleColor(id)),
        mHighlighted(false)
    {;}

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

    QVariant ModuleItem::data(int column) const
    {
        // DEBUG CODE, USE STYLED DELEGATES OR SOMETHING
        if (column != 0)
            return QVariant();
        return mName;
    }

    QString ModuleItem::name() const
    {
        return mName;
    }

    u32 ModuleItem::id() const
    {
        return mId;
    }

    QColor ModuleItem::color() const
    {
        return mColor;
    }

    bool ModuleItem::highlighted() const
    {
        return mHighlighted;
    }

    void ModuleItem::setParent(ModuleItem* parent)
    {
        mParent = parent;
    }

    void ModuleItem::set_name(const QString& name)
    {
        mName = name;
    }

    void ModuleItem::setColor(const QColor& color)
    {
        mColor = color;
    }

    void ModuleItem::setHighlighted(const bool highlighted)
    {
        mHighlighted = highlighted;
    }
}
