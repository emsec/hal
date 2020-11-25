#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void gate_library_manager_init(py::module& m)
    {
        auto py_gate_library_manager = m.def_submodule("GateLibraryManager", R"(
            HAL GateLibraryManager functions.
        )");

        py_gate_library_manager.def(
            "load_file",
            [](std::filesystem::path path, bool reload_if_existing) { return RawPtrWrapper<GateLibrary>(gate_library_manager::load_file(path, reload_if_existing)); },
            py::arg("path"),
            py::arg("reload_if_existing"),
            R"(
            Loads a gate library file.

            :param path - the file to load.
            :param reload_if_existing - If true, reloads all libraries that are already loaded.
            :returns: Loaded gate library or nullptr on error.
            :rtype: hal_py.GateLibrary
        )");

        py_gate_library_manager.def("load_all", &gate_library_manager::load_all, py::arg("reload_if_existing"), R"(
            Loads all gate libraries which are available.

            :param reload_if_existing - If true, reloads all libraries that are already loaded.
        )");

        py_gate_library_manager.def(
            "get_gate_library", [](const std::string& file_name) { return RawPtrWrapper<GateLibrary>(gate_library_manager::get_gate_library(file_name)); }, py::arg("file_name"), R"(
            If no library with the given name is already loaded this function will attempt to load the file.

            :param file_name - file name of the gate library.
            :returns: Gate library object or nullptr on error.
            :rtype: hal_py.GateLibrary
        )");

        py_gate_library_manager.def(
            "get_gate_library_by_name", [](const std::string& lib_name) { return RawPtrWrapper<GateLibrary>(gate_library_manager::get_gate_library_by_name(lib_name)); }, py::arg("lib_name"), R"(
            Get a gate library object by library name.
            If no library with the given name is loaded this function returns nullptr.

            :param lib_name - name of the gate library.
            :returns: Gate library object or nullptr on error.
            :rtype: hal_py.GateLibrary
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

            :param lib_name - name of the gate library.
            :returns: A vector of pointers to the gate library objects.
            :rtype:  list[hal_py.GateLibrary]
        )");
    }
}    // namespace hal
