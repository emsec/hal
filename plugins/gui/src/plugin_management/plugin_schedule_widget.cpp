#include "gui/plugin_management/plugin_schedule_widget.h"

#include "gui/frames/labeled_frame.h"
#include "gui/graphics_effects/shadow_effect.h"
#include "gui/plugin_management/loaded_plugins_widget.h"
#include "gui/plugin_management/no_scheduled_plugins_widget.h"
#include "gui/plugin_management/plugin_arguments_widget.h"
#include "gui/plugin_management/scheduled_plugin_item_area.h"
#include "gui/plugin_management/scheduled_plugins_widget.h"
#include "gui/searchbar/searchbar.h"

#include <QHBoxLayout>
#include <QShortcut>
#include <QVBoxLayout>

namespace hal
{
    PluginScheduleWidget::PluginScheduleWidget(QWidget* parent)
        : QFrame(parent), m_vertical_layout(new QVBoxLayout()), m_searchbar_container(new QFrame()), m_container_layout(new QHBoxLayout()), m_searchbar(new Searchbar()),
          m_horizontal_layout(new QHBoxLayout()), m_plugin_frame(new LabeledFrame()), m_schedule_frame(new LabeledFrame()), m_schedule_frame_layout_container(new QWidget()),
          m_horizontal_schedule_frame_layout(new QHBoxLayout()), m_vertical_schedule_frame_layout(new QVBoxLayout()), m_NoScheduledPluginsWidget(new NoScheduledPluginsWidget()),
          m_PluginArgumentsWidget(new PluginArgumentsWidget()), m_ScheduledPluginsWidget(new ScheduledPluginsWidget()), m_LoadedPluginsWidget(new LoadedPluginsWidget())
    {
        connect(m_NoScheduledPluginsWidget, &NoScheduledPluginsWidget::append_plugin, m_ScheduledPluginsWidget, &ScheduledPluginsWidget::append_plugin);
        connect(m_ScheduledPluginsWidget->area(), &ScheduledPluginItemArea::no_scheduled_plugins, this, &PluginScheduleWidget::handle_no_scheduled_plugins);

        connect(m_ScheduledPluginsWidget->area(), &ScheduledPluginItemArea::plugin_selected, m_PluginArgumentsWidget, &PluginArgumentsWidget::handle_plugin_selected);

        m_vertical_layout->setContentsMargins(0, 0, 0, 0);
        m_vertical_layout->setSpacing(0);

        m_searchbar_container->setObjectName("Searchbar-container");

        m_container_layout->setContentsMargins(0, 0, 0, 0);
        m_container_layout->setSpacing(0);

        m_searchbar->setObjectName("Searchbar");

        m_horizontal_layout->setContentsMargins(0, 0, 0, 0);
        m_horizontal_layout->setSpacing(0);

        m_plugin_frame->setObjectName("plugin-frame");
        m_plugin_frame->add_content(m_LoadedPluginsWidget);

        m_schedule_frame->setObjectName("schedule-frame");

        m_horizontal_schedule_frame_layout->setContentsMargins(0, 0, 0, 0);
        m_horizontal_schedule_frame_layout->setSpacing(0);

        m_vertical_schedule_frame_layout->setContentsMargins(0, 0, 0, 0);
        m_vertical_schedule_frame_layout->setSpacing(0);

        setLayout(m_vertical_layout);
        m_vertical_layout->addWidget(m_searchbar_container);
        m_vertical_layout->setAlignment(m_searchbar_container, Qt::AlignTop);
        m_searchbar_container->setLayout(m_container_layout);
        m_container_layout->addWidget(m_searchbar);
        //m_container_layout->setAlignment(m_searchbar, Qt::AlignLeft);
        m_vertical_layout->addLayout(m_horizontal_layout);
        //m_vertical_layout->setAlignment(m_horizontal_layout, Qt::AlignCenter);
        m_horizontal_layout->addWidget(m_plugin_frame);
        m_horizontal_layout->addWidget(m_schedule_frame, Qt::AlignRight);
        m_schedule_frame->add_content(m_schedule_frame_layout_container);
        m_schedule_frame_layout_container->setLayout(m_horizontal_schedule_frame_layout);
        m_horizontal_schedule_frame_layout->addLayout(m_vertical_schedule_frame_layout);
        m_vertical_schedule_frame_layout->addWidget(m_NoScheduledPluginsWidget);
        m_vertical_schedule_frame_layout->addWidget(m_ScheduledPluginsWidget);
        m_ScheduledPluginsWidget->hide();
        m_horizontal_schedule_frame_layout->addWidget(m_PluginArgumentsWidget);

        m_plugin_frame->setGraphicsEffect(new ShadowEffect());
        m_schedule_frame->setGraphicsEffect(new ShadowEffect());

        m_NoScheduledPluginsWidget->repolish();    // MOVE TO REPOLISH METHOD

        m_schedule_frame->ensurePolished();
        ensurePolished();

        QShortcut* debug_shortcut = new QShortcut(QKeySequence(tr("Ctrl+x")), this);
        connect(debug_shortcut, &QShortcut::activated, this, &PluginScheduleWidget::debug_stuff);
    }

    void PluginScheduleWidget::debug_stuff()
    {
        QString plugin = "libfsm_detection";
        m_PluginArgumentsWidget->setup(plugin);
    }

    void PluginScheduleWidget::handle_no_scheduled_plugins()
    {
        m_ScheduledPluginsWidget->hide();
        m_NoScheduledPluginsWidget->show();
    }
}
