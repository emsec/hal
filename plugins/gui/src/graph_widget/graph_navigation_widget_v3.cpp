#include "gui/graph_widget/graph_navigation_widget_v3.h"
#include "gui/gui_globals.h"
#include "gui/graph_widget/layouters/graph_layouter.h"
#include "gui/context_manager_widget/context_manager_widget.h"
#include "gui/graph_widget/items/nodes/graphics_node.h"
#include <QHeaderView>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QLabel>

namespace hal {

    const int GraphNavigationWidgetV3::sDefaultColumnWidth[] = {250, 50, 100, 80, 250};

    GraphNavigationWidgetV3::GraphNavigationWidgetV3(QWidget *parent)
        : QWidget(parent), mViaNet(nullptr),
          mDirection(SelectionRelay::Subfocus::None)
    {
        QStringList headerLabels;
        headerLabels << "Name" << "ID" << "Type" << "Pin" << "Parent Module";

        QVBoxLayout* layTop = new QVBoxLayout(this);
        mNavigateFrame = new QFrame(this);
        mNavigateFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
        QVBoxLayout* layNavigateView = new QVBoxLayout(mNavigateFrame);
        layNavigateView->addWidget(new QLabel("Navigate to ...", mNavigateFrame));
        mNavigateWidget = new QTableWidget(mNavigateFrame);
        mNavigateWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        mNavigateWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
        mNavigateWidget->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
        mNavigateWidget->horizontalHeader()->setStretchLastSection(false);
        mNavigateWidget->verticalHeader()->hide();
        mNavigateWidget->setColumnCount(5);
        mNavigateWidget->setHorizontalHeaderLabels(headerLabels);
        connect(mNavigateWidget,&QTableWidget::cellDoubleClicked,this,&GraphNavigationWidgetV3::handleSelectionChanged);
        layNavigateView->addWidget(mNavigateWidget);
        layTop->addWidget(mNavigateFrame);

        mAddToViewFrame = new QFrame(this);
        mAddToViewFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
        QVBoxLayout* layAddtoView = new QVBoxLayout(mAddToViewFrame);
        layAddtoView->addWidget(new QLabel("Add to view ...", mAddToViewFrame));
        mAddToViewWidget = new QTreeWidget(mAddToViewFrame);
        mAddToViewWidget->setSelectionMode(QAbstractItemView::MultiSelection);
        mAddToViewWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
        mAddToViewWidget->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
        mAddToViewWidget->header()->setStretchLastSection(false);
        mAddToViewWidget->setColumnCount(5);
        mAddToViewWidget->setHeaderLabels(headerLabels);
        mAddToViewWidget->setAllColumnsShowFocus(true);
        layAddtoView->addWidget(mAddToViewWidget);
        layTop->addWidget(mAddToViewFrame);
    }

