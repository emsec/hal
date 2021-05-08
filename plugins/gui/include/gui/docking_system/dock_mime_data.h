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

#include <QMimeData>
#include <QObject>

namespace hal
{
    class ContentWidget;

    /**
     * @ingroup docking
     * @brief QMineData to store and transfer information while dragging a DockButton
     *
     * The DockMimeData class is used for the drag system. It holds the ContentWidget of the corresponding DockButton as metadata
     * when the DockButton is dragged. It is needed to use Qt's default drag implementation.
     */
    class DockMimeData : public QMimeData
    {
        Q_OBJECT

    public:
        /**
         * The constructor.
         *
         * @param widget - The content widget to be saved in this object.
         */
        DockMimeData(ContentWidget* widget);

        /**
         * Get the content widget.
         *
         * @return The content widget.
         */
        ContentWidget* widget() const;

    private:
        ContentWidget* mWidget;
    };
}
