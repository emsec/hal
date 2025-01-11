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
#include "gui/module_model/module_model.h"
#include "gui/selection_details_widget/module_details_widget/filter_elements_proxy_model.h"

#include <QTreeView>

namespace hal
{
    class Module;
    class ModuleTreeModel;

    /**
     * @brief A widget to display the specific (direct submodules, gates) items of a given module.
     */
    class ModuleElementsTree : public QTreeView
    {
        Q_OBJECT

    public:
        /**
         * The constructor.
         *
         * @param parent - The widget's parent.
         */
        ModuleElementsTree(QWidget* parent = nullptr);

        /**
         * Sets the module and updates its model to display the module's
         * direct submodules and gates.
         *
         * @param moduleID - The module id.
         */
        void setModule(u32 moduleID);

        /**
         * Sets the module and updates its model to display the module's
         * direct submodules and gates.
         *
         * @param moduleID - The module.
         */
        void setModule(Module* m);

        /**
         * Resets the model and shows an empty view as a result.
         */
        void removeContent();

        /** @name Event Handler Functions
         */
        ///@{
        void handleContextMenuRequested(const QPoint &pos);
        ///@}

    Q_SIGNALS:
        /**
         * Q_SIGNAL that is emitted when the number of direct submodules changes.
         * (Primarily used to update 'the' section header in the details widget)
         *
         * @param newHeadline - The new headline.
         */
        void updateText(const QString& newHeadline);

    private:
        ModuleModel* mModel;
        FilterElementsProxyModel* mProxyModel;
        int mModuleID;

        void handleNumberSubmodulesChanged(const int number);
    };

}
