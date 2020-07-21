#include "gui_globals.h"
#include "selection_details_widget/tree_navigation/selection_tree_item.h"

namespace hal
{
    SelectionTreeItem::SelectionTreeItem(SelectionTreeItem::itemType_t t, u32 id_)
        : mItemType(t), mId(id_), mParent(0)
    {;}

    SelectionTreeItem::~SelectionTreeItem()
    {;}

    SelectionTreeItem::itemType_t SelectionTreeItem::itemType() const
    {
        return mItemType;
    }

    u32 SelectionTreeItem::id() const
    {
        return mId;
    }

    SelectionTreeItem* SelectionTreeItem::parent() const
    {
        return mParent;
    }

    void SelectionTreeItem::setParent(SelectionTreeItem* p)
    {
        mParent = p;
    }

    int SelectionTreeItem::childCount() const
    {
        return 0;
    }

    SelectionTreeItem* SelectionTreeItem::child(int row) const
    {
        Q_UNUSED(row);
        return nullptr;
    }

    QVariant SelectionTreeItem::data(int column) const
    {
        switch (column) {
        case 0: return name();
        case 1: return mId;
        case 2: return gateType();
        }
        return QVariant();
    }

    QVariant SelectionTreeItem::gateType() const
    {
        return QVariant();
    }

    //------- Module ----
    SelectionTreeItemModule::SelectionTreeItemModule(u32 id_)
        : SelectionTreeItem(SelectionTreeItem::ModuleItem, id_)
    {;}

    SelectionTreeItemModule::~SelectionTreeItemModule()
    {
        for (SelectionTreeItem* sti : mChildItem)
            delete sti;
    }

    int SelectionTreeItemModule::childCount() const
    {
        return mChildItem.size();
    }


    SelectionTreeItem* SelectionTreeItemModule::child(int row) const
    {
        if (row<0 || row >= mChildItem.size()) return nullptr;
        return mChildItem.at(row);
    }

    QVariant SelectionTreeItemModule::name() const
    {
        const std::shared_ptr<Module>& module = g_netlist->get_module_by_id(mId);
        if(!module) return QVariant();
        return QString::fromStdString(module->get_name());
    }

    QIcon SelectionTreeItemModule::icon() const
    {
        return QIcon(":/icons/sel_module");
    }

    void SelectionTreeItemModule::addChild(SelectionTreeItem* cld)
    {
        cld->setParent(this);
        mChildItem.append(cld);
    }

    //------- Gate ------
    SelectionTreeItemGate::SelectionTreeItemGate(u32 id_)
        : SelectionTreeItem(SelectionTreeItem::GateItem, id_)
    {;}

    QVariant SelectionTreeItemGate::name() const
    {
        const std::shared_ptr<Gate>& gate = g_netlist->get_gate_by_id(mId);
        if(!gate) return QVariant();
        return QString::fromStdString(gate->get_name());
    }

    QIcon SelectionTreeItemGate::icon() const
    {
        return QIcon(":/icons/sel_gate");
    }

    QVariant SelectionTreeItemGate::gateType() const
    {
        const std::shared_ptr<Gate>& gate = g_netlist->get_gate_by_id(mId);
        if(!gate) return QVariant();
        return QString::fromStdString(gate->get_type()->get_name());
    }

    //------- Net -------
    SelectionTreeItemNet::SelectionTreeItemNet(u32 id_)
        : SelectionTreeItem(SelectionTreeItem::NetItem, id_)
    {;}

    QVariant SelectionTreeItemNet::name() const
    {
        const std::shared_ptr<Net>& net = g_netlist->get_net_by_id(mId);
        if(!net) return QVariant();
        return QString::fromStdString(net->get_name());
    }

    QIcon SelectionTreeItemNet::icon() const
    {
        return QIcon(":/icons/sel_net");
    }
}
