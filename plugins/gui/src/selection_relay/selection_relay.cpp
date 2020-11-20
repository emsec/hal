#include "gui/selection_relay/selection_relay.h"

#include "gui/gui_globals.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/utilities/log.h"

namespace hal
{
    // SET VIA SETTINGS OR TOOLBUTTON
    bool SelectionRelay::sNavigationSkipsEnabled = false;

    SelectionRelay::SelectionRelay(QObject* parent) : QObject(parent),
        mCurrentType(ItemType::None), mFocusType(ItemType::None), mSubfocus(Subfocus::None)
    {
        clear();
    }

    void SelectionRelay::clear()
    {
        mModulesSuppressedByFilter.clear();
        mGatesSuppressedByFilter.clear();
        mNetsSuppressedByFilter.clear();
        mSelectedGates.clear();
        mSelectedNets.clear();
        mSelectedModules.clear();
        mSubfocus       = Subfocus::None;
        mSubfocusIndex = 0;
        mFocusId       = 0;
    }

    void SelectionRelay::clearAndUpdate()
    {
        clear();
        Q_EMIT selectionChanged(nullptr);
    }

    void SelectionRelay::registerSender(void* sender, QString name)
    {
        mSenderRegister.append(QPair<void*, QString>(sender, name));
    }

    void SelectionRelay::removeSender(void* sender)
    {
        for (QPair<void*, QString> pair : mSenderRegister)
        {
            if (pair.first == sender)
                mSenderRegister.removeOne(pair);
        }
    }

    void SelectionRelay::relaySelectionChanged(void* sender)
    {
#ifdef HAL_STUDY
        evaluateSelectionChanged(sender);
#endif
        Q_EMIT selectionChanged(sender);
    }

    void SelectionRelay::relaySubfocusChanged(void* sender)
    {
        Q_EMIT subfocusChanged(sender);
    }

    // TODO deduplicate navigateUp and navigateDown
    void SelectionRelay::navigateUp()
    {
        u32 size = 0;

        switch (mFocusType)
        {
        case ItemType::None: {
                return;
            }
        case ItemType::Gate: {
                Gate* g = gNetlist->get_gate_by_id(mFocusId);

                if (!g)
                    return;

                if (mSubfocus == Subfocus::Left)
                {
                    size = g->get_input_pins().size();

                    if (!size)     // CHECK NECESSARY ???
                        return;    // INVALID STATE, FIX OR IGNORE ???

                    break;
                }

                if (mSubfocus == Subfocus::Right)
                {
                    size = g->get_output_pins().size();

                    if (!size)     // CHECK NECESSARY ???
                        return;    // INVALID STATE, FIX OR IGNORE ???

                    break;
                }

                return;
            }
        case ItemType::Net: {
                Net* n = gNetlist->get_net_by_id(mFocusId);

                if (!n)
                    return;

                if (mSubfocus == Subfocus::Right)
                {
                    size = n->get_destinations().size();

                    if (!size)     // CHECK NECESSARY ???
                        return;    // INVALID STATE, FIX OR IGNORE ???

                    break;
                }

                return;
            }
        case ItemType::Module: {
                Module* m = gNetlist->get_module_by_id(mFocusId);

                if (!m)
                    return;

                if (mSubfocus == Subfocus::Left)
                {
                    size = m->get_input_nets().size();

                    if (!size)     // CHECK NECESSARY ???
                        return;    // INVALID STATE, FIX OR IGNORE ???

                    break;
                }

                if (mSubfocus == Subfocus::Right)
                {
                    size = m->get_output_nets().size();

                    if (!size)     // CHECK NECESSARY ???
                        return;    // INVALID STATE, FIX OR IGNORE ???

                    break;
                }

                return;
            }
        }

        if (mSubfocusIndex == 0)
            mSubfocusIndex = size - 1;
        else
            --mSubfocusIndex;

        Q_EMIT subfocusChanged(nullptr);
    }

