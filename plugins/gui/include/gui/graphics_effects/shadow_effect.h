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

#include <QGraphicsEffect>

namespace hal
{

    /**
     * @ingroup graph-visuals
     * @brief Creates a shadow effect around QObject%s.
     *
     * The ShadowEffect is a graphics effect that simulates a small shadow around the outline of the object
     * it is applied on.
     */
    class ShadowEffect : public QGraphicsEffect
    {
        Q_OBJECT

    public:
        /**
         * Constructor.
         *
         * @param parent - The parent widget.
         */
        explicit ShadowEffect(QObject* parent = nullptr);

        /**
         * Draws the effect.
         *
         * @param painter - The used painter
         */
        void draw(QPainter* painter);

        /**
         * Returns the effective bounding rectangle for this effect given the bounding rectangle of the object it is
         * applied on.
         *
         * @param rect - The bounding rectangle this effect should be applied on
         * @returns The bounding rectangle of the effect.
         */
        QRectF boundingRectFor(const QRectF& rect) const;

        /**
         * Customizes how far from the edges of the object the shadow should be drawn.
         *
         * @param distance - The new distance
         */
        inline void setDistance(qreal distance)
        {
            mDistance = distance;
            updateBoundingRect();
        }

        /**
         * Gets the distance of the shadow, i.e. how far from the edges of the object it is drawn.
         *
         * @returns the distance of the shadow effect
         */
        inline qreal distance() const
        {
            return mDistance;
        }

        /**
         * Customizes the blur radius that is used to draw the shadow.
         *
         * @param blurRadius - The blur radius
         */
        inline void setBlurRadius(qreal blurRadius)
        {
            mBlurRadius = blurRadius;
            updateBoundingRect();
        }

        /**
         * Gets the blur radius ths is used to draw the shadow.
         *
         * @returns the blur radius.
         */
        inline qreal blurRadius() const
        {
            return mBlurRadius;
        }

        /**
         * Customizes the color of the shadow.
         *
         * @param color - The color of the shadow.
         */
        inline void setColor(const QColor& color)
        {
            mColor = color;
        }

        /**
         * Gets the color of the shadow.
         *
         * @returns the color of the shadow.
         */
        inline QColor color() const
        {
            return mColor;
        }

    private:
        qreal mDistance;
        qreal mBlurRadius;
        QColor mColor;
    };
}
