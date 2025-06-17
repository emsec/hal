#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/graph_widget/layout_locker.h"

#include "hal_core/netlist/module.h"
#include "hal_core/utilities/log.h"
#include "gui/graph_tab_widget/graph_tab_widget.h"
#include "gui/graph_widget/layout_locker.h"
#include "gui/graph_widget/graphics_scene.h"
#include "gui/graph_widget/graph_widget.h"
#include "gui/graph_widget/items/nodes/modules/standard_graphics_module.h"
#include "gui/context_manager_widget/context_manager_widget.h"
#include "gui/gui_globals.h"
#include "gui/gui_def.h"
#include "gui/implementations/qpoint_extension.h"
#include "gui/user_action/user_action_manager.h"
#include "gui/user_action/action_rename_object.h"
#include <QVector>
#include <QJsonArray>
#include "gui/main_window/main_window.h"
#include "gui/settings/settings_items/settings_item_dropdown.h"

namespace hal
{
    static const bool sLazyUpdates = false;

    GraphContext::GraphContext(u32 id_, const QString& name, QObject* parent)
        : QObject(parent),
          mId(id_),
          mName(name),
          mParentWidget(nullptr),
          mDirty(false),
          mLayouter(nullptr),
          mShader(nullptr),
          mUserUpdateCount(0),
          mUnappliedChanges(false),
          mSceneUpdateRequired(false),
          mSceneUpdateInProgress(false),
          mSpecialUpdate(false),
          mExclusiveModuleId(0)
    {
        mTimestamp = QDateTime::currentDateTime();
        connect(MainWindow::sSettingStyle,&SettingsItemDropdown::intChanged,this,&GraphContext::handleStyleChanged);
        connect(gNetlistRelay, &NetlistRelay::moduleNameChanged, this, &GraphContext::handleModuleNameChanged);
        connect(this, &GraphContext::exclusiveModuleLost, this, &GraphContext::handleExclusiveModuleLost);
    }

    GraphContext::~GraphContext()
    {
        LayoutLockerManager::instance()->removeWaitingContext(this);
        if (mParentWidget) mParentWidget->handleContextAboutToBeDeleted();
        delete mLayouter;
        delete mShader;
    }

    void GraphContext::setLayouter(GraphLayouter* layouter)
    {
        assert(layouter);

        if (mLayouter)
            delete mLayouter;

        mLayouter = layouter;
    }

    void GraphContext::setShader(GraphShader* shader)
    {
        assert(shader);

        if (mShader)
            delete mShader;

        mShader = shader;
    }

    void GraphContext::beginChange()
    {
        ++mUserUpdateCount;
    }

    void GraphContext::endChange()
    {
        --mUserUpdateCount;
        if (mUserUpdateCount == 0)
        {
            evaluateChanges();
            update();
        }
    }

    void GraphContext::add(const QSet<u32>& modules, const QSet<u32>& gates, PlacementHint placement)
    {
        QSet<u32> new_modules = modules - mModules;
        QSet<u32> new_gates   = gates - mGates;

        QSet<u32> old_modules = mRemovedModules & modules;
        QSet<u32> old_gates   = mRemovedGates & gates;

        // if we have a placement hint for the added nodes, we ignore it and leave
        // the nodes where they are (i.e. we just deschedule their removal and not
        // re-run the placement algo on them)
        mRemovedModules -= old_modules;
        mRemovedGates -= old_gates;

        mAddedModules += new_modules;
        mAddedGates += new_gates;

        mPlacementList[placement.mode()].append(
                    PlacementEntry(placement,new_modules,new_gates));

        if (mUserUpdateCount == 0)
        {
            evaluateChanges();
            update();
        }
        if (!new_modules.isEmpty() || !new_gates.isEmpty()) setDirty(true);
    }

    void GraphContext::remove(const QSet<u32>& modules, const QSet<u32>& gates)
    {
        QSet<u32> old_modules = modules & mModules;
        QSet<u32> old_gates   = gates & mGates;

        mRemovedModules += old_modules;
        mRemovedGates += old_gates;

        mAddedModules -= modules;
        mAddedGates -= gates;

        // We don't update mPlacementList here. Keeping elements
        // in memory is less of an issue than finding the modules/gates _by value_
        // in the maps and removing them. At the end of applyChanges() the lists are
        // cleared anyway.

        if (mUserUpdateCount == 0)
        {
            evaluateChanges();
            update();
        }
        if (!old_modules.isEmpty() || !old_gates.isEmpty()) setDirty(true);
    }

