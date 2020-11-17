#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void gate_library_manager_init(py::module& m)
    {
        auto py_gate_library_manager = m.def_submodule("GateLibraryManager", R"(
            HAL GateLibraryManager functions.
        )");

        py_gate_library_manager.def("load_file", &gate_library_manager::load_file, py::arg("path"), py::arg("reload_if_existing"), R"(
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

        py_gate_library_manager.def("get_gate_library", &gate_library_manager::get_gate_library, py::arg("file_name"), R"(
            If no library with the given name is already loaded this function will attempt to load the file.

            :param file_name - file name of the gate library.
            :returns: Gate library object or nullptr on error.
            :rtype: hal_py.GateLibrary
        )");

        py_gate_library_manager.def("get_gate_library_by_name", &gate_library_manager::get_gate_library_by_name, py::arg("lib_name"), R"(
            Get a gate library object by library name.
            If no library with the given name is loaded this function returns nullptr.

            :param lib_name - name of the gate library.
            :returns: Gate library object or nullptr on error.
            :rtype: hal_py.GateLibrary
        )");


        py_gate_library_manager.def("get_gate_libraries", &gate_library_manager::get_gate_libraries, R"(
            Get all loaded gate libraries.

            :param lib_name - name of the gate library.
            :returns: A vector of pointers to the gate library objects.
            :rtype:  set[hal_py.GateLibrary]
        )");


    }
}    // namespace hal
