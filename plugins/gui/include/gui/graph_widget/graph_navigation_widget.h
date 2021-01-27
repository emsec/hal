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

#include <QTreeWidget>
#include <QTableWidget>
#include <QFrame>
#include <QHash>
#include <QStringList>
#include "hal_core/defines.h"

#include "hal_core/netlist/net.h"
#include "hal_core/netlist/endpoint.h"
#include "gui/gui_def.h"
#include "gui/selection_relay/selection_relay.h"
#include "gui/graph_widget/layouters/node_box.h"

namespace hal {

    class GraphNavigationWidget;

    /**
     * A QTableWidget that is used to navigate from a pin with multiple adjacent pins. The table shows information about
     * the gates/modules that are connected to the starting pin, such as name, id, type, the adjacent input/ouput pin
     * and the parent modules name. <br>
     * The user can select one of these destination gates/modules to navigate to.
     */
    class GraphNavigationTableWidget : public QTableWidget
    {
        GraphNavigationWidget* mNavigationWidget;
    protected:
        /**
         * Overridden keyPressEvent. Used to navigate in the table with the keyboard.
         *
         * @param event - The QKeyEvent
         */
        void keyPressEvent(QKeyEvent *event) override;

        /**
         * Overridden focusInEvent. Used to remove the selection in the GraphNavigationTreeWidget of the NavigationWidget
         * when switching the focus to this widget.
         *
         * @param event - The QFocusEvent
         */
        void focusInEvent(QFocusEvent* event) override;
    public:
        /**
         * Constructor.
         *
         * @param nav - The GraphNavigationWidget that wraps this widget
         * @param parent - The parent widget (where to embed this widget)
         */
        GraphNavigationTableWidget(GraphNavigationWidget* nav, QWidget* parent = nullptr)
            : QTableWidget(parent), mNavigationWidget(nav) {;}
    };

    /**
     * A QTreeWidget that is used to add (and navigate to) a gate or module to the current view (i.e. context) while navigating from a pin.
     * The table shows information about the pins and modules that are connected to the starting pin which are not
     * added to the view yet. These information include the gates/modules name, id, type, adjacent input/output pin and
     * parent module. <br>
     * The user can select one (or multiple) of these destination gates/modules to navigate to while adding them to the current view.
     * If the destination gate/module is within a module, the user can decide whether to add the destination gate/module
     * itself or one of the parent modules instead. Therefore the tree hierarchy is useful.
     */
    class GraphNavigationTreeWidget : public QTreeWidget
    {
        GraphNavigationWidget* mNavigationWidget;
        QList<QTreeWidgetItem*> selectedItemRecursion(QTreeWidgetItem* item) const;
    protected:
        /**
         * Overridden keyPressEvent. Used to navigate in the tree with the keyboard.
         *
         * @param event - The QKeyEvent
         */
        void keyPressEvent(QKeyEvent *event) override;
        /**
         * Overridden focusInEvent. Used to remove the selection in the GGraphNavigationTableWidget of the NavigationWidget
         * when switching the focus to this widget.
         *
         * @param event - The QFocusEvent
         */
        void focusInEvent(QFocusEvent* event) override;

        /**
         * TODO: Put this in keyPressEvent?
         * Overridden event. Used to toggle the navigation widget when pressing <i>TAB</i>.
         *
         * @param ev - The QEvent
         * @returns true if the event e was recognized and processed (see Qt documentation)
         */
        bool event(QEvent *ev) override;
    public:
        /**
         * Constructor.
         *
         * @param nav - The GraphNavigationWidget that wraps this widget
         * @param parent - The parent widget (where to embed this widget)
         */
        GraphNavigationTreeWidget(GraphNavigationWidget* nav, QWidget* parent = nullptr)
            : QTreeWidget(parent), mNavigationWidget(nav) {;}

        /**
         * Returns a list of all selected items.
         *
         * @returns a list of all selected items
         */
        QList<QTreeWidgetItem*> selectedItems() const;

        /**
         * Returns the index of the first top-level item.
         *
         * @return the QModelIndex of the first top-level item
         */
        QModelIndex firstIndex() const;
    };

    /**
     * The GraphNavigationWidget is a widget that appears whenever the user navigates from a pin with multiple navigation
     * destinations. It wraps both an instance of GraphNavigationTableWidget (for destinations within the context) and an
     * instance of GraphNavigationTreeWidget (for destinations outside the context). <br>
     * The user can navigate in both navigation widgets to choose a destination.
     */
    class GraphNavigationWidget : public QWidget
    {
        Q_OBJECT

        friend class GraphNavigationTableWidget;
        friend class GraphNavigationTreeWidget;
    public:
        /**
         * Constructor. <br>
         * If the onlyNavigate flag is set to true, this widget can not be used to add gates to the view. In this case
         * only the GraphNavigationTableWidget will appear while navigating.
         *
         * @param onlyNavigate - Set to <b>true</b> if the widget should be used for navigation only.
         * @param parent - The parent widget (where to embed the navigation widget)
         */
        GraphNavigationWidget(bool onlyNavigate, QWidget* parent = nullptr);
        // TODO: IN_PROGRESS: Documentation...
        void setup(SelectionRelay::Subfocus direction);
        void setup(Node origin, Net* via_net, SelectionRelay::Subfocus dir);
        SelectionRelay::Subfocus direction() const { return mDirection; }
        void closeRequest();
        bool isEmpty() const;
        bool hasBothWidgets() const;
        void toggleWidget();

    Q_SIGNALS:
        void navigationRequested(const Node& origin, const u32 via_net, const QSet<u32>& to_gates, const QSet<u32>& to_modules);
        void closeRequested();
        void resetFocus();

    private Q_SLOTS:
        void handleNavigateSelected(int irow, int icol);
        void handleAddToViewSelected(QTreeWidgetItem* item, int icol);

    protected:
        void keyPressEvent(QKeyEvent *event) override;
        void focusInEvent(QFocusEvent* event) override;

    private:
        bool mOnlyNavigate;
        QFrame*       mNavigateFrame;
        QFrame*       mAddToViewFrame;
        GraphNavigationTableWidget* mNavigateWidget;
        GraphNavigationTreeWidget*  mAddToViewWidget;

        bool mNavigateVisible;
        bool mAddToViewVisible;

        Node mOrigin;
        Net* mViaNet;
        SelectionRelay::Subfocus mDirection;

        QList<Node> mNavigateNodes;
        QHash<QTreeWidgetItem*,Node> mAddToViewNodes;

        /// module visible in view (might be implicitly as parent of gate or module)
        QSet<Module*> mModulesInView;

        /// gates or modulues connected with endpoint not shown in current view
        QList<Endpoint*> mEndpointNotInView;

        QHash<Module*, QTreeWidgetItem*> mListedModules;

        void viaNetByNode();
        void fillTable();
        void resizeToFit();
        void setModulesInView();
        void setModuleInView(Module* m);
        bool addNavigateItem(Endpoint* ep);
        bool addToViewItem(Endpoint* ep);

        QTreeWidgetItem* itemFactory(const QStringList& fields, const Node& nd);
        QStringList gateEntry(Gate* g, Endpoint* ep);
        QStringList moduleEntry(Module* m, Endpoint* ep);

        static const int sDefaultColumnWidth[];
        static const int sMaxHeight = 550;
        static const int sLabelHeight = 24;
    };

}
