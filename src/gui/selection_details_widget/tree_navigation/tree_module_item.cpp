#include "selection_details_widget/tree_navigation/tree_module_item.h"

namespace hal
{
    TreeModuleItem::TreeModuleItem(const QVector<QVariant>& data, TreeModuleItem::item_type type, TreeModuleItem* parent)
    {
        m_parent_item = parent;
        m_data        = data;
        m_type        = type;
    }

    TreeModuleItem::~TreeModuleItem()
    {
        qDeleteAll(m_child_items);
    }

    int TreeModuleItem::get_child_count() const
    {
        return m_child_items.size();
    }

    int TreeModuleItem::get_column_count() const
    {
        return m_data.size();
    }

    int TreeModuleItem::get_row_number() const
    {
        if (m_parent_item)
            return m_parent_item->m_child_items.indexOf(const_cast<TreeModuleItem*>(this));
        return 0;
    }

    TreeModuleItem* TreeModuleItem::get_child(int row)
    {
        return m_child_items.at(row);
    }

    TreeModuleItem* TreeModuleItem::get_parent()
    {
        return m_parent_item;
    }

    QVariant TreeModuleItem::data(int column) const
    {
        return m_data.at(column);
    }

    TreeModuleItem::item_type TreeModuleItem::get_type()
    {
        return m_type;
    }

    bool TreeModuleItem::insert_child(int row_position, TreeModuleItem* item)
    {
        if (row_position < 0 || row_position > m_child_items.size())
            return false;

        m_child_items.insert(row_position, item);
        return true;
    }

    bool TreeModuleItem::set_data(int column, const QVariant& value)
    {
        if (column < 0 || column >= m_data.size())
            return false;

        m_data[column] = value;
        return true;
    }

    bool TreeModuleItem::remove_all_children()
    {
        for(int i = 0; i < m_child_items.size();i++)
            delete m_child_items.takeAt(0);

        return true;
    }

    bool TreeModuleItem::remove_children(int position, int count)
    {
        if (position < 0 || position + count > m_child_items.size())
            return false;

        for (int row = 0; row < count; row++)
            delete m_child_items.takeAt(position);

        return true;
    }

    bool TreeModuleItem::remove_child(TreeModuleItem* item)
    {
        return m_child_items.removeOne(item);
    }
}
