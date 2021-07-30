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
#include "gui/module_dialog/gate_select_model.h"
#include <QDialog>
#include <QAction>
#include <QKeySequence>
#include <QSet>

class QDialogButtonBox;
class QTreeView;
class QTabWidget;
class QPushButton;

namespace hal {

    class GateProxyModel;
    class Searchbar;

    /**
     * @brief The GateDialog class opens a popup window for module selection.
     *
     * This feature is mainly used to select the target for the 'move to module...' action.
     * Dialog returns 'accepted' upon any selection or when pushing 'Create new module'
     * Dialog returns 'rejected' upon Cancel button and 'Pick from graph' button
     */
    class GateDialog : public QDialog
    {
        Q_OBJECT
    public:
        /**
         * The constructor.
         *
         * @param parent - The dialog's parent.
         */


        GateDialog(u32 orig, bool succ, const QSet<u32>& selectable = QSet<u32>(), QWidget* parent=nullptr);

        /**
         * Get the through this dialog selected id.
         *
         * @return The selected id.
         */
        u32 selectedId() const { return mSelectedId; }

        /**
         * hide picker button
         */
        void hidePicker();

    private Q_SLOTS:
        void handlePickFromGraph();
        void handleToggleSearchbar();
        void handleTreeSelectionChanged(const QModelIndex& current, const QModelIndex& previous);
        void handleTreeDoubleClick(const QModelIndex& index);

    public Q_SLOTS:
        /**
         *
         * Checks if the selected module id is valid (not excluded). If yes and the doubleClick
         * parameter is set to True, the dialog is accepted. If doubleClick is False, the dialog
         * buttons will be enabled.
         *
         * @param id - The selected id.
         * @param doubleClick - True to accept the dialog when id is valid. False to enable buttons.
         */
        void handleTableSelection(u32 id, bool doubleClick);

        /**
         * Updates the module select history and accepts the dialog.
         */
        void accept() override;

        /**
         * Sets the keybind to toggle the searchbar. Used when the settings changes.
         *
         * @param seq - The new sequence.
         */
        void keybindToggleSearchbar(const QKeySequence& seq);

    private:
        u32 mSelectedId;
        u32 mOrigin;
        bool mQuerySuccessor;
        QSet<u32> mSelectableGates;
        QDialogButtonBox* mButtonBox;
        GateSelectView* mTableView;
        QTreeView* mTreeView;
        GateSelectView* mLastUsed;
        QTabWidget* mTabWidget;

        GateProxyModel* mGateTreeProxyModel;
        Searchbar* mSearchbar;
        QAction* mToggleSearchbar;

        QPushButton* mButtonPick;

        void enableButtons();
        u32 treeGateId(const QModelIndex& index);
    };
}

