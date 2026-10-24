#include "gui/grouping/grouping_content_dialog.h"

#include "gui/file_manager/file_manager.h"
#include "gui/graph_widget/layout_locker.h"
#include "gui/grouping/grouping_table_model.h"
#include "gui/gui_globals.h"
#include "gui/module_model/module_model.h"
#include "gui/netlist_relay/netlist_relay.h"
#include "gui/selection_details_widget/tree_navigation/selection_tree_proxy.h"
#include "gui/selection_details_widget/tree_navigation/selection_tree_view.h"
#include "hal_core/netlist/grouping.h"
#include "hal_core/netlist/netlist.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

namespace hal
{
    GroupingContentDialog* GroupingContentDialog::showGrouping(Grouping* grouping, QWidget* parent)
    {
        for (GroupingContentDialog* dialog : parent->findChildren<GroupingContentDialog*>(QString(), Qt::FindDirectChildrenOnly))
        {
            if (dialog->grouping() == grouping && dialog->isVisible())
            {
                dialog->raise();
                dialog->activateWindow();
                return dialog;
            }
        }
        GroupingContentDialog* dialog = new GroupingContentDialog(grouping, parent);
        dialog->show();
        return dialog;
    }

    GroupingContentDialog::GroupingContentDialog(Grouping* grouping, QWidget* parent)
        : QDialog(parent), mGrouping(grouping), mGroupingId(grouping->get_id()), mColorLabel(new QLabel(this)), mSelectionTreeView(new SelectionTreeView(this, true)), mUpdateTimer(new QTimer(this))
    {
        setAttribute(Qt::WA_DeleteOnClose);

        mColorLabel->setFixedSize(25, 25);
        mColorLabel->setAutoFillBackground(true);

        SelectionTreeProxyModel* proxyModel = new SelectionTreeProxyModel(this);
        proxyModel->setSourceModel(new ModuleModel(this));
        mSelectionTreeView->setModel(proxyModel);

        QPushButton* closeButton = new QPushButton("Close", this);
        connect(closeButton, &QPushButton::clicked, this, &QDialog::close);

        QHBoxLayout* colorLayout = new QHBoxLayout;
        colorLayout->addStretch();
        colorLayout->addWidget(mColorLabel);
        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->addLayout(colorLayout);
        layout->addWidget(mSelectionTreeView);
        layout->addWidget(closeButton);

        mUpdateTimer->setSingleShot(true);
        connect(mUpdateTimer, &QTimer::timeout, this, &GroupingContentDialog::handleUpdateTimeout);

        connect(gNetlistRelay, &NetlistRelay::groupingNameChanged, this, &GroupingContentDialog::handleGroupingNameChanged);
        connect(gNetlistRelay, &NetlistRelay::groupingColorChanged, this, &GroupingContentDialog::handleGroupingColorChanged);
        connect(gNetlistRelay, &NetlistRelay::groupingGateAssigned, this, &GroupingContentDialog::handleGroupingContentChanged);
        connect(gNetlistRelay, &NetlistRelay::groupingGateRemoved, this, &GroupingContentDialog::handleGroupingContentChanged);
        connect(gNetlistRelay, &NetlistRelay::groupingNetAssigned, this, &GroupingContentDialog::handleGroupingContentChanged);
        connect(gNetlistRelay, &NetlistRelay::groupingNetRemoved, this, &GroupingContentDialog::handleGroupingContentChanged);
        connect(gNetlistRelay, &NetlistRelay::groupingModuleAssigned, this, &GroupingContentDialog::handleGroupingContentChanged);
        connect(gNetlistRelay, &NetlistRelay::groupingModuleRemoved, this, &GroupingContentDialog::handleGroupingContentChanged);
        connect(gNetlistRelay, &NetlistRelay::groupingRemoved, this, &GroupingContentDialog::handleGroupingRemoved);
        connect(FileManager::get_instance(), &FileManager::fileAboutToClose, this, &QDialog::close);

        refresh();
    }

    Grouping* GroupingContentDialog::grouping() const
    {
        return mGrouping;
    }

    void GroupingContentDialog::handleGroupingNameChanged(Grouping* grp)
    {
        if (grp == mGrouping)
        {
            updateTitle();
        }
    }

    void GroupingContentDialog::handleGroupingColorChanged(Grouping* grp)
    {
        if (grp == mGrouping)
        {
            updateColor();
        }
    }

    void GroupingContentDialog::handleGroupingContentChanged(Grouping* grp)
    {
        // rebuild the tree once after a burst of changes instead of once per change
        if (grp == mGrouping && !mUpdateTimer->isActive())
        {
            mUpdateTimer->start(0);
        }
    }

    void GroupingContentDialog::handleGroupingRemoved(Grouping* grp)
    {
        if (grp == mGrouping)
        {
            close();
        }
    }

    void GroupingContentDialog::handleUpdateTimeout()
    {
        if (isVisible())
        {
            refresh();
        }
    }

    void GroupingContentDialog::refresh()
    {
        // the lock is held while the netlist is being changed, possibly by a Python thread, so it must not be read yet
        if (LayoutLockerManager::instance()->isLocked())
        {
            mUpdateTimer->start(100);
            return;
        }
        if (!gNetlist || gNetlist->get_grouping_by_id(mGroupingId) != mGrouping)
        {
            return;
        }

        updateTitle();
        updateColor();
        mSelectionTreeView->populate(true, mGroupingId);
    }

    void GroupingContentDialog::updateTitle()
    {
        setWindowTitle(QString("Content of %1 (ID: %2)").arg(QString::fromStdString(mGrouping->get_name())).arg(mGroupingId));
    }

    void GroupingContentDialog::updateColor()
    {
        mColorLabel->setStyleSheet("background-color: " + GroupingTableEntry(mGrouping).color().name());
    }
}
