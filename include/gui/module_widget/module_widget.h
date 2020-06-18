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

#include "gui/module_model/module_item.h"
#include "module_tree_view.h"

#include <QAction>
#include <QItemSelection>
#include <QList>
#include <QObject>
#include <QSortFilterProxyModel>
namespace hal{
class module_proxy_model;

class QTreeView;

class module_widget : public content_widget
{
    Q_OBJECT

public:
    module_widget(QWidget* parent = nullptr);

    virtual void setup_toolbar(toolbar* toolbar) override;
    virtual QList<QShortcut*> create_shortcuts() override;

public Q_SLOTS:
    void toggle_searchbar();
    void filter(const QString& text);
    void handle_tree_view_context_menu_requested(const QPoint& point);
    void handle_tree_selection_changed(const QItemSelection& selected, const QItemSelection& deselected);
    void handle_item_double_clicked(const QModelIndex &index);
    void handle_selection_changed(void* sender);
    void handle_module_removed(std::shared_ptr<Module> module, u32 module_id);

private:
    module_tree_view* m_tree_view;
    searchbar m_searchbar;

    QAction* m_filter_action;

    QSortFilterProxyModel* m_current_model;

    QList<QRegExp*> m_regexps;

    bool m_ignore_selection_change;

    module_proxy_model* m_module_proxy_model;

    void open_module_in_view(const QModelIndex &index);

    module_item* get_module_item_from_index(const QModelIndex &index);
};
}

#endif // MODULE_WIDGET_H
