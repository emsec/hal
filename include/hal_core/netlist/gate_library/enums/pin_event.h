// MIT License
//
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include "hal_core/utilities/enums.h"
#include "hal_core/defines.h"
#include <unordered_map>
#include <vector>

namespace hal
{
    /**
     * Spezifies the pin_changed event type
     *
     * The order of events in enum class defines the order in which events are handled.
     *
     */
    enum class PinEvent
    {
        unknown,
        GroupCreate,                    /// new pin group created
        GroupRename,                    /// pin group renamed
        GroupTypeChange,                /// changed PinType attribute of group (like data)
        GroupDirChange,                 /// changed PinDirection attribute of group (like input)
        GroupReorder,                   /// moved group to a new position within containing module
        PinCreate,                      /// new pin created
        PinAssignToGroup,               /// pin assigned to new group
        PinRename,                      /// pin renamed
        PinTypeChange,                  /// changed PinType attribute of pin (like data)
        PinDirChange,                   /// changed PinDirection attribute of pin (like input)
        PinReorder,                     /// moved pin to a new position within containing group
        PinDelete,                      /// pin deleted
        GroupDelete                     /// group deleted
    };

    template<>
    std::map<PinEvent, std::string> EnumStrings<PinEvent>::data;

    class Module;

    /**
     * Wrapper class for core pin_changed events.
     *
     * Events can be send immediately or stacked and send at according to their priority.
     */
    class PinChangedEvent
    {
        friend bool pin_event_order(const PinChangedEvent& a, const PinChangedEvent& b);
        friend class PinChangedEventScope;

        /**
         * Subclass for event stack.
         */
        class EventStack : public std::vector<PinChangedEvent>
        {
        public:
            /**
             * Scope count indicates the nesting depth of event-throwing subroutines.
             * Only the top level (m_count=0) is allowed to send the events from stack.
             */
            int m_count;

            /**
             * Construct empty stack
             */
            EventStack() : m_count(0) {;}

            /**
             * Attempts to send events, typically at the end of a pin-changing subroutine.
             * Events will only be send if m_count is zero.
             */
            void send_events(Module* m);
        };

        static std::unordered_map<Module*,EventStack*> s_event_stack;

        Module* m_module;
        PinEvent m_event;
        u32 m_id;

    public:
        /**
         * PinChangedEvent class for single event
         * @param m   - The module comprising pins and pin groups
         * @param pev - The pin event enum
         * @param id  - pin or pin group ID
         */
        PinChangedEvent(Module* m, PinEvent pev, u32 id);

        /**
         * Returns the module for which pins or pin groups have been changed
         * @return The module comprising pins and pin groups
         */
        Module* get_module() const;

        /**
         * Return bitwise binary encoded PinEvent and ID
         *  4LSB =  The pin event enum as 4 bit int
         * 28HSB =  The ID as 28 bit int
         * @return The bitcode according to scheme above
         */
        u32 associated_data();

        /**
         * Attempts to send event.
         * If this routine or any calling routine wants to collect events the event gets written on stack instead.
         */
        void send();
    };

    /**
     * By creating an instance of this class a new scope gets created thus collecting events.
     */
    class PinChangedEventScope
    {
        Module* m_module;
    public:

        /**
         * Constructor for scope instance incrementing scope count
         * @param m The module comprising pins and pin groups
         */
        PinChangedEventScope(Module* m);

        /**
         * Destructor for scope instance decrementing scope count
         */
        ~PinChangedEventScope();

        /**
         * Attempts to send all stacked events. Will do nothing if not issued from top-level scope.
         */
        void send_events();
    };

    /**
     * Function used by sort algorithm to organize events according to their priority.
     * @param a - Pin changed event A
     * @param b - Pin changed event B
     * @return true if A should be handled before B, false otherwise.
     */
    bool pin_event_order(const PinChangedEvent& a, const PinChangedEvent& b);
}
