#include "gui/graph_widget/layouters/graph_layouter.h"

#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/graph_widget/graphics_factory.h"
#include "gui/graph_widget/graphics_scene.h"
#include "gui/graph_widget/items/nets/arrow_separated_net.h"
#include "gui/graph_widget/items/nets/circle_separated_net.h"
#include "gui/graph_widget/items/nets/labeled_separated_net.h"
#include "gui/graph_widget/items/nets/standard_arrow_net.h"
#include "gui/graph_widget/items/nets/standard_graphics_net.h"
#include "gui/gui_def.h"
#include "gui/gui_globals.h"
#include "gui/implementations/qpoint_extension.h"
#include "gui/selection_details_widget/selection_details_widget.h"
#include "gui/comment_system/comment_entry.h"
#include "gui/comment_system/comment_speech_bubble.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"

#include <QDebug>
#include <QApplication>
#include <QElapsedTimer>
#include <qmath.h>

namespace hal
{
    template<typename T1, typename T2>
    static void storeMax(QMap<T1, T2>& map, T1 key, T2 value)
    {
        if (map.contains(key))
            if (map.value(key) >= value)
                return;

        map.insert(key, value);
    }

    const static qreal sLaneSpacing           = 10;
    const static qreal sHRoadPadding          = 10;
    const static qreal sVRoadPadding          = 10;
    const static qreal sMinimumVChannelWidth  = 20;
    const static qreal sMinimumHChannelHeight = 20;

    GraphLayouter::GraphLayouter(GraphContext* context, QObject* parent)
        : QObject(parent), mScene(new GraphicsScene(this)), mParentContext(context), mDone(false), mRollbackStatus(0), mDumpJunctions(true)
    {
        SelectionDetailsWidget* details = gContentManager->getSelectionDetailsWidget();
        if (details)
            connect(details, &SelectionDetailsWidget::triggerHighlight, mScene, &GraphicsScene::handleHighlight);

        connect(gCommentManager, &CommentManager::entryAboutToBeDeleted, this, &GraphLayouter::handleCommentAboutToDeleted);
        connect(gCommentManager, &CommentManager::entryAdded, this, &GraphLayouter::handleCommentAdded);
        connect(gCommentManager, &CommentManager::entryModified, this, &GraphLayouter::handleCommentAdded); // can be connected to the same func, does the same
    }

    GraphLayouter::~GraphLayouter()
    {
        mScene->deleteLater();
    }

    GraphicsScene* GraphLayouter::scene() const
    {
        return mScene;
    }

    const QMap<Node, QPoint> GraphLayouter::nodeToPositionMap() const
    {
        return mNodeToPositionMap;
    }

    NetLayoutPoint GraphLayouter::positonForNode(const Node& nd) const
    {
        if (nd.isNull()) return NetLayoutPoint();
        auto it = mNodeToPositionMap.find(nd);
        if (it == mNodeToPositionMap.constEnd()) return NetLayoutPoint();
        return NetLayoutPoint(it.value());
    }

    void GraphLayouter::dumpNodePositions(const QPoint& search) const
    {
        QTextStream xout(stderr, QIODevice::WriteOnly);
        xout << "Node positions " << search.x() << " " << search.y() << "\n";
        xout.setFieldAlignment(QTextStream::AlignRight);
        xout.setFieldWidth(4);
        for (auto it = mPositionToNodeMap.constBegin(); it != mPositionToNodeMap.constEnd(); ++it)
        {
            xout << (it.key() == search ? "*" : " ") << it.key().x() << it.key().y() << (it.value().type() == Node::Module ? "M" : "G") << it.value().id() << "\n";
        }
        xout << "=======\n";
    }

    const QMap<QPoint, Node> GraphLayouter::positionToNodeMap() const
    {
        return mPositionToNodeMap;
    }

    void GraphLayouter::setNodePosition(const Node& n, const QPoint& p)
    {
        if (mNodeToPositionMap.contains(n))
        {
            QPoint old_p = mNodeToPositionMap.value(n);
            mPositionToNodeMap.remove(old_p);
        }

        mNodeToPositionMap.insert(n, p);
        mPositionToNodeMap.insert(p, n);

        //manual relayout call needed
    }

    void GraphLayouter::swapNodePositions(const Node& n1, const Node& n2)
    {
        assert(mNodeToPositionMap.contains(n1));
        assert(mNodeToPositionMap.contains(n2));

        QPoint p1 = mNodeToPositionMap.value(n1);
        QPoint p2 = mNodeToPositionMap.value(n2);

        mNodeToPositionMap.insert(n1, p2);    // implicit replace
        mNodeToPositionMap.insert(n2, p1);

        mPositionToNodeMap.insert(p1, n2);
        mPositionToNodeMap.insert(p2, n1);
    }

    void GraphLayouter::removeNodeFromMaps(const Node& n)
    {
        if (mNodeToPositionMap.contains(n))
        {
            QPoint old_p = mNodeToPositionMap.value(n);
            mNodeToPositionMap.remove(n);
            mPositionToNodeMap.remove(old_p);
        }
    }

    QPoint GraphLayouter::gridPointByItem(GraphicsNode* item) const
    {
        QPoint retval(INT_MIN, INT_MIN);
        if (!item)
            return retval;
        const NodeBox* nbox = mBoxes.boxForItem(item);
        if (!nbox)
            return retval;
        return QPoint(nbox->x(), nbox->y());
    }

    int GraphLayouter::minXIndex() const
    {
        return mMinXIndex;
    }

    int GraphLayouter::minYIndex() const
    {
        return mMinYIndex;
    }

    bool GraphLayouter::done() const
    {
        return mDone;
    }

    QVector<qreal> GraphLayouter::xValues() const
    {
        return mXValues;
    }

    QVector<qreal> GraphLayouter::yValues() const
    {
        return mYValues;
    }

    qreal GraphLayouter::maxNodeWidth() const
    {
        return mMaxNodeWidth;
    }

    qreal GraphLayouter::maxNodeHeight() const
    {
        return mMaxNodeHeight;
    }

    qreal GraphLayouter::defaultGridWidth() const
    {
        return mMaxNodeWidth + sMinimumVChannelWidth;
    }

    qreal GraphLayouter::defaultGridHeight() const
    {
        return mMaxNodeHeight + sMinimumHChannelHeight;
    }

    qreal GraphLayouter::gridXposition(int ix) const
    {
        Q_ASSERT(!mXValues.isEmpty());
        int inx = ix - mMinXIndex;
        if (inx < 0)
            return mXValues[0] - inx * defaultGridWidth();
        if (inx < mXValues.size())
            return mXValues[inx];
        return mXValues.last() + (inx - mXValues.size() - 1) * defaultGridWidth();
    }

