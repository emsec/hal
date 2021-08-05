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
          mGroupingTableModel(new GroupingTableModel(this)),
          mSearchbar(new Searchbar(this))
    {
        setWindowTitle("Move to grouping …");
        QGridLayout* layout = new QGridLayout(this);

        QPushButton* butNew = new QPushButton("New grouping", this);
        butNew->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        connect(butNew, &QPushButton::pressed, this, &GroupingDialog::handleNewGroupingClicked);
        layout->addWidget(butNew, 0, 0);

//        QPushButton* butRemove = new QPushButton("Remove grouping", this);
//        butRemove->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
//        connect(butRemove, &QPushButton::pressed, this, &GroupingDialog::handleRemoveGroupingClicked);
//        layout->addWidget(butRemove, 0, 1);

        QPushButton* butSearch = new QPushButton("Search", this);
        butSearch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        connect(butSearch, &QPushButton::pressed, this, &GroupingDialog::handleToggleSearchbar);
        layout->addWidget(butSearch, 0, 1);

        mSearchbar->hide();
        layout->addWidget(mSearchbar, 1, 0, 1, 3);

        mProxyModel->setSourceModel(mGroupingTableModel);
        mProxyModel->setSortRole(Qt::UserRole);

        mGroupingTableView->setModel(mProxyModel);
        mGroupingTableView->setSelectionMode(QAbstractItemView::SingleSelection);
        mGroupingTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        mGroupingTableView->verticalHeader()->hide();
        mGroupingTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        mGroupingTableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        mGroupingTableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        mGroupingTableView->horizontalHeader()->setDefaultAlignment(Qt::AlignHCenter | Qt::AlignCenter);
        layout->addWidget(mGroupingTableView, 2, 0, 1, 3);

        mButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, this);
        mButtonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        layout->addWidget(mButtonBox, 3, 0, 1, 3, Qt::AlignHCenter);

        connect(mSearchbar, &Searchbar::textEdited, this, &GroupingDialog::handleFilterTextChanged);
        connect(mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        connect(mGroupingTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &GroupingDialog::handleSelectionChanged);
    }

    void GroupingDialog::handleNewGroupingClicked()
    {
        ActionCreateObject* act = new ActionCreateObject(UserActionObjectType::Grouping);
        act->exec();
    }

    void GroupingDialog::handleRemoveGroupingClicked()
    {
        int irow = mProxyModel->mapToSource(mGroupingTableView->currentIndex()).row();
        u32 grpId = mGroupingTableModel->groupingAt(irow).id();
        ActionDeleteObject* act = new ActionDeleteObject;
        act->setObject(UserActionObject(grpId,UserActionObjectType::Grouping));
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

    void GroupingDialog::handleFilterTextChanged(const QString& text)
    {
        mProxyModel->setFilterRegularExpression(text);
    }

    void GroupingDialog::handleSelectionChanged()
    {
        auto rowId = mGroupingTableView->selectionModel()->selectedRows().at(0).row();
        mGroupName = mGroupingTableModel->groupingAt(rowId).name();
        if (mGroupingTableView->selectionModel()->selectedRows().empty())
        {
            mButtonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        } else {
            mButtonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        }
    }
}
