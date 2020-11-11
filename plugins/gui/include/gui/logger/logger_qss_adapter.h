//  MIT License
//
//  Copyright (c) 2019 Ruhr-University Bochum, Germany, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
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

#include <QWidget>

namespace hal
{
    class LoggerQssAdapter : public QWidget
    {
        Q_OBJECT
        Q_PROPERTY(QColor traceColor READ traceColor WRITE setTraceColor)
        Q_PROPERTY(QColor debugColor READ debugColor WRITE setDebugColor)
        Q_PROPERTY(QColor infoColor READ infoColor WRITE setInfoColor)
        Q_PROPERTY(QColor warningColor READ warningColor WRITE setWarningColor)
        Q_PROPERTY(QColor errorColor READ errorColor WRITE setErrorColor)
        Q_PROPERTY(QColor criticalColor READ criticalColor WRITE setCriticalColor)
        Q_PROPERTY(QColor defaultColor READ defaultColor WRITE setDefaultColor)
        Q_PROPERTY(QColor markerColor READ markerColor WRITE setMarkerColor)

        Q_PROPERTY(QColor traceHighlight READ traceHighlight WRITE setTraceHighlight)
        Q_PROPERTY(QColor debugHighlight READ debugHighlight WRITE setDebugHighlight)
        Q_PROPERTY(QColor infoHighlight READ infoHighlight WRITE setInfoHighlight)
        Q_PROPERTY(QColor warningHighlight READ warningHighlight WRITE setWarningHighlight)
        Q_PROPERTY(QColor errorHighlight READ errorHighlight WRITE setErrorHighlight)
        Q_PROPERTY(QColor criticalHighlight READ criticalHighlight WRITE setCriticalHighlight)
        Q_PROPERTY(QColor defaultHighlight READ defaultHighlight WRITE setDefaultHighlight)

    public:
        static LoggerQssAdapter* instance();

        QColor traceColor() const;
        QColor debugColor() const;
        QColor infoColor() const;
        QColor warningColor() const;
        QColor errorColor() const;
        QColor criticalColor() const;
        QColor defaultColor() const;
        QColor markerColor() const;

        QColor traceHighlight() const;
        QColor debugHighlight() const;
        QColor infoHighlight() const;
        QColor warningHighlight() const;
        QColor errorHighlight() const;
        QColor criticalHighlight() const;
        QColor defaultHighlight() const;

        void setTraceColor(const QColor& color);
        void setDebugColor(const QColor& color);
        void setInfoColor(const QColor& color);
        void setWarningColor(const QColor& color);
        void setErrorColor(const QColor& color);
        void setCriticalColor(const QColor& color);
        void setDefaultColor(const QColor& color);
        void setMarkerColor(const QColor& color);

        void setTraceHighlight(const QColor& color);
        void setDebugHighlight(const QColor& color);
        void setInfoHighlight(const QColor& color);
        void setWarningHighlight(const QColor& color);
        void setErrorHighlight(const QColor& color);
        void setCriticalHighlight(const QColor& color);
        void setDefaultHighlight(const QColor& color);

    private:
        explicit LoggerQssAdapter(QWidget* parent = nullptr);

        QColor mTraceColor;
        QColor mDebugColor;
        QColor mInfoColor;
        QColor mWarningColor;
        QColor mErrorColor;
        QColor mCriticalColor;
        QColor mDefaultColor;
        QColor mMarkerColor;

        QColor mTraceHighlight;
        QColor mDebugHighlight;
        QColor mInfoHighlight;
        QColor mWarningHighlight;
        QColor mErrorHighlight;
        QColor mCriticalHighlight;
        QColor mDefaultHighlight;
    };
}    // namespace hal