    void GraphContext::clear()
    {
        mRemovedModules = mModules;
        mRemovedGates   = mGates;

        mAddedModules.clear();
        mAddedGates.clear();

        for (int i=0; i<4; i++)
            mPlacementList[i].clear();

        if (mUserUpdateCount == 0)
        {
            evaluateChanges();
            update();
        }
    }

    bool GraphContext::isGateUnfolded(u32 gateId) const
    {
        QSet<u32> containedGates = mGates + mAddedGates - mRemovedGates;
        return containedGates.contains(gateId);
    }

    bool GraphContext::isModuleUnfolded(const u32 moduleId) const
    {
        QSet<u32> containedGates = mGates + mAddedGates - mRemovedGates;
        QSet<u32> containedModules = mModules + mAddedModules - mRemovedModules;

        auto m = gNetlist->get_module_by_id(moduleId);

        for (const Gate* g : m->get_gates())
        {
            if (!containedGates.contains(g->get_id())) return false;
        }
        for (const Module* sm : m->get_submodules())
        {
            if (!isModuleUnfolded(sm->get_id()) && !containedModules.contains(sm->get_id()))
                return false;
        }
        return true;
    }

    void GraphContext::unfoldModule(const u32 id, const PlacementHint& plc)
    {
        auto contained_modules = mModules + mAddedModules - mRemovedModules;

        if (contained_modules.find(id) != contained_modules.end())
        {
            auto m = gNetlist->get_module_by_id(id);
            QSet<u32> gates;
            QSet<u32> modules;

            Node singleContentNode;
            PlacementHint childPlc;

            for (const Gate* g : m->get_gates())
            {
                singleContentNode = Node(g->get_id(),Node::Gate);
                gates.insert(g->get_id());
            }
            for (const Module* sm : m->get_submodules())
            {
                singleContentNode = Node(sm->get_id(),Node::Module);
                modules.insert(sm->get_id());
            }

            if (plc.mode() == PlacementHint::GridPosition)
            {
                // placement determined by caller
                childPlc = plc;
            }
            else if (gates.size() + modules.size() == 1)
            {
                // There is only a single child in this module, keep the grid position
                childPlc = PlacementHint(PlacementHint::GridPosition);
                NetLayoutPoint childPos = mLayouter->positonForNode(Node(id,Node::Module));
                if (!childPos.isUndefined())
                    childPlc.addGridPosition(singleContentNode,childPos);
            }

            // That would unfold the empty module into nothing, meaning there would
            // be no way back
            assert(!gates.empty() || !modules.empty());

            beginChange();
            mLayouter->prepareRollback();
            remove({id}, {});
            add(modules, gates, childPlc);
            endChange();
        }
    }

    bool GraphContext::empty() const
    {
        return mGates.empty() && mModules.empty();
    }

    bool GraphContext::willBeEmptied() const
    {
        QSet<u32> tempMod = mModules + mAddedModules - mRemovedModules;
        QSet<u32> tempGat = mGates   + mAddedGates   - mRemovedGates;
        return tempMod.isEmpty() && tempGat.isEmpty();
    }

    void GraphContext::testIfAffected(const u32 id, const u32* moduleId, const u32* gateId)
    {
        if (testIfAffectedInternal(id, moduleId, gateId))
            scheduleSceneUpdate();
    }

    bool GraphContext::testIfAffectedInternal(const u32 id, const u32* moduleId, const u32* gateId)
    {
        Node nd(id,Node::Module);
        if (getLayouter()->boxes().boxForNode(nd))
            return true;

        std::vector<Gate*> modifiedGates;
        if (moduleId)
        {
            Module* m = gNetlist->get_module_by_id(*moduleId);
            if (m) modifiedGates = m->get_gates(nullptr,true);
        }
        if (gateId)
        {
            Gate* g = gNetlist->get_gate_by_id(*gateId);
            if (g) modifiedGates.push_back(g);
        }
        for (Gate* mg : modifiedGates)
            if (getLayouter()->boxes().boxForGate(mg))
                return true;

        return false;
    }

