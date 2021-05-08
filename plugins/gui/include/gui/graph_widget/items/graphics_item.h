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

#include "hal_core/defines.h"

#include "gui/gui_def.h"

#include <QColor>
#include <QStyle>
#include <QGraphicsItem>

namespace hal
{
    /**
     * @ingroup graph-visuals
     * @brief Superclass for all graphic items used ins the GraphicsScene.
     * It contains information about the underlying objects that are represented, i.e. the type and the id.
     */
    class GraphicsItem : public QGraphicsItem
    {
    public:
        /**
         * Loads some settings used by this GraphicsItem.
         */
        static void loadSettings();

        /**
         * Set the level-of-detail this GraphicItem uses to be drawn. It should be set based on the zoom level of the view.
         * If the level-of-detail falls below a certain threshold some details of the GraphicsItem (or the item as a whole)
         * wont be drawn anymore. There thresholds are configured in the graph_widget_constants namespace.
         *
         * @param lod - The level of detail that should be applied.
         */
        static void setLod(const qreal lod);

        /**
         * Constructor.
         *
         * @param type - The ItemType of the GraphicsItem (i.e. Module/Gate/Net)
         * @param id - The id of the underlying object (e.g. the module id if ItemType::Module)
         */
        GraphicsItem(const ItemType type, const u32 id);

        /**
         * Gets the type of the GraphicsItem.
         *
         * @returns the ItemType of the GraphicsItem
         */
        ItemType itemType() const;

        /**
         * Gets the id of the underlying object that is represented by this GraphicsItem.
         *
         * @returns the id of the underlying object in its netlist
         */
        u32 id() const;

        /**
         * Highlights or normalizes(removes highlight) this GraphicsItem.
         *
         * @param hl - <b>true</b> to highlight the GraphicsItem. <b>false</b> to normalize the GraphicsItem.
         */
        void setHightlight(bool hl);

    protected:
        /**
         * The current level-of-detail this GraphicItem uses to be drawn
         */
        static qreal sLod;

        /**
         * The pen color used for selected GraphItems
         */
        static QColor sSelectionColor;

        /**
         * The pen color used for highlighted GraphItems
         */
        static QColor sHighlightColor;

        /**
         * Returns the pen color that should be used based on the current state (i.e. State_Selected or not) and
         * the grouping of the underlying object is in. <br>
         * If the Item in not selected and not in a grouping, a colorHint can be passed to manually chose a pen color. <br>
         *
         * @param state - The state of the widget.
         * @param colorHint - A color to be chosen if the widget is not selected and no grouping color was found
         * @returns the penColor to use
         */
        QColor penColor(QStyle::State state, const QColor& colorHint=QColor()) const;

        /**
         * Gets the pen color for selected GraphicItems.
         *
         * @returns the color for selected GraphicItems
         */
        QColor selectionColor() const;

        /**
         * Gets a pen color based on the grouping the underlying object is in. <br>
         * If the underlying object is in no grouping, an invalid color is returned.
         *
         * @returns the color of the grouping the underlying object is in
         */
        QColor groupingColor() const;

        /**
         * The ItemType of the GraphicsItem (i.e. Module/Gate/Net)
         */
        ItemType mItemType; // USE QT ITEM TYPE SYSTEM INSTEAD ???

        /**
         * <b>true</b> if the GraphicsItem is highlighted.
         */
        bool mHighlight;  // paint with hightlight color

        /**
         * The id of the underlying object in its netlist
         */
        u32 mId;

        /**
         * The color of this GraphicsItem
         */
        QColor mColor;
    };
}
