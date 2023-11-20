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

#include "gui/selection_details_widget/details_tab_widget.h"
#include "gui/selection_details_widget/data_table_widget.h"
#include "gui/selection_details_widget/groupings_of_item_widget.h"

namespace hal
{
    class Module;
    class DetailsFrameWidget;
    class ModuleInfoTable;
    class ModulePinsTree;
    class ModuleElementsTree;
    class DataTableWidget;
    class GroupingsOfItemWidget;
    class CommentWidget;

    /**
     * @ingroup utility_widgets-selection_details
     * @brief The DetailsTabWidget that is responsible for showing Module details
     * 
     * This tab widget contains and manages the tabs that are shown when selecting a Module in the SelectionDetailsWidet's tree.
     */
    class ModuleDetailsTabWidget : public DetailsTabWidget
    {
        Q_OBJECT

    public:

        /**
         * The constructor.
         *
         * @param parent - The widget's parent.
         */
         ModuleDetailsTabWidget(QWidget* parent = nullptr);

        /**
         * Sets the module which details are shown in the tabs and various widgets.
         *
         * @param module - The module.
         */
        void setModule(Module* module);

        /**
         * Resets the widget's pin- and elements-widget to its initial state.
         */
        void clear();

    private Q_SLOTS:
        void handleModuleColorChanged(u32 id);

    private:

        //general tab
        ModuleInfoTable* mModuleInfoTable;
        DetailsFrameWidget* mModuleInformationFrame;

        //groupings tab
        GroupingsOfItemWidget* mGroupingsOfItemTable;
        DetailsFrameWidget* mGroupingsFrame;

        //ports tab
        ModulePinsTree* mPinsTree;
        DetailsFrameWidget* mPinsFrame;

        //elements tab
        ModuleElementsTree* mElementsTree;
        DetailsFrameWidget* mElementsFrame;

        //data tab
        DataTableWidget* mDataTable;
        DetailsFrameWidget* mDataFrame;

        //comment tab
        CommentWidget* mCommentWidget;

        //store module id
        u32 mModuleId;
    };
}
