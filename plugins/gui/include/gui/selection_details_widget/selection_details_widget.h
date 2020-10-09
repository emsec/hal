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

#include "gui/content_widget/content_widget.h"
#include "gui/selection_details_widget/tree_navigation/selection_tree_item.h"
#include "hal_core/defines.h"

class QTableWidget;
class QStackedWidget;
class QSplitter;
class QListWidget;
class QLabel;

namespace hal
{
    class Grouping;
    class Searchbar;
    class SelectionTreeView;
    class ModuleDetailsWidget;
    class GateDetailsWidget;
    class NetDetailsWidget;
    class SelectionHistoryNavigator;

    class SelectionDetailsWidget : public ContentWidget
    {
        Q_OBJECT
        Q_PROPERTY(QString search_icon_path READ search_icon_path WRITE set_search_icon_path)
        Q_PROPERTY(QString search_icon_style READ search_icon_style WRITE set_search_icon_style)

    public:
        SelectionDetailsWidget(QWidget* parent = 0);
        void clear();

        virtual void setup_toolbar(Toolbar* toolbar) Q_DECL_OVERRIDE;

        QString search_icon_path() const;
        QString search_icon_style() const;

        void set_search_icon_path(const QString &path);
        void set_search_icon_style(const QString &style);

    Q_SIGNALS:
        void triggerHighlight(QVector<const SelectionTreeItem*> highlight);

    public Q_SLOTS:
        void handle_selection_update(void* sender);
        void handleTreeSelection(const SelectionTreeItem* sti);
        QList<QShortcut*> create_shortcuts() Q_DECL_OVERRIDE;


    private Q_SLOTS:
        void restoreLastSelection();
        void selectionToGrouping();
        void toggle_searchbar();
        void selectionToNewGrouping();
        void selectionToExistingGrouping();

    private:
        void selectionToGroupingInternal(Grouping* grp);
        void singleSelectionInternal(const SelectionTreeItem* sti);

        QSplitter*           m_splitter;
        SelectionTreeView*   m_selectionTreeView;
        QWidget*             m_selectionDetails;
        unsigned int         m_numberSelectedItems;

        QStackedWidget* m_stacked_widget;

        GateDetailsWidget* m_gate_details;
        NetDetailsWidget* m_net_details;
        ModuleDetailsWidget* m_module_details;
        QLabel* m_item_deleted_label;
        QLabel* m_no_selection_label;

        Searchbar* m_searchbar;

        QAction* m_restoreLastSelection;
        QAction* m_selectionToGrouping;
        QAction* m_search_action;
        QString m_search_icon_path;
        QString m_search_icon_style;

        SelectionHistoryNavigator* m_history;

        void handle_filter_text_changed(const QString& filter_text);
        void canRestoreSelection();
        void enableSearchbar(bool enable);

        static const QString disableIconStyle;
    };
}