    qreal GraphLayouter::gridYposition(int iy) const
    {
        Q_ASSERT(!mYValues.isEmpty());
        int inx = iy - mMinYIndex;
        if (inx < 0)
            return mYValues[0] - inx * defaultGridHeight();
        if (inx < mYValues.size())
            return mYValues[inx];
        return mYValues.last() + (inx - mYValues.size() - 1) * defaultGridHeight();
    }

    void GraphLayouter::layout()
    {
        QElapsedTimer timer;
        timer.start();
        mParentContext->layoutProgress(0);
        mScene->deleteAllItems();
        clearLayoutData();

        createBoxes();

        mParentContext->layoutProgress(1);
        getWireHash();

        mParentContext->layoutProgress(2);
        findMaxBoxDimensions();
        mParentContext->layoutProgress(3);
        findMaxChannelLanes();
        mParentContext->layoutProgress(4);
        calculateJunctionMinDistance();
        mParentContext->layoutProgress(5);
        calculateGateOffsets();
        mParentContext->layoutProgress(6);
        placeGates();
        mParentContext->layoutProgress(7);
        mDone = true;
        drawNets();
        drawComments();
        updateSceneRect();

        mScene->moveNetsToBackground();
        mScene->handleExternSelectionChanged(nullptr);

#ifdef GUI_DEBUG_GRID
        mScene->debugSetLayouterGrid(xValues(), yValues(), defaultGridHeight(), defaultGridWidth());
#endif
        mRollbackStatus = 0;

        qDebug() << "elapsed time (experimental new) layout [ms]" << timer.elapsed();


        return;

        mDone = true;
    }

    void GraphLayouter::prepareRollback()
    {
        mNodeToPositionRollback = mNodeToPositionMap;
        mRollbackStatus         = 1;
    }

    bool GraphLayouter::canRollback() const
    {
        return mRollbackStatus > 0 && !mNodeToPositionRollback.isEmpty();
    }

    bool GraphLayouter::rollback()
    {
        if (!canRollback())
            return false;
        mRollbackStatus    = -1;
        mNodeToPositionMap = mNodeToPositionRollback;
        mNodeToPositionRollback.clear();
        mPositionToNodeMap.clear();
        for (auto it = mNodeToPositionMap.begin(); it != mNodeToPositionMap.end(); it++)
            mPositionToNodeMap.insert(it.value(), it.key());
        return true;
    }

    void GraphLayouter::clearComments()
    {
        for (CommentSpeechBubble* csb : mCommentBubbles)
        {
            mScene->removeItem(csb);
            delete csb;
        }
        mCommentBubbles.clear();
    }

    void GraphLayouter::clearLayoutData()
    {
        mDone = false;

        mBoxes.clearBoxes();
        clearComments();

        for (const GraphLayouter::Junction* j : mJunctions.values())
            delete j;
        mJunctions.clear();

        mMaxNodeWidthForX.clear();
        mMaxNodeHeightForY.clear();

        mNodeOffsetForX.clear();
        mNodeOffsetForY.clear();

        mMaxLeftIoPaddingForChannelX.clear();
        mMaxRightIoPaddingForChannelX.clear();

        mMinXIndex = INT_MAX;
        mMinYIndex = INT_MAX;

        mMaxXIndex = INT_MIN;
        mMaxYIndex = INT_MIN;

        mXValues.clear();
        mYValues.clear();

        mMaxNodeWidth  = 0;
        mMaxNodeHeight = 0;

        mConnectionMetric.clearAll();
        mJunctionHash.clearAll();
        mEndpointHash.clear();
        mWireHash.clear();
        mJunctionEntries.clear();
        mSeparatedWidth.clear();
        mCoordX.clear();
        mCoordY.clear();
        mJunctionMinDistanceY.clear();
        mWireEndpoint.clear();
        mGlobalInputHash.clear();
        mGlobalOutputHash.clear();
        mNodeBoundingBox = QRect();
        mViewInput.clear();
        mViewOutput.clear();
    }

    void GraphLayouter::createBoxes()
    {
        bool first = true;
        int xmin, xmax, ymin, ymax;
        xmin = ymin = xmax = ymax            = 0;
        QMap<QPoint, Node>::const_iterator i = positionToNodeMap().constBegin();
        while (i != positionToNodeMap().constEnd())
        {
            int x = i.key().x();
            int y = i.key().y();
            if (first || x + 1 > xmax)
                xmax = x + 1;
            if (first || y + 1 > ymax)
                ymax = y + 1;
            if (first || x < xmin)
                xmin = x;
            if (first || y < ymin)
                ymin = y;
            first = false;
            mBoxes.addBox(i.value(), x, y);
            ++i;
        }
        mNodeBoundingBox = QRect(xmin, ymin, xmax - xmin, ymax - ymin);
    }

    bool GraphLayouter::verifyModulePort(Net* n, const Node& modNode, bool isModInput)
    {
        // bypass test for gates
        if (modNode.type() != Node::Module)
            return true;

        Module* m = gNetlist->get_module_by_id(modNode.id());
        Q_ASSERT(m);
        if (isModInput)
        {
            if (std::unordered_set<Net*> nets = m->get_input_nets(); nets.find(n) != nets.end())
            {
                return true;
            }
        }
        else
        {
            if (std::unordered_set<Net*> nets = m->get_output_nets(); nets.find(n) != nets.end())
            {
                return true;
            }
        }

        return false;
    }

    void GraphLayouter::handleCommentAboutToDeleted(CommentEntry* entry)
    {
        Q_UNUSED(entry)
        // if this becomes too slow, go through the bubble list and only update
        // the corresponding gate/module
        clearComments();
        drawComments();
    }

    void GraphLayouter::handleCommentAdded(CommentEntry* entry)
    {
        Q_UNUSED(entry)
        // if this becomes too slow, go through the bubble list and only update
        // the corresponding gate/module
        clearComments();
        drawComments();
    }

