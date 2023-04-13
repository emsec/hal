#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void set_log_level_of_channel(std::string channel_name, std::string level)
    {
        LogManager* lm = LogManager::get_instance();
        log_info(channel_name, "Set log level of channel '{}' to '{}'.", channel_name, level);
        if (lm) lm->set_level_of_channel(channel_name,level);
    }

#ifdef PYBIND11_MODULE
    PYBIND11_MODULE(hal_py, m)
    {
        m.doc() = "hal python bindings";
#else
    PYBIND11_PLUGIN(hal_py)
    {
        py::module m("hal_py", "hal python bindings");
#endif    // ifdef PYBIND11_MODULE

        m.def(
            "log_info", [](std::string& message) { log_info("python_context", message); }, R"( some documentation info)");

        m.def("set_log_level_of_channel", set_log_level_of_channel, py::arg("channel_name"), py::arg("level"), R"(
              Set log level for channel.

              :param str channel_name: Name of channel.
              :param str level: Selected level, one out of [trace, debug, info, warn, err, critical, off].
              )");

        data_container_init(m);

        core_utils_init(m);

        gate_type_init(m);

        gate_type_components_init(m);

        gate_library_init(m);

        gate_library_manager_init(m);

        endpoint_init(m);

        netlist_init(m);

        project_manager_init(m);

        netlist_serializer_init(m);

        netlist_utils_init(m);

        base_pin_init(m);

        gate_pin_init(m);

        gate_pin_group_init(m);

        module_pin_init(m);

        module_pin_group_init(m);

        gate_init(m);

        net_init(m);

        module_init(m);

        grouping_init(m);

        netlist_factory_init(m);

        netlist_writer_manager_init(m);

        plugin_manager_init(m);

        plugin_interfaces_init(m);

        boolean_function_init(m);

        smt_init(m);

        boolean_function_net_decorator_init(m);

        subgraph_netlist_decorator_init(m);

        boolean_function_decorator_init(m);

        netlist_modification_decorator_init(m);

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
