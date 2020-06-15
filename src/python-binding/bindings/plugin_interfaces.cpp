#include "bindings.h"

namespace hal
{
    class Pyi_base : public i_base
    {
    public:
        using i_base::i_base;

        std::string get_name() const override
        {
            PYBIND11_OVERLOAD_PURE(std::string, /* Return type */
                                   i_base,      /* Parent class */
                                   get_name,
                                   NULL); /* Name of function in C++ (must match Python name) */
        }

        std::string get_version() const override
        {
            PYBIND11_OVERLOAD_PURE(std::string, /* Return type */
                                   i_base,      /* Parent class */
                                   get_version,
                                   NULL); /* Name of function in C++ (must match Python name) */
        }
    };

    class Pyi_gui : public i_gui
    {
    public:
        using i_gui::i_gui;

        bool exec(std::shared_ptr<netlist> g) override
        {
            PYBIND11_OVERLOAD_PURE(bool,  /* Return type */
                                   i_gui, /* Parent class */
                                   exec,  /* Name of function in C++ (must match Python name) */
                                   g);
        }
    };

    void plugin_interfaces_init(py::module& m)
    {
        py::class_<i_base, std::shared_ptr<i_base>, Pyi_base> py_i_base(m, "i_base");

        py_i_base.def_property_readonly("name", &i_base::get_name, R"(
        The name of the plugin.

        :type: str
)");

        py_i_base.def("get_name", &i_base::get_name, R"(
        Get the name of the plugin.

        :returns: Plugin name.
        :rtype: str
)");

        py_i_base.def_property_readonly("version", &i_base::get_version, R"(
        The version of the plugin.

        :type: str
)");

        py_i_base.def("get_version", &i_base::get_version, R"(
        Get the version of the plugin.

        :returns: Plugin version.
        :rtype: str
)");

        py::class_<i_gui, std::shared_ptr<i_gui>, Pyi_gui> py_i_gui(m, "i_gui");

        py_i_gui.def("exec", &i_gui::exec, py::arg("netlist"), R"(
        Generic call to run the GUI.

        :param netlist: The netlist object for the GUI.
        :type netlist: hal_py.netlist
        :returns: True on success.
        :rtype: bool
)");
    }
}    // namespace hal