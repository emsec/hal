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
#include "gui/selection_details_widget/details_widget.h"

#include <QFont>
#include <QTableWidget>

namespace hal
{
    class DataContainer;

    /**
     * @ingroup utility_widgets-selection_details
     * @brief Displays commonly shared fields.
     *
     * A specific tablewidget to display the data fields of a gate, net or module (or any data that is given to
     * the updateData function).
     */
    class DataFieldsTable : public QTableWidget
    {
        Q_OBJECT
    public:
        /**
         * The constructor.
         *
         * @param parent - The widget's parent.
         */
        DataFieldsTable(DetailsWidget* parent = nullptr);

        /**
         * Fills the table with the given data in the form of key-value pairs.
         *
         * @param id - The id of the item to which the data belongs.
         * @param dc - The data in the form of key - vlaue pairs.
         */
        void updateData(const u32 id, const std::map<std::tuple<std::string, std::string>, std::tuple<std::string, std::string>>& dc);

    private Q_SLOTS:

        /**
         * Handler function to handle a context-menu-requested signal. Creates a context menu in which the python
         * code for the data can be extracted.
         *
         * @param pos - The position from where the menu is requested.
         */
        void handleContextMenuRequest(const QPoint& pos);

    private:
        u32 m_itemId;
        QString m_pythonType;
        QFont mKeyFont;
    };
}    // namespace hal