    void SelectionRelay::navigateDown()
    {
        u32 size = 0;

        switch (mFocusType)
        {
        case ItemType::None: {
                return;
            }
        case ItemType::Gate: {
                Gate* g = gNetlist->get_gate_by_id(mFocusId);

                if (!g)
                    return;

                if (mSubfocus == Subfocus::Left)
                {
                    size = g->get_input_pins().size();

                    if (!size)     // CHECK NECESSARY ???
                        return;    // INVALID STATE, FIX OR IGNORE ???

                    break;
                }

                if (mSubfocus == Subfocus::Right)
                {
                    size = g->get_output_pins().size();

                    if (!size)     // CHECK NECESSARY ???
                        return;    // INVALID STATE, FIX OR IGNORE ???

                    break;
                }

                return;
            }
        case ItemType::Net: {
                Net* n = gNetlist->get_net_by_id(mFocusId);

                if (!n)
                    return;

                if (mSubfocus == Subfocus::Right)
                {
                    size = n->get_destinations().size();

                    if (!size)     // CHECK NECESSARY ???
                        return;    // INVALID STATE, FIX OR IGNORE ???

                    break;
                }

                return;
            }
        case ItemType::Module: {
                Module* m = gNetlist->get_module_by_id(mFocusId);

                if (!m)
                    return;

                if (mSubfocus == Subfocus::Left)
                {
                    size = m->get_input_nets().size();

                    if (!size)     // CHECK NECESSARY ???
                        return;    // INVALID STATE, FIX OR IGNORE ???

                    break;
                }

                if (mSubfocus == Subfocus::Right)
                {
                    size = m->get_output_nets().size();

                    if (!size)     // CHECK NECESSARY ???
                        return;    // INVALID STATE, FIX OR IGNORE ???

                    break;
                }

                return;
            }
        }

        if (mSubfocusIndex == size - 1)
            mSubfocusIndex = 0;
        else
            ++mSubfocusIndex;

        Q_EMIT subfocusChanged(nullptr);
    }

    // TODO nothing is using this method - do we need it?
    void SelectionRelay::navigateLeft()
    {
        switch (mFocusType)
        {
        case ItemType::None: {
                return;
            }
        case ItemType::Gate: {
                Gate* g = gNetlist->get_gate_by_id(mFocusId);

                if (!g)
                    return;

                if (g->get_input_pins().size())    // CHECK HERE OR IN PRIVATE METHODS ?
                {
                    if (mSubfocus == Subfocus::Left)
                        followGateInputPin(g, mSubfocusIndex);
                    else
                    {
                        if (sNavigationSkipsEnabled && g->get_input_pins().size() == 1)
                            followGateInputPin(g, 0);
                        else
                            subfocusLeft();
                    }
                }

                return;
            }
        case ItemType::Net: {
                Net* n = gNetlist->get_net_by_id(mFocusId);

                if (!n)
                    return;

                if (mSubfocus == Subfocus::Left)
                    followNetToSource(n);
                else
                {
                    if (sNavigationSkipsEnabled && n->get_destinations().size() == 1)
                        followNetToSource(n);
                    else
                        subfocusLeft();
                }

                return;
            }
        case ItemType::Module: {
                Module* m = gNetlist->get_module_by_id(mFocusId);

                if (!m)
                    return;

                if (m->get_input_nets().size())    // CHECK HERE OR IN PRIVATE METHODS ?
                {
                    if (mSubfocus == Subfocus::Left)
                        followModuleInputPin(m, mSubfocusIndex);
                    else
                    {
                        if (sNavigationSkipsEnabled && m->get_input_nets().size() == 1)
                            followModuleInputPin(m, 0);
                        else
                            subfocusLeft();
                    }
                }

                return;
            }
        }
    }

    // TODO nothing is using this method - do we need it?
    void SelectionRelay::navigateRight()
    {
        switch (mFocusType)
        {
        case ItemType::None: {
                return;
            }
        case ItemType::Gate: {
                Gate* g = gNetlist->get_gate_by_id(mFocusId);

                if (!g)
                    return;

                if (mSubfocus == Subfocus::Right)
                    followGateOutputPin(g, mSubfocusIndex);
                else
                {
                    if (sNavigationSkipsEnabled && g->get_output_pins().size() == 1)
                        followGateOutputPin(g, 0);
                    else
                        subfocusRight();
                }

                return;
            }
        case ItemType::Net: {
                Net* n = gNetlist->get_net_by_id(mFocusId);

                if (!n)
                    return;

                if (mSubfocus == Subfocus::Right)
                {
                    followNetToDestination(n, mSubfocusIndex);
                    return;
                }

                if (sNavigationSkipsEnabled && n->get_destinations().size() == 1)
                    followNetToDestination(n, 0);
                else
                    subfocusRight();

                return;
            }
        case ItemType::Module: {
                Module* m = gNetlist->get_module_by_id(mFocusId);

                if (!m)
                    return;

                if (mSubfocus == Subfocus::Right)
                    followModuleOutputPin(m, mSubfocusIndex);
                else
                {
                    if (sNavigationSkipsEnabled && m->get_output_nets().size() == 1)
                        followModuleOutputPin(m, 0);
                    else
                        subfocusRight();
                }

                return;
            }
        }
    }

