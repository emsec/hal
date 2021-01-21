#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void gate_library_manager_init(py::module& m)
    {
        auto py_gate_library_manager = m.def_submodule("GateLibraryManager", R"(
            The gate library manager keeps track of all gate libraries that are used within HAL. Further, it takes care of loading and saving gate libraries on demnand.
        )");

        py_gate_library_manager.def(
            "load",
            [](std::filesystem::path file_path, bool reload) { return RawPtrWrapper<GateLibrary>(gate_library_manager::load(file_path, reload)); },
            py::arg("file_path"),
            py::arg("reload") = false,
            R"(
            Load a gate library from file.

            :param hal_py.hal_path file_path: The input path.
            :param bool reload: If true, reloads the library in case it is already loaded.
            :returns: The gate library on success, None otherwise.
            :rtype: hal_py.GateLibrary or None
        )");

        py_gate_library_manager.def("load_all", &gate_library_manager::load_all, py::arg("reload") = false, R"(
            Load all gate libraries available in standard gate library directories.

            :param bool reload: If true, reloads all libraries that have already been loaded.
        )");

        py_gate_library_manager.def("save", &gate_library_manager::save, py::arg("file_path"), py::arg("gate_lib"), py::arg("overwrite") = false, R"(
            Save a gate library to file.

            :param hal_py.hal_path file_path: The output path. 
            :param hal_py.GateLibrary gate_lib: The gate library.
            :param bool overwrite: If true, overwrites already existing files.
            :returns: True on success, false otherwise.
            :rtype: bool
        )");

        py_gate_library_manager.def(
            "get_gate_library", [](const std::string& file_name) { return RawPtrWrapper<GateLibrary>(gate_library_manager::get_gate_library(file_name)); }, py::arg("file_path"), R"(
            Get a gate library by file path. If no library with the given name is loaded, loading the gate library from file will be attempted.

            :param str file_path: The input path.
            :returns: The gate library on success, None otherwise.
            :rtype: hal_py.GateLibrary or None
        )");

        py_gate_library_manager.def(
            "get_gate_library_by_name",
            [](const std::string& lib_name) { return RawPtrWrapper<GateLibrary>(gate_library_manager::get_gate_library_by_name(lib_name)); },
            py::arg("lib_name"),
            R"(
            Get a gate library by name. If no library with the given name is loaded, None will be returned.

            :param str lib_name: The name of the gate library.
            :returns: The gate library on success, None otherwise.
            :rtype: hal_py.GateLibrary or None
        )");

        py_gate_library_manager.def(
            "get_gate_libraries",
            [] {
                std::vector<RawPtrWrapper<GateLibrary>> result;
                for (auto lib : gate_library_manager::get_gate_libraries())
                {
                    result.emplace_back(lib);
                }
                return result;
            },
            R"(
            Get all loaded gate libraries.

            :returns: A list of gate libraries.
            :rtype:  list[hal_py.GateLibrary]
        )");
    }
}    // namespace hal
