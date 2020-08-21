#include "module_model/module_item.h"

#include "netlist/module.h"

#include "gui/gui_globals.h"
#include "gui/gui_utils/graphics.h"

namespace hal
{
    ModuleItem::ModuleItem(const u32 id) :
        m_parent(nullptr),
        m_id(id),
        m_name(QString::fromStdString(g_netlist->get_module_by_id(id)->get_name())),
        m_color(g_netlist_relay.get_module_color(id)),
        m_highlighted(false)
    {

    }

    ModuleItem::ModuleItem(const QString& name, const u32 id) :
        m_parent(nullptr),
        m_id(id),
        m_name(name),
        m_color(g_netlist_relay.get_module_color(id)),
        m_highlighted(false)
    {
    }

    void ModuleItem::insert_child(int row, ModuleItem* child)
    {
        m_child_items.insert(row, child);
    }

    void ModuleItem::remove_child(ModuleItem* child)
    {
        m_child_items.removeOne(child);
    }

    void ModuleItem::append_child(ModuleItem* child)
    {
        // PROBABLY OBSOLETE
        m_child_items.append(child);
    }

    void ModuleItem::prepend_child(ModuleItem* child)
    {
        // PROBABLY OBSOLETE
        m_child_items.prepend(child);
    }

    ModuleItem* ModuleItem::parent()
    {
        return m_parent;
    }

    ModuleItem* ModuleItem::child(int row)
    {
        return m_child_items.value(row);
    }

    const ModuleItem* ModuleItem::const_parent() const
    {
        return m_parent;
    }

    const ModuleItem* ModuleItem::const_child(int row) const
    {
        return m_child_items.value(row);
    }

    int ModuleItem::childCount() const
    {
        return m_child_items.count();
    }

    int ModuleItem::row() const
    {
        if (m_parent)
            return m_parent->m_child_items.indexOf(const_cast<ModuleItem*>(this));
        else
            return 0;
    }

    QVariant ModuleItem::data(int column) const
    {
        // DEBUG CODE, USE STYLED DELEGATES OR SOMETHING
        if (column != 0)
            return QVariant();
        return m_name;
    }

    QString ModuleItem::name() const
    {
        return m_name;
    }

    u32 ModuleItem::id() const
    {
        return m_id;
    }

    QColor ModuleItem::color() const
    {
        return m_color;
    }

    bool ModuleItem::highlighted() const
    {
        return m_highlighted;
    }

    void ModuleItem::set_parent(ModuleItem* parent)
    {
        m_parent = parent;
    }

    void ModuleItem::set_name(const QString& name)
    {
        m_name = name;
    }

    void ModuleItem::set_color(const QColor& color)
    {
        m_color = color;
    }

    void ModuleItem::set_highlighted(const bool highlighted)
    {
        m_highlighted = highlighted;
    }
}
