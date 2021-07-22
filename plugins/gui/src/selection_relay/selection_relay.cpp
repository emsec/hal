#include "gui/selection_relay/selection_relay.h"

#include "gui/gui_globals.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/utilities/log.h"
#include "gui/user_action/action_set_selection_focus.h"

namespace hal
{
    // SET VIA SETTINGS OR TOOLBUTTON
    bool SelectionRelay::sNavigationSkipsEnabled = false;

    SelectionRelay::SelectionRelay(QObject* parent) : QObject(parent),
        mAction(nullptr), mDisableExecution(false),
        mFocusType(ItemType::None), mSubfocus(Subfocus::None)
    {
        clear();
    }

    void SelectionRelay::clear()
    {
        initializeAction();
        mAction->mModules.clear();
        mAction->mGates.clear();
        mAction->mNets.clear();
        mAction->mObject        = UserActionObject();
        mAction->mSubfocus      = Subfocus::None;
        mAction->mSubfocusIndex = 0;
        mModulesSuppressedByFilter.clear();
        mGatesSuppressedByFilter.clear();
        mNetsSuppressedByFilter.clear();
    }

    void SelectionRelay::initializeAction()
    {
        if (!mAction)
        {
            mAction = new ActionSetSelectionFocus;
            mAction->mModules = mSelectedModules;
            mAction->mGates   = mSelectedGates;
            mAction->mNets    = mSelectedNets;
            mAction->mObject  = UserActionObject(
                        mFocusId,
                        UserActionObjectType::fromSelectionType(mFocusType));
            mAction->mSubfocus      = mSubfocus;
            mAction->mSubfocusIndex = mSubfocusIndex;
        }
    }

    void SelectionRelay::executeAction()
    {
        if (!mAction || mDisableExecution) return;

        mDisableExecution = true;
        if (mAction->hasModifications())
            mAction->exec();
        else
            delete mAction;
        mAction = nullptr;
        mDisableExecution = false;
    }

    void SelectionRelay::addGate(u32 id)
    {
        initializeAction();
        mAction->mGates.insert(id);
    }

    void SelectionRelay::addNet(u32 id)
    {
        initializeAction();
        mAction->mNets.insert(id);
    }

    void SelectionRelay::addModule(u32 id)
    {
        initializeAction();
        mAction->mModules.insert(id);
    }

    QList<UserActionObject> SelectionRelay::toUserActionObject() const
    {
        QList<UserActionObject> retval;
        for (u32 id : mSelectedModules)
            retval.append(UserActionObject(id,UserActionObjectType::Module));
        for (u32 id : mSelectedGates)
            retval.append(UserActionObject(id,UserActionObjectType::Gate));
        for (u32 id : mSelectedNets)
            retval.append(UserActionObject(id,UserActionObjectType::Net));
        return retval;
    }

    void SelectionRelay::setSelectedGates(const QSet<u32>& ids)
    {
        initializeAction();
        mAction->mGates = ids;
    }

    void SelectionRelay::setSelectedNets(const QSet<u32>& ids)
    {
        initializeAction();
        mAction->mNets = ids;
    }

    void SelectionRelay::setSelectedModules(const QSet<u32>& ids)
    {
        initializeAction();
        mAction->mModules = ids;
    }

    void SelectionRelay::actionSetSelected(const QSet<u32>& mods, const QSet<u32>& gats, const QSet<u32>& nets)
    {
        mSelectedModules = mods;
        mSelectedGates   = gats;
        mSelectedNets    = nets;
        Q_EMIT selectionChanged(nullptr);
    }

    void SelectionRelay::removeGate(u32 id)
    {
        initializeAction();
        mAction->mGates.remove(id);
    }

    void SelectionRelay::removeNet(u32 id)
    {
        initializeAction();
        mAction->mNets.remove(id);
    }

    void SelectionRelay::removeModule(u32 id)
    {
        initializeAction();
        mAction->mModules.remove(id);
    }

