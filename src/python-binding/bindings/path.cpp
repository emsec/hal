#include "bindings.h"

void path_init(py::module& m)
{
    py::class_<hal::path> py_path(m, "hal_path");

    py_path.def(py::init<>());

    py_path.def(py::init<const hal::path&>());

    py_path.def(py::init<const std::string&>()).def("__str__", [](hal::path& p) -> std::string { return std::string(p.c_str()); });

    py::implicitly_convertible<std::string, hal::path>();
}