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

        py_netlist_serializer.def("deserialize_from_file",
                                  [](const std::filesystem::path& hal_file) {
                                  return std::shared_ptr<Netlist>(netlist_serializer::deserialize_from_file(hal_file,nullptr));
                                  },
            py::arg("hal_file"), R"(
            Deserializes a netlist from a .hal file.
        
            :param pathlib.Path hal_file: The source .hal file.
            :returns: The deserialized netlist.
            :rtype: hal_py.Netlist
        )");
    }
}    // namespace hal
