#include "gui/plugin_management/loaded_plugins_widget.h"

#include "hal_core/plugin_system/plugin_interface_cli.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include "gui/gui_globals.h"
#include "gui/plugin_management/loaded_plugin_item.h"

#include <QFrame>
#include <QVBoxLayout>

namespace hal
{
    LoadedPluginsWidget::LoadedPluginsWidget(QWidget* parent) : QScrollArea(parent), mContainer(new QFrame()), mLayout(new QVBoxLayout()), mSpacer(new QFrame())
    {
        connect(gPluginRelay, &PluginRelay::pluginLoaded, this, &LoadedPluginsWidget::handlePluginLoaded);
        connect(gPluginRelay, &PluginRelay::pluginUnloaded, this, &LoadedPluginsWidget::handlePluginUnloaded);

        mContainer->setObjectName("container");

        mLayout->setContentsMargins(0, 0, 0, 0);
        mLayout->setSpacing(0);
        mLayout->setAlignment(Qt::AlignTop);

        mSpacer->setObjectName("spacer");

        mContainer->setLayout(mLayout);
        mLayout->addWidget(mSpacer);

        setFrameStyle(QFrame::NoFrame);
        setWidgetResizable(true);
        setWidget(mContainer);
    }

    void LoadedPluginsWidget::handlePluginLoaded(const QString& name, const QString& path)
    {
        Q_UNUSED(path)

        if (plugin_manager::get_plugin_instance<CLIPluginInterface>(name.toStdString(), false) == nullptr)
        {
            return;
        }

        mLayout->addWidget(new LoadedPluginItem(name, this));
    }

    void LoadedPluginsWidget::handlePluginUnloaded(const QString& name, const QString& path)
    {
        Q_UNUSED(path)

        for (QObject* child : mLayout->children())
        {
            LoadedPluginItem* item = static_cast<LoadedPluginItem*>(child);

            if (item->name() == name)
                item->deleteLater();
        }
    }
}
