#pragma once

#include "hal_core/plugin_system/plugin_interface_base.h"

namespace hal
{
    /**
     * The plugin that provides the GENLIB gate library writer.
     */
    class PLUGIN_API GenlibWriterPlugin : public BasePluginInterface
    {
    public:
        std::string get_name() const override;
        std::string get_version() const override;

        void on_load() override;
        void on_unload() override;
    };
}    // namespace hal
