#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{

    class Pyi_base : public BasePluginInterface
    {
    public:
        using BasePluginInterface::BasePluginInterface;

        std::string get_name() const override
        {
            PYBIND11_OVERLOAD_PURE(std::string,         /* Return type */
                                   BasePluginInterface, /* Parent class */
                                   get_name,
                                   NULL); /* Name of function in C++ (must match Python name) */
        }

        std::string get_version() const override
        {
            PYBIND11_OVERLOAD_PURE(std::string,         /* Return type */
                                   BasePluginInterface, /* Parent class */
                                   get_version,
                                   NULL); /* Name of function in C++ (must match Python name) */
        }
    };

    void plugin_interfaces_init(py::module& m)
    {
        py::class_<BasePluginInterface, RawPtrWrapper<BasePluginInterface>, Pyi_base> py_i_base(m, "BasePluginInterface");

        py_i_base.def_property_readonly("name", &BasePluginInterface::get_name, R"(
        The name of the plugin.

        :type: str
)");

        py_i_base.def("get_name", &BasePluginInterface::get_name, R"(
        Get the name of the plugin.

        :returns: Plugin name.
        :rtype: str
)");

        py_i_base.def_property_readonly("version", &BasePluginInterface::get_version, R"(
        The version of the plugin.

        :type: str
)");

        py_i_base.def("get_version", &BasePluginInterface::get_version, R"(
        Get the version of the plugin.

        :returns: Plugin version.
        :rtype: str
)");


    }
}    // namespace hal
