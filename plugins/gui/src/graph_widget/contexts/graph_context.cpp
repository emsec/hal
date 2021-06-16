#include "gui/graph_widget/contexts/graph_context.h"

#include "hal_core/netlist/module.h"

#include "gui/graph_widget/contexts/graph_context_subscriber.h"
#include "gui/graph_widget/graphics_scene.h"
#include "gui/gui_globals.h"
#include "gui/gui_def.h"
#include "gui/implementations/qpoint_extension.h"
#include "gui/user_action/user_action_manager.h"
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
          mDirty(false),
          mLayouter(nullptr),
          mShader(nullptr),
          mUserUpdateCount(0),
          mUnappliedChanges(false),
          mSceneUpdateRequired(false),
          mSceneUpdateInProgress(false)
    {
        mTimestamp = QDateTime::currentDateTime();
        UserActionManager::instance()->clearWaitCount();
        connect(MainWindow::sSettingStyle,&SettingsItemDropdown::intChanged,this,&GraphContext::handleStyleChanged);
    }

    GraphContext::~GraphContext()
    {
        for (GraphContextSubscriber* subscriber : mSubscribers)
            subscriber->handleContextAboutToBeDeleted();

        delete mLayouter;
        delete mShader;
    }

    void GraphContext::setLayouter(GraphLayouter* layouter)
    {
        assert(layouter);

        if (mLayouter)
            delete mLayouter;

        mLayouter = layouter;

        connect(layouter, qOverload<int>(&GraphLayouter::statusUpdate), this, qOverload<int>(&GraphContext::handleLayouterUpdate), Qt::ConnectionType::QueuedConnection);
        connect(layouter, qOverload<const QString&>(&GraphLayouter::statusUpdate), this, qOverload<const QString&>(&GraphContext::handleLayouterUpdate), Qt::ConnectionType::QueuedConnection);
    }

    void GraphContext::setShader(GraphShader* shader)
    {
        assert(shader);

        if (mShader)
            delete mShader;

        mShader = shader;
    }

    void GraphContext::subscribe(GraphContextSubscriber* const subscriber)
    {
        assert(subscriber);
        assert(!mSubscribers.contains(subscriber));

        mSubscribers.append(subscriber);
        update();
    }

    void GraphContext::unsubscribe(GraphContextSubscriber* const subscriber)
    {
        assert(subscriber);

        mSubscribers.removeOne(subscriber);
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

    bool GraphContext::foldModuleAction(u32 moduleId, const PlacementHint &plc)
    {
        Module* m = gNetlist->get_module_by_id(moduleId);
        if (!m) return false;
        QSet<u32> gats;
        QSet<u32> mods;
        for (const auto& g : m->get_gates(nullptr, true))
            gats.insert(g->get_id());
        for (auto sm : m->get_submodules(nullptr, true))
            mods.insert(sm->get_id());
        beginChange();
        remove(mods, gats);
        add({m->get_id()}, {}, plc);
        endChange();
        return true;
    }

    bool GraphContext::isGateUnfolded(u32 gateId) const
    {
        QSet<u32> containedGates = mGates + mAddedGates - mRemovedGates;
        return containedGates.contains(gateId);
    }

    void GraphContext::unfoldModule(const u32 id)
    {
        auto contained_modules = mModules + mAddedModules - mRemovedModules;

        if (contained_modules.find(id) != contained_modules.end())
        {
            auto m = gNetlist->get_module_by_id(id);
            QSet<u32> gates;
            QSet<u32> modules;

            Node singleContentNode;

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

            PlacementHint plc;
            if (gates.size() + modules.size() == 1)
            {
                plc = PlacementHint(PlacementHint::GridPosition);
                plc.addGridPosition(singleContentNode,
                                    mLayouter->nodeToPositionMap().value(Node(id,Node::Module)));
            }

            // That would unfold the empty module into nothing, meaning there would
            // be no way back
            assert(!gates.empty() || !modules.empty());

            beginChange();
            remove({id}, {});
            add(modules, gates, plc);
            endChange();
        }
        setDirty(false);
    }

    bool GraphContext::empty() const
    {
        return mGates.empty() && mModules.empty();
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

        Module* m = gNetlist->get_module_by_id(id);
        // TODO deduplicate
        QSet<u32> gates;
        QSet<u32> modules;
        for (const Gate* g : m->get_gates())
        {
            gates.insert(g->get_id());
        }
        for (const Module* sm : m->get_submodules())
        {
            modules.insert(sm->get_id());
        }
        // qDebug() << "GATES" << gates;
        // qDebug() << "MINUS_GATES" << minus_gates;
        // qDebug() << "PLUS_GATES" << plus_gates;
        // qDebug() << "MGATES" << mGates;
        return (mGates - mRemovedGates) + mAddedGates == (gates - minus_gates) + plus_gates
                && (mModules - mRemovedModules) + mAddedModules == (modules - minus_modules) + plus_modules;
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
            if (mSubscribers.empty())
                return;

        if(mUserUpdateCount == 0)
            update();
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

    void GraphContext::handleLayouterUpdate(const int percent)
    {
        for (GraphContextSubscriber* s : mSubscribers)
            s->handleStatusUpdate(percent);
    }

    void GraphContext::handleLayouterUpdate(const QString& message)
    {
        for (GraphContextSubscriber* s : mSubscribers)
            s->handleStatusUpdate(message);
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
            startSceneUpdate();
        }
        else
        {
            mShader->update();
            mLayouter->scene()->updateVisuals(mShader->getShading());

            mSceneUpdateInProgress = false;

            mLayouter->scene()->connectAll();

            for (GraphContextSubscriber* s : mSubscribers)
                s->handleSceneAvailable();
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

        if (mUnappliedChanges)
            applyChanges();

        if (mSceneUpdateRequired)
            startSceneUpdate();
    }

    void GraphContext::applyChanges()
    {
        mModules -= mRemovedModules;
        mGates -= mRemovedGates;

        mModules += mAddedModules;
        mGates += mAddedGates;

        mLayouter->remove(mRemovedModules, mRemovedGates, mNets);
        mShader->remove(mRemovedModules, mRemovedGates, mNets);

        mNets.clear();
        for (const auto& id : mGates)
        {
            auto g = gNetlist->get_gate_by_id(id);
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
            auto m = gNetlist->get_module_by_id(id);
            for (auto net : m->get_input_nets())
            {
                mNets.insert(net->get_id());
            }
            for (auto net : m->get_output_nets())
            {
                mNets.insert(net->get_id());
            }
        }

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

    void GraphContext::startSceneUpdate()
    {
        mSceneUpdateRequired = false;
        mSceneUpdateInProgress = true;

        for (GraphContextSubscriber* s : mSubscribers)
            s->handleSceneUnavailable();

        mLayouter->scene()->disconnectAll();

    //    LayouterTask* task = new LayouterTask(mLayouter);
    //    connect(task, &LayouterTask::finished, this, &GraphContext::handleLayouterFinished, Qt::ConnectionType::QueuedConnection);
    //    gThreadPool->queueTask(task);

        mLayouter->layout();
        handleLayouterFinished();
    }

    void GraphContext::handleStyleChanged(int istyle)
    {
        Q_UNUSED(istyle);
        handleLayouterFinished();
    }

    QDateTime GraphContext::getTimestamp() const
    {
        return mTimestamp;
    }

    void GraphContext::readFromFile(const QJsonObject& json)
    {
        if (json.contains("timestamp") && json["timestamp"].isString())
            mTimestamp = QDateTime::fromString(json["timestamp"].toString());

        if (json.contains("modules") && json["modules"].isArray())
        {
            QJsonArray jsonMods = json["modules"].toArray();
            int nmods = jsonMods.size();
            for (int imod=0; imod<nmods; imod++)
            {
                QJsonObject jsonMod = jsonMods.at(imod).toObject();
                if (!jsonMod.contains("id") || !jsonMod["id"].isDouble()) continue;
                u32 id = jsonMod["id"].toInt();
                mModules.insert(id);
                if (!jsonMod.contains("x") || !jsonMod["x"].isDouble()) continue;
                int x = jsonMod["x"].toInt();
                if (!jsonMod.contains("y") || !jsonMod["y"].isDouble()) continue;
                int y = jsonMod["y"].toInt();
                Node nd(id,Node::Module);
                mLayouter->setNodePosition(nd,QPoint(x,y));
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
                mGates.insert(id);
                if (!jsonGat.contains("x") || !jsonGat["x"].isDouble()) continue;
                int x = jsonGat["x"].toInt();
                if (!jsonGat.contains("y") || !jsonGat["y"].isDouble()) continue;
                int y = jsonGat["y"].toInt();
                Node nd(id,Node::Gate);
                mLayouter->setNodePosition(nd,QPoint(x,y));
            }
        }

        if (json.contains("nets") && json["nets"].isArray())
        {
            QJsonArray jsonNets = json["nets"].toArray();
            int nnets = jsonNets.size();
            for (int inet=0; inet<nnets; inet++)
            {
                QJsonObject jsonNet = jsonNets.at(inet).toObject();
                if (!jsonNet.contains("id") || !jsonNet["id"].isDouble()) continue;
                u32 id = jsonNet["id"].toInt();
                mNets.insert(id);
            }
        }

        scheduleSceneUpdate();
        setDirty(false);
    }

    void GraphContext::writeToFile(QJsonObject& json)
    {
        json["id"] = (int) mId;
        json["name"] = mName;
        json["timestamp"] = mTimestamp.toString();

        /// modules
        QJsonArray jsonMods;
        for (u32 id : mModules)
        {
            Node searchMod(id, Node::Module);
            const NodeBox* box = getLayouter()->boxes().boxForNode(searchMod);
            Q_ASSERT(box);
            QJsonObject jsonMod;
            jsonMod["id"] = (int) id;
            jsonMod["x"]  = (int) box->x();
            jsonMod["y"]  = (int) box->y();
            jsonMods.append(jsonMod);
        }
        json["modules"] = jsonMods;

        /// gates
        QJsonArray jsonGats;
        for (u32 id : mGates)
        {
            Node searchGat(id, Node::Gate);
            const NodeBox* box = getLayouter()->boxes().boxForNode(searchGat);
            Q_ASSERT(box);
            QJsonObject jsonGat;
            jsonGat["id"] = (int) id;
            jsonGat["x"]  = (int) box->x();
            jsonGat["y"]  = (int) box->y();
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
}
