#include "gui/selection_details_widget/module_details_widget/module_elements_tree.h"
#include "gui/selection_details_widget/tree_navigation/selection_tree_view.h"
#include "gui/graph_tab_widget/graph_tab_widget.h"
#include "gui/python/py_code_provider.h"
#include "gui/gui_globals.h"
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
        int id = clickedItem->id();
        ModuleItem::TreeItemType type = clickedItem->getType();
        QMenu menu;

        //menu.addSection("here comes the plaintext");

        menu.addAction("Name to clipboard",
           [clickedItem]()
           {
               QApplication::clipboard()->setText(clickedItem->getData(0).toString());
           }
        );

        menu.addAction("ID to clipboard",
           [id]()
           {
               QApplication::clipboard()->setText(QString::number(id));
           }
        );

        menu.addAction("Type to clipboard",
           [clickedItem]()
           {
               QApplication::clipboard()->setText(clickedItem->getData(2).toString());
           }
        );

        menu.addSection("Misc");

        menu.addAction("Set as current selection",
           [this, id, type]()
           {
            gSelectionRelay->clear();
            switch(type)
            {
                case ModuleItem::TreeItemType::Module: gSelectionRelay->addModule(id); break;
                case ModuleItem::TreeItemType::Gate: gSelectionRelay->addGate(id); break;
            }
            gSelectionRelay->relaySelectionChanged(this);
           }
        );

        menu.addAction("Add to current selection",
           [this, id, type]()
           {
            switch(type)
            {
                case ModuleItem::TreeItemType::Module: gSelectionRelay->addModule(id); break;
                case ModuleItem::TreeItemType::Gate: gSelectionRelay->addGate(id); break;
            }
            gSelectionRelay->relaySelectionChanged(this);
           }
        );

        menu.addAction("Isolate in new view",
            [id, type]()
            {
                Node nd;
                switch(type)
                {
                    case ModuleItem::TreeItemType::Module: nd = Node(id, Node::Module); break;
                    case ModuleItem::TreeItemType::Gate: nd = Node(id, Node::Gate); break;
                }
                SelectionTreeView::isolateInNewViewAction(nd);
            }
        );

        menu.addAction("Focus item in Graph View",
            [id, type]()
            {
                switch(type)
                {
                    case ModuleItem::TreeItemType::Module: gContentManager->getGraphTabWidget()->handleModuleFocus(id); break;
                    case ModuleItem::TreeItemType::Gate: gContentManager->getGraphTabWidget()->handleGateFocus(id); break;
                }
            }
        );

        menu.addSection("Python Code");

        QString pythonGetObject = (type == ModuleItem::TreeItemType::Module) ? PyCodeProvider::pyCodeModule(id) : PyCodeProvider::pyCodeGate(id);
        QString pythonDescription = (type == ModuleItem::TreeItemType::Module) ? "Get module" : "Get gate";
        menu.addAction(QIcon(":/icons/python"), pythonDescription,
           [pythonGetObject]()
           {
               QApplication::clipboard()->setText(pythonGetObject);
           }
        );

//        menu.addAction(QIcon(":/icons/python"), descriptions.at(1),
//           [pythonGetName]()
//           {
//               QApplication::clipboard()->setText(pythonGetName);
//           }
//        );

//        menu.addAction(QIcon(":/icons/python"), descriptions.at(2),
//           [pythonGetType]()
//           {
//               QApplication::clipboard()->setText(pythonGetType);
//           }
//        );

        menu.move(this->mapToGlobal(pos));
        menu.exec();
    }

    void ModuleElementsTree::handleNumberSubmodulesChanged(const int number)
    {
        Q_EMIT updateText(QString("Submodules(%1)").arg(number));
    }
}
