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

#include "gui/graph_widget/items/graphics_item.h"

#include <QString>

namespace hal
{
    class GraphicsNode : public GraphicsItem
    {
    public:
        struct Visuals
        {
            bool mVisible;

            QColor mMainColor;
            QColor mNameColor;
            QColor mTypeColor;
            QColor mPinColor;
            QColor mBackgroundColor;
            QColor mHighlightColor;

            // DRAW TYPE ENUM
        };

        GraphicsNode(const ItemType type, const u32 id, const QString& name);

        virtual QRectF boundingRect() const override;
        virtual QPainterPath shape() const override;

        virtual QPointF getInputScenePosition(const u32 mNetId, const QString& pin_type) const = 0;
        virtual QPointF getOutputScenePosition(const u32 mNetId, const QString& pin_type) const = 0;
        virtual QPointF endpointPositionByIndex(int index, bool isInput) const = 0;
        virtual float   yEndpointDistance() const = 0;
        virtual float   yTopPinDistance() const = 0;

        int inputByNet(u32 netId) const;
        int outputByNet(u32 netId) const;
        QList<u32> inputNets() const;
        QList<u32> outputNets() const;

    //    virtual std::string get_input_pin_type_at_position(const size_t pos) const = 0;
    //    virtual std::string get_output_pin_type_at_position(const size_t pos) const = 0;

        virtual void setVisuals(const Visuals& v);

        qreal width() const;
        qreal height() const;

        void set_name(const QString& name);

    //    qreal xOffset() const;
    //    qreal yOffset() const;

    protected:
        QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

        QString mNodeText[3];

        qreal mWidth;
        qreal mHeight;

        // pin by net number
        // (multiple) pins not assigned to network are stored with id=0
        QMultiHash<u32,int> mInputByNet;
        QMultiHash<u32,int> mOutputByNet;
    };
}
