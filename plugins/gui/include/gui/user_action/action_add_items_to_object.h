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
#include "user_action.h"
#include "gui/gui_def.h"
#include <QSet>

namespace hal
{
    /**
     * @ingroup user_action
     * @brief Adds an item to a module or grouping.
     *
     * Adds Module%s, Gate%s, or Net%s to the UserActionObject which must exist and must be of type Module or Grouping.
     *
     * Undo Action: ActionDeleteObject
     */
    class ActionAddItemsToObject : public UserAction
    {
        QSet<u32> mModules;
        QSet<u32> mGates;
        QSet<u32> mNets;
        PlacementHint mPlacementHint;
        Node mPlacementOrigin;
    public:
        /**
         * Action constructor.
         *
         * @param mods - The ids of the modules to add
         * @param gats - The ids of the gates to add
         * @param nets - The ids of the nets to add
         */
        ActionAddItemsToObject(const QSet<u32>& mods = QSet<u32>(),
                               const QSet<u32>& gats = QSet<u32>(),
                               const QSet<u32>& nets = QSet<u32>())
            : mModules(mods), mGates(gats), mNets(nets),
              mPlacementHint(PlacementHint::Standard)
        {;}
        bool exec() override;

        /**
         * Sets the placement hints that are used to layout the added items.
         *
         * @param hint - The new placement hint
         */
        void setPlacementHint(PlacementHint hint) { mPlacementHint = hint; }
        QString tagname() const override;
        void writeToXml(QXmlStreamWriter& xmlOut) const override;
        void readFromXml(QXmlStreamReader& xmlIn) override;
        void addToHash(QCryptographicHash& cryptoHash) const override;
    };

    /**
     * @ingroup user_action
     * @brief UserActionFactory for ActionAddItemsToObject
     */
    class ActionAddItemsToObjectFactory : public UserActionFactory
    {
    public:
        ActionAddItemsToObjectFactory();
        UserAction* newAction() const;
        static ActionAddItemsToObjectFactory* sFactory;
    };
}
