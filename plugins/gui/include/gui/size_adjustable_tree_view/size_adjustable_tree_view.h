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
#include <QTreeView>

namespace hal
{
    class SizeAdjustableTreeView : public QTreeView
    {
        Q_OBJECT
    public:
        SizeAdjustableTreeView(QWidget* parent = nullptr);

        /**
         * Adjust the size of the view depending on its elements, sets a
         * fixed height and a minimum width.
         * Only works if the underlying model is either a BaseTreeModel
         * or a model that is derived from it.
         */
        void adjustSizeToContents();

        /**
         * Overwritten function to connect to own signals.
         *
         * @param model - The model to set to.
         */
        void setModel(QAbstractItemModel *model) override;

    Q_SIGNALS:

        /**
         * SIGNAL that is emitted when adjustSizeToContents was called. The embeding widget
         * must update its layout/geometry/content (debug purposes for now)
         */
        void adjustSizeToContentsCalled();

    private:
        void handleExpandedOrCollapsed(const QModelIndex &index);
        void handleRowsInsertedOrRemoved(const QModelIndex &parent, int first, int last);
        void handleModelReset();
    };

}
