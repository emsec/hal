#include "module_model/module_item.h"

#include "netlist/module.h"

#include "gui/gui_globals.h"
#include "gui/gui_utils/graphics.h"

module_item::module_item(const u32 id) :
    m_parent(nullptr),
    m_id(id),
    m_name(QString::fromStdString(g_netlist->get_module_by_id(id)->get_name())),
    m_color(g_netlist_relay.get_module_color(id)),
    m_highlighted(false)
{

}

module_item::module_item(const QString& name, const u32 id) :
    m_parent(nullptr),
    m_name(name),
    m_id(id),
    m_color(g_netlist_relay.get_module_color(id)),
    m_highlighted(false)
{
}

void module_item::insert_child(int row, module_item* child)
{   
    m_child_items.insert(row, child);
}

void module_item::remove_child(module_item* child)
{
    m_child_items.removeOne(child);
}

void module_item::append_child(module_item* child)
{
    // PROBABLY OBSOLETE
    m_child_items.append(child);
}

void module_item::prepend_child(module_item* child)
{
    // PROBABLY OBSOLETE
    m_child_items.prepend(child);
}

module_item* module_item::parent()
{
    return m_parent;
}

module_item* module_item::child(int row)
{
    return m_child_items.value(row);
}

const module_item* module_item::const_parent() const
{
    return m_parent;
}

const module_item* module_item::const_child(int row) const
{
    return m_child_items.value(row);
}

int module_item::childCount() const
{
    return m_child_items.count();
}

int module_item::row() const
{
    if (m_parent)
        return m_parent->m_child_items.indexOf(const_cast<module_item*>(this));
    else
        return 0;
}

QVariant module_item::data(int column) const
{
    // DEBUG CODE, USE STYLED DELEGATES OR SOMETHING
    if (column != 0)
        return QVariant();
    return m_name;
}

QString module_item::name() const
{
    return m_name;
}

u32 module_item::id() const
{
    return m_id;
}

QColor module_item::color() const
{
    return m_color;
}

bool module_item::highlighted() const
{
    return m_highlighted;
}

void module_item::set_parent(module_item* parent)
{
    m_parent = parent;
}

void module_item::set_name(const QString& name)
{
    m_name = name;
}

void module_item::set_color(const QColor& color)
{
    m_color = color;
}

void module_item::set_highlighted(const bool highlighted)
{
    m_highlighted = highlighted;
}