    void GraphLayouter::getWireHash()
    {
        for (const u32 id : mParentContext->nets())
        {
            qApp->processEvents(QEventLoop::AllEvents, 100);
            Net* n = gNetlist->get_net_by_id(id);
            if (!n)
                continue;

            QSet<NetLayoutPoint> srcPoints;
            QSet<NetLayoutPoint> dstPoints;

            mWireEndpoint[id] = EndpointList();

            for (const Endpoint* src : n->get_sources())
            {
                // FIND SRC BOX
                const NodeBox* srcBox = mBoxes.boxForGate(src->get_gate());
                if (!srcBox)
                {
                    // not among visible boxes
                    mViewInput.insert(n->get_id());
                    continue;
                }

                if (!verifyModulePort(n, srcBox->getNode(), false))
                    continue;

                NetLayoutPoint srcPnt(srcBox->x() + 1, 2 * srcBox->y());
                srcPoints.insert(srcPnt);
                mWireEndpoint[id].addSource(srcPnt);
            }

            for (const Endpoint* dst : n->get_destinations())
            {
                // find dst box
                const NodeBox* dstBox = mBoxes.boxForGate(dst->get_gate());
                if (!dstBox)
                {
                    // not among visible boxes
                    mViewOutput.insert(n->get_id());
                    continue;
                }

                if (!verifyModulePort(n, dstBox->getNode(), true))
                    continue;

                NetLayoutPoint dstPnt(dstBox->x(), 2 * dstBox->y());
                dstPoints.insert(dstPnt);
                mWireEndpoint[id].addDestination(dstPnt);
            }

            if (isConstNet(n))
                mWireEndpoint[id].setNetType(EndpointList::ConstantLevel);

            // test for global inputs
            EndpointList::EndpointType nType = mWireEndpoint.value(id).netType();
            if ((nType == EndpointList::SingleDestination && dstPoints.size() > 1) || (nType == EndpointList::SourceAndDestination && mViewInput.contains(n->get_id())))
            {
                // global input connects to multiple boxes
                int ypos = mGlobalInputHash.size();
                NetLayoutPoint srcPnt(mNodeBoundingBox.left(), 2 * ypos);
                srcPoints.insert(srcPnt);
                mWireEndpoint[id].addSource(srcPnt);
                mGlobalInputHash[id] = ypos;
                mWireEndpoint[id].setNetType(EndpointList::HasGlobalEndpoint);
                mWireEndpoint[id].setInputArrow();
            }

            if ((nType == EndpointList::SingleSource && srcPoints.size() > 1) || (nType == EndpointList::SourceAndDestination && mViewOutput.contains(n->get_id())))
            {
                // multi-driven global output or global output back coupled to net gate
                int ypos = mGlobalOutputHash.size();
                NetLayoutPoint dstPnt(mNodeBoundingBox.right() + 1, 2 * ypos);
                dstPoints.insert(dstPnt);
                mWireEndpoint[id].addDestination(dstPnt);
                mGlobalOutputHash[id] = ypos;
                mWireEndpoint[id].setNetType(EndpointList::HasGlobalEndpoint);
                mWireEndpoint[id].setOutputArrow();
            }

            const EndpointList& epl = mWireEndpoint.value(id);
            switch (epl.netType())
            {
                case EndpointList::SingleSource:
                case EndpointList::SingleDestination:
                case EndpointList::ConstantLevel: {
                    int ipnt = 0;
                    for (const NetLayoutPoint& pnt : epl)
                    {
                        bool isInput                   = epl.isInput(ipnt++);
                        SeparatedGraphicsNet* net_item = epl.netType() == EndpointList::ConstantLevel
                                                             ? static_cast<SeparatedGraphicsNet*>(new LabeledSeparatedNet(n, QString::fromStdString(n->get_name())))
                                                             : static_cast<SeparatedGraphicsNet*>(new ArrowSeparatedNet(n));
                        if (isInput)
                            mSeparatedWidth[pnt].requireInputSpace(net_item->inputWidth() + sLaneSpacing);
                        else
                        {
                            const NodeBox* nb = mBoxes.boxForPoint(QPoint(pnt.x() - 1, pnt.y() / 2));
                            Q_ASSERT(nb);
                            mSeparatedWidth[pnt].requireOutputSpace(nb->item()->width() + net_item->outputWidth() + sLaneSpacing);
                        }
                        delete net_item;
                    }
                }
                break;
                case EndpointList::SourceAndDestination:
                case EndpointList::HasGlobalEndpoint:
                {
                    NetLayoutConnectionFactory nlcf(srcPoints.toList(), dstPoints.toList());
                    // nlcf.dump(QString("wire %1").arg(id));
                    mConnectionMetric.insert(NetLayoutMetric(id, nlcf.connection), nlcf.connection);
                }
                break;
                default:
                    break;
            }
        }

        /// logic nets -> wire mLanes
        for (auto it = mConnectionMetric.constBegin(); it != mConnectionMetric.constEnd(); ++it)
        {
            u32 id                         = it.key().getId();
            const NetLayoutConnection* nlc = it.value();
            for (const NetLayoutWire& w : *nlc)
            {
                mWireHash[w].append(id);
            }
        }

        /// wires -> junction entries
        for (auto it = mWireHash.constBegin(); it != mWireHash.constEnd(); ++it)
        {
            for (int iend = 0; iend < 2; iend++)
            {
                // iend == 0 =>  horizontal wire: right endpoint   junction: left entry
                NetLayoutPoint pnt = iend ? it.key().endPoint(NetLayoutWire::SourcePoint) : it.key().endPoint(NetLayoutWire::DestinationPoint);
                int idirBase       = it.key().isHorizontal() ? NetLayoutDirection::Left : NetLayoutDirection::Up;
                mJunctionEntries[pnt].mEntries[idirBase + iend] = it.value();
            }
        }

        /// end points -> junction entries
        for (const NodeBox* nbox : mBoxes)
        {
            NetLayoutPoint inPnt(nbox->x(), nbox->y() * 2);
            QList<u32> inpNets = nbox->item()->inputNets();
            mJunctionEntries[inPnt].mEntries[NetLayoutDirection::Right] = nbox->item()->inputNets();
            mEndpointHash[inPnt].setInputPins(nbox->item()->inputNets(), nbox->item()->yTopPinDistance(), nbox->item()->yEndpointDistance());
            NetLayoutPoint outPnt(nbox->x() + 1, nbox->y() * 2);
            mJunctionEntries[outPnt].mEntries[NetLayoutDirection::Left] = nbox->item()->outputNets();
            mEndpointHash[outPnt].setOutputPins(nbox->item()->outputNets(), nbox->item()->yTopPinDistance(), nbox->item()->yEndpointDistance());
        }

        for (auto itGlInp = mGlobalInputHash.constBegin(); itGlInp != mGlobalInputHash.constEnd(); ++itGlInp)
        {
            QList<u32> netIds;
            netIds.append(itGlInp.key());
            NetLayoutPoint pnt(mNodeBoundingBox.left(), 2 * itGlInp.value());
            mJunctionEntries[pnt].mEntries[NetLayoutDirection::Left] = netIds;
            if (!mEndpointHash.contains(pnt))
                mEndpointHash[pnt].setOutputPins(netIds, 0, 0);
        }

        for (auto itGlOut = mGlobalOutputHash.constBegin(); itGlOut != mGlobalOutputHash.constEnd(); ++itGlOut)
        {
            QList<u32> netIds;
            netIds.append(itGlOut.key());
            NetLayoutPoint pnt(mNodeBoundingBox.right() + 1, 2 * itGlOut.value());
            mJunctionEntries[pnt].mEntries[NetLayoutDirection::Right] = netIds;
            if (!mEndpointHash.contains(pnt))
                mEndpointHash[pnt].setInputPins(netIds, 0, 0);
        }

        auto it = mJunctionEntries.constBegin();
        while (it != mJunctionEntries.constEnd() || !mJunctionThreads.isEmpty())
        {
            if (it != mJunctionEntries.constEnd() && mJunctionThreads.size() < QThread::idealThreadCount())
            {
                if (mDumpJunctions)
                    it.value().dumpToFile(it.key());
                JunctionThread* jt = new JunctionThread(it.key(), it.value());
                connect(jt,&QThread::finished,this,&GraphLayouter::handleJunctionThreadFinished);
                mJunctionThreads.append(jt);
                jt->start();
                ++it;
            }
            qApp->processEvents();
        }
    }

