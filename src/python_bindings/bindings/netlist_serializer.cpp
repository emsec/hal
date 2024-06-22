#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void netlist_serializer_init(py::module& m)
    {
        auto py_netlist_serializer = m.def_submodule("NetlistSerializer", R"(
            HAL Netlist Serializer functions.
        )");

        py_netlist_serializer.def("serialize_to_file", netlist_serializer::serialize_to_file, py::arg("netlist"), py::arg("hal_file"), R"(
            Serializes a netlist into a ``.hal`` file.
        
            :param hal_py.Netlist netlist: The netlist to serialize.
            :param pathlib.Path hal_file: The path to the ``.hal`` file.
            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool
        )");

        py_netlist_serializer.def(
            "deserialize_from_file",
            [](const std::filesystem::path& hal_file, GateLibrary* gate_lib = nullptr) { return std::shared_ptr<Netlist>(netlist_serializer::deserialize_from_file(hal_file, gate_lib)); },
            py::arg("hal_file"),
            py::arg("gate_lib") = nullptr,
            R"(
                Deserializes a netlist from a ``.hal`` file using the provided gate library.
                If no gate library is provided, a gate library path must be specified within the ``.hal`` file.
            
                :param pathlib.Path hal_file: The path to the ``.hal`` file.
                :param hal_py.GateLibrary gate_lib: The gate library. Defaults to ``None``.
                :returns: The deserialized netlist on success, ``None`` otherwise.
                :rtype: hal_py.Netlist or None
            )");

        py_netlist_serializer.def(
            "deserialize_from_string",
            [](const std::string& hal_string, GateLibrary* gate_lib = nullptr) { return std::shared_ptr<Netlist>(netlist_serializer::deserialize_from_string(hal_string, gate_lib)); },
            py::arg("hal_string"),
            py::arg("gate_lib") = nullptr,
            R"(
                Deserializes a string which contains a netlist in HAL-(JSON)-format using the provided gate library.
                If no gate library is provided, a gate library path must be specified within the ``.hal`` file.

                :param pathlib.Path hal_file: The string containing the netlist in HAL-(JSON)-format.
                :param hal_py.GateLibrary gate_lib: The gate library. Defaults to ``None``.
                :returns: The deserialized netlist on success, ``None`` otherwise.
                :rtype: hal_py.Netlist or None
            )");
    }
}    // namespace hal
