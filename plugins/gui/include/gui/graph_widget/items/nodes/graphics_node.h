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

#include "gui/graph_widget/items/graphics_item.h"

#include <QString>

namespace hal
{
    /**
     * @ingroup graph-visuals
     * @brief Abstract base class for nodes (e.g. gates, modules)
     *
     * Abstract base class of every node in the netlist graph, i.e. GraphicsGates and GraphicsModules.
     */
    class GraphicsNode : public GraphicsItem
    {
    public:
        /**
         * Struct to store visuals properties of a graphics node, i.e. whether the node is visible or not as well as
         * the different colors used to draw the node.
         */
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

        /**
         * Constructor of the abstract class GraphicsNode.
         *
         * @param type - The type of the GraphicsItem (i.e. module, gate or net)
         * @param id - The id of the underlying object (e.g. the module id if ItemType::Module)
         * @param name - The name of the node
         */
        GraphicsNode(const ItemType type, const u32 id, const QString& name);

         /**
          * Get the bounding rectangle of the GrahpicsNode that represent its size. Therefore the returned rectangle is
          * set to position (0, 0) with the width and hight of its GraphicsNode.
          *
          * @returns The bounding rectangle as a QRectF
          */
        virtual QRectF boundingRect() const override;

        /**
         * Creates and returns a painter path of the frame of the GraphicsNode.
         *
         * @returns a QPainterPath of the frame of the GraphicsNode
         */
        virtual QPainterPath shape() const override;

        /**
         * Given the id of a net and the type of the pin of one of its destinations, this function returns
         * the position of the input pin in the scene.
         *
         * @param mNetId - The net id
         * @param pin_type - The input pin type
         * @returns The pin's position in the scene
         */
        virtual QPointF getInputScenePosition(const u32 mNetId, const QString& pin_type) const = 0;

        /**
         * Given the id of a net and the type of the pin of one of its sources, this function returns
         * the position of the output pin in the scene.
         *
         * @param mNetId - The net id
         * @param pin_type - The output pin type
         * @returns The pin's position in the scene
         */
        virtual QPointF getOutputScenePosition(const u32 mNetId, const QString& pin_type) const = 0;

        /**
         * Given the index of an input/output pin, this function returns the position of this pin in the scene. <br>
         * (Pins are positioned from top (idx=0) to bottom (idx=maxIdx))
         *
         * @param index - The index of the pin
         * @param isInput - <b>true</b> if the pin is an input pin <br>
         *                - <b>false</b> if the pin is an output pin
         * @returns the position of the endpoint in the scene
         */
        virtual QPointF endpointPositionByIndex(int index, bool isInput) const = 0;

        /**
         * Gets the vertical space between two endpoints (in scene coordinates).
         *
         * @returns the vertical space between two endpoints
         */
        virtual float   yEndpointDistance() const = 0;

        /**
         * Gets the distance between the top edge of the node and the first pin (in scene coordinates).
         *
         * @returns the vertical space between the top edge of the node and the first pin
         */
        virtual float   yTopPinDistance() const = 0;

        /**
         * Given a net by its id, this function returns the index of a pin that is a destination of the net. If no
         * pin of this gate is a destination of this net -1 is returned. <br>
         * Note(TODO): Undefined behaviour if there are multiple destinations on this gate!
         *
         * @param netId - The id of the net
         * @returns the index of a pin that is connected to the net.
         */
        int inputByNet(u32 netId) const;

        /**
         * Given a net by its id, this function returns the index of a pin that is a source of the net. If no
         * pin of this gate is a source of this net -1 is returned. <br>
         * Note(TODO): Undefined behaviour if there are multiple sources on this gate!
         *
         * @param netId - The id of the net
         * @returns the index of a pin that is connected to the net.
         */
        int outputByNet(u32 netId) const;

        /**
         * Returns the following list: <ul>
         * <li> The list contains an entry for each input pin.
         * <li> An entry <i>netId</i> at index <i>idx</i> implies that the net of id <i>netId</i> is connected to
         *      the input pin at position <i>idx</i>.
         * <li> If <i>netId=0</i> there is no net connected to the pin at index <i>idx</i>.  </ul>
         *
         * @returns the list of input nets
         */
        QList<u32> inputNets() const;

        /**
         * Returns the following list: <ul>
         * <li> The list contains an entry for each output pin.
         * <li> An entry <i>netId</i> at index <i>idx</i> implies that the net of id <i>netId</i> is connected to
         *      the output pin at position <i>idx</i>.
         * <li> If <i>netId=0</i> there is no net connected to the pin at index <i>idx</i>.  </ul>
         *
         * @returns the list of output nets
         */
        QList<u32> outputNets() const;

    //    virtual std::string get_input_pin_type_at_position(const size_t pos) const = 0;
    //    virtual std::string get_output_pin_type_at_position(const size_t pos) const = 0;

        /**
         * Configures the passed visuals.
         *
         * @param v - The visuals to apply
         */
        virtual void setVisuals(const Visuals& v);

        /**
         * Gets the width of the GraphicsNode
         *
         * @returns the width of the GraphicsNode
         */
        qreal width() const;

        /**
         * Gets the height of the GraphicsNode
         *
         * @returns the height of the GraphicsNode
         */
        qreal height() const;

        /**
         * Changes the name text of the GraphicsNode to the passed name.
         *
         * @param name - The new name
         */
        void set_name(const QString& name);

    //    qreal xOffset() const;
    //    qreal yOffset() const;

    protected:
        /**
         * The text in the center of the GraphicsNode. Each index stores one line of text. Therefore there is a maximum
         * of 3 lines in total.
         */
        QString mNodeText[3];

        /**
         * The <b>width</b> of the GraphicsNode
         */
        qreal mWidth;

        /**
         * The <b>height</b> of the GraphicsNode
         */
        qreal mHeight;

        // pin by net number
        // (multiple) pins not assigned to network are stored with id=0
        /**
         * Stores pairs (netId, pinIdx) for every input pin of this gate. <br>
         * netId=0 implies that no net is connected to the input pin at pinIdx.
         */
        QMultiHash<u32,int> mInputByNet;

        /**
         * Stores pairs (netId, pinIdx) for every output pin of this gate. <br>
         * netId=0 implies that no net is connected to the output pin at pinIdx.
         */
        QMultiHash<u32,int> mOutputByNet;
    };
}
