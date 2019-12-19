#include "selection_details_widget/tree_navigation/tree_module_item.h"

tree_module_item::tree_module_item(const QVector<QVariant>& data, tree_module_item::item_type type, tree_module_item* parent)
{
    m_parent_item = parent;
    m_data        = data;
    m_type        = type;
}

tree_module_item::~tree_module_item()
{
    qDeleteAll(m_child_items);
}

int tree_module_item::get_child_count() const
{
    return m_child_items.size();
}

int tree_module_item::get_column_count() const
{
    return m_data.size();
}

int tree_module_item::get_row_number() const
{
    if (m_parent_item)
        return m_parent_item->m_child_items.indexOf(const_cast<tree_module_item*>(this));
    return 0;
}

tree_module_item* tree_module_item::get_child(int row)
{
    return m_child_items.at(row);
}

tree_module_item* tree_module_item::get_parent()
{
    return m_parent_item;
}

QVariant tree_module_item::data(int column) const
{
    return m_data.at(column);
}

tree_module_item::item_type tree_module_item::get_type()
{
    return m_type;
}

bool tree_module_item::insert_child(int row_position, tree_module_item* item)
{
    if (row_position < 0 || row_position > m_child_items.size())
        return false;

    m_child_items.insert(row_position, item);
    return true;
}

bool tree_module_item::set_data(int column, const QVariant& value)
{
    if (column < 0 || column >= m_data.size())
        return false;

    m_data[column] = value;
    return true;
}

bool tree_module_item::remove_all_children()
{
    for(int i = 0; i < m_child_items.size();i++)
        delete m_child_items.takeAt(0);

    return true;
}

bool tree_module_item::remove_children(int position, int count)
{
    if (position < 0 || position + count > m_child_items.size())
        return false;

    for (int row = 0; row < count; row++)
        delete m_child_items.takeAt(position);

    return true;
}

bool tree_module_item::remove_child(tree_module_item* item)
{
    return m_child_items.removeOne(item);
}
