#include "gui/plugin_management/scheduled_plugin_item_area.h"

#include "gui/drop_marker/drop_marker.h"
#include "gui/plugin_management/plugin_arguments_widget.h"
#include "gui/plugin_management/plugin_schedule_manager.h"
#include "gui/plugin_management/scheduled_plugin_item.h"

#include <QDragEnterEvent>
#include <QMimeData>
#include <QVBoxLayout>

namespace hal
{
    ScheduledPluginItemArea::ScheduledPluginItemArea(QWidget* parent)
        : QFrame(parent), mLayout(new QVBoxLayout()), mSpacer(new QFrame()), mActiveMarker(nullptr), mInternalDragActive(false), mDragIndex(-1)
    {
        mLayout->setContentsMargins(0, 0, 0, 0);
        mLayout->setSpacing(0);
        mLayout->setAlignment(Qt::AlignTop);

        mSpacer->setObjectName("spacer");

        setLayout(mLayout);
        mLayout->addWidget(mSpacer);

        setAcceptDrops(true);
    }

    void ScheduledPluginItemArea::dragEnterEvent(QDragEnterEvent* event)
    {
        const QMimeData* mime_data = event->mimeData();
        QString name               = QString(mime_data->data("hal/plugin_name"));

        if (name.isEmpty())
            return;

        mYValues.clear();
        int y = mSpacer->height();

        for (auto& pair : mList)
        {
            y += pair.first->height();
            mYValues.append(y);

            // FIX MAGIC NUMBER
            pair.second->setEndValue(30);
        }

        event->acceptProposedAction();
    }

    void ScheduledPluginItemArea::dragMoveEvent(QDragMoveEvent* event)
    {
        int y = event->pos().y();

        for (int i = 0; i < mYValues.length(); i++)
        {
            if (y < mYValues.at(i))
            {
                DropMarker* new_marker = mList.at(i).second;

                if (mActiveMarker == new_marker)
                    return;

                if (mActiveMarker)
                    mActiveMarker->collapse();

                mActiveMarker = new_marker;
                mActiveMarker->expand();
                return;
            }
        }

        if (mActiveMarker)
            mActiveMarker->collapse();

        mActiveMarker = nullptr;
        // SHOW APPEND MARKER (MAYBE)
    }

    void ScheduledPluginItemArea::dragLeaveEvent(QDragLeaveEvent* event)
    {
        Q_UNUSED(event)

        if (mActiveMarker)
            mActiveMarker->collapse();

        mActiveMarker = nullptr;
    }

    void ScheduledPluginItemArea::dropEvent(QDropEvent* event)
    {
        const QMimeData* mime_data = event->mimeData();
        QString name               = QString(mime_data->data("hal/plugin_name"));
        insertPlugin(name);
        event->acceptProposedAction();
    }

    void ScheduledPluginItemArea::insertPlugin(const QString& name)
    {
        ScheduledPluginItem* item = new ScheduledPluginItem(name);
        connect(item, &ScheduledPluginItem::clicked, this, &ScheduledPluginItemArea::handleItemClicked);
        connect(item, &ScheduledPluginItem::dragStarted, this, &ScheduledPluginItemArea::handleItemDragStarted);
        connect(item, &ScheduledPluginItem::removed, this, &ScheduledPluginItemArea::handleItemRemoved);
        DropMarker* marker = new DropMarker(Qt::Vertical);

        int drop_index = -1;
        if (mActiveMarker)
        {
            for (int i = 0; i < mYValues.length(); i++)
            {
                if (mList.at(i).second == mActiveMarker)
                {
                    mList.insert(i, QPair<ScheduledPluginItem*, DropMarker*>(item, marker));
                    //                mLayout->insertWidget(i+1, item);
                    //                mLayout->insertWidget(i+1, marker); //DOESNT WORK, FIX

                    //TEST
                    for (auto& pair : mList)
                    {
                        mLayout->removeWidget(pair.second);
                        mLayout->removeWidget(pair.first);

                        mLayout->addWidget(pair.second);
                        mLayout->addWidget(pair.first);
                    }
                    //END OF TEST
                    drop_index = i;
                    break;
                }
            }
            mActiveMarker->reset();
            mActiveMarker = nullptr;
        }
        else
        {
            mList.append(QPair<ScheduledPluginItem*, DropMarker*>(item, marker));
            mLayout->addWidget(marker);
            mLayout->addWidget(item);
            drop_index = mList.length() - 1;
        }

        if (mInternalDragActive)
        {
            PluginScheduleManager::get_instance()->movePlugin(mDragIndex, drop_index);
            mInternalDragActive = false;
        }
        else
            PluginScheduleManager::get_instance()->addPlugin(name, drop_index);
    }

    void ScheduledPluginItemArea::handleItemClicked(ScheduledPluginItem* item)
    {
        int i = 0;

        for (auto& pair : mList)
        {
            if (pair.first == item)
            {
                PluginScheduleManager::get_instance()->setCurrentIndex(i);
                Q_EMIT pluginSelected(i);
                return;
            }
            i++;
        }
    }

    void ScheduledPluginItemArea::handleItemDragStarted(ScheduledPluginItem* item)
    {
        mInternalDragActive = true;
        mDragIndex           = 0;
        DropMarker* marker    = nullptr;

        for (auto& pair : mList)
        {
            if (pair.first == item)
            {
                marker = pair.second;
                break;
            }
            mDragIndex++;
        }

        item->hide();
        marker->hide();
        mLayout->removeWidget(item);
        mLayout->removeWidget(marker);
        item->deleteLater();
        marker->deleteLater();
        mList.removeAt(mDragIndex);
    }

    void ScheduledPluginItemArea::handleItemRemoved(ScheduledPluginItem* item)
    {
        Q_UNUSED(item)

        mInternalDragActive = false;
        PluginScheduleManager::get_instance()->removePlugin(mDragIndex);

        if (mList.isEmpty())
            Q_EMIT noScheduledPlugins();
    }
}
