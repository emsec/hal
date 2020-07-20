#include "bindings.h"

namespace hal
{
    void data_container_init(py::module& m)
    {
        py::class_<DataContainer, std::shared_ptr<DataContainer>> py_data_container(m, "DataContainer");

        py_data_container.def(py::init<>(), R"(
        Construct a new data container.
)");

        py_data_container.def("set_data", &DataContainer::set_data, py::arg("category"), py::arg("key"), py::arg("value_data_type"), py::arg("value"), py::arg("log_with_info_level") = false, R"(
        Sets a custom data entry
        If it does not exist yet, it is added.

        :param str category: Key category
        :param str key: Data key
        :param str data_type: Data type of value
        :param str value: Data value
        :param bool log_with_info_level: Force explicit logging channel 'netlist' with log level info to trace GUI events (default = false)
        :returns: True on success.
        :rtype: bool
)");

        py_data_container.def("delete_data", &DataContainer::delete_data, py::arg("category"), py::arg("key"), py::arg("log_with_info_level") = false, R"(
        Deletes custom data.

        :param str category: Category of key
        :param str key: Data key
        :param bool log_with_info_level: Force explicit logging channel 'netlist' with log level info to trace GUI events (default = false)
        :returns: True on success.
        :rtype: bool
)");

        py_data_container.def_property_readonly("data", &DataContainer::get_data, R"(
        A dict from ((1) category, (2) key) to ((1) type, (2) value) containing the stored data.

        :type: dict[tuple(str,str),tuple(str,str)]
)");

        py_data_container.def("get_data_by_key", &DataContainer::get_data_by_key, py::arg("category"), py::arg("key"), R"(
        Gets data specified by key and category

        :param str category: Category of key
        :param str key: Data key
        :returns: The tuple ((1) type, (2) value)
        :rtype: tuple(str,str)
)");

        py_data_container.def_property_readonly("data_keys", &DataContainer::get_data_keys, R"(
        A list of tuples ((1) category, (2) key) containing all the data keys.

        :type: list[tuple(str,str)]
)");
    }
}    // namespace hal