    void GraphContext::removeModuleContents(const u32 moduleId)
    {
        QSet<u32> childGates;
        QSet<u32> childModules;

        for (const Gate* g : gNetlist->get_module_by_id(moduleId)->get_gates(nullptr, true))
        {
            childGates.insert(g->get_id());
        }
        for (const Module* sm : gNetlist->get_module_by_id(moduleId)->get_submodules(nullptr, true))
        {
            childModules.insert(sm->get_id());
        }
        remove(childModules, childGates);
    }

    bool GraphContext::isShowingModule(const u32 id) const
    {
        return isShowingModule(id, {}, {}, {}, {});
    }

    bool GraphContext::isShowingModule(const u32 id, const QSet<u32>& minus_modules, const QSet<u32>& minus_gates, const QSet<u32>& plus_modules, const QSet<u32>& plus_gates) const
    {
        // There are all sorts of problems when we allow this, since now any empty
        // module thinks that it is every other empty module. Blocking this,
        // however, essentially means that we must destroy all empty contexts:
        // With the block in place, a context won't recognize it's module anymore
        // once the last gate has removed, so adding a new gate won't update the
        // context.
        if (empty())
        {
            return false;
        }

        // TODO deduplicate
        QSet<u32> gates;
        QSet<u32> modules;

        getModuleChildrenRecursively(id, &gates, &modules);

        auto contextGates = (mGates - mRemovedGates) + mAddedGates;
        auto contextModules = (mModules - mRemovedModules) + mAddedModules;
        auto moduleGates = (gates - minus_gates) + plus_gates;
        auto moduleModules = (modules - minus_modules) + plus_modules;

        return contextGates == moduleGates && contextModules == moduleModules;
    }


    bool GraphContext::isShowingFoldedTopModule() const
    {
        auto contextGates = (mGates - mRemovedGates) + mAddedGates;
        if (!contextGates.isEmpty()) return false;
        auto contextModules = (mModules - mRemovedModules) + mAddedModules;
        if (contextModules.size() != 1) return false;
        return (*mModules.constBegin() == 1); // top_module has ID=1
    }

    void GraphContext::getModuleChildrenRecursively(const u32 id, QSet<u32>* gates, QSet<u32>* modules) const
    {

        auto containedModules = mModules + mAddedModules - mRemovedModules;

        Module* m = gNetlist->get_module_by_id(id);
        if (!m) return;

        for (const Gate* g : m->get_gates())
        {
            gates->insert(g->get_id());
        }
        for (const Module* sm : m->get_submodules())
        {
            if (!containedModules.contains(sm->get_id()) && isModuleUnfolded(sm->get_id()))
                getModuleChildrenRecursively(sm->get_id(), gates, modules);
            else
                modules->insert(sm->get_id());
        }
    }

    bool GraphContext::isShowingModuleExclusively()
    {
        // Module Context connection is lost when a unfolded submodule (visible in view) is deleted
        // The connection is lost, because the deletion of an unfolded module does not add its submodules/gates
        // to the view

        if (!mExclusiveModuleId) return false;

        auto containedModules = mModules + mAddedModules - mRemovedModules;
        auto containedGates = mGates + mAddedGates - mRemovedGates;

        // folded module
        if (containedGates.empty() && containedModules.size() == 1 && *containedModules.begin() == mExclusiveModuleId)
            return true;
        // unfolded module
        if (isShowingModule(mExclusiveModuleId, {}, {}, {}, {}))
            return true;
        return false;
    }

    bool GraphContext::isShowingNetSource(const u32 mNetId) const
    {
        auto net = gNetlist->get_net_by_id(mNetId);
        auto src_pins = net->get_sources();

        for(auto pin : src_pins)
        {
            if(pin->get_gate() != nullptr)
            {
                if(mGates.contains(pin->get_gate()->get_id()))
                    return true;
            }
        }

        return false;
    }

    bool GraphContext::isShowingNetDestination(const u32 mNetId) const
    {
        auto net = gNetlist->get_net_by_id(mNetId);
        auto dst_pins = net->get_destinations();

        for(auto pin : dst_pins)
        {
            if(pin->get_gate() != nullptr)
            {
                if(mGates.contains(pin->get_gate()->get_id()))
                    return true;
            }
        }

        return false;
    }

