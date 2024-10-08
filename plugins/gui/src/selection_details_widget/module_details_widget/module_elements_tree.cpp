#include "gui/selection_details_widget/module_details_widget/module_elements_tree.h"
#include "gui/graph_tab_widget/graph_tab_widget.h"
#include "gui/gui_globals.h"
#include "gui/module_context_menu/module_context_menu.h"
#include "gui/plugin_relay/gui_plugin_manager.h"

#include <QMenu>
#include <QHeaderView>
#include <QApplication>
#include <QClipboard>

namespace hal
{

    ModuleElementsTree::ModuleElementsTree(QWidget *parent) : QTreeView(parent), mModuleID(-1)
    {
        setContextMenuPolicy(Qt::CustomContextMenu);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setSelectionMode(QAbstractItemView::SingleSelection);
        setSelectionBehavior(QAbstractItemView::SelectRows);
        setFocusPolicy(Qt::NoFocus);
        header()->setStretchLastSection(true);
        mProxyModel = new FilterElementsProxyModel(this);
        mModel = new ModuleModel(mProxyModel);
        mProxyModel->setSourceModel(mModel);
        mProxyModel->setFilterNets(true);
        setModel(mProxyModel);

        //connections
        connect(this, &QTreeView::customContextMenuRequested, this, &ModuleElementsTree::handleContextMenuRequested);
        //connect(mModel, &ModuleTreeModel::numberOfSubmodulesChanged, this, &ModuleElementsTree::handleNumberSubmodulesChanged);
    }

    void ModuleElementsTree::setModule(u32 moduleID)
    {
        Module* m = gNetlist->get_module_by_id(moduleID);
        if(!m) return;

        setModule(m);
    }

    void ModuleElementsTree::setModule(Module *m)
    {
        if(!m) return;

        mModel->populateTree({m->get_id()});
        setRootIndex(mProxyModel->mapFromSource(mModel->getIndexFromItem(mModel->getItem(m->get_id())))); // hide top-element m in TreeView
        
        mModuleID = m->get_id();
    }

    void ModuleElementsTree::removeContent()
    {
        mModel->clear();
        mModuleID = -1;
    }

    void ModuleElementsTree::handleContextMenuRequested(const QPoint &pos)
    {
        QModelIndex clickedIndex = indexAt(pos);
        if(!clickedIndex.isValid())
            return;

        ModuleItem* clickedItem = dynamic_cast<ModuleItem*>(mModel->getItemFromIndex(mProxyModel->mapToSource(clickedIndex)));
        u32 id = clickedItem->id();
        ModuleItem::TreeItemType type = clickedItem->getType();
        QMenu menu;

        if(type == ModuleItem::TreeItemType::Module)
            ModuleContextMenu::addModuleSubmenu(&menu, id);
        else if(type == ModuleItem::TreeItemType::Gate)
            ModuleContextMenu::addGateSubmenu(&menu, id);
        else if(type == ModuleItem::TreeItemType::Net)
            ModuleContextMenu::addNetSubmenu(&menu, id);

        GuiPluginManager::addPluginSubmenus(&menu, gNetlist, 
            type==ModuleItem::TreeItemType::Module ? std::vector<u32>({id}) : std::vector<u32>(),
            type==ModuleItem::TreeItemType::Gate ? std::vector<u32>({id}) : std::vector<u32>(),
            type==ModuleItem::TreeItemType::Net ? std::vector<u32>({id}) : std::vector<u32>());

        menu.exec(this->viewport()->mapToGlobal(pos));
    }

    void ModuleElementsTree::handleNumberSubmodulesChanged(const int number)
    {
        Q_EMIT updateText(QString("Submodules(%1)").arg(number));
    }
}
