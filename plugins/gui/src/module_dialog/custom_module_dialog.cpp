#include "gui/module_dialog/custom_module_dialog.h"
#include "gui/module_dialog/module_select_model.h"
#include "gui/gui_globals.h"
#include "gui/module_model/module_model.h"
#include "gui/module_model/module_proxy_model.h"
#include "gui/module_model/module_item.h"
#include "gui/searchbar/searchbar.h"
#include "gui/content_manager/content_manager.h"
#include "gui/settings/settings_items/settings_item_keybind.h"
#include "hal_core/utilities/log.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QPushButton>
#include <QTabWidget>
#include <QTableView>
#include <QTreeView>

#include <QDebug>

namespace hal {
    CustomModuleDialog::CustomModuleDialog(QSet<u32> exclude_ids, QWidget* parent)
        : QDialog(parent),
          mSelectedId(0)
    {
        mExcludeIds = exclude_ids;
        setWindowTitle("Add module to view");
        QGridLayout* layout = new QGridLayout(this);

        if (exclude_ids.size() < gNetlist->get_modules().size())
        {
            mSearchbar = new Searchbar(this);
            layout->addWidget(mSearchbar, 1, 0, 1, 2);
            mTabWidget = new QTabWidget(this);
            mTreeView  = new QTreeView(mTabWidget);
            mTabWidget->addTab(mTreeView, "Module tree");

            mTableView = new ModuleSelectView(false, mSearchbar, &exclude_ids, mTabWidget);
            connect(mTableView, &ModuleSelectView::moduleSelected, this, &CustomModuleDialog::handleTableSelection);
            mTabWidget->addTab(mTableView, "Module list");

            layout->addWidget(mTabWidget, 2, 0, 1, 3);

            mModuleTreeProxyModel = new ModuleProxyModel(this);
            mModuleTreeProxyModel->setFilterKeyColumn(-1);
            mModuleTreeProxyModel->setDynamicSortFilter(true);
            mModuleTreeProxyModel->setSourceModel(gNetlistRelay->getModuleModel());
            mTreeView->setModel(mModuleTreeProxyModel);
            mTreeView->expandAll();

            mButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, this);
            layout->addWidget(mButtonBox, 3, 0, 1, 3, Qt::AlignHCenter);

            mTabWidget->setCurrentIndex(1);
            enableButtons();

            connect(mTabWidget, &QTabWidget::currentChanged, this, &CustomModuleDialog::handleCurrentTabChanged);
            connect(ContentManager::sSettingSearch, &SettingsItemKeybind::keySequenceChanged, this, &CustomModuleDialog::keybindToggleSearchbar);
            connect(mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
            connect(mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
            connect(mTreeView, &QTreeView::doubleClicked, this, &CustomModuleDialog::handleTreeDoubleClick);
            connect(mTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &CustomModuleDialog::handleTreeSelectionChanged);
        }
        else
        {
            mNoAvailable = new QLabel(this);
            mNoAvailable->setText("There is no addable module.");
            layout->addWidget(mNoAvailable, 0, 1);

            mButtonBox = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
            layout->addWidget(mButtonBox, 3, 1);
            connect(mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        }

    }

    void CustomModuleDialog::enableButtons()
    {
        mButtonBox->button(QDialogButtonBox::Ok)->setEnabled(mSelectedId>0);
        QString target = "…";
        if (mSelectedId>0)
        {
            Module* m = gNetlist->get_module_by_id(mSelectedId);
            if (m) target = QString("%1[%2]").arg(QString::fromStdString(m->get_name())).arg(mSelectedId);
        }
        setWindowTitle("Add module to View");
    }


    u32 CustomModuleDialog::treeModuleId(const QModelIndex& index)
    {
       ModuleModel* treeModel = static_cast<ModuleModel*>(mModuleTreeProxyModel->sourceModel());
       Q_ASSERT(treeModel);
       QModelIndex sourceIndex = mModuleTreeProxyModel->mapToSource(index);
       if (!sourceIndex.isValid()) return 0;
       ModuleItem* item = treeModel->getItem(sourceIndex);
       if (!item) return 0;
       return item->id();
    }

    void CustomModuleDialog::handleTreeSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
    {
       Q_UNUSED(deselected);
       auto sel = selected.indexes();
       if (sel.empty())
           handleTableSelection(0, false);
       else
           handleTableSelection(treeModuleId(sel.at(0)), false);
    }

    void CustomModuleDialog::handleTreeDoubleClick(const QModelIndex& index)
    {
       u32 moduleId = treeModuleId(index);
       handleTableSelection(moduleId, true);
    }

    void CustomModuleDialog::handleTableSelection(u32 id, bool doubleClick)
    {
        //mSelectedId = mSelectExclude.isAccepted(id) ? id : 0;
        if (mExcludeIds.contains(id))
        {
            mSelectedId = 0;
        }
        enableButtons();
        if (mSelectedId && doubleClick)
        {
            accept();
        }
    }

    void CustomModuleDialog::accept()
    {
        ModuleSelectHistory::instance()->add(mSelectedId);
        QDialog::accept();
    }

    void CustomModuleDialog::handleCurrentTabChanged(int index)
    {
        Q_UNUSED(index);
        mTreeView->clearSelection();
        mTableView->clearSelection();
        mSearchbar->clear();
        //if (mLastUsed) mLastUsed->clearSelection();
    }

    void CustomModuleDialog::keybindToggleSearchbar(const QKeySequence& seq)
    {
        mToggleSearchbar->setShortcut(seq);
    }
}

