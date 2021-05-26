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

#include <QPushButton>
#include <QString>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QWidget>

namespace hal
{

    /**
     * @ingroup utility_widgets-selection_details
     * @brief Represents a single section consisting of a header and the section itself.
     *
     * The details section encapsulates a header and its corresponding content into a single object. These sections
     * are used by the specific details widgets to display its data. The visibility of the contents can be toggled
     * by clicking on the header.
     */
    class DetailsSectionWidget : public QWidget
    {
        Q_OBJECT
    public:
        /**
         * The first constructor.
         *
         * @param content - The content to be displayed.
         * @param txt - The displayed text of the header.
         * @param parent - The widget's parent.
         */
        DetailsSectionWidget(QWidget* content, const QString& txt, QWidget* parent = nullptr);

        /**
         * The second constructor.
         *
         * @param txt - The displayed text of the header.
         * @param tab - The table as the section's content.
         * @param parent - The widget's parent.
         */
        DetailsSectionWidget(const QString& txt, QTableWidget* tab, QWidget* parent = nullptr);

        /**
         * Get the details section's content as a table (most sections display tables).
         *
         * @return The table of the section. If the first constructor was used, a nullptr is returned.
         */
        QTableWidget* table() const;

        /**
         * Updates the header text by using the given rowcount (used when displaying a table).
         *
         * @param rc - The new rowcount of the widget's table.
         */
        void setRowCount(int rc);

        /**
         * Sets the hideWhenEmpty flag and hides itself when there is no data to show.
         *
         * @param hide - True if should be hidden when empty.
         */
        void hideWhenEmpty(bool hide);

    private Q_SLOTS:

        /**
         * Toggles the visibily of the section's content.
         */
        void toggleBodyVisible();

        /**
         * A handler function to catch signals that are emitted from the settings-relay. Checks if the key belongs
         * to the widget and if yes, its value is updated. In this case, the setting manages if the sections should
         * hide itself if the content is empty (e.g. the table has no entries).
         *
         * @param sender - The sender that triggered the change.
         * @param key - The setting's key.
         * @param value - The new value of the setting.
         */

    private:
        /**
         * Hides itself when the hideEmpty setting is set and the table(the content) is empty.
         */
        void hideEmpty();

        /**
         * Shows or hides the body depending on a set boolean.
         */
        void bodyVisible();

        /**
         * Helperfunction to set up the widget (such as setting the header text and connecting signals).
         *
         * @param txt - The displayed text of the header.
         */
        void constructor(const QString& txt);

        QVBoxLayout* mLayout;
        QPushButton* mHeader;
        QWidget* mBody;
        QTableWidget* mTable;
        QString mHeaderText;
        int mRows;
        bool mHideEmpty;
        bool mBodyVisible;
    };
}    // namespace hal
