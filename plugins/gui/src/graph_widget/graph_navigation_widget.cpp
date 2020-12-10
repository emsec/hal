#include "gui/graph_widget/graph_navigation_widget.h"

#include "gui/context_manager_widget/context_manager_widget.h"
#include "gui/graph_widget/items/nodes/graphics_node.h"
#include "gui/graph_widget/layouters/graph_layouter.h"
#include "gui/gui_globals.h"

#include <QGridLayout>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLabel>
#include <QScrollBar>
#include <QVBoxLayout>

namespace hal
{
    void GraphNavigationTableWidget::keyPressEvent(QKeyEvent* event)
    {
        Q_ASSERT(mNavigationWidget);

        if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return || (event->key() == Qt::Key_Right && mNavigationWidget->direction() == SelectionRelay::Subfocus::Right)
            || (event->key() == Qt::Key_Left && mNavigationWidget->direction() == SelectionRelay::Subfocus::Left))
        {
            Q_EMIT cellDoubleClicked(currentRow(), 0);
            return;
        }

        if (event->key() == Qt::Key_Escape || (event->key() == Qt::Key_Left && mNavigationWidget->direction() == SelectionRelay::Subfocus::Right)
            || (event->key() == Qt::Key_Right && mNavigationWidget->direction() == SelectionRelay::Subfocus::Left))
        {
            mNavigationWidget->closeRequest();
            return;
        }

        if (event->key() == Qt::Key_Tab && mNavigationWidget->hasBothWidgets())
        {
            mNavigationWidget->toggleWidget();
            return;
        }

