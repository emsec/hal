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

#include <QIconEngine>

namespace hal
{
    /**
     * @ingroup gui
     * @brief Creates Icons from svg data.
     *
     * The SvgIconEngine is a QIconEngine that is used for create QIcons from given svg data.
     */
    class SvgIconEngine : public QIconEngine
    {
    public:
        /**
         * Constructor.
         *
         * @param svg_data - The svg data to create the QIcon from
         */
        explicit SvgIconEngine(const std::string& svg_data);

        /**
         * Uses the given painter to paint the icon into the rectangle rect. Mode and state are unused.
         *
         * @param painter - The painter to paint the icon with
         * @param rect - The rectangle to draw the icon in
         * @param mode - The mode (unused)
         * @param state - The state (unused)
         */
        void paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state) override;

        /**
         * Returns a clone of this icon engine
         *
         * @returns a clone of this icon engine
         */
        QIconEngine* clone() const override;

        /**
         * Returns the icon as a pixmap with the required size. Mode and state are unused.
         *
         * @param size - The size of the pixmap
         * @param mode - The mode (unused)
         * @param state - The state (unused)
         * @returns the pixmap for the icon
         */
        QPixmap pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state) override;

    private:
        QByteArray mData;
    };
}
