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

#include "hal_core/defines.h"
#include "module_select_model.h"
#include <QDialog>
#include <QAction>
#include <QKeySequence>
#include <QSet>

class QDialogButtonBox;
class QTreeView;
class QTabWidget;
class QPushButton;

namespace hal {

    class ModuleProxyModel;
    class Searchbar;

    /**
     * @brief The ModuleDialog class opens a popup window for module selection.
     *
     * This feature is mainly used to select the target for the 'move to module...' action.
     * Dialog returns 'accepted' upon any selection or when pushing 'Create new module'
     * Dialog returns 'rejected' upon Cancel button and 'Pick from graph' button
     */
    class ModuleDialog : public QDialog
    {
        Q_OBJECT
    public:
        ModuleDialog(QWidget* parent=nullptr);
        u32 selectedId() const { return mSelectedId; }
        bool isNewModule() const { return mNewModule; }

    private Q_SLOTS:
        void handlePickFromGraph();
        void handleCreateNewModule();
        void handleToggleSearchbar();
        void handleTreeSelectionChanged(const QModelIndex& current, const QModelIndex& previous);
        void handleTreeDoubleClick(const QModelIndex& index);

    public Q_SLOTS:
        void handleTableSelection(u32 id, bool doubleClick);
        void accept() override;
        void keybindToggleSearchbar(const QKeySequence& seq);

    private:
        u32 mSelectedId;
        QDialogButtonBox* mButtonBox;
        ModuleSelectView* mTableView;
        QTreeView* mTreeView;
        ModuleSelectView* mLastUsed;
        QTabWidget* mTabWidget;

        ModuleProxyModel* mModuleTreeProxyModel;
        ModuleSelectModel* mModuleSelectModel;
        Searchbar* mSearchbar;
        QAction* mToggleSearchbar;
        ModuleSelectExclude mSelectExclude;

        QPushButton* mButtonPick;
        bool mNewModule;

        void enableButtons();
        u32 treeModuleId(const QModelIndex& index);
    };
}

