#include "selection_details_widget/tree_navigation/selection_tree_model.h"
#include "selection_details_widget/tree_navigation/selection_tree_item.h"
#include "gui_globals.h"
#include "netlist/gate.h"
#include "netlist/net.h"
#include "netlist/module.h"
#include "gui_utils/graphics.h"
#include <QTimer>

namespace hal
{
    SelectionTreeModel::SelectionTreeModel(QObject* parent)
        : QAbstractItemModel(parent), mDoNotDisturb(0)
    {
        mRootItem = new SelectionTreeItemModule(SelectionTreeItem::RootItem);  // root item has no parent
    }

    SelectionTreeModel::~SelectionTreeModel()
    {
        delete mRootItem;
    }

    bool SelectionTreeModel::doNotDisturb(const QModelIndex& inx) const
    {
        if (inx.isValid())
        {
            intptr_t ptr = reinterpret_cast<intptr_t>(inx.internalPointer());
            if (ptr && ptr < 0x100) {
                return false;
            }
        }
        return (mDoNotDisturb != 0);
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

    QModelIndex SelectionTreeModel::index(int row, int column, const QModelIndex& parent) const
    {
        if (doNotDisturb(parent)) return QModelIndex();

        if (!hasIndex(row,column,parent)) return QModelIndex();

        SelectionTreeItem* parentItem = parent.isValid()
                ? itemFromIndex(parent)
                : mRootItem;

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
        if (parentItem == mRootItem) return QModelIndex();

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
                : mRootItem;
        return item->childCount();
    }

    int SelectionTreeModel::columnCount(const QModelIndex& parent) const
    {
        Q_UNUSED(parent)
        return MAX_COLUMN;
    }

    void SelectionTreeModel::fetchSelection(bool hasEntries)
    {
        SelectionTreeItemModule* nextRootItem
                = new SelectionTreeItemModule(SelectionTreeItem::RootItem);

        if (hasEntries)
        {
            for(u32 id : g_selection_relay.m_selected_modules)
            {
                SelectionTreeItemModule* stim = new SelectionTreeItemModule(id);
                moduleRecursion(stim);
                nextRootItem->addChild(stim);
            }

            for(u32 id : g_selection_relay.m_selected_gates)
                nextRootItem->addChild(new SelectionTreeItemGate(id));

            for(u32 id : g_selection_relay.m_selected_nets)
                nextRootItem->addChild(new SelectionTreeItemNet(id));
        }

        Q_EMIT layoutAboutToBeChanged();

        ++mDoNotDisturb;
        SelectionTreeModelDisposer* disposer = new SelectionTreeModelDisposer(mRootItem,this);
        mRootItem = nextRootItem;
        QTimer::singleShot(50,disposer,&SelectionTreeModelDisposer::dispose);
        --mDoNotDisturb;

        Q_EMIT layoutChanged();
    }

    void SelectionTreeModel::moduleRecursion(SelectionTreeItemModule* modItem)
    {
        if (modItem->id() <= 0) return;
        std::shared_ptr<Module> mod = g_netlist->get_module_by_id(modItem->id());
        if (!mod) return;
        for (std::shared_ptr<Module> m : mod->get_submodules() )
        {
            SelectionTreeItemModule* subItem = new SelectionTreeItemModule(m->get_id());
            moduleRecursion(subItem);
            modItem->addChild(subItem);
        }
        for (std::shared_ptr<Gate> g : mod->get_gates() )
        {
            modItem->addChild(new SelectionTreeItemGate(g->get_id()));
        }
        for (std::shared_ptr<Net> n : mod->get_internal_nets() )
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
            return createIndex(0,0,mRootItem);

        int n = parentItem->childCount();
        for (int irow=0; irow<n; irow++)
        {
            if (parentItem->child(irow) == item)
                return createIndex(irow,0,item);
        }

        // not found in parent
        return QModelIndex();
    }



    SelectionTreeModelDisposer::SelectionTreeModelDisposer(SelectionTreeItemModule* stim, QObject* parent)
        : QObject(parent), mRootItem(stim)
    {;}

    void SelectionTreeModelDisposer::dispose()
    {
        delete mRootItem;
        deleteLater();
    }

}
