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

        py_data_container.def("has_data", &DataContainer::has_data, py::arg("category"), py::arg("key"), R"(
            Determine whether an entry of given category and key exists.

            :param str category: The data key category.
            :param str key: The data key.
            :returns: True if the entry exists, False otherwise.
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

        py_data_container.def("set_parameter", [](DataContainer& self, const Parameter& param, const std::string& value) -> bool {
                auto res = self.set_parameter(param, value);
                if (res.is_ok())
                {
                    return true;
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return false;
                }
            }, py::arg("param"), py::arg("value"), R"(
            Set (or overwrite) the value of a typed parameter as a string. The value is validated, normalized, and stored; any existing entry with the same name is replaced.

            :param hal_py.Parameter param: The parameter declaration.
            :param str value: The value to store as a string.
            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool
        )");

        py_data_container.def("get_parameter_value", [](DataContainer& self, const std::string& name) -> std::optional<std::string> {
                auto res = self.get_parameter_value(name);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return std::nullopt;
                }
            }, py::arg("name"), R"(
            Get the value of a typed parameter. Returns the explicitly-stored value or ``None`` if the parameter does not exist.

            :param str name: The parameter name.
            :returns: The value string on success, ``None`` otherwise.
            :rtype: str or None
        )");

        py_data_container.def("get_parameter_value", [](DataContainer& self, const Parameter& param) -> std::optional<std::string> {
                auto res = self.get_parameter_value(param);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return std::nullopt;
                }
            }, py::arg("name"), R"(
            Get the value of a typed parameter. Returns the explicitly-stored value or ``None`` if the parameter does not exist.

            :param hal_py.Parameter param: The parameter.
            :returns: The value string on success, ``None`` otherwise.
            :rtype: str or None
        )");

        py_data_container.def("get_parameter_declaration", [](DataContainer& self, const std::string& name) -> std::optional<Parameter> {
                auto res = self.get_parameter_declaration(name);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return std::nullopt;
                }
            }, py::arg("name"), R"(
            Get the full declaration of a typed parameter.

            :param str name: The parameter name.
            :returns: The parameter declaration on success, ``None`` otherwise.
            :rtype: hal_py.Parameter or None
        )");

        py_data_container.def("has_parameter", py::overload_cast<const std::string&>(&DataContainer::has_parameter, py::const_), py::arg("name"), R"(
            Check whether a parameter with the given name exists and has an explicitly stored value.

            :param str name: The parameter name.
            :returns: ``True`` if the parameter is explicitly set, ``False`` otherwise.
            :rtype: bool
        )");

        py_data_container.def("has_parameter", py::overload_cast<const Parameter&>(&DataContainer::has_parameter, py::const_), py::arg("param"), R"(
            Check whether a parameter exists and has an explicitly stored value.

            :param hal_py.Parameter param: The parameter.
            :returns: ``True`` if the parameter is explicitly set, ``False`` otherwise.
            :rtype: bool
        )");

        py_data_container.def("delete_parameter", &DataContainer::delete_parameter, py::arg("name"), R"(
            Delete an explicitly stored parameter value, if any.

            :param str name: The parameter name.
            :returns: ``True`` if a value was deleted, ``False`` otherwise.
            :rtype: bool
        )");

        py_data_container.def_property_readonly("parameters", &DataContainer::get_parameters, R"(
            All explicitly stored parameter values as a dict from ``name`` to `(declaration, value)`.

            :type: dict[str,tuple(hal_py.Parameter,str)]
        )");

        py_data_container.def("get_parameters", &DataContainer::get_parameters, R"(
            Get all explicitly stored parameter values as a dict from ``name`` to `(declaration, value)`.

            :returns: The parameter dict.
            :rtype: dict[str,tuple(hal_py.Parameter,str)]
        )");
    }
}    // namespace hal
