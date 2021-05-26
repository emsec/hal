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
     * @ingroup graph
     * @brief A widget used to select destinations when traversing through the graph.
     *
     * A QTableWidget that is used to navigate from a pin with multiple adjacent pins. The table shows information about
     * the Gate%s/Module%s that are connected to the starting pin, such as name, id, type, the adjacent input/ouput pin
     * and the parent Module's name. <br>
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
         * Overridden event. Used to toggle the navigation widget when pressing <i>TAB</i>.
         * Note: It seems that this piece of code can't be put in the keyPressEvent. I won't work then.
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

        /**
         * Prepares the widget to be shown. Based on the current focus and subfocus of gSelectionRelay, it searches for
         * all valid navigation destinations in the given <i>direction</i>, i.e.: <ul>
         *    <li> search for destination endpoints if <i>direction</i> = SelectionRelay::Subfocus::Right   <br>
         *    <li> search for source endpoints if <i>direction</i> = SelectionRelay::Subfocus::Left         <br> </ul>
         * Both navigation widgets (GraphNavigationTableWidget and GraphNavigationTreeWidget) are filled up with these
         * navigation destinations depending on whether they are in the current context or not. <br>
         * After calling the <i>setup</i>-function one can call <i>setFocus()</i> to open and activate the
         * navigation widget.
         *
         * @param direction - The direction to navigate
         */
        void setup(SelectionRelay::Subfocus direction);

        /**
         * Prepares the widget to be shown. Based on the passed origin node and its connected net,
         * it searches for all valid navigation destinations in the given <i>direction</i>, i.e.: <ul>
         *   <li> search for destination endpoints if <i>direction</i> = SelectionRelay::Subfocus::Right   <br>
         *   <li> search for source endpoints if <i>direction</i> = SelectionRelay::Subfocus::Left         <br> </ul>
         * Both navigation widgets (GraphNavigationTableWidget and GraphNavigationTreeWidget) are filled up with these
         * navigation destinations depending on whether they are in the current context or not. <br>
         * After calling the <i>setup</i>-function one can call <i>setFocus()</i> to open and activate the
         * navigation widget.
         *
         * @param origin - The gate to navigate from
         * @param via_net - The net to navigate along
         * @param dir - The direction to navigate
         */
        void setup(Node origin, Net* via_net, SelectionRelay::Subfocus dir);

        /**
         * Gets the direction to navigate along. The direction is set in the <i>setup</i>-function. <br>
         * This function is internally used by both managed navigation widgets but should be irrelevant to
         * other classes.
         *
         * @returns the current navigation direction
         */
        SelectionRelay::Subfocus direction() const { return mDirection; }

        /**
         * Closes the navigation widget.
         */
        void closeRequest();

        /**
         * Checks if the navigation widget is empty. It is considered empty if neither the GraphNavigationTableWidget nor
         * the GraphNavigationTreeWidget contain data to show.
         *
         * @returns <b>true</b> if the navigation widget is considered empty.
         */
        bool isEmpty() const;

        /**
         * Checks if both the GraphNavigationTableWidget and the GraphNavigationTreeWidget contain data to show.
         *
         * @returns <b>true</b> if both widgets contain data to show.
         */
        bool hasBothWidgets() const;

        /**
         * Switches the focus within this widget from the GraphNavigationTableWidget to the GraphNavigationTreeWidget or
         * the other way around.
         */
        void toggleWidget();

    Q_SIGNALS:
        /**
         * This signal is sent after the user has selected a navigation target (chosen in the GraphNavigationTableWidget)
         * or gates and modules to add to the view (chosen in the GraphNavigationTreeWidget). <br>
         * If he has chosen a <b>navigation target</b>: <ul>
         *   <li> If the target is a gate, <i>to_gates</i> will contain the one id of the target gate while <i>to_modules</i>
         *   will be empty. <br>
         *   <li> If the target is a module, <i>to_gates</i> will be empty while <i>to_modules</i> will contain the one id of the
         *   target module. <br></ul>
         *
         * If he has chosen <b>gates and modules to add to the view</b>: <ul>
         *   <li> The sets <i>to_gates</i> and <i>to_modules</i> will be filled with the ids of the chosen gates and modules. </ul>
         *
         * @param origin - The gate to navigate from
         * @param via_net - The net to navigate along
         * @param to_gates - The navigation target / The gates to add to the view
         * @param to_modules - The navigation target / The modules to add to the view
         */
        void navigationRequested(const Node& origin, const u32 via_net, const QSet<u32>& to_gates, const QSet<u32>& to_modules);

        /**
         * This signal notifies that the navigation widget is about to be closed.
         */
        void closeRequested();

        /**
         * This signal notifies that the navigation widget is about to be closed.
         */
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

        /// gates or modules connected with endpoint not shown in current view
        QList<Endpoint*> mEndpointNotInView;

        QHash<Module*, QTreeWidgetItem*> mListedModules;

        void viaNetByNode();
        void fillTable();
        void resizeToFit();
        void setModulesInView();
        void setModuleInView(Module* m);
        void addNavigateItem(Endpoint* ep, const Node &targetNode);
        bool addToViewItem(Endpoint* ep);

        QTreeWidgetItem* itemFactory(const QStringList& fields, const Node& nd);
        QStringList gateEntry(Gate* g, Endpoint* ep);
        QStringList moduleEntry(Module* m, Endpoint* ep);

        static const int sDefaultColumnWidth[];
        static const int sMaxHeight = 550;
        static const int sLabelHeight = 24;
    };

}
