#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{

    class PyBasePluginInterface : public BasePluginInterface
    {
    public:
        using BasePluginInterface::BasePluginInterface;

        std::string get_name() const override
        {
            PYBIND11_OVERRIDE_PURE(std::string,         /* Return type */
                                   BasePluginInterface, /* Parent class */
                                   get_name);           /* Name of function in C++ (must match Python name) */
        }

        std::string get_version() const override
        {
            PYBIND11_OVERRIDE_PURE(std::string,         /* Return type */
                                   BasePluginInterface, /* Parent class */
                                   get_version);        /* Name of function in C++ (must match Python name) */
        }

        std::string get_description() const override
        {
            PYBIND11_OVERRIDE(std::string,         /* Return type */
                              BasePluginInterface, /* Parent class */
                              get_description);    /* Name of function in C++ (must match Python name) */
        }

        std::set<std::string> get_dependencies() const override
        {
            PYBIND11_OVERRIDE(std::set<std::string>, /* Return type */
                              BasePluginInterface,   /* Parent class */
                              get_dependencies);     /* Name of function in C++ (must match Python name) */
        }
    };

    void plugin_interfaces_init(py::module& m)
    {
        py::class_<BasePluginInterface, RawPtrWrapper<BasePluginInterface>, PyBasePluginInterface> py_base_plugin_interface(m, "BasePluginInterface");

        py_base_plugin_interface.def_property_readonly("name", &BasePluginInterface::get_name, R"(
            The name of the plugin.

            :type: str
        )");

        py_base_plugin_interface.def("get_name", &BasePluginInterface::get_name, R"(
            Get the name of the plugin.

            :returns: The name of the plugin.
            :rtype: str
        )");

        py_base_plugin_interface.def_property_readonly("version", &BasePluginInterface::get_version, R"(
            The version of the plugin.

            :type: str
        )");

        py_base_plugin_interface.def("get_version", &BasePluginInterface::get_version, R"(
            Get the version of the plugin.

            :returns: The version of the plugin.
            :rtype: str
        )");

        py_base_plugin_interface.def_property_readonly("description", &BasePluginInterface::get_description, R"(
            The description of the plugin.

            :type: str
        )");

        py_base_plugin_interface.def("get_description", &BasePluginInterface::get_description, R"(
            Get the description of the plugin.

            :returns: The description of the plugin.
            :rtype: str
        )");

        py_base_plugin_interface.def_property_readonly("dependencies", &BasePluginInterface::get_dependencies, R"(
            The plugins that this plugin depends on.

            :type: set[str]
        )");

        py_base_plugin_interface.def("get_dependencies", &BasePluginInterface::get_dependencies, R"(
            Get all dependencies of this plugin.

            :returns: The plugins that this plugin depends on.
            :rtype: set[str]
        )");
    }
}    // namespace hal