    void SelectionRelay::suppressedByFilter(const QList<u32>& modIds, const QList<u32>& gatIds, const QList<u32>& netIds)
    {
        mModulesSuppressedByFilter = modIds.toSet();
        mGatesSuppressedByFilter   = gatIds.toSet();
        mNetsSuppressedByFilter    = netIds.toSet();
        Q_EMIT selectionChanged(nullptr);
    }

    bool SelectionRelay::isModuleSelected(u32 id) const
    {
        return mSelectedModules.contains(id) && !mModulesSuppressedByFilter.contains(id);
    }

    bool SelectionRelay::isGateSelected(u32 id) const
    {
        return mSelectedGates.contains(id) && !mGatesSuppressedByFilter.contains(id);
    }

    bool SelectionRelay::isNetSelected(u32 id) const
    {
        return mSelectedNets.contains(id) && !mNetsSuppressedByFilter.contains(id);
    }

    void SelectionRelay::handleModuleRemoved(const u32 id)
    {
        auto it = mSelectedModules.find(id);
        if (it != mSelectedModules.end())
        {
            mSelectedModules.erase(it);
            Q_EMIT selectionChanged(nullptr);
        }
    }

    void SelectionRelay::handleGateRemoved(const u32 id)
    {
        auto it = mSelectedGates.find(id);
        if (it != mSelectedGates.end())
        {
            mSelectedGates.erase(it);
            Q_EMIT selectionChanged(nullptr);
        }
    }

    void SelectionRelay::handleNetRemoved(const u32 id)
    {
        auto it = mSelectedNets.find(id);
        if (it != mSelectedNets.end())
        {
            mSelectedNets.erase(it);
            Q_EMIT selectionChanged(nullptr);
        }
    }

    // GET CORE GUARANTEES
    // UNCERTAIN ABOUT UNROUTED (GLOBAL) NETS, DECIDE
    void SelectionRelay::followGateInputPin(Gate* g, u32 input_pin_index)
    {
        std::string pin_type = *std::next(g->get_input_pins().begin(), input_pin_index);
        Net* n               = g->get_fan_in_net(pin_type);

        if (!n)
            return;    // ADD SOUND OR SOMETHING, ALTERNATIVELY ADD BOOL RETURN VALUE TO METHOD ???

        clear();

        mSelectedNets.insert(n->get_id());

        mFocusType = ItemType::Net;
        mFocusId   = n->get_id();

        if (n->get_destinations().size() == 1)
        {
            if (sNavigationSkipsEnabled)
                mSubfocus = Subfocus::None;
            else
                mSubfocus = Subfocus::Right;

            mSubfocusIndex = 0;
        }
        else
        {
            int i = 0;
            for (auto e : n->get_destinations())
            {
                if (e->get_gate() == g && e->get_pin() == pin_type)
                    break;

                ++i;
            }

            mSubfocus       = Subfocus::Right;
            mSubfocusIndex = i;
        }

        Q_EMIT selectionChanged(nullptr);
    }

    void SelectionRelay::followGateOutputPin(Gate* g, u32 output_pin_index)
    {
        std::string pin_type = *std::next(g->get_output_pins().begin(), output_pin_index);
        auto n               = g->get_fan_out_net(pin_type);

        if (!n)
            return;    // ADD SOUND OR SOMETHING, ALTERNATIVELY ADD BOOL RETURN VALUE TO METHOD ???

        clear();

        mSelectedNets.insert(n->get_id());

        mFocusType = ItemType::Net;
        mFocusId   = n->get_id();

        if (sNavigationSkipsEnabled)
            mSubfocus = Subfocus::None;
        else
            mSubfocus = Subfocus::Left;

        mSubfocusIndex = 0;

        Q_EMIT selectionChanged(nullptr);
    }

