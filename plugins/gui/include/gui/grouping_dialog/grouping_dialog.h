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

#include "gui/grouping/grouping_table_model.h"
#include "gui/grouping/grouping_proxy_model.h"
#include "gui/searchbar/searchbar.h"

#include <QDialog>
#include <QTableView>
#include <QDialogButtonBox>

namespace hal {

    class GroupingDialog : public QDialog
    {
        Q_OBJECT

    public:
        /**
         * The constructor.
         *
         * @param parent - The dialog's parent.
         */
        GroupingDialog(QWidget* parent=nullptr);

        /**
         * Get the grouping id which was selected through this dialog.
         *
         * @return The selected id.
         */
        u32 groupId() const { return mGroupId; }

        /**
         * Get the flag that states if a new grouping should be created.
         *
         * @return True if a new grouping should be created, False otherwise.
         */
        bool isNewGrouping() const { return mNewGrouping; }

    public Q_SLOTS:
        /**
         * Enables the dialog buttons if groupId is not zero and vice versa. If groupId is not zero and
         * doubleClick is true, the dialog is accepted.
         *
         * @param groupId - The selected id.
         * @param doubleClick - True if grouping table view emitted doubleClicked signal.
         */
        void handleGroupingSelected(u32 groupId, bool doubleClick);

        /**
         * Updates the grouping history and accepts the dialog.
         */
        void accept() override;

        /**
         * Overwrites the filter with the regular expression given in text.
         *
         * @param text - Contains the regular expression filter as a string
         */
        void filter(const QString& text);

    private Q_SLOTS:
        void handleNewGroupingClicked();
        void handleToggleSearchbar();
        void handleCurrentTabChanged(int index);

    private:
        QAction* mToggleSearchbar;
        Searchbar* mSearchbar;

        GroupingTableView* mGroupingTableView;
        GroupingTableView* mLastUsed;

        QDialogButtonBox* mButtonBox;
        QTabWidget* mTabWidget;

        u32 mGroupId;
        bool mNewGrouping;
    };
}