    void GraphNavigationWidgetV3::viaNetByNode()
    {
        mViaNet = nullptr;
        if (mOrigin.isNull()) return;

        const NodeBoxes& boxes =
                gContentManager->getContextManagerWidget()->
                getCurrentContext()->getLayouter()->boxes();
        NodeBox* nbox = boxes.boxForNode(mOrigin);
        if (!nbox) return;

        u32 netId = 0;
        switch (mDirection) {
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

    QStringList GraphNavigationWidgetV3::moduleEntry(Module* m, Endpoint* ep)
    {
        Module* pm = m->get_parent_module();
        QString pname = pm ? QString::fromStdString(pm->get_name()) : QString("top level");
        QString mtype = QString::fromStdString(m->get_type());
        if (mtype.isEmpty())
            mtype = "module";
        else
            mtype += " (module)";

        return QStringList()
                << QString::fromStdString(m->get_name())
                << QString::number(m->get_id())
                << mtype
                << QString::fromStdString(ep->get_pin())
                << pname;
    }

    QStringList GraphNavigationWidgetV3::gateEntry(Gate* g, Endpoint *ep)
    {
        return QStringList()
                << QString::fromStdString(g->get_name())
                << QString::number(g->get_id())
                << QString::fromStdString(g->get_type()->get_name())
                << QString::fromStdString(ep->get_pin())
                << QString::fromStdString(g->get_module()->get_name());
    }

    bool GraphNavigationWidgetV3::addToViewItem(Endpoint *ep)
    {
        Gate* g = ep->get_gate();
        if (!g) return false;

        QStringList fields = gateEntry(g,ep);
        QTreeWidgetItem* item = new QTreeWidgetItem(fields);
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
            fields = moduleEntry(pm,ep);
            QTreeWidgetItem* parentItem = new QTreeWidgetItem(fields);
            parentItem->addChild(item);
            mListedModules.insert(pm, parentItem);
            item = parentItem;
            pm = pm->get_parent_module();
        }
        mAddToViewWidget->addTopLevelItem(item);
        return true;
    }

    bool GraphNavigationWidgetV3::addNavigateItem(Endpoint* ep)
    {
        Gate* g = ep->get_gate();
        if (!g) return false;

        const NodeBoxes& boxes =
                gContentManager->getContextManagerWidget()->
                getCurrentContext()->getLayouter()->boxes();
        const NodeBox* nbox = boxes.boxForGate(g);
        if (!nbox) return false;

        QStringList fields;

        switch (nbox->type()) {
        case Node::None:
            return false;
        case Node::Gate:
            fields = gateEntry(g,ep);
            break;
        case Node::Module:
            Module* m = gNetlist->get_module_by_id(nbox->id());
            Q_ASSERT(m);
            fields = moduleEntry(m,ep);
            break;
        }

        int n = mNavigateWidget->rowCount();
        mNavigateNodes.append(nbox->getNode());
        mNavigateWidget->insertRow(n);
        for (int icol=0; icol < fields.size(); icol++)
            mNavigateWidget->setItem(n,icol, new QTableWidgetItem(fields.at(icol)));
        return true;
    }

    void GraphNavigationWidgetV3::setup(Node origin, Net* via_net, SelectionRelay::Subfocus direction)
    {
        mOrigin = origin;
        mViaNet = via_net;
        mDirection = direction;
        fillTable();
    }

    void GraphNavigationWidgetV3::setup(SelectionRelay::Subfocus direction)
    {
        mViaNet = nullptr;
        mDirection = direction;

        switch (gSelectionRelay->mFocusType)
        {
        case SelectionRelay::ItemType::Net:
            mViaNet = gNetlist->get_net_by_id(gSelectionRelay->mFocusId);
            break;
        case SelectionRelay::ItemType::Gate:
        {
            Gate* g = gNetlist->get_gate_by_id(gSelectionRelay->mFocusId);
            Q_ASSERT(g);
            mOrigin = Node(g->get_id(),Node::Gate);
            viaNetByNode();
            break;
        }
        case SelectionRelay::ItemType::Module:
        {
            Module* m = gNetlist->get_module_by_id(gSelectionRelay->mFocusId);
            Q_ASSERT(m);
            mOrigin = Node(m->get_id(),Node::Module);
            viaNetByNode();
            break;
        }
        default:
            break;
        }
        fillTable();
    }

    void GraphNavigationWidgetV3::setModulesInView()
    {
        for (const NodeBox* nbox :
                gContentManager->getContextManagerWidget()->
                getCurrentContext()->getLayouter()->boxes())
        {
            Module* m;
            Gate* g;
            switch (nbox->type()) {
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

    void GraphNavigationWidgetV3::setModuleInView(Module* m)
    {
        if (!m) return;
        mModulesInView.insert(m);
        Module* parentModule = m->get_parent_module();
        setModuleInView(parentModule);
    }

    void GraphNavigationWidgetV3::fillTable()
    {
        mNavigateWidget->clearContents();
        mNavigateWidget->setRowCount(0);
        mNavigateNodes.clear();
        mAddToViewWidget->clear();
        mModulesInView.clear();
        mEndpointNotInView.clear();
        mListedModules.clear();

        if (!mViaNet || mDirection == SelectionRelay::Subfocus::None) return;
        setModulesInView();
        bool hasNavigateEntries = false;

        for (Endpoint* ep : (mDirection == SelectionRelay::Subfocus::Left)
             ? mViaNet->get_sources()
             : mViaNet->get_destinations())
        {
            if (addNavigateItem(ep))
            {
                hasNavigateEntries = true;
                mNavigateWidget->setCurrentCell(0,0);
                mNavigateWidget->selectRow(0);
            }
            else
                mEndpointNotInView.append(ep);
        }

        if (hasNavigateEntries)
            mNavigateFrame->show();
        else
            mNavigateFrame->hide();

        bool hasToViewEntries = false;
        if (!mEndpointNotInView.isEmpty())
        {
            for (Endpoint* ep : mEndpointNotInView)
            {
                if (addToViewItem(ep))
                    hasToViewEntries = true;
            }
        }

        if (hasToViewEntries)
        {
            mAddToViewWidget->expandAll();
            mAddToViewFrame->show();
        }
        else
            mAddToViewFrame->hide();

        resizeToFit();
    }

    void GraphNavigationWidgetV3::resizeToFit()
    {
        if (mNavigateFrame->isVisible())
        {
            mNavigateWidget->resizeColumnsToContents();

            int width = 4;
            for (int i=0; i<mNavigateWidget->columnCount(); i++)
            {
                mNavigateWidget->setColumnWidth(i, sDefaultColumnWidth[i]);
                width += sDefaultColumnWidth[i];
            }
            int height = 24;
            for (int i=0; i<mNavigateWidget->rowCount(); i++)
                height += 24;

            mNavigateWidget->setFixedSize(width,height);
        }
            /*
        int width = 2;
        if (mNavigateFrame->isVisible())
        {
            for (int i=0; i<mNavigateWidget->columnCount(); i++)
            {
                mNavigateWidget->resizeColumnToContents(i);
                width += mNavigateWidget->columnWidth(i);
            }
        }

        // Qt apparently needs these 2 pixels extra, otherwise you get scollbars


        int width = verticalScrollBar()->isVisible() ? verticalScrollBar()->width() : 2;
        for (int i = 0; i < columnCount(); i++)
        {
            resizeColumnToContents(i);
            width += columnWidth(i);
        }

        int height = header()->height() + 2;
        height += sumRowHeights(this->invisibleRootItem());

        int MAXIMUM_ALLOWED_HEIGHT = std::min(500, static_cast<QWidget*>(parent())->height());
        int MAXIMUM_ALLOWED_WIDTH = std::min(700, static_cast<QWidget*>(parent())->width());
        setFixedWidth((width > MAXIMUM_ALLOWED_WIDTH) ? MAXIMUM_ALLOWED_WIDTH : width);
        setFixedHeight((height > MAXIMUM_ALLOWED_HEIGHT) ? MAXIMUM_ALLOWED_HEIGHT : height);
        */
    }

    void GraphNavigationWidgetV3::keyPressEvent(QKeyEvent* event)
    {
        if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return ||
                (event->key() == Qt::Key_Right && mDirection == SelectionRelay::Subfocus::Right) ||
                (event->key() == Qt::Key_Left && mDirection == SelectionRelay::Subfocus::Left))
        {
            bool navigationHasFocus = true;
            if (!mAddToViewFrame->isVisible())     navigationHasFocus = true;
            else if (!mNavigateFrame->isVisible()) navigationHasFocus = false;
            else if (mNavigateWidget->hasFocus())  navigationHasFocus = true;
            else if (mAddToViewWidget->hasFocus()) navigationHasFocus = false;

            if (navigationHasFocus)
                handleNavigateSelected(mNavigateWidget->currentRow(),0);
            else
            {

            }
        }

        if (event->key() == Qt::Key_Escape || event->key() == Qt::Key_Left)
        {
            Q_EMIT closeRequested();
            Q_EMIT resetFocus();
            mViaNet = nullptr;
            return;
        }

        return QWidget::keyPressEvent(event);
    }

    void GraphNavigationWidgetV3::handleNavigateSelected(int irow, int icol)
    {
        Q_UNUSED(icol);
        QSet<u32> navigateGates;
        QSet<u32> navigateModules;
        const Node& nd = mNavigateNodes.at(irow);
        switch (nd.type()) {
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
}
