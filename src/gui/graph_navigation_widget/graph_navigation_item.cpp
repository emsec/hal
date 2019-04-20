#include "graph_navigation_widget/graph_navigation_item.h"

graph_navigation_item::graph_navigation_item(QString name, int id, item_type type, graph_navigation_item* parent_item) : m_parent_item(parent_item), m_name(name), m_id(id), m_type(type)
{
    if (parent_item)
        parent_item->append_child(this);
}

graph_navigation_item::~graph_navigation_item()
{
    //chain reaction to delete all items, the root item is destroyed in the model's destructor
    for (graph_navigation_item* item : m_child_items)
        delete item;

    if (m_parent_item)
        m_parent_item->remove_child(this);
}

void graph_navigation_item::append_child(graph_navigation_item* child)
{
    m_child_items.append(child);
}

void graph_navigation_item::prepend_child(graph_navigation_item* child)
{
    m_child_items.prepend(child);
}

void graph_navigation_item::insert_child(int row, graph_navigation_item* child)
{
    m_child_items.insert(row, child);
}

void graph_navigation_item::remove_child(graph_navigation_item* child)
{
    m_child_items.removeOne(child);
}

graph_navigation_item* graph_navigation_item::child(int row)
{
    return m_child_items.value(row);
}

int graph_navigation_item::childCount() const
{
    return m_child_items.count();
}

int graph_navigation_item::row() const
{
    if (m_parent_item)
        return m_parent_item->m_child_items.indexOf(const_cast<graph_navigation_item*>(this));
    else
        return 0;
}

QVariant graph_navigation_item::data(int column) const
{
    if (column != 0)
        return QVariant();
    return m_name;    // debug code, will later be handeled with styled delegates
}

graph_navigation_item* graph_navigation_item::parent_item()
{
    return m_parent_item;
}

QString graph_navigation_item::name()
{
    return m_name;
}

u32 graph_navigation_item::id()
{
    return m_id;
}

graph_navigation_item::item_type graph_navigation_item::type()
{
    return m_type;
}
