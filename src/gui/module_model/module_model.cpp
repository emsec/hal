#include "module_model/module_model.h"

#include "module_model/module_item.h"

#include "gui/gui_globals.h"
#include "gui/gui_utility.h"

#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/module.h"

module_model::module_model(QObject* parent) : QAbstractItemModel(parent)
{
    m_root_item = new module_item("", 0);
}

module_model::~module_model()
{
    delete m_root_item;
}

QModelIndex module_model::index(int row, int column, const QModelIndex& parent) const
{
    // BEHAVIOR FOR ILLEGAL INDICES IS UNDEFINED
    // SEE QT DOCUMENTATION

    // NECESSARY ???
    if (column != 0)
        return QModelIndex();

    // PROBABLY REDUNDANT
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    module_item* parent_item = get_item(parent);
    module_item* child_item = parent_item->child(row);

    if (child_item)
        return createIndex(row, column, child_item);
    else
        return QModelIndex();
}

QModelIndex module_model::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();

    module_item* child_item  = get_item(index);
    module_item* parent_item = child_item->parent();

    if (parent_item == m_root_item)
        return QModelIndex();

    return createIndex(parent_item->row(), 0, parent_item);
}

int module_model::rowCount(const QModelIndex& parent) const
{
//    module_item* parent_item;
//    if (parent.column() > 0)
//        return 0;

    module_item* parent_item = get_item(parent);
    return parent_item->childCount();
}

int module_model::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant module_model::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    module_item* item = static_cast<module_item*>(index.internalPointer());

    if (!item)
        return QVariant();

    switch (role)
    {
    case Qt::DecorationRole:
    {
        if (index.column() == 0)
        {
            QString run_icon_style = "all->" + item->color().name();
            QString run_icon_path  = ":/icons/filled-circle";

            return gui_utility::get_styled_svg_icon(run_icon_style, run_icon_path);
        }
        break;
    }
    case Qt::DisplayRole:
    {
        return item->data(index.column());
    }
    default: return QVariant();
    }

//    if (role == Qt::UserRole && index.column() == 0)
//    {
//        module_item* item = static_cast<module_item*>(index.internalPointer());
//        return QVariant::fromValue(item->id());
//    }

//    if (role == Qt::UserRole + 1 && index.column() == 0)
//    {
//        // PROBABLY OBSOLETE
//        module_item* item = static_cast<module_item*>(index.internalPointer());
//        return QVariant();
//    }

}

Qt::ItemFlags module_model::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return 0;

    return QAbstractItemModel::flags(index);
}

QVariant module_model::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)
    Q_UNUSED(orientation)
    Q_UNUSED(role)
    return QVariant();
}

module_item* module_model::get_item(const QModelIndex& index) const
{
    if (index.isValid())
    {
        module_item* item = static_cast<module_item*>(index.internalPointer());
        if (item)
            return item;
    }

    return m_root_item;
}

QModelIndex module_model::get_index(const module_item* const item) const
{
    QVector<int> row_numbers;
    const module_item* current_item = item;

    while (current_item != m_root_item)
    {
        if (!current_item)
            return QModelIndex(); // SHOULD NEVER BE REACHED

        row_numbers.append(current_item->row());
        current_item = current_item->const_parent();
    }

    QModelIndex model_index = QModelIndex();

    for (QVector<int>::const_reverse_iterator i = row_numbers.crbegin(); i != row_numbers.crend(); ++i)
        model_index = index(*i, 0, model_index);

    return model_index;
}

void module_model::add_item(module_item* item, module_item* parent)
{
    if (!item)
        return; //SHOULD NEVER BE REACHED

    if (!parent)
        parent = m_root_item;

    item->set_parent(parent);

    QModelIndex index = get_index(parent);

    int row = 0;

    while (row < parent->childCount())
    {
        if (item->name() < parent->child(row)->name())
            break;
        else
            ++row;
    }

    beginInsertRows(index, row, row);
    parent->insert_child(row, item);
    endInsertRows();
}

void module_model::remove_item(module_item* item)
{
    if (!item)
        return; // SHOULD NEVER BE REACHED

    int row = item->row();

    module_item* parent_item = item->parent();

    if (!parent_item)
        return; // SHOULD NEVER BE REACHED

    QModelIndex index = get_index(parent_item);

    beginRemoveRows(index, row, row);
    parent_item->remove_child(item);
    endRemoveRows();
}
