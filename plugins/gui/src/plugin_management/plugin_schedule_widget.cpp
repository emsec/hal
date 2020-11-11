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
        : QFrame(parent), mVerticalLayout(new QVBoxLayout()), mSearchbarContainer(new QFrame()), mContainerLayout(new QHBoxLayout()), mSearchbar(new Searchbar()),
          mHorizontalLayout(new QHBoxLayout()), mPluginFrame(new LabeledFrame()), mScheduleFrame(new LabeledFrame()), mScheduleFrameLayoutContainer(new QWidget()),
          mHorizontalScheduleFrameLayout(new QHBoxLayout()), mVerticalScheduleFrameLayout(new QVBoxLayout()), mNoScheduledPluginsWidget(new NoScheduledPluginsWidget()),
          mPluginArgumentsWidget(new PluginArgumentsWidget()), mScheduledPluginsWidget(new ScheduledPluginsWidget()), mLoadedPluginsWidget(new LoadedPluginsWidget())
    {
        connect(mNoScheduledPluginsWidget, &NoScheduledPluginsWidget::appendPlugin, mScheduledPluginsWidget, &ScheduledPluginsWidget::appendPlugin);
        connect(mScheduledPluginsWidget->area(), &ScheduledPluginItemArea::noScheduledPlugins, this, &PluginScheduleWidget::handleNoScheduledPlugins);

        connect(mScheduledPluginsWidget->area(), &ScheduledPluginItemArea::pluginSelected, mPluginArgumentsWidget, &PluginArgumentsWidget::handlePluginSelected);

        mVerticalLayout->setContentsMargins(0, 0, 0, 0);
        mVerticalLayout->setSpacing(0);

        mSearchbarContainer->setObjectName("Searchbar-container");

        mContainerLayout->setContentsMargins(0, 0, 0, 0);
        mContainerLayout->setSpacing(0);

        mSearchbar->setObjectName("Searchbar");

        mHorizontalLayout->setContentsMargins(0, 0, 0, 0);
        mHorizontalLayout->setSpacing(0);

        mPluginFrame->setObjectName("plugin-frame");
        mPluginFrame->addContent(mLoadedPluginsWidget);

        mScheduleFrame->setObjectName("schedule-frame");

        mHorizontalScheduleFrameLayout->setContentsMargins(0, 0, 0, 0);
        mHorizontalScheduleFrameLayout->setSpacing(0);

        mVerticalScheduleFrameLayout->setContentsMargins(0, 0, 0, 0);
        mVerticalScheduleFrameLayout->setSpacing(0);

        setLayout(mVerticalLayout);
        mVerticalLayout->addWidget(mSearchbarContainer);
        mVerticalLayout->setAlignment(mSearchbarContainer, Qt::AlignTop);
        mSearchbarContainer->setLayout(mContainerLayout);
        mContainerLayout->addWidget(mSearchbar);
        //mContainerLayout->setAlignment(mSearchbar, Qt::AlignLeft);
        mVerticalLayout->addLayout(mHorizontalLayout);
        //mVerticalLayout->setAlignment(mHorizontalLayout, Qt::AlignCenter);
        mHorizontalLayout->addWidget(mPluginFrame);
        mHorizontalLayout->addWidget(mScheduleFrame, Qt::AlignRight);
        mScheduleFrame->addContent(mScheduleFrameLayoutContainer);
        mScheduleFrameLayoutContainer->setLayout(mHorizontalScheduleFrameLayout);
        mHorizontalScheduleFrameLayout->addLayout(mVerticalScheduleFrameLayout);
        mVerticalScheduleFrameLayout->addWidget(mNoScheduledPluginsWidget);
        mVerticalScheduleFrameLayout->addWidget(mScheduledPluginsWidget);
        mScheduledPluginsWidget->hide();
        mHorizontalScheduleFrameLayout->addWidget(mPluginArgumentsWidget);

        mPluginFrame->setGraphicsEffect(new ShadowEffect());
        mScheduleFrame->setGraphicsEffect(new ShadowEffect());

        mNoScheduledPluginsWidget->repolish();    // MOVE TO REPOLISH METHOD

        mScheduleFrame->ensurePolished();
        ensurePolished();

        QShortcut* debug_shortcut = new QShortcut(QKeySequence(tr("Ctrl+x")), this);
        connect(debug_shortcut, &QShortcut::activated, this, &PluginScheduleWidget::debugStuff);
    }

    void PluginScheduleWidget::debugStuff()
    {
        QString plugin = "libfsm_detection";
        mPluginArgumentsWidget->setup(plugin);
    }

    void PluginScheduleWidget::handleNoScheduledPlugins()
    {
        mScheduledPluginsWidget->hide();
        mNoScheduledPluginsWidget->show();
    }
}
