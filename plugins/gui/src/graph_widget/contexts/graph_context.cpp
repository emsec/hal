#include "gui/graph_widget/contexts/graph_context.h"

#include "hal_core/netlist/module.h"

#include "gui/graph_widget/contexts/graph_context_subscriber.h"
#include "gui/graph_widget/graphics_scene.h"
#include "gui/graph_widget/layouters/layouter_task.h"
#include "gui/gui_globals.h"

#include <QVector>

namespace hal
{
    static const bool sLazyUpdates = false;

    GraphContext::GraphContext(const QString& name, QObject* parent)
        : QObject(parent),
          mName(name),
          mUserUpdateCount(0),
          mUnappliedChanges(false),
          mSceneUpdateRequired(false),
          mSceneUpdateInProgress(false)
    {
        mTimestamp = QDateTime::currentDateTime();
    }

    void GraphContext::setLayouter(GraphLayouter* layouter)
    {
        assert(layouter);

        connect(layouter, qOverload<int>(&GraphLayouter::statusUpdate), this, qOverload<int>(&GraphContext::handleLayouterUpdate), Qt::ConnectionType::QueuedConnection);
        connect(layouter, qOverload<const QString&>(&GraphLayouter::statusUpdate), this, qOverload<const QString&>(&GraphContext::handleLayouterUpdate), Qt::ConnectionType::QueuedConnection);

        mLayouter = layouter;
    }

    void GraphContext::setShader(GraphShader* shader)
    {
        assert(shader);

        mShader = shader;
    }

    GraphContext::~GraphContext()
    {
        for (GraphContextSubscriber* subscriber : mSubscribers)
            subscriber->handleContextAboutToBeDeleted();
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

        for (const u32 m : new_modules)
        {
            mModuleHints.insert(placement, m);
        }
        for (const u32 g : new_gates)
        {
            mGateHints.insert(placement, g);
        }

        if (mUserUpdateCount == 0)
        {
            evaluateChanges();
            update();
        }
    }

    void GraphContext::remove(const QSet<u32>& modules, const QSet<u32>& gates)
    {
        QSet<u32> old_modules = modules & mModules;
        QSet<u32> old_gates   = gates & mGates;

        mRemovedModules += old_modules;
        mRemovedGates += old_gates;

        mAddedModules -= modules;
        mAddedGates -= gates;

        // We don't update mModuleHints and mGateHints here. Keeping elements
        // in memory is less of an issue than finding the modules/gates _by value_
        // in the maps and removing them. At the end of applyChanges() the maps are
        // cleared anyway.

        if (mUserUpdateCount == 0)
        {
            evaluateChanges();
            update();
        }
    }

    void GraphContext::clear()
    {
        mRemovedModules = mModules;
        mRemovedGates   = mGates;

        mAddedModules.clear();
        mAddedGates.clear();

        mModuleHints.clear();
        mGateHints.clear();

        if (mUserUpdateCount == 0)
        {
            evaluateChanges();
            update();
        }
    }

    void GraphContext::foldModuleOfGate(const u32 id)
    {
        auto contained_gates = mGates + mAddedGates - mRemovedGates;
        if (contained_gates.find(id) != contained_gates.end())
        {
            auto m = gNetlist->get_gate_by_id(id)->get_module();
            QSet<u32> gates;
            QSet<u32> modules;
            for (const auto& g : m->get_gates(nullptr, true))
            {
                gates.insert(g->get_id());
            }
            for (auto sm : m->get_submodules(nullptr, true))
            {
                modules.insert(sm->get_id());
            }
            beginChange();
            remove(modules, gates);
            add({m->get_id()}, {});
            endChange();
        }
    }

    void GraphContext::unfoldModule(const u32 id)
    {
        auto contained_modules = mModules + mAddedModules - mRemovedModules;

        if (contained_modules.find(id) != contained_modules.end())
        {
            auto m = gNetlist->get_module_by_id(id);
            QSet<u32> gates;
            QSet<u32> modules;
            for (const auto& g : m->get_gates())
            {
                gates.insert(g->get_id());
            }
            for (auto sm : m->get_submodules())
            {
                modules.insert(sm->get_id());
            }

            // That would unfold the empty module into nothing, meaning there would
            // be no way back
            assert(!gates.empty() || !modules.empty());

            beginChange();
            remove({id}, {});
            add(modules, gates);
            endChange();
        }
    }

    bool GraphContext::empty() const
    {
        return mGates.empty() && mModules.empty();
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

        auto m = gNetlist->get_module_by_id(id);
        // TODO deduplicate
        QSet<u32> gates;
        QSet<u32> modules;
        for (const auto& g : m->get_gates())
        {
            gates.insert(g->get_id());
        }
        for (auto sm : m->get_submodules())
        {
            modules.insert(sm->get_id());
        }
        // qDebug() << "GATES" << gates;
        // qDebug() << "MINUS_GATES" << minus_gates;
        // qDebug() << "PLUS_GATES" << plus_gates;
        // qDebug() << "MGATES" << mGates;
        return (mGates - mRemovedGates) + mAddedGates == (gates - minus_gates) + plus_gates && (mModules - mRemovedModules) + mAddedModules == (modules - minus_modules) + plus_modules;
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

    QString GraphContext::name() const
    {
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

    GraphLayouter* GraphContext::debugGetLayouter() const
    {
        return mLayouter;
    }

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

        // number of placement hints is small, not performance critical
        QVector<PlacementHint> queued_hints;
        queued_hints.append(mModuleHints.uniqueKeys().toVector());
        // we can't use QSet for enum class
        for (auto h : mGateHints.uniqueKeys())
        {
            if (!queued_hints.contains(h))
            {
                queued_hints.append(h);
            }
        }

        for (PlacementHint h : queued_hints)
        {
            // call the placer once for each placement hint

            QSet<u32> modules_for_hint = QSet<u32>::fromList(mModuleHints.values(h));
            modules_for_hint &= mAddedModules; // may contain obsolete entries that we must filter out
            QSet<u32> gates_for_hint = QSet<u32>::fromList(mGateHints.values(h));
            gates_for_hint &= mAddedGates;
            mLayouter->add(modules_for_hint, gates_for_hint, mNets, h);
        }

        mShader->add(mAddedModules, mAddedGates, mNets);

        mAddedModules.clear();
        mAddedGates.clear();

        mRemovedModules.clear();
        mRemovedGates.clear();

        mModuleHints.clear();
        mGateHints.clear();

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

    QDateTime GraphContext::getTimestamp() const
    {
        return mTimestamp;
    }
}
