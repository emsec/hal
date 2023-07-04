#include "gui/gui_globals.h"
#include "gui/selection_details_widget/tree_navigation/selection_tree_item.h"
#include "gui/selection_details_widget/selection_details_icon_provider.h"

namespace hal
{
    SelectionTreeItem::SelectionTreeItem(SelectionTreeItem::TreeItemType t, u32 id_)
        : mItemType(t), mId(id_), mParent(0)
    {;}

    SelectionTreeItem::~SelectionTreeItem()
    {;}

    SelectionTreeItem::TreeItemType SelectionTreeItem::itemType() const
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
        case 2: return boxType();
        }
        return QVariant();
    }

    QVariant SelectionTreeItem::boxType() const
    {
        return QVariant();
    }

    bool SelectionTreeItem::match(const QRegularExpression& regex) const
    {
        if (!regex.isValid()) return true;
        return  regex.match(name().toString()).hasMatch() ||
                regex.match(QString::number(mId)).hasMatch() ||
                regex.match(boxType().toString()).hasMatch();
    }

    bool SelectionTreeItem::isEqual(const SelectionTreeItem* sti) const
    {
        return mItemType == sti->mItemType && mId == sti->mId;
    }

    //------- Module ----
    SelectionTreeItemModule::SelectionTreeItemModule(u32 id_)
        : SelectionTreeItem(SelectionTreeItem::ModuleItem, id_), mIsRoot(false)
    {;}

    SelectionTreeItemModule::~SelectionTreeItemModule()
    {
        for (SelectionTreeItem* sti : mChildItem)
            delete sti;
    }

    SelectionTreeItemRoot::SelectionTreeItemRoot()
        : SelectionTreeItemModule(0)
    {
        mIsRoot = true;
    }

    bool SelectionTreeItemModule::isRoot() const
    {
        return mIsRoot;
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
        Module* module = gNetlist->get_module_by_id(mId);
        if(!module) return QVariant();
        return QString::fromStdString(module->get_name());
    }

    QIcon SelectionTreeItemModule::icon() const
    {
        return QIcon(*SelectionDetailsIconProvider::instance()->getIcon(SelectionDetailsIconProvider::ModuleIcon,mId));
    }

    void SelectionTreeItemModule::addChild(SelectionTreeItem* cld)
    {
        cld->setParent(this);
        mChildItem.append(cld);
    }

    QVariant SelectionTreeItemModule::boxType() const
    {
        Module* module = gNetlist->get_module_by_id(mId);
        if(!module) return QVariant();
        return QString::fromStdString(module->get_type());
    }

    bool SelectionTreeItemModule::match(const QRegularExpression& regex) const
    {
        for (SelectionTreeItem* sti : mChildItem)
            if (sti->match(regex)) return true;

        return SelectionTreeItem::match(regex);
    }

    void SelectionTreeItemModule::suppressedByFilterRecursion(QList<u32>& modIds, QList<u32>& gatIds, QList<u32>& netIds,
                                                           const QRegularExpression& regex) const
    {
        if (!isRoot() && !match(regex)) modIds.append(mId);
        for (SelectionTreeItem* sti : mChildItem)
            sti->suppressedByFilterRecursion(modIds, gatIds, netIds, regex);
    }

    //------- Gate ------
    SelectionTreeItemGate::SelectionTreeItemGate(u32 id_)
        : SelectionTreeItem(SelectionTreeItem::GateItem, id_)
    {;}

    QVariant SelectionTreeItemGate::name() const
    {
        Gate* gate = gNetlist->get_gate_by_id(mId);
        if(!gate) return QVariant();
        return QString::fromStdString(gate->get_name());
    }

    QIcon SelectionTreeItemGate::icon() const
    {
        return QIcon(*SelectionDetailsIconProvider::instance()->getIcon(SelectionDetailsIconProvider::GateIcon,mId));
    }

    QVariant SelectionTreeItemGate::boxType() const
    {
        Gate* gate = gNetlist->get_gate_by_id(mId);
        if(!gate) return QVariant();
        return QString::fromStdString(gate->get_type()->get_name());
    }

    void SelectionTreeItemGate::suppressedByFilterRecursion(QList<u32>& modIds, QList<u32>& gatIds, QList<u32>& netIds,
                                                           const QRegularExpression& regex) const
    {
        Q_UNUSED(modIds)
        Q_UNUSED(netIds)
        if (!match(regex)) gatIds.append(mId);
    }

    //------- Net -------
    SelectionTreeItemNet::SelectionTreeItemNet(u32 id_)
        : SelectionTreeItem(SelectionTreeItem::NetItem, id_)
    {;}

    QVariant SelectionTreeItemNet::name() const
    {
        Net* net = gNetlist->get_net_by_id(mId);
        if(!net) return QVariant();
        return QString::fromStdString(net->get_name());
    }

    QIcon SelectionTreeItemNet::icon() const
    {
        return QIcon(*SelectionDetailsIconProvider::instance()->getIcon(SelectionDetailsIconProvider::NetIcon,mId));
    }

    void SelectionTreeItemNet::suppressedByFilterRecursion(QList<u32>& modIds, QList<u32>& gatIds, QList<u32>& netIds,
                                                           const QRegularExpression& regex) const
    {
        Q_UNUSED(modIds)
        Q_UNUSED(gatIds)
        if (!match(regex)) netIds.append(mId);
    }

}
