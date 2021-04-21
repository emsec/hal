//  MIT License
//
//  Copyright (c) 2019 Ruhr-University Bochum, Germany, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once

#include "hal_core/defines.h"
#include "hal_config.h"
#include <vector>
#include <QObject>
#include <QPair>
#include <QVector>
#include <QSet>
#include <QList>

namespace hal
{
    class Gate;
    class Module;
    class Net;

    class ActionSetSelectionFocus;
    class UserActionObject;

    /**
     * @ingroup gui
     * The SelectionRelay is used to manage the selection and the focus of Module%s, Gate%s and Net%s. <br>
     * There can be any number of selected modules and/or gates and/or nets, but only one focused item at a time. <br>
     * This class contains signals to notify objects about changes of the focus and the selection. However these
     * signals must be emitted manually by calling the functions relaySelectionChanged and relaySubfocusChanged.
     */
    class SelectionRelay : public QObject
    {
        Q_OBJECT

    public:
        /**
         * <b>enum</b> used to describe the type of a selected/focused item. ItemType::None can be used to specify that
         * the Item is empty e.g. no item is focused.
         */
        enum class ItemType
        {
            None   = 0,
            Gate   = 1,
            Net    = 2,
            Module = 3
        };

        /**
         * <b>enum</b> used to describe the sub-focus. The sub-focus is the focus within the focused module/gate item. <br>
         *  - If an input pin is selected, the sub-focus is Subfocus::Left <br>
         *  - If an output pin is selected, the sub-focus is Subfocus::Right <br>
         *  - In any other case the sub-focus is Subfocus::None
         */
        enum class Subfocus
        {
            None  = 0,
            Left  = 1,
            Right = 2
            //        up     = 3,
            //        down   = 4,
            //        center = 5,
        };

        /**
         * Unused? TODO: Remove?
         */
        enum class Mode
        {
            Override = 0,
            Add      = 1,
            Remove   = 2
        };

        /**
         * Constructor.
         *
         * @param parent - The parent QObject
         */
        explicit SelectionRelay(QObject* parent = nullptr);

        /**
         * Clears all member variables.
         * Does not emit the selectionChanged signal.
         */
        void clear();

        /**
         * Clears all member variables.
         * Emits selectionChanged.
         */
        void clearAndUpdate();

        /**
         * Registers a sender object. Used in the hal user study to keep track of the selection changes.
         *
         * @param sender - The object to register
         * @param name - The name of the object
         */
        void registerSender(void* sender, QString name);

        /**
         * Unregisters a sender object. Used in the hal user study to keep track of the selection changes.
         *
         * @param sender - The object to unregister
         */
        void removeSender(void* sender);

        // TEST METHOD
        // USE RELAY METHODS OR ACCESS SIGNALS DIRECTLY ???
        /**
         * Emits the selectionChanged signal. If compiled with the HAL_STUDY flag it also invokes the
         * evaluateSelectionChanged function. <br>
         * If the receiver wants to prevent event handling of certain senders (e.g. itself to prevent infinite loops),
         * it can check the sender pointer. Therefore the sender should always pass a <i>this</i>-pointer.
         *
         * @param sender - The object that invokes this function.
         */
        void relaySelectionChanged(void* sender);

        /**
         * Emits the sub-focusChanged signal.
         * If the receiver wants to prevent event handling of certain senders (e.g. itself to prevent infinite loops),
         * it can check the sender pointer. Therefore the sender should always pass a <i>this</i>-pointer.
         *
         * @param sender - The object that invokes this function.
         */
        void relaySubfocusChanged(void* sender);

        /**
         * Navigates up in the currently focused object. It only adjusts the sub-focus index mSubfocusIndex of the
         * current sub-focus. Afterwards if the index has changed, it emits the signal subfocusChanged.
         */
        void navigateUp();

        /**
         * Navigates down in the currently focused object. It only adjusts the sub-focus index mSubfocusIndex of the
         * current sub-focus. Afterwards if the index has changed, it emits the signal subfocusChanged.
         */
        void navigateDown();

        /**
         * Called whenever a module was removed from the netlist. Used to remove the id of the removed module from the
         * selection (if available).
         *
         * @param id - The id of the removed module
         */
        void handleModuleRemoved(const u32 id);