    Node GraphContext::getNetSource(const Net *n) const
    {
        for (Endpoint* ep : n->get_sources())
        {
            Gate* g = ep->get_gate();
            NodeBox* box = mLayouter->boxes().boxForGate(g);
            if (box) return box->getNode();
        }
        return Node();
    }

    Node GraphContext::getNetDestination(const Net* n) const
    {
        for (Endpoint* ep : n->get_destinations())
        {
            Gate* g = ep->get_gate();
            NodeBox* box = mLayouter->boxes().boxForGate(g);
            if (box) return box->getNode();
        }
        return Node();
    }

    const QSet<u32>& GraphContext::modules() const
    {
        return mModules;
    }

    const QSet<u32>& GraphContext::gates() const
    {
        return mGates;
    }

    const QSet<u32>& GraphContext::nets() const
    {
        return mNets;
    }

    GraphicsScene* GraphContext::scene()
    {
        return mLayouter->scene();
    }

    u32 GraphContext::id() const
    {
        return mId;
    }

    QString GraphContext::name() const
    {
        return mName;
    }

    QString GraphContext::getNameWithDirtyState() const
    {
        if (mDirty) return mName + "*";
        return mName;
    }

    bool GraphContext::sceneUpdateInProgress() const
    {
        return mSceneUpdateInProgress;
    }

    void GraphContext::scheduleSceneUpdate()
    {
        mSceneUpdateRequired = true;

        if (sLazyUpdates)
            if (!mParentWidget)
                return;

        if(mUserUpdateCount == 0)
            update();
    }

    void GraphContext::refreshModule(const u32 moduleId)
    {
        NodeBox* box = getLayouter()->boxes().boxForNode(Node(moduleId,Node::Module));
        if (!box) return;
        Module* m = gNetlist->get_module_by_id(moduleId);
        if (!m) return;
        GraphicsModule* gm = dynamic_cast<GraphicsModule*>(box->item());
        if (!gm) return;
        gm->setModuleLabel(m);
        gm->update();
    }

    Node GraphContext::nodeForGate(const u32 id) const
    {
        if (mGates.contains(id))
            return Node(id, Node::Gate);

        Gate* g = gNetlist->get_gate_by_id(id);

        if (!g)
            return Node();

        Module* m = g->get_module();

        while (m)
        {
            if (mModules.contains(m->get_id()))
                return Node(m->get_id(), Node::Module);
            m = m->get_parent_module();
        }

        return Node();
    }

    /*GraphLayouter* GraphContext::debugGetLayouter() const
    {
        return mLayouter;
    }*/

    void GraphContext::layoutProgress(int percent) const
    {
        QString text;
        if (!percent) text = QString("Layout %1[%2]").arg(mName).arg(mId);
        if (mParentWidget) mParentWidget->showProgress(percent,text);
    }

    void GraphContext::storeViewport()
    {
        if (mParentWidget) mParentWidget->storeViewport();
    }

    void GraphContext::moveNodeAction(const QPoint& from, const QPoint& to)
    {
        const QMap<QPoint,Node> nodeMap = mLayouter->positionToNodeMap();
        auto it = nodeMap.find(from);
        if (it==nodeMap.constEnd()) return;
        mLayouter->setNodePosition(it.value(),to);
        scheduleSceneUpdate();
    }

    void GraphContext::handleLayouterFinished()
    {
        if (mUnappliedChanges)
            applyChanges();

        if (mSceneUpdateRequired)
        {
            requireSceneUpdate();
        }
        else
        {
            mShader->update();
            mLayouter->scene()->updateVisuals(mShader->getShading());

            mSceneUpdateInProgress = false;

            mLayouter->scene()->connectAll();

            if (mParentWidget) mParentWidget->handleSceneAvailable();
        }
    }

    void GraphContext::evaluateChanges()
    {
        if (!mAddedGates.isEmpty() || !mRemovedGates.isEmpty() || !mAddedModules.isEmpty() || !mRemovedModules.isEmpty())
            mUnappliedChanges = true;
    }

