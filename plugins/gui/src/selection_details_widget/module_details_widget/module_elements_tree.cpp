#include "gui/selection_details_widget/module_details_widget/module_elements_tree.h"
#include "gui/selection_details_widget/module_details_widget/netlist_elements_tree_model.h"
#include "gui/selection_details_widget/module_details_widget/module_tree_model.h"
#include "gui/python/py_code_provider.h"
#include "gui/gui_globals.h"
#include <QMenu>
#include <QHeaderView>
#include <QApplication>
#include <QClipboard>

namespace hal
{

    ModuleElementsTree::ModuleElementsTree(QWidget *parent) : QTreeView(parent), //mNetlistElementsModel(new NetlistElementsTreeModel(this)),
        mModel(new ModuleTreeModel(this)), mModuleID(-1)
    {
        setContextMenuPolicy(Qt::CustomContextMenu);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setSelectionMode(QAbstractItemView::NoSelection);
        setFocusPolicy(Qt::NoFocus);
        header()->setStretchLastSection(true);
        //setModel(mNetlistElementsModel);
        setModel(mModel);

        //connections
        connect(this, &QTreeView::customContextMenuRequested, this, &ModuleElementsTree::handleContextMenuRequested);
        connect(mModel, &ModuleTreeModel::numberOfSubmodulesChanged, this, &ModuleElementsTree::handleNumberSubmodulesChanged);
    }

    void ModuleElementsTree::setModule(u32 moduleID)
    {
        Module* m = gNetlist->get_module_by_id(moduleID);
        if(!m) return;

        setModule(m);
    }

    void ModuleElementsTree::setModule(Module *m)
    {
        //if(!m) return;

        //mNetlistElementsModel->setModule(m, true, false, false);

        mModel->setModule(m);
        mModuleID = m->get_id();
    }

    void ModuleElementsTree::removeContent()
    {
        //mNetlistElementsModel->clear();
        mModel->clear();
        mModuleID = -1;
    }

    void ModuleElementsTree::handleContextMenuRequested(const QPoint &pos)
    {
        QModelIndex clickedIndex = indexAt(pos);
        if(!clickedIndex.isValid())
            return;

        TreeItem* clickedItem = mModel->getItemFromIndex(clickedIndex);
        int id = clickedItem->getData(ModuleTreeModel::sIdColumn).toInt();
        ModuleTreeModel::itemType type = mModel->getTypeOfItem(clickedItem);
        QMenu menu;

        //menu.addSection("here comes the plaintext");

        menu.addAction("Name to clipboard",
           [clickedItem]()
           {
               QApplication::clipboard()->setText(clickedItem->getData(NetlistElementsTreeModel::sNameColumn).toString());
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
               QApplication::clipboard()->setText(clickedItem->getData(NetlistElementsTreeModel::sTypeColumn).toString());
           }
        );

        menu.addSection("Misc");

        menu.addAction("Set as current selection",
           [this, id, type]()
           {
            gSelectionRelay->clear();
            switch(type)
            {
                case ModuleTreeModel::itemType::module: gSelectionRelay->addModule(id); break;
                case ModuleTreeModel::itemType::gate: gSelectionRelay->addGate(id); break;
            }
            gSelectionRelay->relaySelectionChanged(this);
           }
        );

        menu.addAction("Add to current selection",
           [this, id, type]()
           {
            switch(type)
            {
                case ModuleTreeModel::itemType::module: gSelectionRelay->addModule(id); break;
                case ModuleTreeModel::itemType::gate: gSelectionRelay->addGate(id); break;
            }
            gSelectionRelay->relaySelectionChanged(this);
           }
        );

        menu.addSection("Python Code");

        QString pythonGetObject = (type == ModuleTreeModel::itemType::module) ? PyCodeProvider::pyCodeModule(id) : PyCodeProvider::pyCodeGate(id);
        QString pythonDescription = (type == ModuleTreeModel::itemType::module) ? "Get module" : "Get gate";
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
