#include "dot_viewer/interaction/solve_fsm_interaction.h"
#include "gui/gui_globals.h"
#include "gui/gui_api/gui_api.h"
#include <QRegularExpression>
#include <QGraphicsView>>
#include "QGVCore/QGVEdge.h"
#include "QGVCore/QGVNode.h"
#include <QPixmap>
#include <QIcon>
#include "gui/grouping/grouping_manager_widget.h"
#include "hal_core/netlist/boolean_function/solver.h"

namespace hal {
    SolveFsmInteractionRegistration SolveFsmInteractionRegistration::sRegistration;

    QGVInteraction* constructSolveFsmInteraction(QGVScene* parent)
    {
        return new SolveFsmInteraction(parent);
    }

    SolveFsmInteractionRegistration::SolveFsmInteractionRegistration()
    {
        QGVInteraction::registerConstructorForPlugin("solve_fsm", constructSolveFsmInteraction);
    }

    SolveFsmInteraction::SolveFsmInteraction(QGVScene *parent)
        : QGVInteraction(parent), mScene(parent)
    {
        connect(gSelectionRelay, &SelectionRelay::selectionChanged, this, &SolveFsmInteraction::handleHALSelectionChanged);
        connect(parent, &QGraphicsScene::selectionChanged, this, &SolveFsmInteraction::handleQGVSelectionChanged);
        connect(parent, &QGVScene::edgeContextMenu, this, &SolveFsmInteraction::handleEdgeContextMenu);
        connect(parent, &QGraphicsScene::changed, this, &SolveFsmInteraction::handleSceneChanged);
    }

    void SolveFsmInteraction::handleSceneChanged(const QList<QRectF>& changedArea)
    {
        Q_UNUSED(changedArea);

        std::cerr << "handleSceneChanged ";
        if (!mItemArea.isNull())
        {
            resetEdgeHighlight();

            std::cerr << "item Area [" << mItemArea.top() << ":" << mItemArea.bottom() << "," << mItemArea.left() << ":" << mItemArea.right() << "]";
            for (QGraphicsView* view : mScene->views())
                view->ensureVisible(mItemArea);
            mItemArea = QRectF(); // call ensure visible only once
        }
        std::cerr << std::endl;
    }

    void SolveFsmInteraction::registerEdge(QGVEdge* edge)
    {
        if (mItemArea.isNull())
            mItemArea = edge->boundingRect();
        else
            mItemArea = mItemArea.united(edge->boundingRect());
        QRegularExpression reNet("net_(\\d+)\\b");
        QRegularExpressionMatchIterator itNet = reNet.globalMatch(edge->label());
        while (itNet.hasNext())
        {
            QRegularExpressionMatch mNet = itNet.next();
            bool ok = false;
            u32 netId = mNet.captured(1).toUInt(&ok);
            if (!ok || !netId) continue;
            Net* n = gNetlist->get_net_by_id(netId);
            if (!n) continue;
            mNetHash.insert(netId, edge);
        }
    }

    void SolveFsmInteraction::registerNode(QGVNode* node)
    {
        if (mItemArea.isNull())
            mItemArea = node->boundingRect();
        else
            mItemArea = mItemArea.united(node->boundingRect());
    }

    QMap<Net*, int> SolveFsmInteraction::transitionValues(QGVEdge* edge) const
    {
        if (!edge || edge->label().isNull())
            return QMap<Net*,int>();
        Result<BooleanFunction> resBf = BooleanFunction::from_string(edge->label().toStdString());
        if (!resBf.is_ok())
            return QMap<Net*,int>();
        Result<BooleanFunction> resEq = BooleanFunction::Eq(resBf.get(),BooleanFunction::Const(BooleanFunction::ONE),1);
        if (!resEq.is_ok())
            return QMap<Net*,int>();
        auto config = hal::SMT::QueryConfig();
        config.with_model_generation();
        auto resSv = SMT::Solver({SMT::Constraint(resEq.get())}).query(config);
        if (!resSv.is_ok())
            return QMap<Net*,int>();
        auto solverResult = resSv.get();
        if (!solverResult.model.has_value())
            return QMap<Net*,int>();
        auto model = solverResult.model.value();
        QMap<Net*,int> retval;
        for (auto jt = model.model.begin(); jt != model.model.end(); ++jt)
        {
            QString netName = QString::fromStdString(jt->first);
            if (!netName.startsWith("net_")) continue;
                netName.remove(0,4);
            bool ok = false;
            u32 netId = netName.toUInt(&ok);
            if (!ok) continue;
                Net* n = gNetlist->get_net_by_id(netId);
            if (!n) continue;
            retval[n] = std::get<0>(jt->second);
        }
        return retval;
    }

