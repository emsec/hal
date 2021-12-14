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

    ModuleElementsTree::ModuleElementsTree(QWidget *parent) : SizeAdjustableTreeView(parent), //mNetlistElementsModel(new NetlistElementsTreeModel(this)),
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
        adjustSizeToContents();
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

        //Strings for first menu entry (python get net/gate/module)
        QString pythonGetObject, pythonGetName, pythonGetType;
        QString plainName, plainType = "not defined";
        QList<QString> descriptions;

        auto createCommonDescriptionList = [](QString objectType){
          QList<QString> descriptionList =
          {
              QString("Extract %1 as python code").arg(objectType),
              "Extract name as python code",
              "Extract type as python code"
          };
          return descriptionList;
        };

        switch (type)
        {
            case ModuleTreeModel::itemType::module:
                descriptions = createCommonDescriptionList("module");
                pythonGetObject = PyCodeProvider::pyCodeModule(id);
                pythonGetName = PyCodeProvider::pyCodeModuleName(id);
                pythonGetType = PyCodeProvider::pyCodeModuleType(id);
                break;
            case ModuleTreeModel::itemType::gate:
                descriptions = createCommonDescriptionList("gate");
                pythonGetObject = PyCodeProvider::pyCodeGate(id);
                pythonGetName = PyCodeProvider::pyCodeGateName(id);
                pythonGetType = PyCodeProvider::pyCodeGateType(id);
                break;
        }

        //menu.addSection("here comes the plaintext");

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

        menu.addAction("Extract ID as plain text",
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
                case ModuleTreeModel::itemType::module: gSelectionRelay->addModule(id); break;
                case ModuleTreeModel::itemType::gate: gSelectionRelay->addGate(id); break;
            }
            gSelectionRelay->relaySelectionChanged(this);
           }
        );

        menu.addSection("Python Code");

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

        menu.move(this->mapToGlobal(pos));
        menu.exec();
    }

    void ModuleElementsTree::handleNumberSubmodulesChanged(const int number)
    {
        Q_EMIT updateText(QString("Submodules(%1)").arg(number));
    }
}