    void GraphContext::update()
    {
        if (mSceneUpdateInProgress)
            return;

        storeViewport();

        if (mUnappliedChanges)
            applyChanges();

        if (mSceneUpdateRequired)
            requireSceneUpdate();
    }

    void GraphContext::applyChanges()
    {
        // since changes are only applied once in a while added module might not exist any more
        auto it = mAddedModules.begin();
        while (it != mAddedModules.end())
        {
            if (gNetlist->get_module_by_id(*it))
                ++it;
            else
                it = mAddedModules.erase(it);
        }

        mModules -= mRemovedModules;
        mGates -= mRemovedGates;

        mModules += mAddedModules;
        mGates += mAddedGates;

        mLayouter->remove(mRemovedModules, mRemovedGates, mNets);
        mShader->remove(mRemovedModules, mRemovedGates, mNets);

        updateNets();

        int placementOrder[4] = { PlacementHint::GridPosition,
                                  PlacementHint::PreferLeft,
                                  PlacementHint::PreferRight,
                                  PlacementHint::Standard};

        for (int iplc = 0; iplc<4; iplc++)
        {
            for (const PlacementEntry& plcEntry : mPlacementList[placementOrder[iplc]])
            {
                QSet<u32> modsForHint = plcEntry.mModules;
                modsForHint &= mAddedModules; // may contain obsolete entries that we must filter out
                QSet<u32> gatsForHint = plcEntry.mGates;
                gatsForHint &= mAddedGates;
                mLayouter->add(modsForHint, gatsForHint, mNets, plcEntry.mPlacementHint);
            }
        }

        mShader->add(mAddedModules, mAddedGates, mNets);

        mAddedModules.clear();
        mAddedGates.clear();

        mRemovedModules.clear();
        mRemovedGates.clear();

        for (int i=0; i<4; i++)
            mPlacementList[i].clear();

        mUnappliedChanges     = false;
        mSceneUpdateRequired = true;
    }

    void GraphContext::updateNets()
    {
        mNets.clear();
        for (const auto& id : mGates)
        {
            if (mRemovedGates.contains(id)) continue;
            auto g = gNetlist->get_gate_by_id(id);
            if (!g) continue;
            for (auto net : g->get_fan_in_nets())
            {
                //if(!net->is_unrouted() || net->is_global_input_net() || net->is_global_output_net())
                    mNets.insert(net->get_id());
            }
            for (auto net : g->get_fan_out_nets())
            {
                //if(!net->is_unrouted() || net->is_global_input_net() || net->is_global_output_net())
                    mNets.insert(net->get_id());
            }
        }
        for (const auto& id : mModules)
        {
            if (mRemovedModules.contains(id)) continue;
            auto m = gNetlist->get_module_by_id(id);
            if (!m) continue;
            for (auto net : m->get_input_nets())
            {
                mNets.insert(net->get_id());
            }
            for (auto net : m->get_output_nets())
            {
                mNets.insert(net->get_id());
            }
        }
    }

    void GraphContext::requireSceneUpdate()
    {
        if (LayoutLockerManager::instance()->canUpdate(this))
            startSceneUpdate();
    }

    void GraphContext::startSceneUpdate()
    {
        mSceneUpdateRequired = false;
        mSceneUpdateInProgress = true;

        if (mParentWidget) mParentWidget->handleSceneUnavailable();
        mLayouter->scene()->disconnectAll();

    //    LayouterTask* task = new LayouterTask(mLayouter);
    //    connect(task, &LayouterTask::finished, this, &GraphContext::handleLayouterFinished, Qt::ConnectionType::QueuedConnection);
    //    gThreadPool->queueTask(task);

        exclusiveModuleCheck();

        mLayouter->layout();
        handleLayouterFinished();
    }

    void GraphContext::abortLayout()
    {
        if (!mSceneUpdateInProgress) return;
        if (!mLayouter->rollback()) return;
        mGates.clear();
        mModules.clear();
        for (const Node& nd : mLayouter->nodeToPositionMap().keys())
        {
            switch (nd.type())
            {
            case Node::Module: mModules.insert(nd.id()); break;
            case Node::Gate:   mGates.insert(nd.id()); break;
            default: break;
            }
        }
        mLayouter->layout();
        handleLayouterFinished();
    }

