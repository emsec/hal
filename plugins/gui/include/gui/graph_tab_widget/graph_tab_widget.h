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

#include "gui/content_widget/content_widget.h"

#include "gui/graph_widget/contexts/graph_context.h"
#include "gui/graph_widget/graph_widget.h"
#include "gui/graph_widget/graphics_scene.h"

#include <QMap>

class QTabWidget;
class QVBoxLayout;

namespace hal
{
    class SettingsItemDropdown;
    class SettingsItemKeybind;
	
   /**
    * @ingroup graph
     * @brief A ContentWidget that holds all GraphWidget objects (GraphView) as tabs of a QTabWidget.
     */
    class GraphTabWidget : public ContentWidget
    {
        Q_OBJECT

    public:
        enum GraphCursor { Select, PickModule, PickGate };

        /**
         * Constructor.
         *
         * @param parent - The parent widget.
         */
        GraphTabWidget(QWidget* parent = nullptr);

        /**
         * Create and register all shortcuts associated with the graph tab widget.
         *
         * @returns the list of shortcuts.
         */
        virtual QList<QShortcut*> createShortcuts() override;

        /**
         * Add a QWidget as a tab to the QTabWidget.
         *
         * @param tab - The wiget to add
         * @param tab_name - The title of the tab
         * @returns the index of the new tab within the QTabWidget
         */
        int addTab(QWidget* tab, QString tab_name = "default");

        /**
         * Shows a certain GraphContext. If there is already a tab showing the context it is selected. Otherwise, a new
         * tab with a graph widget tab for this context is created and selected.
         *
         * @param context - The GraphContext to show
         */
        void showContext(GraphContext* context);

        /**
         * Moves (and scales) the camera in the current GraphWidget to the current selection so that every selected
         * GraphicsItem can be seen.
         */
        void ensureSelectionVisible();

        /**
         * returns whether shape of cursor has been changed to indicate pick-select-module mode
         */
        GraphCursor selectCursor() const { return mSelectCursor; }

        enum KeyboardModifier{Alt, Ctrl, Shift};
        Q_ENUM(KeyboardModifier)

    public Q_SLOTS:

        /**
         * Q_SLOT that should be called whenever a new GraphContext was created. It is used to show the new context in
         * a new tab.
         *
         * @param context - The new context
         */
        void handleContextCreated(GraphContext* context);

        /**
         * Q_SLOT that should be called whenever a GraphContext was renamed. It is used to rename the associated tab.
         *
         * @param context - The renamed context
         */
        void handleContextRenamed(GraphContext* context);

        /**
         * Q_SLOT that should be called whenever a GraphContext is about to be removed.
         * It is used to close the associated tab.
         *
         * @param context - The context that will be removed.
         */
        void handleContextRemoved(GraphContext* context);

        /**
         *
         * @param index - The index of the tab within the QTabWidget
         */
        void handleTabChanged(int index);

        /**
         * Q_SLOT that should be called whenever the focus is set to a gate. Used to show the focused gate in the
         * GraphWidget.
         *
         * @param gateId - The id of the gate in focus.
         */
        void handleGateFocus(u32 gateId);

        /**
         * Q_SLOT that should be called whenever the focus is set to a net. Used to show the focused net in the
         * GraphWidget.
         *
         * @param netId - The id of the net in focus.
         */
        void handleNetFocus(u32 netId);

        /**
         * Q_SLOT that should be called whenever the focus is set to a module. Used to show the focused module in the
         * GraphWidget.
         *
         * @param moduleId - The id of the module in focus.
         */
        void handleModuleFocus(u32 moduleId);

        /**
         * Q_SLOT that should be called whenever a tab is rightclicked.
         *
         * @param pos - Mouse position as QPoint.
         */
        void handleCustomContextMenuRequested(const QPoint &pos);

        /**
         * Change shape of cursor to indicate that a module or gate should be picked by user.
         * Possible values are provided by GraphCursor enumeration.
         *
         * @param icurs - 0 = standard select, 1 = module pick, 2 = gate pick
         */
        void setSelectCursor(int icurs);

        /**
         * Q_SLOT to close a single tab.
         *
         * @param index - The index of the tab within the QTabWidget
         */
        void handleTabCloseRequested(int index);

    private:
        QTabWidget* mTabWidget;
        QVBoxLayout* mLayout;

        float mZoomFactor;

        QMap<GraphContext*, QWidget*> mContextWidgetMap;

        int getContextTabIndex(GraphContext* context) const;

        void addGraphWidgetTab(GraphContext* context);

        void zoomInShortcut();
        void zoomOutShortcut();

        void handleCloseTabsToRight(int index);
        void handleCloseTabsToLeft(int index);

        static SettingsItemDropdown* sSettingGridType;
        static SettingsItemDropdown* sSettingDragModifier;
        static SettingsItemDropdown* sSettingPanModifier;

        static SettingsItemKeybind* sSettingZoomIn;
        static SettingsItemKeybind* sSettingZoomOut;

        static bool sSettingsInitialized;

        static bool initSettings();

        QMap<KeyboardModifier, Qt::KeyboardModifier> mKeyModifierMap;
        GraphCursor mSelectCursor;
    };
}
