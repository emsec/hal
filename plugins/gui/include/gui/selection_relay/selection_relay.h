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

#include <QObject>
#include <QPair>
#include <QVector>
#include <QSet>

namespace hal
{
    class Gate;
    class Module;
    class Net;

    class SelectionRelay : public QObject
    {
        Q_OBJECT

    public:
        enum class ItemType
        {
            None   = 0,
            Gate   = 1,
            Net    = 2,
            Module = 3
        };

        enum class Subfocus
        {
            None  = 0,
            Left  = 1,
            Right = 2
            //        up     = 3,
            //        down   = 4,
            //        center = 5,
        };

        enum class Mode
        {
            Override = 0,
            Add      = 1,
            Remove   = 2
        };

        explicit SelectionRelay(QObject* parent = nullptr);

        void clear(); // does not emit the "update" signal!
        void clearAndUpdate();

        void registerSender(void* sender, QString name);
        void removeSender(void* sender);

        // TEST METHOD
        // USE RELAY METHODS OR ACCESS SIGNALS DIRECTLY ???
        void relaySelectionChanged(void* sender);
        void relaySubfocusChanged(void* sender);

        void navigateUp();
        void navigateDown();
        void navigateLeft();
        void navigateRight();

        void handleModuleRemoved(const u32 id);
        void handleGateRemoved(const u32 id);
        void handleNetRemoved(const u32 id);

        bool isModuleSelected(u32 id) const;
        bool isGateSelected(u32 id) const;
        bool isNetSelected(u32 id) const;

        void suppressedByFilter(const QList<u32>& modIds = QList<u32>(),
                                const QList<u32>& gatIds = QList<u32>(),
                                const QList<u32>& netIds = QList<u32>());

    Q_SIGNALS:
        // TEST SIGNAL
        // ADD ADDITIONAL INFORMATION (LIKE PREVIOUS FOCUS) OR LEAVE THAT TO SUBSCRIBERS ???
        // USE SEPARATE OR COMBINED SIGNALS ??? MEANING DOES A SELECTION CAHNGE FIRE A SUBSELECTION CHANGED SIGNAL OR IS THAT IMPLICIT
        void selectionChanged(void* sender);
        //void focus_changed(void* sender); // UNCERTAIN
        void subfocusChanged(void* sender);

    public:
        QSet<u32> mSelectedGates;
        QSet<u32> mSelectedNets;
        QSet<u32> mSelectedModules;

        // MAYBE UNNECESSARY
        ItemType mCurrentType;
        u32 mCurrentId;

        // USE ARRAY[0] INSTEAD OF MEMBER ???
        ItemType mFocusType;
        u32 mFocusId;

        Subfocus mSubfocus;
        u32 mSubfocusIndex;    // HANDLE VIA INT OR STRING ?? INDEX HAS TO BE KNOWN ANYWAY TO FIND NEXT / PREVIOUS BOTH OPTIONS KIND OF BAD

    private:
        QSet<u32> mModulesSuppressedByFilter;
        QSet<u32> mGatesSuppressedByFilter;
        QSet<u32> mNetsSuppressedByFilter;

        static bool sNavigationSkipsEnabled;    // DOES THIS HAVE ANY USE ???

        // RENAME THESE METHODS ???
        void followGateInputPin(Gate* g, u32 input_pin_index);
        void followGateOutputPin(Gate* g, u32 output_pin_index);
        void followModuleInputPin(Module* m, u32 input_pin_index);
        void followModuleOutputPin(Module* m, u32 output_pin_index);

        void followNetToSource(Net* n);
        void followNetToDestination(Net* n, u32 dst_index);


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