    void GraphContext::handleStyleChanged(int istyle)
    {
        Q_UNUSED(istyle);
        handleLayouterFinished();
    }

    void GraphContext::exclusiveModuleCheck()
    {
        if (!isShowingModuleExclusively())
            setExclusiveModuleId(0);
    }
    
    void GraphContext::handleModuleNameChanged(Module* m)
    {
        if (mExclusiveModuleId == m->get_id())
        {
            QString name = QString::fromStdString(m->get_name()) + " (ID: " + QString::number(m->get_id()) + ")";
            ActionRenameObject* act = new ActionRenameObject(name);
            act->setObject(UserActionObject(this->id(), UserActionObjectType::ContextView));
            act->exec();
        }
        Q_EMIT(dataChanged());
    }

    QDateTime GraphContext::getTimestamp() const
    {
        return mTimestamp;
    }

    bool GraphContext::readFromFile(const QJsonObject& json)
    {
        if (json.contains("timestamp") && json["timestamp"].isString())
            mTimestamp = QDateTime::fromString(json["timestamp"].toString());

        QList<QPair<Node,QPoint>> nodesToPlace;

        if (json.contains("modules") && json["modules"].isArray())
        {
            QJsonArray jsonMods = json["modules"].toArray();
            int nmods = jsonMods.size();
            for (int imod=0; imod<nmods; imod++)
            {
                QJsonObject jsonMod = jsonMods.at(imod).toObject();
                if (!jsonMod.contains("id") || !jsonMod["id"].isDouble()) continue;
                u32 id = jsonMod["id"].toInt();
                if (!gNetlist->get_module_by_id(id))
                {
                    log_warning("gui", "Module id={} not found in netlist, view id={} not restored.", id, mId);
                    return false;
                }
                if (!jsonMod.contains("x") || !jsonMod["x"].isDouble()) continue;
                int x = jsonMod["x"].toInt();
                if (!jsonMod.contains("y") || !jsonMod["y"].isDouble()) continue;
                int y = jsonMod["y"].toInt();
                Node nd(id,Node::Module);
                nodesToPlace.append(QPair<Node,QPoint>(nd,QPoint(x,y)));
            }
        }

        if (json.contains("gates") && json["gates"].isArray())
        {
            QJsonArray jsonGats = json["gates"].toArray();
            int ngats = jsonGats.size();
            for (int igat=0; igat<ngats; igat++)
            {
                QJsonObject jsonGat = jsonGats.at(igat).toObject();
                if (!jsonGat.contains("id") || !jsonGat["id"].isDouble()) continue;
                u32 id = jsonGat["id"].toInt();
                if (!gNetlist->get_gate_by_id(id))
                {
                    log_warning("gui", "Gate id={} not found in netlist, view id={} not restored.", id, mId);
                    return false;
                }
                if (!jsonGat.contains("x") || !jsonGat["x"].isDouble()) continue;
                int x = jsonGat["x"].toInt();
                if (!jsonGat.contains("y") || !jsonGat["y"].isDouble()) continue;
                int y = jsonGat["y"].toInt();
                Node nd(id,Node::Gate);
                nodesToPlace.append(QPair<Node,QPoint>(nd,QPoint(x,y)));
            }
        }

        if (nodesToPlace.isEmpty())
        {
            log_warning("gui", "Cannot restore view id={}, there are no nodes to place.", mId);
            return false;
        }

        mModules.clear();
        mGates.clear();
        for (const QPair<Node,QPoint>& box : nodesToPlace)
        {
            if (box.first.type() == Node::Module)
                mModules.insert(box.first.id());
            else
                mGates.insert(box.first.id());
            mLayouter->setNodePosition(box.first,box.second);
        }
        if (mModules.size()==1 && mGates.isEmpty())
            setExclusiveModuleId(*(mModules.begin()),false);

        if (json.contains("nets") && json["nets"].isArray())
        {
            QJsonArray jsonNets = json["nets"].toArray();
            int nnets = jsonNets.size();
            for (int inet=0; inet<nnets; inet++)
            {
                QJsonObject jsonNet = jsonNets.at(inet).toObject();
                if (!jsonNet.contains("id") || !jsonNet["id"].isDouble()) continue;
                u32 id = jsonNet["id"].toInt();
                if (!gNetlist->get_net_by_id(id)) return false;
                mNets.insert(id);
            }
        }

        //scheduleSceneUpdate();
        setDirty(false);
        return true;
    }