    void GraphLayouter::findMaxBoxDimensions()
    {
        for (const NodeBox* box : mBoxes)
        {
            if (box->x() < mMinXIndex)
                mMinXIndex = box->x();
            else if (box->x() > mMaxXIndex)
                mMaxXIndex = box->x();

            if (box->y() < mMinYIndex)
                mMinYIndex = box->y();
            else if (box->y() > mMaxYIndex)
                mMaxYIndex = box->y();

            if (mMaxNodeWidth < box->item()->width())
                mMaxNodeWidth = box->item()->width();

            if (mMaxNodeHeight < box->item()->height())
                mMaxNodeHeight = box->item()->height();

            storeMax(mMaxNodeWidthForX, box->x(), box->item()->width());
            storeMax(mMaxNodeHeightForY, box->y(), box->item()->height());

            storeMax(mMaxRightIoPaddingForChannelX, box->x(), box->inputPadding());
            storeMax(mMaxLeftIoPaddingForChannelX, box->x() + 1, box->outputPadding());
        }
    }

    void GraphLayouter::findMaxChannelLanes()
    {
        // maximum parallel wires for atomic network
        for (auto it = mWireHash.constBegin(); it != mWireHash.constEnd(); ++it)
        {
            const NetLayoutPoint& pnt = it.key().endPoint(NetLayoutWire::SourcePoint);
            unsigned int nw           = it.value().size();
            if (it.key().isHorizontal())
                mCoordY[pnt.y()].testMinMax(nw);
            else
                mCoordX[pnt.x()].testMinMax(nw);
        }

        // maximal roads per junction
        for (auto it = mJunctionHash.constBegin(); it != mJunctionHash.constEnd(); ++it)
        {
            const NetLayoutPoint& pnt = it.key();
            const QRect& rect         = it.value()->rect();
            mCoordX[pnt.x()].testMinMax(rect.left());
            mCoordX[pnt.x()].testMinMax(rect.right());
            mCoordY[pnt.y()].testMinMax(rect.top());
            mCoordY[pnt.y()].testMinMax(rect.bottom());
        }

        // fill gaps in coordinate system if any
        if (!mCoordX.isEmpty())
        {
            auto itx0 = mCoordX.begin();
            for (auto itx1 = itx0 + 1; itx1 != mCoordX.end(); ++itx1)
            {
                for (int x = itx0.key() + 1; x < itx1.key(); x++)
                    mCoordX[x].testMinMax(0);
                itx0 = itx1;
            }
        }
        if (!mCoordY.isEmpty())
        {
            auto ity0 = mCoordY.begin();
            for (auto ity1 = ity0 + 1; ity1 != mCoordY.end(); ++ity1)
            {
                for (int y = ity0.key() + 1; y < ity1.key(); y++)
                    mCoordY[y].testMinMax(0);
                ity0 = ity1;
            }
        }
    }

    void GraphLayouter::calculateJunctionMinDistance()
    {
        for (auto itJun = mJunctionHash.constBegin(); itJun != mJunctionHash.constEnd(); ++itJun)
        {
            const NetLayoutPoint& pnt0  = itJun.key();
            NetLayoutPoint pnt1         = pnt0 + QPoint(0, 1);
            const NetLayoutJunction* j1 = mJunctionHash.value(pnt1);
            if (!j1)
                continue;
            const NetLayoutJunction* j0 = itJun.value();
            auto itEdp                  = mEndpointHash.find(pnt1.isEndpoint() ? pnt1 : pnt0);
            if (itEdp == mEndpointHash.constEnd())
                continue;
            float minDistance = 0;
            int iy            = pnt1.y();
            if (pnt1.isEndpoint())
            {
                // net junction -> endpoint
                minDistance = (j0->rect().bottom() + 1) * sLaneSpacing - itEdp.value().lanePosition(j1->rect().top(), false);
            }
            else
            {
                // endpoint -> net junction
                minDistance = itEdp.value().lanePosition(j0->rect().bottom(), false) + (1 - j1->rect().top()) * sLaneSpacing;
            }
            if (minDistance > mJunctionMinDistanceY[iy])
                mJunctionMinDistanceY[iy] = minDistance;
        }
    }

    void GraphLayouter::calculateGateOffsets()
    {
        QHash<int, float> xInputPadding;
        QHash<int, float> xOutputPadding;
        for (auto itSep = mSeparatedWidth.constBegin(); itSep != mSeparatedWidth.constEnd(); itSep++)
        {
            NetLayoutJunction* jx = mJunctionHash.value(itSep.key());
            if (!jx)
                continue;
            int ix     = itSep.key().x();
            float xinp = jx->rect().right() * sLaneSpacing + itSep.value().mInputSpace;
            float xout = itSep.value().mOutputSpace - jx->rect().left() * sLaneSpacing;
            if (xinp > xInputPadding[ix])
                xInputPadding[ix] = xinp;
            if (xout > xOutputPadding[ix])
                xOutputPadding[ix] = xout;
        }

        int ix0 = mNodeBoundingBox.x();

        float x0 = mCoordX[ix0].preLanes() * sLaneSpacing + sHRoadPadding;
        if (!mGlobalInputHash.isEmpty())
            x0 += 50;
        mCoordX[ix0].setOffset(x0);
        mCoordX[ix0].setPadding(xInputPadding[ix0]);

        mXValues.append(mCoordX.value(ix0).xBoxOffset());

        auto itxLast = mCoordX.begin();
        for (auto itNext = itxLast + 1; itNext != mCoordX.end(); ++itNext)
        {
            ix0        = itxLast.key();
            int ix1    = itNext.key();
            float xsum = 0;

            // loop in case that we span several columns
            for (int ix = ix0; ix < ix1; ix++)
            {
                auto xn = mMaxNodeWidthForX.find(ix);
                if (xn != mMaxNodeWidthForX.end())
                    xsum += xn.value();
            }
            itNext->setOffsetX(itxLast.value(), xsum + 2 * sHRoadPadding, xOutputPadding[ix1], xInputPadding[ix1]);
            mXValues.append(itNext.value().xBoxOffset());
            itxLast = itNext;
        }

        int iy0  = mNodeBoundingBox.y() * 2;
        float y0 = mCoordY[iy0].preLanes() * sLaneSpacing + sVRoadPadding;
        mCoordY[iy0].setOffset(y0);
        mYValues.append(mCoordY.value(iy0).lanePosition(0));
        auto ityLast = mCoordY.begin();
        for (auto itNext = ityLast + 1; itNext != mCoordY.end(); ++itNext)
        {
            iy0     = ityLast.key();
            int iy1 = itNext.key();
            Q_ASSERT(iy1 == iy0 + 1);
            if (iy0 % 2 != 0)
            {
                // netjunction -> endpoint
                itNext->setOffsetYje(ityLast.value(), mJunctionMinDistanceY.value(iy1));
                mYValues.append(itNext.value().lanePosition(0));
            }
            else
            {
                // endpoint -> netjunction
                float ydelta = sVRoadPadding;
                auto yn      = mMaxNodeHeightForY.find(iy0 / 2);
                if (yn != mMaxNodeHeightForY.constEnd())
                    ydelta += yn.value();
                itNext->setOffsetYej(ityLast.value(), ydelta, mJunctionMinDistanceY.value(iy1));
            }
            ityLast = itNext;
        }
    }

