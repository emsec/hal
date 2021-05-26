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
#include <QWidget>

namespace hal
{
    /**
     * @ingroup python-console
     * @brief Connects the stylesheet properties to the console code.
     *
     * Singleton class that provides access to the style sheets configuration of the python console.
     */
    class PythonConsoleQssAdapter : public QWidget
    {
        Q_OBJECT
        Q_PROPERTY(QColor standardColor READ standardColor WRITE setStandardColor)
        Q_PROPERTY(QColor errorColor READ errorColor WRITE setErrorColor)
        Q_PROPERTY(QColor promtColor READ promtColor WRITE setPromtColor)

    public:
        /**
         * Get the instance of the singleton class.
         *
         * @returns a pointer to the PythonConsoleQssAdapter object.
         */
        static PythonConsoleQssAdapter* instance();

        /** @name Q_PROPERTY READ Functions
         */
        ///@{
        QColor standardColor() const;
        QColor errorColor() const;
        QColor promtColor() const;
        ///@}

        /** @name Q_PROPERTY WRITE Functions
         */
        ///@{
        void setStandardColor(const QColor& color);
        void setErrorColor(const QColor& color);
        void setPromtColor(const QColor& color);
        ///@}

    private:
        /**
         * Constructor.
         *
         * @param parent - The parent widget.
         */
        explicit PythonConsoleQssAdapter(QWidget* parent = nullptr);

        QColor mStandardColor;
        QColor mErrorColor;
        QColor mPromtColor;
    };
}    // namespace hal