        /**
         * Called whenever a gate was removed from the netlist. Used to remove the id of the removed gate from the
         * selection (if available).
         *
         * @param id - The id of the removed gate
         */
        void handleGateRemoved(const u32 id);

        /**
         * Called whenever a gate was removed from the netlist. Used to remove the id of the removed gate from the
         * selection (if available).
         *
         * @param id - The id of the removed net
         */
        void handleNetRemoved(const u32 id);

        /**
         * Decides whether a module is selected or not. <br>
         * Note that this function will always return <b>false</b> for modules that appear in the
         * mModulesSuppressedByFilter member (set by suppressedByFilter).
         *
         * @param id - The id of the module
         * @returns <b>true</b> if the module is selected and not suppressed.
         */
        bool isModuleSelected(u32 id) const;

        /**
         * Decides whether a gate is selected or not. <br>
         * Note that this function will always return <b>false</b> for gates that appear in the
         * mGatesSuppressedByFilter member (set by suppressedByFilter).
         *
         * @param id - The id of the gate
         * @returns <b>true</b> if the gate is selected and not suppressed.
         */
        bool isGateSelected(u32 id) const;

        /**
         * Decides whether a net is selected or not. <br>
         * Note that this function will always return <b>false</b> for nets that appear in the
         * mNetsSuppressedByFilter member (set by suppressedByFilter).
         *
         * @param id - The id of the net
         * @returns <b>true</b> if the net is selected and not suppressed.
         */
        bool isNetSelected(u32 id) const;

        /**
         * Overwrites the list of suppressed modules, gates and nets. Suppressed items can't be selected
         * i.e. isModuleSelected/isGateSelected/isNetSelected will always return <b>false</b>.
         * Note that this only applies to these three functions. It does not restrict their appearance in the
         * public members mSelectedGates/mSelectedNets/mSelectedModules.
         *
         * @param modIds - A list of suppressed module ids
         * @param gatIds - A list of suppressed gate ids
         * @param netIds - A list of suppressed net ids
         */
        void suppressedByFilter(const QList<u32>& modIds = QList<u32>(),
                                const QList<u32>& gatIds = QList<u32>(),
                                const QList<u32>& netIds = QList<u32>());

        QList<u32> selectedGatesList()   const { return mSelectedGates.toList(); }
        QList<u32> selectedNetsList()    const { return mSelectedNets.toList(); }
        QList<u32> selectedModulesList() const { return mSelectedModules.toList(); }

        std::vector<u32> selectedGatesVector()   const { return std::vector<u32>(mSelectedGates.begin(), mSelectedGates.end()); }
        std::vector<u32> selectedNetsVector()    const { return std::vector<u32>(mSelectedNets.begin(), mSelectedNets.end()); }
        std::vector<u32> selectedModulesVector() const { return std::vector<u32>(mSelectedModules.begin(), mSelectedModules.end()); }

        const QSet<u32>& selectedGates()   const { return mSelectedGates; }
        const QSet<u32>& selectedNets()    const { return mSelectedNets; }
        const QSet<u32>& selectedModules() const { return mSelectedModules; }

        int numberSelectedGates()   const { return mSelectedGates.size(); }
        int numberSelectedNets()    const { return mSelectedNets.size(); }
        int numberSelectedModules() const { return mSelectedModules.size(); }
        int numberSelectedNodes()   const { return mSelectedGates.size() + mSelectedModules.size(); }
        int numberSelectedItems()   const { return mSelectedGates.size() + mSelectedModules.size() + mSelectedNets.size(); }

        bool containsGate(u32 id) const   { return mSelectedGates.contains(id); }
        bool containsNet(u32 id) const    { return mSelectedNets.contains(id); }
        bool containsModule(u32 id) const { return mSelectedModules.contains(id); }

        void addGate(u32 id);
        void addNet(u32 id);
        void addModule(u32 id);

        void setSelectedGates(const QSet<u32>& ids);
        void setSelectedNets(const QSet<u32>& ids);
        void setSelectedModules(const QSet<u32>& ids);
        void actionSetSelected(const QSet<u32>& mods, const QSet<u32>& gats, const QSet<u32>& nets);

