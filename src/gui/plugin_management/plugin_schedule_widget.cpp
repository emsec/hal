#include "plugin_management/plugin_schedule_widget.h"

#include "frames/labeled_frame.h"
#include "graphics_effects/shadow_effect.h"
#include "plugin_management/loaded_plugins_widget.h"
#include "plugin_management/no_scheduled_plugins_widget.h"
#include "plugin_management/plugin_arguments_widget.h"
#include "plugin_management/scheduled_plugin_item_area.h"
#include "plugin_management/scheduled_plugins_widget.h"
#include "searchbar/searchbar.h"

#include <QHBoxLayout>
#include <QShortcut>
#include <QVBoxLayout>
namespace hal{
plugin_schedule_widget::plugin_schedule_widget(QWidget* parent)
    : QFrame(parent), m_vertical_layout(new QVBoxLayout()), m_searchbar_container(new QFrame()), m_container_layout(new QHBoxLayout()), m_searchbar(new searchbar()),
      m_horizontal_layout(new QHBoxLayout()), m_plugin_frame(new labeled_frame()), m_schedule_frame(new labeled_frame()), m_schedule_frame_layout_container(new QWidget()),
      m_horizontal_schedule_frame_layout(new QHBoxLayout()), m_vertical_schedule_frame_layout(new QVBoxLayout()), m_no_scheduled_plugins_widget(new no_scheduled_plugins_widget()),
      m_plugin_arguments_widget(new plugin_arguments_widget()), m_scheduled_plugins_widget(new scheduled_plugins_widget()), m_loaded_plugins_widget(new loaded_plugins_widget())
{
    connect(m_no_scheduled_plugins_widget, &no_scheduled_plugins_widget::append_plugin, m_scheduled_plugins_widget, &scheduled_plugins_widget::append_plugin);
    connect(m_scheduled_plugins_widget->area(), &scheduled_plugin_item_area::no_scheduled_plugins, this, &plugin_schedule_widget::handle_no_scheduled_plugins);

    connect(m_scheduled_plugins_widget->area(), &scheduled_plugin_item_area::plugin_selected, m_plugin_arguments_widget, &plugin_arguments_widget::handle_plugin_selected);

    m_vertical_layout->setContentsMargins(0, 0, 0, 0);
    m_vertical_layout->setSpacing(0);

    m_searchbar_container->setObjectName("searchbar-container");

    m_container_layout->setContentsMargins(0, 0, 0, 0);
    m_container_layout->setSpacing(0);

    m_searchbar->setObjectName("searchbar");

    m_horizontal_layout->setContentsMargins(0, 0, 0, 0);
    m_horizontal_layout->setSpacing(0);

    m_plugin_frame->setObjectName("plugin-frame");
    m_plugin_frame->add_content(m_loaded_plugins_widget);

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
    m_vertical_schedule_frame_layout->addWidget(m_no_scheduled_plugins_widget);
    m_vertical_schedule_frame_layout->addWidget(m_scheduled_plugins_widget);
    m_scheduled_plugins_widget->hide();
    m_horizontal_schedule_frame_layout->addWidget(m_plugin_arguments_widget);

    m_plugin_frame->setGraphicsEffect(new shadow_effect());
    m_schedule_frame->setGraphicsEffect(new shadow_effect());

    m_no_scheduled_plugins_widget->repolish();    // MOVE TO REPOLISH METHOD

    m_schedule_frame->ensurePolished();
    ensurePolished();

    QShortcut* debug_shortcut = new QShortcut(QKeySequence(tr("Ctrl+x")), this);
    connect(debug_shortcut, &QShortcut::activated, this, &plugin_schedule_widget::debug_stuff);
}

void plugin_schedule_widget::debug_stuff()
{
    QString plugin = "libfsm_detection";
    m_plugin_arguments_widget->setup(plugin);
}

void plugin_schedule_widget::handle_no_scheduled_plugins()
{
    m_scheduled_plugins_widget->hide();
    m_no_scheduled_plugins_widget->show();
}
}
