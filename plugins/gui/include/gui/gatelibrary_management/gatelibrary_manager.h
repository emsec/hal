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

#include "gatelibrary_table_model.h"
#include "gui/gatelibrary_management/gatelibrary_wizard.h"
#include "gui/gatelibrary_management/gatelibrary_content_widget.h"
#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/netlist/gate_library/gate_type.h"


#include <QFrame>
#include "gui/gatelibrary_management/gatelibrary_tab_widgets/gatelibrary_tab_truth_table.h"
#include "gui/gatelibrary_management/gatelibrary_tab_widgets/gatelibrary_tab_general.h"
#include "gui/gatelibrary_management/gatelibrary_tab_widgets/gatelibrary_tab_pin.h"

class QGridLayout;
class QPushButton;
class QTabWidget;
class QSplitter;

namespace hal
{
    class GatelibraryGraphicsView;

    class GateLibraryManager : public QFrame
    {
        Q_OBJECT

    protected:
        void resizeEvent(QResizeEvent* evt) override;
    public:
        /**
         * Constructor.
         *
         * @param parent - the parent widget
         */
        explicit GateLibraryManager(MainWindow* parent);

        /**
         * Reinitializes the appearance of the widget and its children.
         */
        void repolish();

        bool initialize(GateLibrary* gateLibrary = nullptr, bool readOnly = false);
        u32 getNextGateId();
        void updateTabs(GateType* gateType);
        bool callUnsavedChangesWindow();

    public Q_SLOTS:
        /**
         * Opens the Wizard to edit or add a new gate
         */
        void handleEditWizard(const QModelIndex& index);
        void handleAddWizard();
        void handleDeleteType(QModelIndex index);

        /**
         * Updates widgets based on the selected item from the TableView
         * @param index
         */
        void handleSelectionChanged(const QModelIndex& index, const QModelIndex& prevIndex);

        /**
         * Emits close() and closes the widget.
         */
        void handleCancelClicked();

        void handleOkClicked();

        void handleSaveAction();
        void handleSaveAsAction();

    Q_SIGNALS:
        /**
         * Q_SIGNAL that is emitted when either the Save or Cancel button is clicked.
         */
        void close();


    private:
        QSplitter* mSplitter;
        int mFrameWidth;

        GateLibraryWizard* mWizard;
        QTabWidget* mTabWidget;
        QGridLayout* mLayout;
        GatelibraryContentWidget* mContentWidget;
        GatelibraryTableModel* mTableModel;
        GatelibraryGraphicsView* mGraphicsView;

        QPushButton* mOkBtn;
        QPushButton* mCancelBtn;

        GateLibraryTabGeneral* mGeneralTab;
        GateLibraryTabTruthTable* mBooleanFunctionTab;
        GateLibraryTabPin* mPinTab;

        const GateLibrary* mNonEditableGateLibrary;
        GateLibrary* mEditableGatelibrary;
        std::unique_ptr<Netlist> mDemoNetlist;
        std::filesystem::path mPath;

        bool mReadOnly = false;

        GateType* getSelectedGate();
    };
}
