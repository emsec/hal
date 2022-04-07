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

#include <QDebug>

namespace hal {
    CustomModuleDialog::CustomModuleDialog(QSet<u32> exclude_ids, QWidget* parent)
        : QDialog(parent),
          mSelectedId(0)
    {

        setWindowTitle("Add module to view");
        QGridLayout* layout = new QGridLayout(this);

        if (exclude_ids.size() < gNetlist->get_modules().size())
        {
            mSearchbar = new Searchbar(this);
            layout->addWidget(mSearchbar, 1, 0, 1, 2);
            mTabWidget = new QTabWidget(this);
            mTableView = new ModuleSelectView(false, mSearchbar, &exclude_ids, mTabWidget);
            connect(mTableView, &ModuleSelectView::moduleSelected, this, &CustomModuleDialog::handleTableSelection);
            mTabWidget->addTab(mTableView, "Module list");

            layout->addWidget(mTabWidget, 2, 0, 1, 3);

            mButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, this);
            layout->addWidget(mButtonBox, 3, 0, 1, 3, Qt::AlignHCenter);

            enableButtons();

            connect(ContentManager::sSettingSearch, &SettingsItemKeybind::keySequenceChanged, this, &CustomModuleDialog::keybindToggleSearchbar);
            connect(mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
            connect(mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
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

    void CustomModuleDialog::handleTableSelection(u32 id, bool doubleClick)
    {
        mSelectedId = mSelectExclude.isAccepted(id) ? id : 0;
        enableButtons();
        if (mSelectedId && doubleClick) accept();
    }

    void CustomModuleDialog::accept()
    {
        ModuleSelectHistory::instance()->add(mSelectedId);
        QDialog::accept();
    }

    void CustomModuleDialog::keybindToggleSearchbar(const QKeySequence& seq)
    {
        mToggleSearchbar->setShortcut(seq);
    }
}

