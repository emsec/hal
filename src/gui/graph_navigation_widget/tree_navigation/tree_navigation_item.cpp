#include "graph_navigation_widget/tree_navigation/tree_navigation_item.h"

tree_navigation_item::tree_navigation_item(const QVector<QVariant>& data, tree_navigation_item::item_type type, tree_navigation_item* parent)
{
    m_parent_item = parent;
    m_data        = data;
    m_type        = type;
}

tree_navigation_item::~tree_navigation_item()
{
    qDeleteAll(m_child_items);
}

int tree_navigation_item::get_child_count() const
{
    return m_child_items.size();
}

int tree_navigation_item::get_column_count() const
{
    return m_data.size();
}

int tree_navigation_item::get_row_number() const
{
    if (m_parent_item)
        return m_parent_item->m_child_items.indexOf(const_cast<tree_navigation_item*>(this));
    return 0;
}

tree_navigation_item* tree_navigation_item::get_child(int row)
{
    return m_child_items.at(row);
}

tree_navigation_item* tree_navigation_item::get_parent()
{
    return m_parent_item;
}

QVariant tree_navigation_item::data(int column) const
{
    return m_data.at(column);
}

tree_navigation_item::item_type tree_navigation_item::get_type()
{
    return m_type;
}

bool tree_navigation_item::insert_child(int row_position, tree_navigation_item* item)
{
    if (row_position < 0 || row_position > m_child_items.size())
        return false;

    m_child_items.insert(row_position, item);
    return true;
}

bool tree_navigation_item::set_data(int column, const QVariant& value)
{
    if (column < 0 || column >= m_data.size())
        return false;

    m_data[column] = value;
    return true;
}

bool tree_navigation_item::remove_children(int position, int count)
{
    if (position < 0 || position + count > m_child_items.size())
        return false;

    for (int row = 0; row < count; row++)
        delete m_child_items.takeAt(position);

    return true;
}

bool tree_navigation_item::remove_child(tree_navigation_item* item)
{
    return m_child_items.removeOne(item);
}
