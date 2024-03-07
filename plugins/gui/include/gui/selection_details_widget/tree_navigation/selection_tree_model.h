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

#include "hal_core/defines.h"
#include "hal_core/netlist/event_system/event_handler.h"
#include "gui/gui_utils/sort.h"
#include "gui/module_model/module_model.h"

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QFont>
#include <QIcon>
#include <QVariant>

namespace hal
{
    /**
     * @ingroup utility_widgets-selection_details
     * @brief A model that contains the current selection.
     *
     * A model that manages the current selection in a tree-styled fashion.
     * Its most important function is fetchSelection that automatically updates
     * the model's internal data.
     */
    class SelectionTreeModel : public ModuleModel
    {
        Q_OBJECT

    public:

        /**
         * The constructor.
         *
         * @param parent - The model's parent.
         */
        SelectionTreeModel(QObject* parent = nullptr);

        /**
         * Updates its internal data. If a groupingId is given, then the items of this grouping are fetched. 
         * Otherwise, if hasEntries is set to True, the current selection is fetched from the selectionRelay. 
         * Elsewise the model is simply cleared.
         *
         * @param hasEntries - Decides whether the current selection is fetched.
         * @param groupingId - if not 0, the id of the grouping to fetch. Otherwise ignored.
         */
        void fetchSelection(bool hasEntries, u32 groupingId);
    };
}
