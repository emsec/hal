#include "gui/grouping/grouping_color_delegate.h"
#include "gui/grouping_dialog/grouping_dialog.h"
#include "gui/selection_details_widget/selection_details_widget.h"
#include "gui/user_action/action_create_object.h"
#include "gui/user_action/action_delete_object.h"
#include "gui/gui_globals.h"

#include <QGridLayout>
#include <QPushButton>
#include <QHeaderView>

namespace hal {
    GroupingDialog::GroupingDialog(QWidget* parent)
        : QDialog(parent),
          mProxyModel(new GroupingProxyModel(this)),
          mGroupingTableView(new QTableView(this)),
          mGroupingTableModel(new GroupingTableModel(false, this)),
          mSearchbar(new Searchbar(this)),
          mTabWidget(new QTabWidget(this))
    {
        setWindowTitle("Move to grouping …");
        QGridLayout* layout = new QGridLayout(this);

        QPushButton* butNew = new QPushButton("New grouping", this);
        butNew->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        connect(butNew, &QPushButton::pressed, this, &GroupingDialog::handleNewGroupingClicked);
        layout->addWidget(butNew, 0, 0);

        QPushButton* butSearch = new QPushButton("Search", this);
        butSearch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        connect(butSearch, &QPushButton::pressed, this, &GroupingDialog::handleToggleSearchbar);
        layout->addWidget(butSearch, 0, 1);

        mSearchbar->hide();
        layout->addWidget(mSearchbar, 1, 0, 1, 3);

        mProxyModel->setSourceModel(mGroupingTableModel);
        mProxyModel->setSortMechanism(gui_utility::natural);
        mProxyModel->setSortRole(Qt::UserRole);

        mGroupingTableView->setModel(mProxyModel);
        mGroupingTableView->setSelectionMode(QAbstractItemView::SingleSelection);
        mGroupingTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        mGroupingTableView->setItemDelegateForColumn(2, new GroupingColorDelegate(mGroupingTableView));
        mGroupingTableView->setSortingEnabled(true);
        mGroupingTableView->sortByColumn(0, Qt::SortOrder::AscendingOrder);

        mGroupingTableView->verticalHeader()->hide();
        mGroupingTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        mGroupingTableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        mGroupingTableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        mGroupingTableView->horizontalHeader()->setDefaultAlignment(Qt::AlignHCenter | Qt::AlignCenter);

        mTabWidget->addTab(mGroupingTableView, "Groupings");

        if (!GroupingTableHistory::instance()->isEmpty())
        {
            mLastUsed = new GroupingTableView(true, this);
            if (mLastUsed->model()->rowCount())
            {
                connect(mLastUsed->selectionModel(), &QItemSelectionModel::selectionChanged, this, &GroupingDialog::handleSelectionChanged);
                mTabWidget->addTab(mLastUsed, "Recent selection");
            }
            else
                delete mLastUsed;
        }

        layout->addWidget(mTabWidget, 2, 0, 1, 3);

        mButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, this);
        mButtonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        layout->addWidget(mButtonBox, 3, 0, 1, 3, Qt::AlignHCenter);

        connect(mSearchbar, &Searchbar::textEdited, this, &GroupingDialog::handleFilterTextChanged);
        connect(mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        connect(mGroupingTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &GroupingDialog::handleSelectionChanged);
        connect(mGroupingTableView, &QTableView::doubleClicked, this, &GroupingDialog::handleDoubleClicked);
    }

    void GroupingDialog::handleNewGroupingClicked()
    {
        ActionCreateObject* act = new ActionCreateObject(UserActionObjectType::Grouping);
        act->exec();
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

    void GroupingDialog::handleDoubleClicked(const QModelIndex& index)
    {
        mGroupName = getGroupName(index);
        accept();
    }

    void GroupingDialog::handleFilterTextChanged(const QString& text)
    {
        mProxyModel->setFilterRegularExpression(text);
    }

    void GroupingDialog::handleSelectionChanged()
    {
        auto selectedRows = mGroupingTableView->selectionModel()->selectedRows();
        if (selectedRows.empty())
        {
            mButtonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        }
        else
        {
            mButtonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
            mGroupName = getGroupName(selectedRows.at(0));
            auto sourceIndex = mProxyModel->mapToSource(selectedRows.at(0));
            mGroupId = mGroupingTableModel->groupingAt(sourceIndex.row()).id();
        }
    }

    QString GroupingDialog::getGroupName(const QModelIndex& proxyIndex)
    {
        auto sourceIndex = mProxyModel->mapToSource(proxyIndex);
        return mGroupingTableModel->groupingAt(sourceIndex.row()).name();
    }

    void GroupingDialog::accept()
    {
        GroupingTableHistory::instance()->add(mGroupId);
        QDialog::accept();
    }
}
