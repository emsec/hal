#include "gui/grouping/grouping_color_delegate.h"
#include "gui/grouping_dialog/grouping_dialog.h"
#include "gui/selection_details_widget/selection_details_widget.h"
#include "gui/user_action/action_create_object.h"
#include "gui/user_action/action_delete_object.h"
#include "gui/gui_globals.h"
#include "gui/settings/settings_items/settings_item_keybind.h"
#include "hal_core/utilities/log.h"

#include <QGridLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QAction>

namespace hal {
    GroupingDialog::GroupingDialog(QWidget* parent)
        : QDialog(parent),
          mSearchbar(new Searchbar(this)),
          mProxyModel(new GroupingProxyModel(this)),
          mGroupingTableView(new GroupingTableView(false, this)),
          mLastUsed(nullptr),
          mTabWidget(new QTabWidget(this)),
          mNewGrouping(false)
    {
        setWindowTitle("Move to grouping …");
        QGridLayout* layout = new QGridLayout(this);

        QPushButton* butNew = new QPushButton("Create new grouping", this);
        butNew->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        connect(butNew, &QPushButton::pressed, this, &GroupingDialog::handleNewGroupingClicked);
        layout->addWidget(butNew, 0, 0);

        QPushButton* butSearch = new QPushButton("Search", this);
        butSearch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        connect(butSearch, &QPushButton::pressed, this, &GroupingDialog::handleToggleSearchbar);
        layout->addWidget(butSearch, 0, 1);

        mProxyModel->setSourceModel(mGroupingTableView->model());
        mProxyModel->setSortRole(Qt::UserRole);
        mGroupingTableView->setModel(mProxyModel);

        mSearchbar->hide();
        mSearchbar->setColumnNames(mProxyModel->getColumnNames());

        layout->addWidget(mSearchbar, 1, 0, 1, 3);

        mTabWidget->addTab(mGroupingTableView, "Groupings");

        if (!GroupingTableHistory::instance()->isEmpty())
        {
            mLastUsed = new GroupingTableView(true, this);
            if (mLastUsed->model()->rowCount())
            {
                connect(mLastUsed, &GroupingTableView::groupingSelected, this, &GroupingDialog::handleGroupingSelected);
                mTabWidget->addTab(mLastUsed, "Recent selection");
            }
            else
            {
                delete mLastUsed;
                mLastUsed = nullptr;
            }
        }

        layout->addWidget(mTabWidget, 2, 0, 1, 3);

        mButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, this);
        mButtonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        layout->addWidget(mButtonBox, 3, 0, 1, 3, Qt::AlignHCenter);

        mToggleSearchbar = new QAction(this);
        mToggleSearchbar->setShortcut(QKeySequence(ContentManager::sSettingSearch->value().toString()));
        addAction(mToggleSearchbar);

        connect(mTabWidget, &QTabWidget::currentChanged, this, &GroupingDialog::handleCurrentTabChanged);

        // connect(mSearchbar, &Searchbar::textEdited, this, &GroupingDialog::filter);

        // TODO: change parameter in filter() method
        // connect(mSearchbar, &Searchbar::triggerNewSearch, this, &GroupingDialog::filter);
        connect(mToggleSearchbar, &QAction::triggered, this, &GroupingDialog::handleToggleSearchbar);
        connect(mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        connect(mGroupingTableView, &GroupingTableView::groupingSelected, this, &GroupingDialog::handleGroupingSelected);
        connect(mSearchbar, &Searchbar::triggerNewSearch, mProxyModel, &GroupingProxyModel::startSearch);
    }

    void GroupingDialog::handleNewGroupingClicked()
    {
        mNewGrouping = true;
        QDialog::accept();
    }

    void GroupingDialog::handleToggleSearchbar()
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

    void GroupingDialog::filter(const QString& text) // TODO : add int parameter search options
    {
        static_cast<GroupingProxyModel*>(mGroupingTableView->model())->setFilterRegularExpression(text);
        if (mLastUsed)
            static_cast<GroupingProxyModel*>(mLastUsed->model())->setFilterRegularExpression(text);
        QString output = "navigation regular expression '" + text + "' entered.";
        log_info("user", output.toStdString());
    }

    void GroupingDialog::handleCurrentTabChanged(int index)
    {
        Q_UNUSED(index);
        mGroupingTableView->clearSelection();
        mSearchbar->clear();
        if (mLastUsed) mLastUsed->clearSelection();
    }

    void GroupingDialog::handleGroupingSelected(u32 groupId, bool doubleClick)
    {
        mButtonBox->button(QDialogButtonBox::Ok)->setEnabled(groupId);
        if (!groupId) return;
        mGroupId = groupId;
        if (doubleClick) accept();
    }

    void GroupingDialog::accept()
    {
        GroupingTableHistory::instance()->add(mGroupId);
        QDialog::accept();
    }
}
