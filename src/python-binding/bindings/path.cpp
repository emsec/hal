#include "bindings.h"

namespace hal
{
    void path_init(py::module& m)
    {
        py::class_<std::filesystem::path> py_path(m, "HalPath");

        py_path.def(py::init<>());

        py_path.def(py::init<const std::filesystem::path&>());

        py_path.def(py::init<const std::string&>()).def("__str__", [](std::filesystem::path& p) -> std::string { return std::string(p.c_str()); });

        py::implicitly_convertible<std::string, std::filesystem::path>();
    }
}    // namespace hal