        void removeGate(u32 id);
        void removeNet(u32 id);
        void removeModule(u32 id);
    Q_SIGNALS:
        // TEST SIGNAL
        // ADD ADDITIONAL INFORMATION (LIKE PREVIOUS FOCUS) OR LEAVE THAT TO SUBSCRIBERS ???
        // USE SEPARATE OR COMBINED SIGNALS ??? MEANING DOES A SELECTION CAHNGE FIRE A SUBSELECTION CHANGED SIGNAL OR IS THAT IMPLICIT
        /**
         * Q_SIGNAL to notify that the selection has been changed.
         * If the receiver wants to prevent event handling of certain senders (e.g. itself to prevent infinite loops),
         * it can check the sender pointer. Therefore the sender should always pass a <i>this</i>-pointer.
         *
         * @param sender - The object
         */
        void selectionChanged(void* sender);
        //void focus_changed(void* sender); // UNCERTAIN

        /**
         * Q_SIGNAL to notify that the sub-focus has been changed.
         * If the receiver wants to prevent event handling of certain senders (e.g. itself to prevent infinite loops),
         * it can check the sender pointer. Therefore the sender should always pass a <i>this</i>-pointer.
         *
         * @param sender - The object
         */
        void subfocusChanged(void* sender);

    public:
        // TODO: encapsulate public members?
        /**
         * Contains the currently selected gates. <br>
         * Please call relaySelectionChanged after manipulations of this member.
         */
        ItemType focusType() const { return mFocusType; }

        /**
         * Contains the currently selected nets. <br>
         * Please call relaySelectionChanged after manipulations of this member.
         */
        u32 focusId()        const { return mFocusId; }

        /**
         * Contains the currently selected modules. <br>
         * Please call relaySelectionChanged after manipulations of this member.
         */
        Subfocus subfocus()  const { return mSubfocus; }
        u32 subfocusIndex()  const { return mSubfocusIndex; }

        void setFocus(ItemType ftype, u32 fid, Subfocus sfoc = Subfocus::None, u32 sfinx = 0);
        /**
         * <b>Unused!</b> <br>
         * \deprecated Please use mFocusType to access the ItemType of the current focus.
         */
        void setFocusDirect(ItemType ftype, u32 fid, Subfocus sfoc = Subfocus::None, u32 sfinx = 0);

        /**
         * <b>Unused!</b> <br>
         * \deprecated Please use mFocusId to access the id of the current focused item.
         */

        QList<UserActionObject> toUserActionObject() const;

    private:
        void initializeAction();
        void executeAction();

        ActionSetSelectionFocus* mAction;
        bool mDisableExecution;

        // USE ARRAY[0] INSTEAD OF MEMBER ???
        /**
         * Contains the ItemType of the currently focused item. <br>
         * ItemType::None represents no focus.
         */
        ItemType mFocusType;

        /**
         * Contains the id of the currently focused item. <br>
         * The id is related to the module id/gate id/net id (depends on the ItemType mFocusType)
         */
        u32 mFocusId;

        /**
         * The sub-focus of the currently focused gate/modules. <br>
         * E.g. if an input pin is selected, this member would be mSubfocus::Left.
         */
        Subfocus mSubfocus;

        /**
         * The index within the currently focused sub-focus. <br>
         * E.g. if the second input pin is selected: <br>
         *  - mSubfocus = Subfocus::Left  <br>
         *  - mSubfocusIndex = 1
         */
        u32 mSubfocusIndex;    // HANDLE VIA INT OR STRING ?? INDEX HAS TO BE KNOWN ANYWAY TO FIND NEXT / PREVIOUS BOTH OPTIONS KIND OF BAD

        QSet<u32> mSelectedGates;
        QSet<u32> mSelectedNets;
        QSet<u32> mSelectedModules;

        QSet<u32> mModulesSuppressedByFilter;
        QSet<u32> mGatesSuppressedByFilter;
        QSet<u32> mNetsSuppressedByFilter;

        static bool sNavigationSkipsEnabled;    // DOES THIS HAVE ANY USE ???

        // RENAME THESE METHODS ???
        void followModuleInputPin(Module* m, u32 input_pin_index);
        void followModuleOutputPin(Module* m, u32 output_pin_index);

#ifdef HAL_STUDY
        void evaluateSelectionChanged(void* sender);
#endif
        void subfocusNone();
        void subfocusLeft();
        void subfocusRight();


        //    bool try_subfocus_left();
        //    bool try_subfocus_right();
        //    bool try_subfocus_up();
        //    bool try_subfocus_down();

        QVector<QPair<void*, QString>> mSenderRegister;
    };
}