    void GraphLayouter::placeGates()
    {
        for (const NodeBox* box : mBoxes)
        {
            box->item()->setPos(mCoordX[box->x()].xBoxOffset(), mCoordY[box->y() * 2].lanePosition(0));
            mScene->addGraphItem(box->item());

            NetLayoutPoint outPnt(box->x() + 1, box->y() * 2);
            QPointF outPos = box->item()->endpointPositionByIndex(0, false);
            mEndpointHash[outPnt].setOutputPosition(outPos);

            NetLayoutPoint inPnt(box->x(), box->y() * 2);
            QPointF inPos = box->item()->endpointPositionByIndex(0, true);
            mEndpointHash[inPnt].setInputPosition(inPos);
        }

        /// place endpoints which are not connected to any box
        for (auto itEp = mEndpointHash.begin(); itEp != mEndpointHash.end(); ++itEp)
        {
            if (itEp.value().lanePosition(0, true) <= 0)
            {
                float px = mCoordX[itEp.key().x()].lanePosition(-1);
                float py = mCoordY[itEp.key().y()].lanePosition(0);
                itEp->setOutputPosition(QPointF(px, py));
            }
        }
    }

    void GraphLayouter::drawComments()
    {
        for (NodeBox* box : mBoxes)
        {
            if (!gCommentManager->contains(box->getNode())) continue;
            CommentEntry* ce = gCommentManager->getEntriesForNode(box->getNode()).at(0);
            CommentSpeechBubble* csb = new CommentSpeechBubble(QString("%1 [%2]").arg(ce->getHeader()).arg(ce->getCreationTime().toString("dd.MM.yy")),
                                                               box->getNode(),mParentContext);
            QPointF pos = box->item()->pos() + QPointF(box->item()->width(),0);
            mScene->addItem(csb);
            mCommentBubbles.append(csb);
            csb->setPos(pos);
        }
    }

    void GraphLayouter::drawNets()
    {
        // lane for given wire and net id

        for (auto it = mWireHash.constBegin(); it != mWireHash.constEnd(); ++it)
        {
            int ilane = 0;
            for (u32 id : it.value())
                mLaneMap[id].insert(it.key(), ilane++);
        }

        int netCount     = mParentContext->nets().size();
        int percentCount = netCount / 93;
        int doneCount    = 0;

        mNetsToDraw = mParentContext->nets();
        mNetIterator = mNetsToDraw.constBegin();



        enum LoopState { LoopInit, CanStartThread, WaitForSlot, WaitForLastThread, LoopDone } loopState = LoopInit;
        while (loopState != LoopDone)
        {
            if (mNetIterator != mNetsToDraw.constEnd())
            {
                if  (mDrawNetThreads.size() < QThread::idealThreadCount())
                    loopState = CanStartThread;
                else
                    loopState = WaitForSlot;
            }
            else
            {
                if (mDrawNetThreads.isEmpty())
                    loopState = LoopDone;
                else
                    loopState = WaitForLastThread;
            }

            if (loopState == LoopDone)
                break;

            if (loopState == WaitForLastThread || loopState == WaitForSlot)
            {
                qApp->processEvents();
                continue;
            }

            Q_ASSERT(loopState == CanStartThread);
            u32 id = *(mNetIterator++);

            Net* n = gNetlist->get_net_by_id(id);
            if (!n)
                continue;

            const EndpointList& epl = mWireEndpoint.value(id);
            bool regularNet         = false;

            switch (epl.netType())
            {
                case EndpointList::NoEndpoint:
                    ++doneCount;
                    break;
                case EndpointList::SingleSource:
                case EndpointList::SingleDestination:
                case EndpointList::ConstantLevel:
                    ++doneCount;
                    drawNetsIsolated(id, n, epl);
                    break;
                    ;
                default:
                    regularNet = true;
                    break;
            }

            if (!regularNet)
                continue;

            ++doneCount;
            if (percentCount)
            {
                if (doneCount % percentCount == 0)
                    mParentContext->layoutProgress(7 + doneCount / percentCount);
            }
            else
                mParentContext->layoutProgress(7 + (int)floor(92. * doneCount / netCount));

            DrawNetThread* dnt = new DrawNetThread(id,this);
            connect(dnt,&QThread::finished,this,&GraphLayouter::handleDrawNetThreadFinished);
            mDrawNetThreads.append(dnt);
            dnt->start();
        }
    }

    void GraphLayouter::handleJunctionThreadFinished()
    {
        JunctionThread* jt = static_cast<JunctionThread*>(sender());
        mJunctionHash.insert(jt->mNetLayoutPoint,jt->mJunction);
        mJunctionThreads.removeAll(jt);
        jt->deleteLater();
    }

