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

    class GraphNavigationTableWidget : public QTableWidget
    {
        GraphNavigationWidget* mNavigationWidget;
    protected:
        void keyPressEvent(QKeyEvent *event) override;
        void focusInEvent(QFocusEvent* event) override;
    public:
        GraphNavigationTableWidget(GraphNavigationWidget* nav, QWidget* parent = nullptr)
            : QTableWidget(parent), mNavigationWidget(nav) {;}
    };

    class GraphNavigationTreeWidget : public QTreeWidget
    {
        GraphNavigationWidget* mNavigationWidget;
        QList<QTreeWidgetItem*> selectedItemRecursion(QTreeWidgetItem* item) const;
    protected:
        void keyPressEvent(QKeyEvent *event) override;
        void focusInEvent(QFocusEvent* event) override;
        bool event(QEvent *ev) override;
    public:
        GraphNavigationTreeWidget(GraphNavigationWidget* nav, QWidget* parent = nullptr)
            : QTreeWidget(parent), mNavigationWidget(nav) {;}
        QList<QTreeWidgetItem*> selectedItems() const;
        QModelIndex firstIndex() const;
    };

    class GraphNavigationWidget : public QWidget
    {
        Q_OBJECT

        friend class GraphNavigationTableWidget;
        friend class GraphNavigationTreeWidget;
    public:
        GraphNavigationWidget(bool onlyNavigate, QWidget* parent = nullptr);
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