    void SolveFsmInteraction::handleEdgeContextMenu(QGVEdge* edge)
    {
        resetEdgeHighlight();

        QPixmap pixOne(32,32);
        pixOne.fill(QColor::fromRgb(255,0,0));
        QPixmap pixZero(32,32);
        pixZero.fill(QColor::fromRgb(0,170,255));
        QIcon icons[2] = { QIcon(pixZero), QIcon(pixOne) } ;
        mDisableHandler = true;
        QMenu menu;
        QAction* act = menu.addAction("Nets for transition");
        act->setDisabled(true);
        menu.addSeparator();

        QMap<Net*, int> tvals = SolveFsmInteraction::transitionValues(edge);
        for (auto jt = tvals.begin(); jt != tvals.end(); ++jt)
        {
            Net* n = jt.key();
            act = menu.addAction(icons[jt.value()], QString("   %1 [ID=%2]").arg(n->get_name().c_str()).arg(n->get_id()));
            connect(act, &QAction::triggered, this, [n](){gGuiApi->selectNet(n,true,true);});
        }
        menu.exec(QCursor::pos());
        mDisableHandler = false;
    }

    void SolveFsmInteraction::handleQGVSelectionChanged()
    {
        if (mDisableHandler) return;
        mDisableHandler = true;
        resetEdgeHighlight();

        /// default I-dont-care to all nets
        QHash<Net*,int> netValueHash;
        for (Net* n : gNetlist->get_nets())
            netValueHash[n] = 0;
        bool hasTransitions = false;

        for (QGraphicsItem* it : mScene->selectedItems())
        {
            QGVEdge* edge = dynamic_cast<QGVEdge*>(it);
            QMap<Net*, int> tvals = SolveFsmInteraction::transitionValues(edge);
            for (auto jt = tvals.begin(); jt != tvals.end(); ++jt)
            {
                netValueHash[jt.key()] = jt.value() + 1;
                hasTransitions = true;
            }

        }

        GroupingTableModel* gtm = gContentManager->getGroupingManagerWidget()->getModel();
        const char* color[] = {"#707071", "#102080", "#802010" };
        static const char* grpNames[3] = {"x state", "0 state", "1 state"};
        Grouping* grp[3];
        for (int i=0; i<3; i++)
        {
            grp[i] = gtm->groupingByName(grpNames[i]);
            if (grp[i])
            {
                if (!hasTransitions)
                    gNetlist->delete_grouping(grp[i]);
            }
            else
            {
                if (hasTransitions)
                {
                    grp[i] = gNetlist->create_grouping(grpNames[i]);
                    gtm->recolorGrouping(grp[i]->get_id(),QColor(color[i]));
                }
            }
        }

        if (hasTransitions)
        {
            for (auto it = netValueHash.constBegin(); it != netValueHash.constEnd(); ++it)
            {
                Q_ASSERT(it.value() >= 0 && it.value() <= 2);
                grp[it.value()]->assign_net(const_cast<Net*>(it.key()),true);
            }
        }
        mDisableHandler = false;
    }

    void SolveFsmInteraction::resetEdgeHighlight()
    {
        for (QGVEdge* edge : mNetHash.values())
            edge->setHightlight(QString());
    }

    void SolveFsmInteraction::handleHALSelectionChanged(void* sender)
    {
        if (mDisableHandler) return;
        Q_UNUSED(sender);
        if (!mScene) return;
        resetEdgeHighlight();

        mDisableHandler = true;
        mScene->clearSelection();
        bool first = true;
        for (u32 netId : gSelectionRelay->selectedNetsList())
        {
            for (QGVEdge * edge : mNetHash.values(netId))
            {
                edge->setHightlight(QString("net_%1").arg(netId));
                if (first)
                    for (QGraphicsView* gv : mScene->views())
                        gv->ensureVisible(edge);
                first = false;
            }
            if (!first) break;
        }
        mDisableHandler = false;
    }
}