    void GraphContext::writeToFile(QJsonObject& json, int parentId)
    {
        json["id"] = (int) mId;
        json["name"] = mName;
        json["timestamp"] = mTimestamp.toString();
        json["exclusiveModuleId"] = (int) mExclusiveModuleId;
        json["visible"] = gContentManager->getGraphTabWidget()->visibleStatus(this);
        json["parentId"] = (int) parentId;
        /// modules
        QJsonArray jsonMods;
        for (u32 id : mModules)
        {
            NetLayoutPoint pos = getLayouter()->positonForNode(Node(id,Node::Module));
            Q_ASSERT(!pos.isUndefined());
            QJsonObject jsonMod;
            jsonMod["id"] = (int) id;
            jsonMod["x"]  = (int) pos.x();
            jsonMod["y"]  = (int) pos.y();
            jsonMods.append(jsonMod);
        }
        json["modules"] = jsonMods;

        /// gates
        QJsonArray jsonGats;
        for (u32 id : mGates)
        {
            NetLayoutPoint pos = getLayouter()->positonForNode(Node(id,Node::Gate));
            Q_ASSERT(!pos.isUndefined());
            QJsonObject jsonGat;
            jsonGat["id"] = (int) id;
            jsonGat["x"]  = (int) pos.x();
            jsonGat["y"]  = (int) pos.y();
            jsonGats.append(jsonGat);
        }
        json["gates"] = jsonGats;

        /// nets
        QJsonArray jsonNets;
        for (u32 id : mNets)
        {
            QJsonObject jsonNet;
            jsonNet["id"] = (int) id;
            jsonNets.append(jsonNet);
        }
        json["nets"] = jsonNets;
        setDirty(false);
    }

    void GraphContext::setDirty(bool dty)
    {
        if (mDirty==dty) return;
        mDirty = dty;
        Q_EMIT(dataChanged());
    }

    void GraphContext::setScheduleRemove(const QSet<u32>& mods, const QSet<u32>& gats)
    {
        mScheduleRemoveModules = mods;
        mScheduleRemoveGates = gats;
    }

    bool GraphContext::isScheduledRemove(const Node& nd)
    {
        switch (nd.type()) {
        case Node::Module:
        {
            auto it = mScheduleRemoveModules.find(nd.id());
            if (it != mScheduleRemoveModules.end())
            {
                mScheduleRemoveModules.erase(it);
                return true;
            }
            break;
        }
        case Node::Gate:
        {
            auto it = mScheduleRemoveGates.find(nd.id());
            if (it != mScheduleRemoveGates.end())
            {
                mScheduleRemoveGates.erase(it);
                return true;
            }
            break;
        }
        default:
            break;
        }
        return false;
    }

    void GraphContext::setSpecialUpdate(bool state)
    {
        mSpecialUpdate = state;
    }

    void GraphContext::showComments(const Node &nd)
    {
        if (mParentWidget)
            mParentWidget->showComments(nd);
    }

    void GraphContext::setExclusiveModuleId(u32 id, bool emitSignal)
    {
        u32 old_id = mExclusiveModuleId;
        mExclusiveModuleId = id;

        // Emit signal if context is not showing an exclusive module anymore
        if ((id == 0 && old_id != 0) && emitSignal)
            Q_EMIT(exclusiveModuleLost(old_id));
    }

    void GraphContext::handleExclusiveModuleLost(u32 old_id)
    {
        Module* m = gNetlist->get_module_by_id(old_id);
        assert(m);

        u32 cnt = 0;
        while (true)
        {
            ++cnt;
            QString new_name = QString::fromStdString(m->get_name()) + " modified";
            if (cnt > 1)
            {
                new_name += " (" + QString::number(cnt) + ")";
            }
            bool found = false;
            for (const auto& ctx : gGraphContextManager->getContexts())
            {
                if (ctx->name() == new_name)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                ActionRenameObject* act = new ActionRenameObject(new_name);
                act->setObject(UserActionObject(this->id(),UserActionObjectType::ContextView));
                act->exec();
                break;
            }
        }
        Q_EMIT(dataChanged());
    }
}
