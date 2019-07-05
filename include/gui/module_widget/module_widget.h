/*
 * MIT License
 *
 * Copyright (c) 2019 Chair for Embedded Security, Ruhr-University Bochum
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef MODULE_WIDGET_H
#define MODULE_WIDGET_H

#include "def.h"

#include "netlist/event_system/module_event_handler.h"
#include "netlist/module.h"

#include "gui/content_widget/content_widget.h"
#include "gui/searchbar/searchbar.h"
#include "gui/selection_relay/selection_relay.h"

#include <QAction>
#include <QItemSelection>
#include <QList>
#include <QObject>
#include <QSortFilterProxyModel>
#include <QTreeView>

class graph_navigation_model;
class QTreeView;
class graph_navigation_proxy_model;

class tree_navigation_model;
class module_proxy_model;

class module_widget : public content_widget
{
    Q_OBJECT

public:
    module_widget(QWidget* parent = nullptr);

    virtual void setup_toolbar(toolbar* toolbar) Q_DECL_OVERRIDE;
    virtual QList<QShortcut*> create_shortcuts() Q_DECL_OVERRIDE;

public Q_SLOTS:
    //void filter();
    void toggle_searchbar();
    void filter(const QString& text);
    void handle_tree_view_context_menu_requested(const QPoint& point);
    void handle_filter_action_triggered();
    void handle_selection_changed(const QItemSelection& selected, const QItemSelection& deselected);
    void handle_current_changed(const QModelIndex& current, const QModelIndex& previous);

    //Selection Relay Slots
    void handle_gate_selection_update(void* sender, const QList<u32>& gate_ids, selection_relay::Mode mode);
    void handle_net_selection_update(void* sender, const QList<u32>& net_ids, selection_relay::Mode mode);
    void handle_module_selection_update(void* sender, const QList<u32>& module_ids, selection_relay::Mode mode);
    void handle_combined_selection_update(void* sender, const QList<u32>& gate_ids, const QList<u32>& net_ids, const QList<u32>& module_ids, selection_relay::Mode mode);

    void handle_current_gate_update(void* sender, u32 id);
    void handle_current_net_update(void* sender, u32 id);
    void handle_current_module_update(void* sender, u32 id);

    void handle_jump_gate_update(void* sender, u32 id);
    void handle_jump_net_update(void* sender, u32 id);
    void handle_jump_module_update(void* sender, u32 id);
    void handle_jump_selection_update(void* sender);

    void handle_gate_highlight_update(void* sender, QList<u32>& ids, selection_relay::Mode mode, u32 channel);
    void handle_net_highlight_update(void* sender, QList<u32>& ids, selection_relay::Mode mode, u32 channel);
    void handle_module_highlight_update(void* sender, QList<u32>& ids, selection_relay::Mode mode, u32 channel);
    void handle_combined_highlight_update(void* sender, QList<u32>& gate_ids, QList<u32>& net_ids, QList<u32>& module_ids, selection_relay::Mode mode, u32 channel);

    void toggle_resize_columns();

private:
    QTreeView* m_tree_view;
    searchbar m_searchbar;

    QAction* m_filter_action;

    QSortFilterProxyModel* m_current_model;

    QList<QRegExp*> m_regexps;

    bool m_ignore_selection_change;

    module_proxy_model* m_module_proxy_model;
};

#endif // MODULE_WIDGET_H
