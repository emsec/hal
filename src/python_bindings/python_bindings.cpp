#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
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
            "log_info", [](std::string& message) { log_info("PythonContext", message); }, R"( some documentation info)");

        path_init(m);

        data_container_init(m);

        core_utils_init(m);

        gate_type_init(m);

        gate_type_lut_init(m);

        gate_type_sequential_init(m);

        gate_library_init(m);

        gate_library_manager_init(m);

        endpoint_init(m);

        netlist_init(m);

        netlist_utils_init(m);

        gate_init(m);

        net_init(m);

        module_init(m);

        grouping_init(m);

        netlist_factory_init(m);

        hdl_writer_manager_init(m);

        plugin_manager_init(m);

        plugin_interfaces_init(m);

        boolean_function_init(m);

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
