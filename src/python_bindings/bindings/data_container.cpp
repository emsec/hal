#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void data_container_init(py::module& m)
    {
        py::class_<DataContainer, RawPtrWrapper<DataContainer>> py_data_container(m, "DataContainer", R"(
            Container to hold data that is associated with an entity.
        )");

        py_data_container.def(py::init<>(), R"(
            Construct a new data container.
        )");

        py_data_container.def("set_data", &DataContainer::set_data, py::arg("category"), py::arg("key"), py::arg("data_type"), py::arg("value"), py::arg("log_with_info_level") = false, R"(
            Add a data entry.<br>
            May overwrite an existing entry.

            :param str category: The data key category.
            :param str key: The data key.
            :param str data_type: The data type.
            :param str value: The data value.
            :param bool log_with_info_level: Force explicit logging channel 'netlist' with log level 'info' to trace GUI events.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_data_container.def("get_data", &DataContainer::get_data, py::arg("category"), py::arg("key"), R"(
            Get the data entry specified by the given category and key.

            :param str category: The data key category.
            :param str key: The data key.
            :returns: The tuple ((1) type, (2) value).
            :rtype: tuple(str,str)
        )");

        py_data_container.def("delete_data", &DataContainer::delete_data, py::arg("category"), py::arg("key"), py::arg("log_with_info_level") = false, R"(
            Delete a data entry.

            :param str category: The data key category.
            :param str key: The data key.
            :param bool log_with_info_level: Force explicit logging channel 'netlist' with log level 'info' to trace GUI events.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_data_container.def_property_readonly("data", &DataContainer::get_data_map, R"(
            A dict from ((1) category, (2) key) to ((1) type, (2) value) containing all stored data entries.

            :type: dict[tuple(str,str),tuple(str,str)]
        )");

        py_data_container.def("get_data_map", &DataContainer::get_data_map, R"(
            Get a dict from ((1) category, (2) key) to ((1) type, (2) value) containing all stored data entries.

            :returns: The stored data as a dict.
            :rtype: dict[tuple(str,str),tuple(str,str)]
        )");
    }
}    // namespace hal