        QTableWidget::keyPressEvent(event);
    }

    void GraphNavigationTableWidget::focusInEvent(QFocusEvent* event)
    {
        mNavigationWidget->mAddToViewWidget->clearSelection();
        QTableWidget::focusInEvent(event);
    }

    bool GraphNavigationTreeWidget::event(QEvent* ev)
    {
        if (ev->type() == QEvent::KeyPress)
        {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(ev);
            if (keyEvent->key() == Qt::Key_Tab)
            {
                mNavigationWidget->toggleWidget();
                return true;
            }
        }
        return QTreeWidget::event(ev);
    }

    void GraphNavigationTreeWidget::focusInEvent(QFocusEvent* ev)
    {
        mNavigationWidget->mNavigateWidget->clearSelection();
        QTreeWidget::focusInEvent(ev);
    }

    void GraphNavigationTreeWidget::keyPressEvent(QKeyEvent* ev)
    {
        Q_ASSERT(mNavigationWidget);

        if (ev->key() == Qt::Key_Enter || ev->key() == Qt::Key_Return || (ev->key() == Qt::Key_Right && mNavigationWidget->direction() == SelectionRelay::Subfocus::Right)
            || (ev->key() == Qt::Key_Left && mNavigationWidget->direction() == SelectionRelay::Subfocus::Left))
        {
            Q_EMIT itemDoubleClicked(currentItem(), 0);
            return;
        }

        if (ev->key() == Qt::Key_Escape || (ev->key() == Qt::Key_Left && mNavigationWidget->direction() == SelectionRelay::Subfocus::Right)
            || (ev->key() == Qt::Key_Right && mNavigationWidget->direction() == SelectionRelay::Subfocus::Left))
        {
            mNavigationWidget->closeRequest();
            return;
        }

        if (ev->key() == Qt::Key_Tab && mNavigationWidget->hasBothWidgets())
        {
            mNavigationWidget->toggleWidget();
            return;
        }

        QTreeWidget::keyPressEvent(ev);
    }

    QModelIndex GraphNavigationTreeWidget::firstIndex() const
    {
        if (topLevelItemCount() < 1)
            return QModelIndex();
        return indexFromItem(topLevelItem(0), 0);
    }

    QList<QTreeWidgetItem*> GraphNavigationTreeWidget::selectedItemRecursion(QTreeWidgetItem* item) const
    {
        QList<QTreeWidgetItem*> retval;
        if (item->isSelected())
            retval.append(item);
        else
            for (int ichild = 0; ichild < item->childCount(); ichild++)
                retval.append(selectedItemRecursion(item->child(ichild)));
        return retval;
    }

    QList<QTreeWidgetItem*> GraphNavigationTreeWidget::selectedItems() const
    {
        QList<QTreeWidgetItem*> retval;
        int n = topLevelItemCount();
        for (int i = 0; i < n; i++)
            retval += selectedItemRecursion(topLevelItem(i));
        return retval;
    }

    const int GraphNavigationWidget::sDefaultColumnWidth[] = {250, 50, 100, 80, 250};

    GraphNavigationWidget::GraphNavigationWidget(bool onlyNavigate, QWidget* parent)
        : QWidget(parent), mOnlyNavigate(onlyNavigate), mNavigateVisible(false), mAddToViewVisible(false), mViaNet(nullptr), mDirection(SelectionRelay::Subfocus::None)
    {
        QStringList headerLabels;
        headerLabels << "Name"
                     << "ID"
                     << "Type"
                     << "Pin"
                     << "Parent Module";

        QGridLayout* layTop = new QGridLayout(this);
        mNavigateFrame      = new QFrame(this);
        mNavigateFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
        QVBoxLayout* layNavigateView = new QVBoxLayout(mNavigateFrame);
        QLabel* labNavigate          = new QLabel("Navigate to …", mNavigateFrame);
        labNavigate->setFixedHeight(sLabelHeight);
        layNavigateView->addWidget(labNavigate);
        mNavigateWidget = new GraphNavigationTableWidget(this, mNavigateFrame);
        mNavigateWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        mNavigateWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
        mNavigateWidget->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
        mNavigateWidget->horizontalHeader()->setStretchLastSection(false);
        mNavigateWidget->verticalHeader()->hide();
        mNavigateWidget->setColumnCount(5);
        mNavigateWidget->setHorizontalHeaderLabels(headerLabels);
        mNavigateWidget->setShowGrid(false);
        connect(mNavigateWidget, &QTableWidget::cellDoubleClicked, this, &GraphNavigationWidget::handleNavigateSelected);
        layNavigateView->addWidget(mNavigateWidget);
        layTop->addWidget(mNavigateFrame);

        mAddToViewFrame = new QFrame(this);
        mAddToViewFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
        QVBoxLayout* layAddtoView = new QVBoxLayout(mAddToViewFrame);
        QLabel* labAddToView      = new QLabel("Add to view …", mAddToViewFrame);
        labAddToView->setFixedHeight(sLabelHeight);
        layAddtoView->addWidget(labAddToView);
        mAddToViewWidget = new GraphNavigationTreeWidget(this, mAddToViewFrame);
        mAddToViewWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
        mAddToViewWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
        mAddToViewWidget->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
        mAddToViewWidget->header()->setStretchLastSection(false);
        mAddToViewWidget->setColumnCount(5);
        mAddToViewWidget->setHeaderLabels(headerLabels);
        // mAddToViewWidget->setAllColumnsShowFocus(true);
        connect(mAddToViewWidget, &QTreeWidget::itemDoubleClicked, this, &GraphNavigationWidget::handleAddToViewSelected);
        layAddtoView->addWidget(mAddToViewWidget);
        layTop->addWidget(mAddToViewFrame);
    }

    void GraphNavigationWidget::closeRequest()
    {
        Q_EMIT closeRequested();
        Q_EMIT resetFocus();
        mViaNet    = nullptr;
        mOrigin    = Node();
        mDirection = SelectionRelay::Subfocus::None;
    }

    void GraphNavigationWidget::viaNetByNode()
    {
        mViaNet = nullptr;
        if (mOrigin.isNull())
            return;

        const NodeBoxes& boxes = gContentManager->getContextManagerWidget()->getCurrentContext()->getLayouter()->boxes();
        NodeBox* nbox          = boxes.boxForNode(mOrigin);
        if (!nbox)
            return;

        u32 netId = 0;
        switch (mDirection)
        {
            case SelectionRelay::Subfocus::None:
                return;
            case SelectionRelay::Subfocus::Left:
                netId = nbox->item()->inputNets().at(gSelectionRelay->mSubfocusIndex);
                break;
            case SelectionRelay::Subfocus::Right:
                netId = nbox->item()->outputNets().at(gSelectionRelay->mSubfocusIndex);
                break;
        }

        mViaNet = gNetlist->get_net_by_id(netId);
    }

    QStringList GraphNavigationWidget::moduleEntry(Module* m, Endpoint* ep)
    {
        Module* pm    = m->get_parent_module();
        QString pname = pm ? QString::fromStdString(pm->get_name()) : QString("top level");
        QString mtype = QString::fromStdString(m->get_type());
        if (mtype.isEmpty())
            mtype = "module";
        else
            mtype += " (module)";

        return QStringList() << QString::fromStdString(m->get_name()) << QString::number(m->get_id()) << mtype << QString::fromStdString(ep->get_pin()) << pname;
    }

    QStringList GraphNavigationWidget::gateEntry(Gate* g, Endpoint* ep)
    {
        return QStringList() << QString::fromStdString(g->get_name()) << QString::number(g->get_id()) << QString::fromStdString(g->get_type()->get_name()) << QString::fromStdString(ep->get_pin())
                             << QString::fromStdString(g->get_module()->get_name());
    }

    QTreeWidgetItem* GraphNavigationWidget::itemFactory(const QStringList& fields, const Node& nd)
    {
        QTreeWidgetItem* retval = new QTreeWidgetItem(fields);
        for (int i = 0; i < fields.size(); i++)
        {
            if (i == 1)
                retval->setTextAlignment(i, Qt::AlignRight | Qt::AlignVCenter);
            else
                retval->setTextAlignment(i, Qt::AlignLeft | Qt::AlignVCenter);
        }
        mAddToViewNodes.insert(retval, nd);
        return retval;
    }

    bool GraphNavigationWidget::addToViewItem(Endpoint* ep)
    {
        Gate* g = ep->get_gate();
        if (!g)
            return false;

        QStringList fields    = gateEntry(g, ep);
        QTreeWidgetItem* item = itemFactory(fields, Node(g->get_id(), Node::Gate));

        Module* pm = g->get_module();
        while (pm && !mModulesInView.contains(pm))
        {
            auto itMod = mListedModules.find(pm);
            if (itMod != mListedModules.end())
            {
                // parent already in tree
                itMod.value()->addChild(item);
                return true;
            }
            fields                      = moduleEntry(pm, ep);
            QTreeWidgetItem* parentItem = itemFactory(fields, Node(pm->get_id(), Node::Module));
            parentItem->addChild(item);
            mListedModules.insert(pm, parentItem);
            item = parentItem;
            pm   = pm->get_parent_module();
        }
        mAddToViewWidget->addTopLevelItem(item);
        return true;
    }

    bool GraphNavigationWidget::addNavigateItem(Endpoint* ep)
    {
        Gate* g = ep->get_gate();
        if (!g)
            return false;

        const NodeBoxes& boxes = gContentManager->getContextManagerWidget()->getCurrentContext()->getLayouter()->boxes();
        const NodeBox* nbox    = boxes.boxForGate(g);
        if (!nbox)
            return false;

        QStringList fields;

        switch (nbox->type())
        {
            case Node::None:
                return false;
            case Node::Gate:
                fields = gateEntry(g, ep);
                break;
            case Node::Module:
                Module* m = gNetlist->get_module_by_id(nbox->id());
                Q_ASSERT(m);
                fields = moduleEntry(m, ep);
                break;
        }

        int n = mNavigateWidget->rowCount();
        mNavigateNodes.append(nbox->getNode());
        mNavigateWidget->insertRow(n);
        for (int icol = 0; icol < fields.size(); icol++)
        {
            QTableWidgetItem* cell = new QTableWidgetItem(fields.at(icol));
            if (icol == 1)
                cell->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            else
                cell->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            mNavigateWidget->setItem(n, icol, cell);
        }
        return true;
    }

    void GraphNavigationWidget::setup(Node origin, Net* via_net, SelectionRelay::Subfocus dir)
    {
        mOrigin    = origin;
        mViaNet    = via_net;
        mDirection = dir;
        fillTable();
    }

    void GraphNavigationWidget::setup(SelectionRelay::Subfocus direction)
    {
        mViaNet    = nullptr;
        mDirection = direction;

        switch (gSelectionRelay->mFocusType)
        {
            case SelectionRelay::ItemType::Net:
                mViaNet = gNetlist->get_net_by_id(gSelectionRelay->mFocusId);
                break;
            case SelectionRelay::ItemType::Gate: {
                Gate* g = gNetlist->get_gate_by_id(gSelectionRelay->mFocusId);
                Q_ASSERT(g);
                mOrigin = Node(g->get_id(), Node::Gate);
                viaNetByNode();
                break;
            }
            case SelectionRelay::ItemType::Module: {
                Module* m = gNetlist->get_module_by_id(gSelectionRelay->mFocusId);
                Q_ASSERT(m);
                mOrigin = Node(m->get_id(), Node::Module);
                viaNetByNode();
                break;
            }
            default:
                break;
        }
        fillTable();
    }

    void GraphNavigationWidget::setModulesInView()
    {
        for (const NodeBox* nbox : gContentManager->getContextManagerWidget()->getCurrentContext()->getLayouter()->boxes())
        {
            Module* m;
            Gate* g;
            switch (nbox->type())
            {
                case Node::Module:
                    m = gNetlist->get_module_by_id(nbox->id());
                    Q_ASSERT(m);
                    setModuleInView(m);
                    break;
                case Node::Gate:
                    g = gNetlist->get_gate_by_id(nbox->id());
                    Q_ASSERT(g);
                    setModuleInView(g->get_module());
                    break;
                default:
                    break;
            }
        }
    }

    bool GraphNavigationWidget::hasBothWidgets() const
    {
        return mNavigateVisible && mAddToViewVisible;
    }

    void GraphNavigationWidget::toggleWidget()
    {
        if (!hasBothWidgets())
            return;
        if (mNavigateWidget->hasFocus())
        {
            mAddToViewWidget->setFocus();
            mAddToViewWidget->setCurrentIndex(mAddToViewWidget->firstIndex());
            mNavigateWidget->clearSelection();
        }
        else
        {
            mNavigateWidget->setFocus();
            mNavigateWidget->setCurrentCell(0, 0);
            mNavigateWidget->selectRow(0);
            mAddToViewWidget->clearSelection();
        }
    }

    bool GraphNavigationWidget::isEmpty() const
    {
        return !mNavigateVisible && !mAddToViewVisible;
    }

    void GraphNavigationWidget::setModuleInView(Module* m)
    {
        if (!m)
            return;
        mModulesInView.insert(m);
        Module* parentModule = m->get_parent_module();
        setModuleInView(parentModule);
    }

    void GraphNavigationWidget::fillTable()
    {
        mNavigateWidget->clearContents();
        mNavigateWidget->setRowCount(0);
        mNavigateNodes.clear();
        mAddToViewWidget->clear();
        mAddToViewNodes.clear();
        mModulesInView.clear();
        mEndpointNotInView.clear();
        mListedModules.clear();

        if (!mViaNet || mDirection == SelectionRelay::Subfocus::None)
            return;
        setModulesInView();
        mNavigateVisible = false;

        for (Endpoint* ep : (mDirection == SelectionRelay::Subfocus::Left) ? mViaNet->get_sources() : mViaNet->get_destinations())
        {
            if (addNavigateItem(ep))
                mNavigateVisible = true;
            else
                mEndpointNotInView.append(ep);
        }

        if (mNavigateVisible)
        {
            mNavigateFrame->show();
            mNavigateWidget->setCurrentCell(0, 0);
            mNavigateWidget->selectRow(0);
            mNavigateWidget->setFocus();
        }
        else
            mNavigateFrame->hide();

        mAddToViewVisible = false;
        if (!mEndpointNotInView.isEmpty() && !mOnlyNavigate)
        {
            for (Endpoint* ep : mEndpointNotInView)
            {
                if (addToViewItem(ep))
                    mAddToViewVisible = true;
            }
        }

        if (mAddToViewVisible)
        {
            mAddToViewFrame->show();
            mAddToViewWidget->expandAll();
            mAddToViewWidget->setCurrentIndex(mAddToViewWidget->firstIndex());
            if (!mNavigateVisible)
                mAddToViewWidget->setFocus();
        }
        else
            mAddToViewFrame->hide();

        resizeToFit();
    }

    void GraphNavigationWidget::resizeToFit()
    {
        if (mNavigateVisible)
        {
            mNavigateWidget->resizeColumnsToContents();

            int width = 4;
            for (int i = 0; i < mNavigateWidget->columnCount(); i++)
            {
                mNavigateWidget->setColumnWidth(i, sDefaultColumnWidth[i]);
                width += sDefaultColumnWidth[i];
            }
            int height = 28;
            mNavigateWidget->horizontalHeader()->setMaximumHeight(24);
            for (int i = 0; i < mNavigateWidget->rowCount(); i++)
            {
                height += mNavigateWidget->verticalHeader()->sectionSize(i);
            }
            if (height > sMaxHeight)
                height = sMaxHeight;
            mNavigateWidget->setFixedSize(width, height);
        }

        if (mAddToViewVisible)
        {
            int width = 4;
            for (int i = 0; i < mAddToViewWidget->columnCount(); i++)
            {
                mAddToViewWidget->setColumnWidth(i, sDefaultColumnWidth[i]);
                width += sDefaultColumnWidth[i];
            }
            mAddToViewWidget->setFixedWidth(width);
            mAddToViewWidget->setMaximumHeight(sMaxHeight);
        }
    }

    void GraphNavigationWidget::keyPressEvent(QKeyEvent* ev)
    {
        if (ev->key() == Qt::Key_Tab)
        {
            toggleWidget();
            return;
        }
        QWidget::keyPressEvent(ev);
    }

    void GraphNavigationWidget::focusInEvent(QFocusEvent* ev)
    {
        Q_UNUSED(ev);
        if (mNavigateVisible)
        {
            mNavigateWidget->setFocus();
            mAddToViewWidget->clearSelection();
        }
        else if (mAddToViewVisible)
        {
            mAddToViewWidget->setFocus();
            mNavigateWidget->clearSelection();
        }
    }

    void GraphNavigationWidget::handleNavigateSelected(int irow, int icol)
    {
        Q_UNUSED(icol);
        QSet<u32> navigateGates;
        QSet<u32> navigateModules;
        const Node& nd = mNavigateNodes.at(irow);
        switch (nd.type())
        {
            case Node::Module:
                navigateModules.insert(nd.id());
                break;
            case Node::Gate:
                navigateGates.insert(nd.id());
                break;
            default:
                return;
        }
        Q_EMIT navigationRequested(mOrigin, mViaNet->get_id(), navigateGates, navigateModules);
    }

    void GraphNavigationWidget::handleAddToViewSelected(QTreeWidgetItem* item, int icol)
    {
        Q_UNUSED(icol);
        Q_UNUSED(item);
        QSet<u32> addGates;
        QSet<u32> addModules;

        for (QTreeWidgetItem* selItem : mAddToViewWidget->selectedItems())
        {
            Node nd = mAddToViewNodes.value(selItem);
            switch (nd.type())
            {
                case Node::Module:
                    addModules.insert(nd.id());
                    break;
                case Node::Gate:
                    addGates.insert(nd.id());
                    break;
                default:
                    continue;
            }
        }
        Q_EMIT navigationRequested(mOrigin, mViaNet->get_id(), addGates, addModules);
    }

}    // namespace hal