    void GraphLayouter::handleDrawNetThreadFinished()
    {
        DrawNetThread* dnt = static_cast<DrawNetThread*>(sender());
        Net* n = gNetlist->get_net_by_id(dnt->id());
        const EndpointList& epl = mWireEndpoint.value(dnt->id());

        GraphicsNet* graphicsNet = nullptr;
        switch (epl.netType())
        {
            case EndpointList::HasGlobalEndpoint:
                if (epl.hasInputArrow())
                {
                    StandardArrowNet* san = new StandardArrowNet(n, dnt->mLines);
                    graphicsNet           = san;
                    int yGridPos          = mGlobalInputHash.value(dnt->id(), -1);
                    Q_ASSERT(yGridPos >= 0);
                    const EndpointCoordinate& epc = mEndpointHash.value(QPoint(mNodeBoundingBox.left(), yGridPos * 2));
                    san->setInputPosition(QPointF(mCoordX.value(mNodeBoundingBox.left()).lanePosition(-1), epc.lanePosition(0, true)));
                }
                if (epl.hasOutputArrow())
                {
                    if (graphicsNet) mScene->addGraphItem(graphicsNet);
                    StandardArrowNet* san = new StandardArrowNet(n, dnt->mLines);
                    graphicsNet           = san;
                    int yGridPos          = mGlobalOutputHash.value(dnt->id(), -1);
                    Q_ASSERT(yGridPos >= 0);
                    QPoint pnt(mNodeBoundingBox.right() + 1, yGridPos * 2);
                    const EndpointCoordinate& epc = mEndpointHash.value(pnt);
                    const NetLayoutJunction* nlj  = mJunctionHash.value(pnt);
                    Q_ASSERT(nlj);
                    san->setOutputPosition(QPointF(mCoordX.value(pnt.x()).lanePosition(nlj->rect().right() + 1), epc.lanePosition(0, true)));
                }
                break;
            case EndpointList::SourceAndDestination:
                if (dnt->mLines.nLines() > 0)
                    graphicsNet = new StandardGraphicsNet(n, dnt->mLines, dnt->mKnots);
                break;
            default:
                Q_ASSERT(0 > 1);    // should never occur
                break;
        }

        if (graphicsNet)
            mScene->addGraphItem(graphicsNet);

        mDrawNetThreads.removeAll(dnt);
        dnt->deleteLater();
    }

    void GraphLayouter::drawNetsIsolated(u32 id, Net* n, const EndpointList& epl)
    {
        SeparatedGraphicsNet* net_item = epl.netType() == EndpointList::ConstantLevel ? static_cast<SeparatedGraphicsNet*>(new LabeledSeparatedNet(n, QString::fromStdString(n->get_name())))
                                                                                      : static_cast<SeparatedGraphicsNet*>(new ArrowSeparatedNet(n));

        int ipnt = 0;
        for (const NetLayoutPoint& pnt : epl)
        {
            bool isInput = epl.isInput(ipnt++);
            auto itPnt   = mEndpointHash.find(pnt);
            Q_ASSERT(itPnt != mEndpointHash.constEnd());
            if (isInput)
            {
                // gack hack : separated net might be connected to several ports
                const NodeBox* nbox = mBoxes.boxForPoint(pnt.gridPoint());
                Q_ASSERT(nbox);
                QList<u32> inpList = nbox->item()->inputNets();
                for (int jnx = 0; jnx < inpList.size(); jnx++)
                {
                    u32 inpNetId = inpList.at(jnx);
                    if (inpNetId != id)
                        continue;
                    QPointF inpPnt(itPnt.value().xInput(), itPnt.value().lanePosition(jnx, true));
                    net_item->addInput(inpPnt);
                }
            }
            else
            {
                for (int inx : itPnt.value().outputPinIndex(id))
                {
                    QPointF outPnt(itPnt.value().xOutput(), itPnt.value().lanePosition(inx, true));
                    net_item->addOutput(outPnt);
                }
            }
        }
        net_item->finalize();
        mScene->addGraphItem(net_item);
    }

    void GraphLayouter::updateSceneRect()
    {
        // SCENE RECT STUFF BEHAVES WEIRDLY, FURTHER RESEARCH REQUIRED
        //QRectF rect = mScene->sceneRect();

        QRectF rect(mScene->itemsBoundingRect());
        rect.adjust(-200, -200, 200, 200);
        mScene->setSceneRect(rect);
    }

    bool GraphLayouter::boxExists(const int x, const int y) const
    {
        return mBoxes.boxForPoint(QPoint(x, y)) != nullptr;
    }

    bool GraphLayouter::hRoadJumpPossible(const int x, const int y1, const int y2) const
    {
        if (y1 == y2)
            return false;

        int bottom_y   = y1;
        int difference = y1 - y2;

        if (y1 < y2)
        {
            bottom_y   = y2;
            difference = y2 - y1;
        }

        while (difference)
        {
            if (boxExists(x, bottom_y - difference))
                return false;

            --difference;
        }

        return true;
    }

    bool GraphLayouter::hRoadJumpPossible(const GraphLayouter::Road* const r1, const GraphLayouter::Road* const r2) const
    {
        // CONVENIENCE METHOD
        assert(r1 && r2);
        assert(r1->x != r2->x);

        return hRoadJumpPossible(r1->x, r1->y, r2->y);
    }

    bool GraphLayouter::vRoadJumpPossible(const int x1, const int x2, const int y) const
    {
        if (x1 == x2)
            return false;

        int right_x    = x1;
        int difference = x1 - x2;

        if (x1 < x2)
        {
            right_x    = x2;
            difference = x2 - x1;
        }

        while (difference)
        {
            if (boxExists(right_x - difference, y))
                return false;

            --difference;
        }

        return true;
    }

    bool GraphLayouter::vRoadJumpPossible(const GraphLayouter::Road* const r1, const GraphLayouter::Road* const r2) const
    {
        // CONVENIENCE METHOD
        assert(r1 && r2);
        assert(r1->y != r2->y);

        return vRoadJumpPossible(r1->x, r2->x, r1->y);
    }

    GraphLayouter::Junction* GraphLayouter::getJunction(const int x, const int y)
    {
        QPoint p(x, y);
        auto it = mJunctions.find(p);
        if (it != mJunctions.end())
        {
            return it.value();
        }

        GraphLayouter::Junction* j = new Junction(x, y);
        mJunctions.insert(p, j);
        return j;
    }

    qreal GraphLayouter::hRoadHeight(const unsigned int mLanes) const
    {
        // LANES COUNTED FROM 1
        qreal height = sHRoadPadding * 2;

        if (mLanes > 1)
            height += (mLanes - 1) * sLaneSpacing;

        return height;
    }

    qreal GraphLayouter::vRoadWidth(const unsigned int mLanes) const
    {
        // LANES COUNTED FROM 1
        qreal width = sVRoadPadding * 2;

        if (mLanes > 1)
            width += (mLanes - 1) * sLaneSpacing;

        return width;
    }

    void GraphLayouter::commitUsedPaths(const UsedPaths& used)
    {
        for (Junction* j : used.mHJunctions)
            j->mHLanes += 1;

        for (Junction* j : used.mVJunctions)
            j->mVLanes += 1;

        for (Junction* j : used.mCloseLeftJunctions)
            j->mCloseLeftLaneChanges += 1;

        for (Junction* j : used.mCloseRightJunctions)
            j->mCloseRightLaneChanges += 1;

        for (Junction* j : used.mCloseTopJunctions)
            j->mCloseTopLaneChanges += 1;

        for (Junction* j : used.mCloseBottomJunctions)
            j->mCloseBottomLaneChanges += 1;

        for (Junction* j : used.mFarLeftJunctions)
            j->mFarLeftLaneChanges += 1;

        for (Junction* j : used.mFarRightJunctions)
            j->mFarRightLaneChanges += 1;

        for (Junction* j : used.mFarTopJunctions)
            j->mFarTopLaneChanges += 1;

        for (Junction* j : used.mFarBottomJunctions)
            j->mFarBottomLaneChanges += 1;
    }