    void SelectionRelay::followModuleInputPin(Module* m, u32 input_pin_index)
    {
        Q_UNUSED(m)
        Q_UNUSED(input_pin_index)
        // TODO implement
    }

    void SelectionRelay::followModuleOutputPin(Module* m, u32 output_pin_index)
    {
        Q_UNUSED(m)
        Q_UNUSED(output_pin_index)
        // TODO implement
    }

    void SelectionRelay::followNetToSource(Net* n)
    {
        if(n->get_sources().empty())
            return;

        auto e = n->get_sources().at(0);
        auto g = e->get_gate();

        if (!g)
            return;

        clear();

        mSelectedGates.insert(g->get_id());

        mFocusType = ItemType::Gate;
        mFocusId   = g->get_id();

        if (sNavigationSkipsEnabled && g->get_output_pins().size() == 1)
        {
            mSubfocus       = Subfocus::Left;    // NONE OR LEFT ???
            mSubfocusIndex = 0;
        }
        else
        {
            int i = 0;
            for (const std::string& pin_type : g->get_output_pins())
            {
                if (pin_type == e->get_pin())
                    break;

                ++i;
            }

            mSubfocus       = Subfocus::Right;
            mSubfocusIndex = i;
        }

        Q_EMIT selectionChanged(nullptr);
    }

    void SelectionRelay::followNetToDestination(Net* n, u32 dst_index)
    {
        auto e  = n->get_destinations().at(dst_index);
        Gate* g = e->get_gate();

        if (!g)
            return;

        clear();

        mSelectedGates.insert(g->get_id());

        mFocusType = ItemType::Gate;
        mFocusId   = g->get_id();

        if (sNavigationSkipsEnabled && g->get_input_pins().size() == 1)
        {
            mSubfocus       = Subfocus::Right;    // NONE OR RIGHT ???
            mSubfocusIndex = 0;
        }
        else
        {
            int i = 0;
            for (const std::string& pin_type : g->get_input_pins())
            {
                if (pin_type == e->get_pin())
                    break;

                ++i;
            }

            mSubfocus       = Subfocus::Left;
            mSubfocusIndex = i;
        }

        Q_EMIT selectionChanged(nullptr);
    }

    void SelectionRelay::subfocusNone()
    {
        mSubfocus       = Subfocus::None;
        mSubfocusIndex = 0;    // TECHNICALLY REDUNDANT, KEEP FOR COMPLETENESS ???

        Q_EMIT subfocusChanged(nullptr);
    }

    void SelectionRelay::subfocusLeft()
    {
        mSubfocus       = Subfocus::Left;
        mSubfocusIndex = 0;

        Q_EMIT subfocusChanged(nullptr);
    }

    void SelectionRelay::subfocusRight()
    {
        mSubfocus       = Subfocus::Right;
        mSubfocusIndex = 0;

        Q_EMIT subfocusChanged(nullptr);
    }
#ifdef HAL_STUDY
    void SelectionRelay::evaluateSelectionChanged(void *sender)
    {
        QString method = "unknown";
        for(const auto pair : mSenderRegister)
        {
            if(pair.first == sender)
            {
                method = pair.second;
                break;
            }
        }

        auto createSubstring = [](std::string first_part, QSet<u32> ids){

            std::string final_string = first_part;
            for(const auto &i : ids)
                final_string += std::to_string(i) + ", ";

            if(!ids.isEmpty())
                final_string.resize(final_string.size()-2);

            return final_string + "}";
        };

        std::string gateIdsSubstring = createSubstring("Gate-Ids: {", mSelectedGates);
        std::string netIdsSubstring = createSubstring("Net-Ids: {", mSelectedNets);
        std::string moduleIdsSubstring = createSubstring("Module-Ids: {", mSelectedModules);
        log_info("UserStudy", "Selection changed, Method: {}, New Sel.: {}, {}, {}", method.toStdString(), gateIdsSubstring, netIdsSubstring, moduleIdsSubstring);


    }
#endif
}    // namespace hal
