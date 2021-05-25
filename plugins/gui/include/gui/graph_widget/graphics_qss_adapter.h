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

#include <QWidget>

namespace hal
{
    /**
     * @ingroup graph
     * @brief Connects the stylesheet properties with the code.
     *
     * Class to capsule all QSS Property functions used for Graphics
     */
    class GraphicsQssAdapter : public QWidget
    {
        Q_OBJECT

        Q_PROPERTY(QColor gateBaseColor READ gateBaseColor WRITE setGateBaseColor)
        Q_PROPERTY(QColor netBaseColor READ netBaseColor WRITE setNetBaseColor)

        Q_PROPERTY(QColor gateSelectionColor READ gateSelectionColor WRITE setGateSelectionColor)
        Q_PROPERTY(QColor netSelectionColor READ netSelectionColor WRITE setNetSelectionColor)

        Q_PROPERTY(QFont gateNameFont READ gateNameFont WRITE setGateNameFont)
        Q_PROPERTY(QFont gateTypeFont READ gateTypeFont WRITE setGateTypeFont)
        Q_PROPERTY(QFont gatePinFont READ gatePinFont WRITE setGatePinFont)

        Q_PROPERTY(QFont netFont READ netFont WRITE setNetFont)

        Q_PROPERTY(QColor gridBaseLineColor READ gridBaseLineColor WRITE setGridBaseLineColor)
        Q_PROPERTY(QColor gridClusterLineColor READ gridClusterLineColor WRITE setGridClusterLineColor)

        Q_PROPERTY(QColor gridBaseDotColor READ gridBaseDotColor WRITE setGridBaseDotColor)
        Q_PROPERTY(QColor gridClusterDotColor READ gridClusterDotColor WRITE setGridClusterDotColor)

    public:
        explicit GraphicsQssAdapter(QWidget* parent = nullptr);

        void repolish();

        /** @name Q_PROPERTY READ Functions
         */
        ///@{
        QColor gateBaseColor() const;
        QColor netBaseColor() const;

        // GATE BACKGROUND COLOR
        // GATE OUTLINE COLOR
        // ...

        QColor gateSelectionColor() const;
        QColor netSelectionColor() const;

        QFont gateNameFont() const;
        QFont gateTypeFont() const;
        QFont gatePinFont() const;

        QFont netFont() const;

        QColor gridBaseLineColor() const;
        QColor gridClusterLineColor() const;

        QColor gridBaseDotColor() const;
        QColor gridClusterDotColor() const;
        ///@}

        /** @name Q_PROPERTY WRITE Functions
         */
        ///@{
        void setGateBaseColor(const QColor& color);
        void setNetBaseColor(const QColor& color);

        void setGateSelectionColor(const QColor& color);
        void setNetSelectionColor(const QColor& color);

        void setGateNameFont(const QFont& font);
        void setGateTypeFont(const QFont& font);
        void setGatePinFont(const QFont& font);

        void setNetFont(const QFont& font);

        void setGridBaseLineColor(const QColor& color);
        void setGridClusterLineColor(const QColor& color);

        void setGridBaseDotColor(const QColor& color);
        void setGridClusterDotColor(const QColor& color);
        ///@}

    private:
        QColor mGateBaseColor;
        QColor mNetBaseColor;

        QColor mGateSelectionColor;
        QColor mNetSelectionColor;

        QFont mGateNameFont;
        QFont mGateTypeFont;
        QFont mGatePinFont;

        QFont mNetFont;

        QColor mGridBaseLineColor;
        QColor mGridClusterLineColor;

        QColor mGridBaseDotColor;
        QColor mGridClusterDotColor;
    };
}
