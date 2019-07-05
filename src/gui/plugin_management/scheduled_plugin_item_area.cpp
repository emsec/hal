#include "plugin_management/scheduled_plugin_item_area.h"

#include "drop_marker/drop_marker.h"
#include "plugin_management/plugin_arguments_widget.h"
#include "plugin_management/plugin_schedule_manager.h"
#include "plugin_management/scheduled_plugin_item.h"

#include <QDragEnterEvent>
#include <QMimeData>
#include <QVBoxLayout>

scheduled_plugin_item_area::scheduled_plugin_item_area(QWidget* parent)
    : QFrame(parent), m_layout(new QVBoxLayout()), m_spacer(new QFrame()), m_active_marker(nullptr), m_internal_drag_active(false), m_drag_index(-1)
{
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_layout->setAlignment(Qt::AlignTop);

    m_spacer->setObjectName("spacer");

    setLayout(m_layout);
    m_layout->addWidget(m_spacer);

    setAcceptDrops(true);
}

void scheduled_plugin_item_area::dragEnterEvent(QDragEnterEvent* event)
{
    const QMimeData* mime_data = event->mimeData();
    QString name               = QString(mime_data->data("hal/plugin_name"));

    if (name.isEmpty())
        return;

    m_y_values.clear();
    int y = m_spacer->height();

    for (auto& pair : m_list)
    {
        y += pair.first->height();
        m_y_values.append(y);

        // FIX MAGIC NUMBER
        pair.second->set_end_value(30);
    }

    event->acceptProposedAction();
}

void scheduled_plugin_item_area::dragMoveEvent(QDragMoveEvent* event)
{
    int y = event->pos().y();

    for (int i = 0; i < m_y_values.length(); i++)
    {
        if (y < m_y_values.at(i))
        {
            drop_marker* new_marker = m_list.at(i).second;

            if (m_active_marker == new_marker)
                return;

            if (m_active_marker)
                m_active_marker->collapse();

            m_active_marker = new_marker;
            m_active_marker->expand();
            return;
        }
    }

    if (m_active_marker)
        m_active_marker->collapse();

    m_active_marker = nullptr;
    // SHOW APPEND MARKER (MAYBE)
}

void scheduled_plugin_item_area::dragLeaveEvent(QDragLeaveEvent* event)
{
    Q_UNUSED(event)

    if (m_active_marker)
        m_active_marker->collapse();

    m_active_marker = nullptr;
}

void scheduled_plugin_item_area::dropEvent(QDropEvent* event)
{
    const QMimeData* mime_data = event->mimeData();
    QString name               = QString(mime_data->data("hal/plugin_name"));
    insert_plugin(name);
    event->acceptProposedAction();
}

void scheduled_plugin_item_area::insert_plugin(const QString& name)
{
    scheduled_plugin_item* item = new scheduled_plugin_item(name);
    connect(item, &scheduled_plugin_item::clicked, this, &scheduled_plugin_item_area::handle_item_clicked);
    connect(item, &scheduled_plugin_item::drag_started, this, &scheduled_plugin_item_area::handle_item_drag_started);
    connect(item, &scheduled_plugin_item::removed, this, &scheduled_plugin_item_area::handle_item_removed);
    drop_marker* marker = new drop_marker(Qt::Vertical);

    int drop_index = -1;
    if (m_active_marker)
    {
        for (int i = 0; i < m_y_values.length(); i++)
        {
            if (m_list.at(i).second == m_active_marker)
            {
                m_list.insert(i, QPair<scheduled_plugin_item*, drop_marker*>(item, marker));
                //                m_layout->insertWidget(i+1, item);
                //                m_layout->insertWidget(i+1, marker); //DOESNT WORK, FIX

                //TEST
                for (auto& pair : m_list)
                {
                    m_layout->removeWidget(pair.second);
                    m_layout->removeWidget(pair.first);

                    m_layout->addWidget(pair.second);
                    m_layout->addWidget(pair.first);
                }
                //END OF TEST
                drop_index = i;
                break;
            }
        }
        m_active_marker->reset();
        m_active_marker = nullptr;
    }
    else
    {
        m_list.append(QPair<scheduled_plugin_item*, drop_marker*>(item, marker));
        m_layout->addWidget(marker);
        m_layout->addWidget(item);
        drop_index = m_list.length() - 1;
    }

    if (m_internal_drag_active)
    {
        plugin_schedule_manager::get_instance()->move_plugin(m_drag_index, drop_index);
        m_internal_drag_active = false;
    }
    else
        plugin_schedule_manager::get_instance()->add_plugin(name, drop_index);
}

void scheduled_plugin_item_area::handle_item_clicked(scheduled_plugin_item* item)
{
    int i = 0;

    for (auto& pair : m_list)
    {
        if (pair.first == item)
        {
            plugin_schedule_manager::get_instance()->set_current_index(i);
            Q_EMIT plugin_selected(i);
            return;
        }
        i++;
    }
}

void scheduled_plugin_item_area::handle_item_drag_started(scheduled_plugin_item* item)
{
    m_internal_drag_active = true;
    m_drag_index           = 0;
    drop_marker* marker    = nullptr;

    for (auto& pair : m_list)
    {
        if (pair.first == item)
        {
            marker = pair.second;
            break;
        }
        m_drag_index++;
    }

    item->hide();
    marker->hide();
    m_layout->removeWidget(item);
    m_layout->removeWidget(marker);
    item->deleteLater();
    marker->deleteLater();
    m_list.removeAt(m_drag_index);
}

void scheduled_plugin_item_area::handle_item_removed(scheduled_plugin_item* item)
{
    Q_UNUSED(item)

    m_internal_drag_active = false;
    plugin_schedule_manager::get_instance()->remove_plugin(m_drag_index);

    if (m_list.isEmpty())
        Q_EMIT no_scheduled_plugins();
}
