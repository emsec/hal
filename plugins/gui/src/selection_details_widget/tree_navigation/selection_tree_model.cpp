#include "gui/selection_details_widget/tree_navigation/selection_tree_model.h"
#include "gui/selection_details_widget/tree_navigation/selection_tree_item.h"
#include "gui/gui_globals.h"
#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/module.h"
#include "gui/gui_utils/graphics.h"
#include <QTimer>

namespace hal
{
    SelectionTreeModel::SelectionTreeModel(QObject* parent)
        : QAbstractItemModel(parent), m_doNotDisturb(0)
    {
        m_rootItem = new SelectionTreeItemRoot();
        // root item has no parent
    }

    SelectionTreeModel::~SelectionTreeModel()
    {
        delete m_rootItem;
    }

    bool SelectionTreeModel::doNotDisturb(const QModelIndex& inx) const
    {
        Q_UNUSED(inx); // could do some tests for debugging
        return (m_doNotDisturb != 0);
    }

    QVariant SelectionTreeModel::data(const QModelIndex& index, int role) const
    {
        if (doNotDisturb(index))
             return QVariant();

        // UserRole is mapped to "is a structure element?"
    //        if (role == Qt::UserRole)
     //       return get_item(index)->get_type() == SelectionTreeItem::item_type::structure;

     //   if (get_item(index)->get_type() == SelectionTreeItem::item_type::structure && index.column() == 0)
     //   {
     //       if (role == Qt::FontRole)
     //           return m_structured_font;

    //        if(get_item(index) == m_gates_item && role == Qt::DecorationRole)
    //            return m_design_icon; }
        SelectionTreeItem* item = itemFromIndex(index);
        if (!item) return QVariant();

        switch (role) {
            case Qt::DecorationRole:
                return index.column() == NAME_COLUMN
                        ? QVariant(item->icon())
                        : QVariant();
            case Qt::DisplayRole:
                return item->data(index.column());
            case Qt::TextAlignmentRole:
                return index.column() == ID_COLUMN
                        ? Qt::AlignRight
                        : Qt::AlignLeft;
            default:
                break;
        }

        return QVariant();

    }

    QVariant SelectionTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        const char* horizontalHeader[] = { "Name", "ID", "Type"};
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section < columnCount())
            return QString(horizontalHeader[section]);

        return QVariant();
    }

    QModelIndex SelectionTreeModel::defaultIndex() const
    {
        if (doNotDisturb()) return QModelIndex();

        if (!m_rootItem->childCount()) return QModelIndex();
        SelectionTreeItem* sti = m_rootItem->child(0);
        return createIndex(0,0,sti);
    }

    QModelIndex SelectionTreeModel::index(int row, int column, const QModelIndex& parent) const
    {
        if (doNotDisturb(parent)) return QModelIndex();

        if (!hasIndex(row,column,parent)) return QModelIndex();

        SelectionTreeItem* parentItem = parent.isValid()
                ? itemFromIndex(parent)
                : m_rootItem;

        SelectionTreeItem* childItem  = parentItem->child(row);
        if (childItem)
            return createIndex(row, column, childItem);
        else
            return QModelIndex();
    }

    QModelIndex SelectionTreeModel::parent(const QModelIndex& index) const
    {
        if (doNotDisturb(index)) return QModelIndex();

        if (!index.isValid()) return QModelIndex();

        SelectionTreeItem* currentItem  = itemFromIndex(index);
        if (!currentItem) return QModelIndex();

        SelectionTreeItem* parentItem   = currentItem->parent();

        // toplevel entries dont reveal their parent
        if (parentItem == m_rootItem) return QModelIndex();

        return indexFromItem(parentItem);
    }

    Qt::ItemFlags SelectionTreeModel::flags(const QModelIndex& index) const
    {
        return QAbstractItemModel::flags(index);
    }

    int SelectionTreeModel::rowCount(const QModelIndex& parent) const
    {
        if (doNotDisturb(parent)) return 0;

        SelectionTreeItem* item = parent.isValid()
                ? itemFromIndex(parent)
                : m_rootItem;
        return item->childCount();
    }

    int SelectionTreeModel::columnCount(const QModelIndex& parent) const
    {
        Q_UNUSED(parent)
        return MAX_COLUMN;
    }

    void SelectionTreeModel::fetchSelection(bool hasEntries)
    {
        SelectionTreeItemRoot* nextRootItem
                = new SelectionTreeItemRoot();

        if (hasEntries)
        {
            for(u32 id : g_selection_relay->m_selected_modules)
            {
                SelectionTreeItemModule* stim = new SelectionTreeItemModule(id);
                moduleRecursion(stim);
                nextRootItem->addChild(stim);
            }

            for(u32 id : g_selection_relay->m_selected_gates)
                nextRootItem->addChild(new SelectionTreeItemGate(id));

            for(u32 id : g_selection_relay->m_selected_nets)
                nextRootItem->addChild(new SelectionTreeItemNet(id));
        }

        Q_EMIT layoutAboutToBeChanged();

        ++m_doNotDisturb;
        // delay disposal of old entries
        //    until all clients are notified that indexes are not valid any more
        SelectionTreeModelDisposer* disposer = new SelectionTreeModelDisposer(m_rootItem,this);
        m_rootItem = nextRootItem;
        QTimer::singleShot(50,disposer,&SelectionTreeModelDisposer::dispose);
        --m_doNotDisturb;

        Q_EMIT layoutChanged();
    }

    void SelectionTreeModel::moduleRecursion(SelectionTreeItemModule* modItem)
    {
        if (modItem->isRoot()) return;
        Module* mod = g_netlist->get_module_by_id(modItem->id());
        if (!mod) return;
        for (Module* m : mod->get_submodules() )
        {
            SelectionTreeItemModule* subItem = new SelectionTreeItemModule(m->get_id());
            moduleRecursion(subItem);
            modItem->addChild(subItem);
        }
        for (Gate* g : mod->get_gates() )
        {
            modItem->addChild(new SelectionTreeItemGate(g->get_id()));
        }
        for (Net* n : mod->get_internal_nets() )
        {
            modItem->addChild(new SelectionTreeItemNet(n->get_id()));
        }
    }

    SelectionTreeItem* SelectionTreeModel::itemFromIndex(const QModelIndex& index) const
    {
        if (index.isValid())
            return static_cast<SelectionTreeItem*>(index.internalPointer());
        return nullptr;
    }

    QModelIndex SelectionTreeModel::indexFromItem(SelectionTreeItem* item) const
    {
        if (!item) return QModelIndex();
        SelectionTreeItem* parentItem = item->parent();

        if (!parentItem) // must be root
            return createIndex(0,0,m_rootItem);

        int n = parentItem->childCount();
        for (int irow=0; irow<n; irow++)
        {
            if (parentItem->child(irow) == item)
                return createIndex(irow,0,item);
        }

        // not found in parent
        return QModelIndex();
    }



    SelectionTreeModelDisposer::SelectionTreeModelDisposer(SelectionTreeItemRoot *stim, QObject* parent)
        : QObject(parent), m_rootItem(stim)
    {;}

    void SelectionTreeModelDisposer::dispose()
    {
        delete m_rootItem;
        deleteLater();
    }
}