    void GraphLayouter::SceneCoordinate::testMinMax(int ilane)
    {
        if (ilane < minLane)
            minLane = ilane;
        if (ilane + 1 > maxLane)
            maxLane = ilane + 1;
    }

    void GraphLayouter::SceneCoordinate::setOffsetX(const SceneCoordinate& previous, float maximumBlock, float sepOut, float sepInp)
    {
        float delta = maximumBlock;
        if (delta < sepOut)
            delta = sepOut;
        mOffset                  = previous.xBoxOffset() + (1 - minLane) * sLaneSpacing + delta;
        float xDefaultBoxPadding = maxLane * sLaneSpacing;
        if (xDefaultBoxPadding < sepInp)
            mPadding = sepInp - xDefaultBoxPadding;
    }

    void GraphLayouter::SceneCoordinate::setOffsetYje(const SceneCoordinate& previous, float minimumJunction)
    {
        float delta = (previous.maxLane) * sLaneSpacing + sVRoadPadding;
        if (delta < minimumJunction)
            delta = minimumJunction;
        mOffset = previous.mOffset + delta;
    }

    void GraphLayouter::SceneCoordinate::setOffsetYej(const SceneCoordinate& previous, float maximumBlock, float minimumJunction)
    {
        float delta = (-minLane - 1) * sLaneSpacing + maximumBlock + sVRoadPadding;
        if (delta < minimumJunction)
            delta = minimumJunction;
        mOffset = previous.mOffset + delta;
    }

    float GraphLayouter::SceneCoordinate::lanePosition(int ilane) const
    {
        return mOffset + ilane * sLaneSpacing;
    }

    float GraphLayouter::SceneCoordinate::xBoxOffset() const
    {
        return junctionExit() + sHRoadPadding + mPadding;
    }

    float GraphLayouter::EndpointCoordinate::lanePosition(int ilane, bool absolute) const
    {
        float y0 = absolute ? mYoffset : mTopPin;
        if (ilane < 0)
            return y0 + ilane * sLaneSpacing;
        int n = numberPins() - 1;
        if (ilane <= n)
            return y0 + ilane * mPinDistance;
        return y0 + n * mPinDistance + (ilane - n) * sLaneSpacing;
    }

    GraphLayouter::EndpointCoordinate::EndpointCoordinate() : mYoffset(0), mXoutput(0), mXinput(0), mPinDistance(0), mTopPin(0), mNumberPins(0)
    {
        ;
    }

    int GraphLayouter::EndpointCoordinate::numberPins() const
    {
        return mNumberPins;
    }

    void GraphLayouter::EndpointCoordinate::setInputPosition(QPointF p0pos)
    {
        mXinput  = p0pos.x();
        mYoffset = p0pos.y();
    }

    void GraphLayouter::EndpointCoordinate::setOutputPosition(QPointF p0pos)
    {
        mXoutput = p0pos.x();
        if (mXinput < mXoutput)
            mXinput = mXoutput;
        mYoffset = p0pos.y();
    }

    QList<int> GraphLayouter::EndpointCoordinate::inputPinIndex(u32 id) const
    {
        return mInputHash.values(id);
    }

    QList<int> GraphLayouter::EndpointCoordinate::outputPinIndex(u32 id) const
    {
        return mOutputHash.values(id);
    }

    void GraphLayouter::EndpointCoordinate::setInputPins(const QList<u32>& pinList, float p0dist, float pdist)
    {
        int n = pinList.size();
        if (n > mNumberPins)
            mNumberPins = n;
        for (int i = 0; i < n; i++)
        {
            u32 id = pinList.at(i);
            if (id)
                mInputHash.insert(id, i);
        }
        if (p0dist > mTopPin)
            mTopPin = p0dist;
        mPinDistance = pdist;
    }

    void GraphLayouter::EndpointCoordinate::setOutputPins(const QList<u32>& pinList, float p0dist, float pdist)
    {
        int n = pinList.size();
        if (n > mNumberPins)
            mNumberPins = n;
        for (int i = 0; i < n; i++)
        {
            u32 id = pinList.at(i);
            if (id)
                mOutputHash.insert(id, i);
        }
        if (p0dist > mTopPin)
            mTopPin = p0dist;
        mPinDistance = pdist;
    }

    void GraphLayouter::EndpointList::addSource(const NetLayoutPoint& pnt)
    {
        mNetType          = static_cast<EndpointType>(mNetType | SingleSource);
        int existingIndex = indexOf(pnt);
        if (existingIndex >= 0 && !mPointIsInput.at(existingIndex))
            return;
        append(pnt);
        mPointIsInput.append(false);
    }

    void GraphLayouter::EndpointList::addDestination(const NetLayoutPoint& pnt)
    {
        mNetType          = static_cast<EndpointType>(mNetType | SingleDestination);
        int existingIndex = indexOf(pnt);
        if (existingIndex >= 0 && mPointIsInput.at(existingIndex))
            return;
        append(pnt);
        mPointIsInput.append(true);
    }

    void GraphLayouter::SeparatedNetWidth::requireInputSpace(float spc)
    {
        if (spc > mInputSpace)
            mInputSpace = spc;
    }

    void GraphLayouter::SeparatedNetWidth::requireOutputSpace(float spc)
    {
        if (spc > mOutputSpace)
            mOutputSpace = spc;
    }

    bool GraphLayouter::isConstNet(const Net* n)
    {
        for (Endpoint* src : n->get_sources())
        {
            if (src->get_gate()->is_gnd_gate() || src->get_gate()->is_vcc_gate())
                return true;
        }
        return false;
    }

    bool GraphLayouter::dumpJunctionEnabled()
    {
        return mDumpJunctions;
    }

    void GraphLayouter::setDumpJunctionEnabled(bool enabled)
    {
        mDumpJunctions = enabled;
    }

    void JunctionThread::run()
    {
        mJunction = new NetLayoutJunction(mEntries);
    }

