#include "gui/new_selection_details_widget/new_module_details_widget/module_elements_tree.h"
#include "gui/new_selection_details_widget/models/netlist_elements_tree_model.h"
#include "gui/new_selection_details_widget/py_code_provider.h"
#include "gui/gui_globals.h"
#include <QMenu>
#include <QHeaderView>
#include <QDebug>
#include <QApplication>
#include <QClipboard>

namespace hal
{

    ModuleElementsTree::ModuleElementsTree(QWidget *parent) : QTreeView(parent), mNetlistElementsModel(new NetlistElementsTreeModel(this)),
        mModuleID(-1)
    {
        setContextMenuPolicy(Qt::CustomContextMenu);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        header()->setStretchLastSection(true);
        setModel(mNetlistElementsModel);

        //connections
        connect(this, &QTreeView::customContextMenuRequested, this, &ModuleElementsTree::handleContextMenuRequested);
    }

    void ModuleElementsTree::setContent(u32 moduleID)
    {
        Module* m = gNetlist->get_module_by_id(moduleID);
        if(!m) return;

        mNetlistElementsModel->setModule(m);
        mModuleID = moduleID;
    }

    void ModuleElementsTree::setContent(Module *m)
    {
        if(!m) return;

        mNetlistElementsModel->setModule(m);
        mModuleID = m->get_id();
    }

    void ModuleElementsTree::removeContent()
    {
        mNetlistElementsModel->clear();
        mModuleID = -1;
    }

    void ModuleElementsTree::handleContextMenuRequested(const QPoint &pos)
    {
        TreeItem* clickedItem = mNetlistElementsModel->getItemFromIndex(indexAt(pos));
        int id = mNetlistElementsModel->getRepresentedIdOfItem(clickedItem);
        NetlistElementsTreeModel::itemType type = mNetlistElementsModel->getTypeOfItem(clickedItem);
        QMenu menu;

        //Strings for first menu entry (python get net/gate/module)
        QString pythonGetObject, pythonGetName, pythonGetType, pythonGetId;
        QString plainName, plainType = "not defined";
        QList<QString> descriptions;

        auto createCommonDescriptionList = [](QString objectType){
          QList<QString> descriptionList =
          {
              QString("Extract %1 as python code").arg(objectType),
              "Extract name as python code",
              "Extract type as python code",
              "Extract id as python code" //does this even makes sense???
          };
          return descriptionList;
        };

        switch (type)
        {
            case NetlistElementsTreeModel::itemType::module:
                descriptions = createCommonDescriptionList("module");
                pythonGetObject = PyCodeProvider::pyCodeModule(id);
                pythonGetName = PyCodeProvider::pyCodeModuleName(id);
                pythonGetType = PyCodeProvider::pyCodeModuleType(id);
                pythonGetId = PyCodeProvider::pyCodeModule(id) + ".get_id()"; // kind of retarded, change that..
                break;
            case NetlistElementsTreeModel::itemType::gate:
                descriptions = createCommonDescriptionList("gate");
                pythonGetObject = PyCodeProvider::pyCodeGate(id);
                pythonGetName = PyCodeProvider::pyCodeGateName(id);
                pythonGetType = PyCodeProvider::pyCodeGateType(id);
                pythonGetId = PyCodeProvider::pyCodeGate(id) + ".get_id()"; // same as above..
                break;
            case NetlistElementsTreeModel::itemType::net:
                descriptions = createCommonDescriptionList("net");
                pythonGetObject = PyCodeProvider::pyCodeNet(id);
                pythonGetName = PyCodeProvider::pyCodeNetName(id);
                pythonGetType = PyCodeProvider::pyCodeNetType(id);
                pythonGetId = PyCodeProvider::pyCodeNet(id) + ".get_id()"; //same as above
                break;
        }

        menu.addAction(QIcon(":/icons/python"), descriptions.at(0),
           [pythonGetObject]()
           {
               QApplication::clipboard()->setText(pythonGetObject);
           }
        );

        menu.addAction(QIcon(":/icons/python"), descriptions.at(1),
           [pythonGetName]()
           {
               QApplication::clipboard()->setText(pythonGetName);
           }
        );

        menu.addAction(QIcon(":/icons/python"), descriptions.at(2),
           [pythonGetType]()
           {
               QApplication::clipboard()->setText(pythonGetType);
           }
        );

        menu.addAction(QIcon(":/icons/python"), descriptions.at(3),
           [pythonGetId]()
           {
               QApplication::clipboard()->setText(pythonGetId);
           }
        );

        menu.addSection("here comes the plaintext");

        menu.addAction("Extract name as plain text",
           [clickedItem]()
           {
               QApplication::clipboard()->setText(clickedItem->getData(NetlistElementsTreeModel::sNameColumn).toString());
           }
        );

        menu.addAction("Extract type as plain text",
           [clickedItem]()
           {
               QApplication::clipboard()->setText(clickedItem->getData(NetlistElementsTreeModel::sTypeColumn).toString());
           }
        );

        menu.addAction("Extract id as plain text",
           [id]()
           {
               QApplication::clipboard()->setText(QString::number(id));
           }
        );

        menu.addSection("Misc");

        menu.addAction("Add to current selection",
           [this, id, type]()
           {
            switch(type)
            {
                case NetlistElementsTreeModel::itemType::module: gSelectionRelay->addModule(id); break;
                case NetlistElementsTreeModel::itemType::gate: gSelectionRelay->addGate(id); break;
                case NetlistElementsTreeModel::itemType::net: gSelectionRelay->addNet(id); break;
            }
            gSelectionRelay->relaySelectionChanged(this);
           }
        );




        menu.move(this->mapToGlobal(pos));
        menu.exec();
    }

}