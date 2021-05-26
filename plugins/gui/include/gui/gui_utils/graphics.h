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

#include <QColor>
#include <QIcon>
#include <QString>
#include <QPixmap>

namespace hal
{
    /**
     * @ingroup gui
     */
    namespace gui_utility
    {
        /**
         * Opens and reads a file and returns its content (Must not even be a svg file).
         *
         * @param svg_path - The path to the file to read.
         * @return The file's content.
         */
        extern QString getSvgData(const QString& svg_path);

        /**
         * Replaces a color within given svg data to a new specified color.
         *
         * @param svg_data - The svg data as a string (e.g. read from a file)
         * @param from - The color to replace.
         * @param to - The new color.
         */
        extern void changeSvgColor(QString& svg_data, const QColor& from, const QColor& to);

        /**
         * Replaces all colors within given svg data to a new specified color.
         *
         * @param svg_data - The svg data as a string (e.g. read from a file)
         * @param to - The new color.
         */
        extern void changeAllSvgColors(QString& svg_data, const QColor& to);

        /**
         * Converts given svg data to a QIcon.
         *
         * @param svg_data - The svg data as a string (e.g. read from a file)
         * @return  The resulting QIcon.
         */
        extern QIcon getIconFromSvgData(const QString& svg_data);

        /**
         * Reads svg data from a file and styles it (replacing colors) according to a format string.
         * When no styling is necessary, an empty string can be passed.
         * The result is then converted into an icon and returned.
         *
         * @param from_to_colors - The format string in the form "from -> to" (e.g.: "all -> #E4E4E4").
         * @param svg_path - The path to the svg file.
         * @return The (perhabs styled) icon.
         */
        extern QIcon getStyledSvgIcon(const QString& from_to_colors, const QString& svg_path);

        /**
         * Returns a somewhat random color through a funny method (should be the same order
         * of colors each time the program starts). This brilliant piece of code MUST NEVER
         * BE REMOVED, PURE COMEDY!
         *
         * @return The "random" color.
         */
        extern QColor getRandomColor();
    }
}