    void DrawNetThread::run()
    {
        Net* n = gNetlist->get_net_by_id(mId);
        if (!n)
            return;

        const QHash<NetLayoutWire, int>& wMap = mLayouter->mLaneMap.value(mId);
        for (auto it = wMap.constBegin(); it != wMap.constEnd(); ++it)
        {
            NetLayoutPoint wFromPoint = it.key().endPoint(NetLayoutWire::SourcePoint);
            NetLayoutPoint wToPoint   = it.key().endPoint(NetLayoutWire::DestinationPoint);
            NetLayoutJunction* j0     = mLayouter->mJunctionHash.value(wFromPoint);
            NetLayoutJunction* j1     = mLayouter->mJunctionHash.value(wToPoint);
            int ilane                 = it.value();
            int ix0                   = wFromPoint.x();
            int iy0                   = wFromPoint.y();
            int ix1                   = wToPoint.x();
            int iy1                   = wToPoint.y();

            if (it.key().isHorizontal())
            {
                float x0 = j0 ? mLayouter->mCoordX[ix0].lanePosition(j0->rect().right()) : mLayouter->mCoordX[ix0].junctionExit();
                float x1 = j1 ? mLayouter->mCoordX[ix1].lanePosition(j1->rect().left()) : mLayouter->mCoordX[ix1].junctionEntry();
                float yy = mLayouter->mCoordY[iy0].lanePosition(ilane);
                mLines.appendHLine(x0, x1, yy);
            }
            else
            {
                float y0, y1;
                float xx = mLayouter->mCoordX[ix0].lanePosition(ilane);
                if (wToPoint.isEndpoint())
                {
                    // netjunction -> endpoint
                    auto itEpc = mLayouter->mEndpointHash.find(wToPoint);
                    y0         = j0 ? mLayouter->mCoordY[iy0].lanePosition(j0->rect().bottom()) : mLayouter->mCoordY[iy0].junctionExit();
                    y1         = itEpc != mLayouter->mEndpointHash.constEnd() ? itEpc.value().lanePosition(j1->rect().top(), true) : mLayouter->mCoordY[iy1].junctionEntry();
                    //                        if (itEpc==mEndpointHash.constEnd())
                    //                            qDebug() << "xxx to endp" << wToPoint.x() << wToPoint.y() << y0 << y1;
                }
                else
                {
                    // endpoint -> netjunction
                    auto itEpc = mLayouter->mEndpointHash.find(wFromPoint);
                    y0         = itEpc != mLayouter->mEndpointHash.constEnd() ? itEpc.value().lanePosition(j0->rect().bottom(), true) : mLayouter->mCoordY[iy0].junctionExit();
                    y1         = j1 ? mLayouter->mCoordY[iy1].lanePosition(j1->rect().top()) : mLayouter->mCoordY[iy1].junctionEntry();
                    //                        if (itEpc==mEndpointHash.constEnd())
                    //                            qDebug() << "xxx fr endp" << wFromPoint.x() << wFromPoint.y() << y0 << y1;
                }
                if (y1 > y0)
                    mLines.appendVLine(xx, y0, y1);
            }
        }
        drawJunction();
        drawEndpoint();
    }

    void DrawNetThread::drawJunction()
    {
        for (auto jt = mLayouter->mJunctionHash.constBegin(); jt != mLayouter->mJunctionHash.constEnd(); ++jt)
        {
            auto epcIt      = mLayouter->mEndpointHash.find(jt.key());
            int x           = jt.key().x();
            int y           = jt.key().y();
            bool isEndpoint = (y % 2 == 0);

            const GraphLayouter::SceneCoordinate& scX = mLayouter->mCoordX.value(x);
            const GraphLayouter::SceneCoordinate& scY = mLayouter->mCoordY.value(y);

            for (const NetLayoutJunctionWire& jw : jt.value()->netById(mId).mWires)
            {
                int li = jw.mIndex.laneIndex();
                if (jw.mIndex.isHorizontal())
                {
                    Q_ASSERT(epcIt != mLayouter->mEndpointHash.constEnd() || !isEndpoint);
                    float x0 = scX.lanePosition(jw.mRange.first());
                    float x1 = scX.lanePosition(jw.mRange.last());
                    float yy = isEndpoint ? epcIt.value().lanePosition(li, true) : scY.lanePosition(li);
                    mLines.appendHLine(x0, x1, yy);
                }
                else
                {
                    float y0, y1;
                    if (!isEndpoint)
                    {
                        y0 = scY.lanePosition(jw.mRange.first());
                        y1 = scY.lanePosition(jw.mRange.last());
                    }
                    else if (epcIt != mLayouter->mEndpointHash.constEnd())
                    {
                        y0 = epcIt.value().lanePosition(jw.mRange.first(), true);
                        y1 = epcIt.value().lanePosition(jw.mRange.last(), true);
                    }
                    else
                    {
                        y0 = scY.junctionEntry();
                        y1 = scY.junctionExit();
                        if (y1 <= y0)
                            y1 = y0 + 1;
                    }
                    float xx = scX.lanePosition(li);
                    mLines.appendVLine(xx, y0, y1);
                }
            }

            for (const QPoint& pnt : jt.value()->netById(mId).mKnots)
            {
                float x = scX.lanePosition(pnt.x());
                float y = isEndpoint ? epcIt.value().lanePosition(pnt.y(), true) : scY.lanePosition(pnt.y());
                mKnots.append(QPointF(x,y));
            }
        }
    }

    void DrawNetThread::drawEndpoint()
    {
        for (auto it = mLayouter->mEndpointHash.constBegin(); it != mLayouter->mEndpointHash.constEnd(); ++it)
        {
            const GraphLayouter::EndpointCoordinate& epc = it.value();

            QList<int> inputsById  = epc.inputPinIndex(mId);
            QList<int> outputsById = epc.outputPinIndex(mId);
            if (inputsById.isEmpty() && outputsById.isEmpty())
                continue;

            const NetLayoutJunction* nlj     = mLayouter->mJunctionHash.value(it.key());
            const GraphLayouter::SceneCoordinate& xScenePos = mLayouter->mCoordX.value(it.key().x());
            float xjLeft                     = xScenePos.lanePosition(nlj->rect().left());
            float xjRight                    = xScenePos.lanePosition(nlj->rect().right());
            Q_ASSERT(nlj);

            for (int inpInx : inputsById)
            {
                if (xjRight >= epc.xInput())
                {
                    // don't complain if "input" is in fact global output pin
                    auto ityOut = mLayouter->mGlobalOutputHash.find(mId);
                    if (ityOut == mLayouter->mGlobalOutputHash.constEnd() || QPoint(mLayouter->mNodeBoundingBox.right() + 1, 2 * ityOut.value()) != it.key())
                        qDebug() << "cannot connect input pin" << mId << it.key().x() << it.key().y() / 2 << xjRight << epc.xInput();
                }
                else
                    mLines.appendHLine(xjRight, epc.xInput(), epc.lanePosition(inpInx, true));
            }
            for (int outInx : outputsById)
            {
                if (epc.xOutput() >= xjLeft)
                    qDebug() << "cannot connect output pin" << mId << it.key().x() << it.key().y() / 2 << xjLeft << epc.xOutput();
                else
                    mLines.appendHLine(epc.xOutput(), xjLeft, epc.lanePosition(outInx, true));
            }
        }
    }
}    // namespace hal
