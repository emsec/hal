//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
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

#include <QMap>
#include <QKeySequence>

namespace hal
{
    class SettingsItemKeybind;

    /**
     * @ingroup settings
     * @brief A map to keep track of the assigned keybinds.
     *
     * The AssignedKeybindMap class keeps track of all configured keybinds and maps
     * the sequence to a specific SettingsItemKeybind. It is implemented
     * as a singleton pattern.
     */
    class AssignedKeybindMap
    {
        static AssignedKeybindMap* inst;
        AssignedKeybindMap() {;}

        QMap<QKeySequence,SettingsItemKeybind*> mKeybindMap;

    public:
        /**
         * Maps a (new) keysequence to the corresponding SettingsItemKeybind. If an old key
         * is provided it is first removed from the map.
         *
         * @param newkey - The (new) key.
         * @param setting - The corresponding keybind setting.
         * @param oldkey - The old key to be removed from the map.
         */
        void assign(const QKeySequence& newkey, SettingsItemKeybind *setting, const QKeySequence& oldkey=QKeySequence());

        /**
         * Get the keybind item that is associated with the given keysequence.
         *
         * @param needle - The keysequence for which the keybind item is wanted.
         * @return The assiciated keybind item.
         */
        SettingsItemKeybind* currentAssignment(const QKeySequence& needle) const;

        /**
         * Clears all entries from the map.
         */
        void initMap() { mKeybindMap.clear(); }

        /**
         * Get the singleton instance of the AssignedKeybindMap.
         *
         * @return The instance.
         */
        static AssignedKeybindMap* instance();
    };
}

