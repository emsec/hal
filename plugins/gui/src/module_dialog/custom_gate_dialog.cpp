#include "gui/module_dialog/custom_gate_dialog.h"
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

#include <QDebug>

namespace hal {
    CustomGateDialog::CustomGateDialog(u32 orig, bool succ, const QSet<u32> &selectable, QWidget* parent)
        : QDialog(parent), mSelectedId(0), mOrigin(orig), mQuerySuccessor(succ),
          mSelectableGates(selectable)
    {
        setWindowTitle("Add gate to view");
        QGridLayout* layout = new QGridLayout(this);

        if (!selectable.empty()) {

            mSearchbar = new Searchbar(this);
            layout->addWidget(mSearchbar, 1, 0, 1, 2);

            mTabWidget = new QTabWidget(this);

            mTableView = new GateSelectView(false,mSearchbar,selectable,mTabWidget);
            connect(mTableView,&GateSelectView::gateSelected,this,&CustomGateDialog::handleTableSelection);
            mTabWidget->addTab(mTableView, "Gate list");

            if (!GateSelectHistory::instance()->isEmpty())
            {
                mLastUsed = new GateSelectView(true,mSearchbar,selectable,mTabWidget);
                if (mLastUsed->model()->rowCount())
                {
                    connect(mLastUsed,&GateSelectView::gateSelected,this,&CustomGateDialog::handleTableSelection);
                    mTabWidget->addTab(mLastUsed, "Recent selection");
                }
                else
                    delete mLastUsed;
            }

            layout->addWidget(mTabWidget, 2, 0, 1, 2);

            mToggleSearchbar = new QAction(this);
            mToggleSearchbar->setShortcut(QKeySequence(ContentManager::sSettingSearch->value().toString()));
            addAction(mToggleSearchbar);

            mTabWidget->setCurrentIndex(1);
            mButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, this);
            layout->addWidget(mButtonBox, 3, 1);
            enableButtons();

            connect(mToggleSearchbar,&QAction::triggered,this,&CustomGateDialog::handleToggleSearchbar);
            connect(ContentManager::sSettingSearch,&SettingsItemKeybind::keySequenceChanged,this,&CustomGateDialog::keybindToggleSearchbar);

            connect(mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
            connect(mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        }
        else {
            mNoAvailable = new QLabel(this);
            mNoAvailable->setText("There is no addable gate.");
            layout->addWidget(mNoAvailable, 0, 1);

            mButtonBox = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
            layout->addWidget(mButtonBox, 3, 1);
            connect(mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        }
    }

    void CustomGateDialog::enableButtons()
    {
        mButtonBox->button(QDialogButtonBox::Ok)->setEnabled(mSelectedId>0);
        QString target = "…";
        if (mSelectedId>0)
        {
            Gate* g = gNetlist->get_gate_by_id(mSelectedId);
            if (g) target = QString("%1[%2]").arg(QString::fromStdString(g->get_name())).arg(mSelectedId);
        }
        setWindowTitle("Add gate to view");
    }

    void CustomGateDialog::handleTableSelection(u32 id, bool doubleClick)
    {
        mSelectedId = GateSelectModel::isAccepted(id,mSelectableGates) ? id : 0;
        enableButtons();
        if (mSelectedId && doubleClick) accept();
    }

    void CustomGateDialog::accept()
    {
        GateSelectHistory::instance()->add(mSelectedId);
        QDialog::accept();
    }

    void CustomGateDialog::handleToggleSearchbar()
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

    void CustomGateDialog::keybindToggleSearchbar(const QKeySequence& seq)
    {
        mToggleSearchbar->setShortcut(seq);
    }
}

