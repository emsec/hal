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
#include "gui/content_widget/content_widget.h"
#include "gui/module_model/module_item.h"
#include "gui/searchbar/searchbar.h"
#include "gui/selection_relay/selection_relay.h"
#include "gui/module_widget/module_tree_view.h"
#include "hal_core/netlist/event_system/module_event_handler.h"
#include "hal_core/netlist/module.h"

#include <QAction>
#include <QItemSelection>
#include <QList>
#include <QObject>
#include <QSortFilterProxyModel>

class QTreeView;

namespace hal
{
    class ModuleProxyModel;

    class ModuleWidget : public ContentWidget
    {
        Q_OBJECT

    public:
        ModuleWidget(QWidget* parent = nullptr);

        virtual void setupToolbar(Toolbar* Toolbar) override;
        virtual QList<QShortcut*> createShortcuts() override;

    public Q_SLOTS:
        void toggleSearchbar();
        void filter(const QString& text);
        void handleTreeViewContextMenuRequested(const QPoint& point);
        void handleTreeSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
        void handleItemDoubleClicked(const QModelIndex& index);
        void handleSelectionChanged(void* sender);
        void handleModuleRemoved(Module* module, u32 module_id);

    private:
        ModuleTreeView* mTreeView;
        Searchbar mSearchbar;

        QAction* mFilterAction;

        QSortFilterProxyModel* mCurrentModel;

        QList<QRegExp*> mRegexps;

        bool mIgnoreSelectionChange;

        ModuleProxyModel* mModuleProxyModel;

        void openModuleInView(const QModelIndex& index);

        ModuleItem* getModuleItemFromIndex(const QModelIndex& index);
    };
}    // namespace hal
