#include "gui/module_dialog/module_dialog.h"
#include "gui/module_dialog/module_select_model.h"
#include "gui/gui_globals.h"
#include "gui/module_model/module_model.h"
#include "gui/module_model/module_proxy_model.h"
#include "gui/module_model/module_item.h"
#include "gui/searchbar/searchbar.h"
#include "gui/content_manager/content_manager.h"
#include "gui/settings/settings_items/settings_item_keybind.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QPushButton>
#include <QTabWidget>
#include <QTableView>
#include <QTreeView>

namespace hal {
    ModuleDialog::ModuleDialog(QWidget* parent)
        : QDialog(parent), mSelectedId(0), mNewModule(false)
    {
        setWindowTitle("Move to module …");
        QGridLayout* layout = new QGridLayout(this);

        QPushButton* butNew = new QPushButton("Create new module", this);
        connect(butNew, &QPushButton::pressed, this, &ModuleDialog::handleCreateNewModule);
        layout->addWidget(butNew, 0, 0);

        QPushButton* butPick = new QPushButton("Pick module from graph", this);
        connect(butPick, &QPushButton::pressed, this, &ModuleDialog::handlePickFromGraph);
        layout->addWidget(butPick, 0, 1);

        mSearchbar = new Searchbar(this);
        layout->addWidget(mSearchbar, 1, 0, 1, 2);

        mTabWidget = new QTabWidget(this);
        mTreeView  = new QTreeView(mTabWidget);
        mTabWidget->addTab(mTreeView, "Module tree");

        mTableView = new ModuleSelectView(false,mSearchbar,mTabWidget);
        connect(mTableView,&ModuleSelectView::moduleSelected,this,&ModuleDialog::handleTableSelection);
        mTabWidget->addTab(mTableView, "Module list");

        if (!ModuleSelectHistory::instance()->isEmpty())
        {
            mLastUsed = new ModuleSelectView(true,mSearchbar,mTabWidget);
            if (mLastUsed->model()->rowCount())
            {
                connect(mLastUsed,&ModuleSelectView::moduleSelected,this,&ModuleDialog::handleTableSelection);
                mTabWidget->addTab(mLastUsed, "Recent selection");
            }
            else
                delete mLastUsed;
        }

        layout->addWidget(mTabWidget, 2, 0, 1, 2);

        mModuleTreeProxyModel = new ModuleProxyModel(this);
        mModuleTreeProxyModel->setFilterKeyColumn(-1);
        mModuleTreeProxyModel->setDynamicSortFilter(true);
        mModuleTreeProxyModel->setSourceModel(gNetlistRelay->getModuleModel());
       //mModuleProxyModel->setRecursiveFilteringEnabled(true);
        mModuleTreeProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
        mTreeView->setModel(mModuleTreeProxyModel);

        mButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, this);
        layout->addWidget(mButtonBox, 3, 1);

        mToggleSearchbar = new QAction(this);
        mToggleSearchbar->setShortcut(QKeySequence(ContentManager::sSettingSearch->value().toString()));
        addAction(mToggleSearchbar);

        mTabWidget->setCurrentIndex(1);
        testOkEnabled();

        connect(mToggleSearchbar,&QAction::triggered,this,&ModuleDialog::handleToggleSearchbar);
        connect(ContentManager::sSettingSearch,&SettingsItemKeybind::keySequenceChanged,this,&ModuleDialog::keybindToggleSearchbar);
        connect(mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        connect(mTreeView->selectionModel(),&QItemSelectionModel::currentChanged,this,&ModuleDialog::handleTreeSelectionChanged);
    }

    void ModuleDialog::testOkEnabled()
    {
        mButtonBox->button(QDialogButtonBox::Ok)->setEnabled(mSelectedId>0);
    }

     u32 ModuleDialog::treeModuleId(const QModelIndex& index)
     {
        ModuleModel* treeModel = static_cast<ModuleModel*>(mModuleTreeProxyModel->sourceModel());
        Q_ASSERT(treeModel);
        QModelIndex sourceIndex = mModuleTreeProxyModel->mapToSource(index);
        if (!sourceIndex.isValid()) return 0;
        ModuleItem* item = treeModel->getItem(sourceIndex);
        if (!item) return 0;
        return item->id();
     }

     void ModuleDialog::handleTreeSelectionChanged(const QModelIndex& current, const QModelIndex& previous)
     {
         Q_UNUSED(previous);
         u32 moduleId = treeModuleId(current);
         if (moduleId) handleTableSelection(moduleId,false);
     }

     void ModuleDialog::handleTreeDoubleClick(const QModelIndex& index)
     {
         u32 moduleId = treeModuleId(index);
         if (moduleId) handleTableSelection(moduleId,true);
     }

    void ModuleDialog::handleTableSelection(u32 id, bool doubleClick)
    {
        if (!mSelectExclude.isAccepted(id)) return;
        mSelectedId = id;
        testOkEnabled();
        if (doubleClick) accept();
    }

    void ModuleDialog::handlePickFromGraph()
    {
        ModuleSelectPicker* msp = new ModuleSelectPicker;
        connect(gSelectionRelay, &SelectionRelay::selectionChanged, msp, &ModuleSelectPicker::handleSelectionChanged);
        reject(); // wait for picker, no selection done in dialog
    }

    void ModuleDialog::accept()
    {
        ModuleSelectHistory::instance()->add(mSelectedId);
        QDialog::accept();
    }

    void ModuleDialog::handleCreateNewModule()
    {
        mNewModule = true;
        accept();
    }

    void ModuleDialog::handleToggleSearchbar()
    {
        if (mSearchbar->isHidden())
            mSearchbar->show();
        else
        {
            mSearchbar->hide();
            mSearchbar->clear();
        }
    }

    void ModuleDialog::keybindToggleSearchbar(const QKeySequence& seq)
    {
        mToggleSearchbar->setShortcut(seq);
    }
}

