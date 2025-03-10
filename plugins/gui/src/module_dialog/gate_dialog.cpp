
#include "gui/module_dialog/gate_dialog.h"
#include "gui/module_dialog/module_select_model.h"
#include "gui/gui_globals.h"
#include "gui/module_model/module_model.h"
#include "gui/module_model/module_proxy_model.h"
#include "gui/module_model/module_item.h"
#include "gui/searchbar/searchbar.h"
#include "gui/content_manager/content_manager.h"
#include "gui/settings/settings_items/settings_item_keybind.h"
#include "gui/graph_widget/graph_graphics_view.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QPushButton>
#include <QTabWidget>
#include <QTableView>
#include <QTreeView>

namespace hal {
    GateDialog::GateDialog(const QSet<u32> &selectable, const QString &title, GateSelectReceiver* receiver, QWidget* parent)
        : QDialog(parent), mSelectedId(0), mReceiver(receiver),
          mSelectableGates(selectable),  mPickerModeActivated(false), mWindowTitle(title)
    {
        setWindowTitle(mWindowTitle + " …");
        QGridLayout* layout = new QGridLayout(this);

        mButtonPick = new QPushButton("Pick gate from graph", this);
        connect(mButtonPick, &QPushButton::pressed, this, &GateDialog::handlePickFromGraph);
        layout->addWidget(mButtonPick, 0, 1);

        mSearchbar = new Searchbar(this);
        layout->addWidget(mSearchbar, 1, 0, 1, 2);

        mTabWidget = new QTabWidget(this);
//        mTreeView  = new QTreeView(mTabWidget);
//        mTabWidget->addTab(mTreeView, "Gate tree");

        mTableView = new GateSelectView(false,selectable,mTabWidget);
        mGateTableProxyModel = static_cast<GateSelectProxy*>(mTableView->model());
        connect(mTableView,&GateSelectView::gateSelected,this,&GateDialog::handleTableSelection);
        mTabWidget->addTab(mTableView, "Gate list");

        if (!GateSelectHistory::instance()->isEmpty())
        {
            mLastUsed = new GateSelectView(true,selectable,mTabWidget);
            if (mLastUsed->model()->rowCount())
            {
                connect(mLastUsed,&GateSelectView::gateSelected,this,&GateDialog::handleTableSelection);
                mTabWidget->addTab(mLastUsed, "Recent selection");
            }
            else
                delete mLastUsed;
        }

        layout->addWidget(mTabWidget, 2, 0, 1, 2);

        /*
         *
         * TODO : tree view
        mGateTreeProxyModel = new GateProxyModel(this);
        mGateTreeProxyModel->setFilterKeyColumn(-1);
        mGateTreeProxyModel->setDynamicSortFilter(true);
        mGateTreeProxyModel->setSourceModel(gNetlistRelay->getGateModel());
       //mGateProxyModel->setRecursiveFilteringEnabled(true);
        mGateTreeProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
        mTreeView->setModel(mGateTreeProxyModel);
*/

        mButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, this);
        layout->addWidget(mButtonBox, 3, 1);

        mToggleSearchbar = new QAction(this);
        mToggleSearchbar->setShortcut(QKeySequence(ContentManager::sSettingSearch->value().toString()));
        addAction(mToggleSearchbar);

        mTabWidget->setCurrentIndex(1);
        enableButtons();

        mSearchbar->setColumnNames(mGateTableProxyModel->getColumnNames());

        connect(mToggleSearchbar,&QAction::triggered,this,&GateDialog::handleToggleSearchbar);
        connect(ContentManager::sSettingSearch,&SettingsItemKeybind::keySequenceChanged,this,&GateDialog::keybindToggleSearchbar);
        connect(mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        connect(mSearchbar, &Searchbar::triggerNewSearch, mGateTableProxyModel, &GateSelectProxy::startSearch);
//        connect(mTreeView->selectionModel(),&QItemSelectionModel::currentChanged,this,&GateDialog::handleTreeSelectionChanged);
        if (receiver == nullptr)
        {
            mButtonPick->setDisabled(true);
            mButtonPick->hide();
        }
    }

    void GateDialog::enableButtons()
    {
        mButtonBox->button(QDialogButtonBox::Ok)->setEnabled(mSelectedId>0);
        mButtonPick->setEnabled(mTableView->model()->rowCount()>0);
        QString target = "…";
        if (mSelectedId>0)
        {
            Gate* g = gNetlist->get_gate_by_id(mSelectedId);
            if (g) target = QString("%1[%2]").arg(QString::fromStdString(g->get_name())).arg(mSelectedId);
        }
        setWindowTitle(mWindowTitle + " " + target);
    }

     u32 GateDialog::treeGateId(const QModelIndex& index)
     {
        Q_UNUSED(index);
         return 0;
         /*
        GateModel* treeModel = static_cast<GateModel*>(mGateTreeProxyModel->sourceModel());
        Q_ASSERT(treeModel);
        QModelIndex sourceIndex = mGateTreeProxyModel->mapToSource(index);
        if (!sourceIndex.isValid()) return 0;
        GateItem* item = treeModel->getItem(sourceIndex);
        if (!item) return 0;
        return item->id();
        */
     }

     void GateDialog::handleTreeSelectionChanged(const QModelIndex& current, const QModelIndex& previous)
     {
        Q_UNUSED(current);
        Q_UNUSED(previous);
         /*
         Q_UNUSED(previous);
         u32 moduleId = treeGateId(current);
         if (moduleId) handleTableSelection(moduleId,false);
         */
     }

     void GateDialog::handleTreeDoubleClick(const QModelIndex& index)
     {
         u32 moduleId = treeGateId(index);
         if (moduleId) handleTableSelection(moduleId,true);
     }

    void GateDialog::handleTableSelection(u32 id, bool doubleClick)
    {
        mSelectedId = GateSelectModel::isAccepted(id,mSelectableGates) ? id : 0;
        enableButtons();
        if (mSelectedId && doubleClick) accept();
    }

    void GateDialog::handlePickFromGraph()
    {
        new GateSelectPicker(mSelectableGates, mReceiver);
        mPickerModeActivated = true;
        reject(); // wait for picker, no selection done in dialog
    }

    void GateDialog::accept()
    {
        GateSelectHistory::instance()->add(mSelectedId);
        QDialog::accept();
    }

    void GateDialog::handleToggleSearchbar()
    {
        if (mSearchbar->isHidden())
        {
            mSearchbar->show();
            mSearchbar->setFocus();
        }
        else
        {
            mSearchbar->hide();
            setFocus();
        }
    }

    void GateDialog::keybindToggleSearchbar(const QKeySequence& seq)
    {
        mToggleSearchbar->setShortcut(seq);
    }
}

