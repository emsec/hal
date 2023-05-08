// MIT License
// 
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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

        Q_PROPERTY(QColor nodeBackgroundColor READ nodeBackgroundColor WRITE setNodeBackgroundColor);
        Q_PROPERTY(QColor nodeTextColor READ nodeTextColor WRITE setNodeTextColor)
    public:

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

        QColor nodeBackgroundColor() const;
        QColor nodeTextColor() const;
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

        void setGridAlpha(int alpha);
        void setGridAlphaF(qreal alpha);
        /**
         * Sets the color of the grid base lines (not the clusters). <br>
         * It does not affect the dot grid color.
         *
         * @param color - The color of the grid base lines
         */
        void setGridBaseLineColor(const QColor& color);

        /**
         * Sets the color of the grid cluster lines. The grid cluster lines are the darker lines in the grid that
         * indicate 8 steps in the main grid. <br>
         * It does not affect the dot cluster grid color.
         *
         * @param color - The color of the grid cluster lines
         */
        void setGridClusterLineColor(const QColor& color);

        /**
         * Sets the color of the grid base dotted lines (only shown if grid_type::Dots are activated). <br>
         *
         * @param color - The color of the grid base dotted lines
         */
        void setGridBaseDotColor(const QColor& color);

        /**
         * Sets the color of the grid dotted cluster lines (only shown if grid_type::Dots are activated).
         * The grid cluster lines are the darker dots in the grid that indicate 8 steps in the main grid.
         *
         * @param color - The color of the grid dotted cluster lines
         */
        void setGridClusterDotColor(const QColor& color);

        void setNodeBackgroundColor(const QColor& color);
        void setNodeTextColor(const QColor& color);
        ///@}

        /**
         * Get singleton instance initialized on first call
         * @return the instance
         */
        static GraphicsQssAdapter* instance();
    private:
        explicit GraphicsQssAdapter(QWidget* parent = nullptr);
        static GraphicsQssAdapter* inst;

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

        QColor mNodeBackgroundColor;
        QColor mNodeTextColor;
    };
}
