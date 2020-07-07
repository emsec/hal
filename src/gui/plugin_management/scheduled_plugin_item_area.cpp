#include "plugin_management/scheduled_plugin_item_area.h"

#include "drop_marker/drop_marker.h"
#include "plugin_management/plugin_arguments_widget.h"
#include "plugin_management/plugin_schedule_manager.h"
#include "plugin_management/scheduled_plugin_item.h"

#include <QDragEnterEvent>
#include <QMimeData>
#include <QVBoxLayout>

namespace hal
{
    ScheduledPluginItemArea::ScheduledPluginItemArea(QWidget* parent)
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

    void ScheduledPluginItemArea::dragEnterEvent(QDragEnterEvent* event)
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

    void ScheduledPluginItemArea::dragMoveEvent(QDragMoveEvent* event)
    {
        int y = event->pos().y();

        for (int i = 0; i < m_y_values.length(); i++)
        {
            if (y < m_y_values.at(i))
            {
                DropMarker* new_marker = m_list.at(i).second;

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

    void ScheduledPluginItemArea::dragLeaveEvent(QDragLeaveEvent* event)
    {
        Q_UNUSED(event)

        if (m_active_marker)
            m_active_marker->collapse();

        m_active_marker = nullptr;
    }

    void ScheduledPluginItemArea::dropEvent(QDropEvent* event)
    {
        const QMimeData* mime_data = event->mimeData();
        QString name               = QString(mime_data->data("hal/plugin_name"));
        insert_plugin(name);
        event->acceptProposedAction();
    }

    void ScheduledPluginItemArea::insert_plugin(const QString& name)
    {
        ScheduledPluginItem* item = new ScheduledPluginItem(name);
        connect(item, &ScheduledPluginItem::clicked, this, &ScheduledPluginItemArea::handle_item_clicked);
        connect(item, &ScheduledPluginItem::drag_started, this, &ScheduledPluginItemArea::handle_item_drag_started);
        connect(item, &ScheduledPluginItem::removed, this, &ScheduledPluginItemArea::handle_item_removed);
        DropMarker* marker = new DropMarker(Qt::Vertical);

        int drop_index = -1;
        if (m_active_marker)
        {
            for (int i = 0; i < m_y_values.length(); i++)
            {
                if (m_list.at(i).second == m_active_marker)
                {
                    m_list.insert(i, QPair<ScheduledPluginItem*, DropMarker*>(item, marker));
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
            m_list.append(QPair<ScheduledPluginItem*, DropMarker*>(item, marker));
            m_layout->addWidget(marker);
            m_layout->addWidget(item);
            drop_index = m_list.length() - 1;
        }

        if (m_internal_drag_active)
        {
            PluginScheduleManager::get_instance()->move_plugin(m_drag_index, drop_index);
            m_internal_drag_active = false;
        }
        else
            PluginScheduleManager::get_instance()->add_plugin(name, drop_index);
    }

    void ScheduledPluginItemArea::handle_item_clicked(ScheduledPluginItem* item)
    {
        int i = 0;

        for (auto& pair : m_list)
        {
            if (pair.first == item)
            {
                PluginScheduleManager::get_instance()->set_current_index(i);
                Q_EMIT plugin_selected(i);
                return;
            }
            i++;
        }
    }

    void ScheduledPluginItemArea::handle_item_drag_started(ScheduledPluginItem* item)
    {
        m_internal_drag_active = true;
        m_drag_index           = 0;
        DropMarker* marker    = nullptr;

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

    void ScheduledPluginItemArea::handle_item_removed(ScheduledPluginItem* item)
    {
        Q_UNUSED(item)

        m_internal_drag_active = false;
        PluginScheduleManager::get_instance()->remove_plugin(m_drag_index);

        if (m_list.isEmpty())
            Q_EMIT no_scheduled_plugins();
    }
}
