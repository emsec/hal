#pragma once

#include "hal_core/plugin_system/plugin_interface_base.h"
#include "utils/utils.h"

namespace hal
{
    /* forward declaration */
    class Gate;
    class GateLibrary;
    class Net;

    class PLUGIN_API GateLibrarySpecificUtils : public BasePluginInterface
    {
    public:
        std::string get_name() const override;
        std::string get_version() const override;

        void initialize() override;

        std::unique_ptr<gate_library_specific_utils::Utils> get_gl_utils(const GateLibrary* gate_lib);

    private:
    };
}    // namespace hal
