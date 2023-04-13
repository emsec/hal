#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void netlist_serializer_init(py::module& m)
    {
        auto py_netlist_serializer = m.def_submodule("NetlistSerializer", R"(
            HAL Netlist Serializer functions.
        )");

        py_netlist_serializer.def("serialize_to_file", netlist_serializer::serialize_to_file, py::arg("netlist"), py::arg("hal_file"), R"(
            Serializes a netlist into a .hal file.
        
            :param hal_py.Netlist netlist: The netlist to serialize.
            :param pathlib.Path hal_file: The destination .hal file.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_netlist_serializer.def("deserialize_from_file", py::overload_cast<const std::filesystem::path&>(&netlist_serializer::deserialize_from_file), py::arg("hal_file"), R"(
            Deserializes a netlist from a .hal file.
        
            :param pathlib.Path hal_file: The source .hal file.
            :returns: The deserialized netlist.
            :rtype: hal_py.Netlist
        )");

        py_netlist_serializer.def("deserialize_from_file",
                                  py::overload_cast<const std::filesystem::path&, const std::filesystem::path&>(&netlist_serializer::deserialize_from_file),
                                  py::arg("hal_file"),
                                  py::arg("gate_library_file"),
                                  R"(
            Deserializes a netlist from a .hal file with a given gate library.
        
            :param pathlib.Path hal_file: The source .hal file.
            :param pathlib.Path gate_library_file: The gate library file.
            :returns: The deserialized netlist.
            :rtype: hal_py.Netlist
        )");
    }
}    // namespace hal