    void SelectionRelay::setFocus(ItemType ftype, u32 fid, Subfocus sfoc, u32 sfinx)
    {
        initializeAction();
        mAction->setObject(UserActionObject(fid,UserActionObjectType::fromSelectionType(ftype)));
        mAction->mSubfocus = sfoc;
        mAction->mSubfocusIndex = sfinx;
    }

    void SelectionRelay::setFocusDirect(ItemType ftype, u32 fid, Subfocus sfoc, u32 sfinx)
    {
        mFocusType = ftype;
        mFocusId   = fid;
        mSubfocus  = sfoc;
        mSubfocusIndex = sfinx;
    }

    void SelectionRelay::clearAndUpdate()
    {
        clear();
        executeAction();
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
#else
        Q_UNUSED(sender);
#endif
        executeAction();
    }

    void SelectionRelay::relaySubfocusChanged(void* sender)
    {
        Q_EMIT subfocusChanged(sender);
        executeAction();
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
                    size = g->get_type()->get_input_pins().size();

                    if (!size)     // CHECK NECESSARY ???
                        return;    // INVALID STATE, FIX OR IGNORE ???

                    break;
                }

                if (mSubfocus == Subfocus::Right)
                {
                    size = g->get_type()->get_output_pins().size();

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

        initializeAction();
        if (mSubfocusIndex == 0)
            mAction->mSubfocusIndex = size - 1;
        else
            --mAction->mSubfocusIndex;
        relaySubfocusChanged(nullptr);
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
                    size = g->get_type()->get_input_pins().size();

                    if (!size)     // CHECK NECESSARY ???
                        return;    // INVALID STATE, FIX OR IGNORE ???

                    break;
                }

                if (mSubfocus == Subfocus::Right)
                {
                    size = g->get_type()->get_output_pins().size();

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

        initializeAction();
        if (mSubfocusIndex == size - 1)
            mAction->mSubfocusIndex = 0;
        else
            ++mAction->mSubfocusIndex;
        relaySubfocusChanged(nullptr);
    }

    void SelectionRelay::suppressedByFilter(const QList<u32>& modIds, const QList<u32>& gatIds, const QList<u32>& netIds)
    {
        initializeAction();
        mModulesSuppressedByFilter = modIds.toSet();
        mGatesSuppressedByFilter   = gatIds.toSet();
        mNetsSuppressedByFilter    = netIds.toSet();
        executeAction();
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
            initializeAction();
            mAction->mModules.remove(id);
            executeAction();
        }
    }

    void SelectionRelay::handleGateRemoved(const u32 id)
    {
        auto it = mSelectedGates.find(id);
        if (it != mSelectedGates.end())
        {
            initializeAction();
            mAction->mGates.remove(id);
            executeAction();
        }
    }

    void SelectionRelay::handleNetRemoved(const u32 id)
    {
        auto it = mSelectedNets.find(id);
        if (it != mSelectedNets.end())
        {
            initializeAction();
            mAction->mGates.remove(id);
            executeAction();
        }
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

    void SelectionRelay::subfocusNone()
    {
        initializeAction();
        mAction->mSubfocus = Subfocus::None;
        mAction->mSubfocusIndex = 0;
        relaySubfocusChanged(nullptr);
    }

    void SelectionRelay::subfocusLeft()
    {
        initializeAction();
        mAction->mSubfocus = Subfocus::Left;
        mAction->mSubfocusIndex = 0;
        relaySubfocusChanged(nullptr);
    }

    void SelectionRelay::subfocusRight()
    {
        mSubfocus       = Subfocus::Right;
        mSubfocusIndex = 0;

        Q_EMIT subfocusChanged(nullptr);
    }

    QList<Node> SelectionRelay::selectedNodesList() const
    {
        QList<Node> retval;
        for (u32 mid : selectedModulesList())
            retval.append(Node(mid,Node::Module));
        for (u32 gid : selectedGatesList())
            retval.append(Node(gid,Node::Gate));
        return retval;
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
