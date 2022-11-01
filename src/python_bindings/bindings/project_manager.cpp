#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void project_manager_init(py::module& m)
    {
        py::class_<ProjectManager, RawPtrWrapper<ProjectManager>> py_project_manager(m, "ProjectManager", R"(
            Project manager class that handles opening, closing, and saving of projects.
        )");

        py::enum_<ProjectManager::ProjectStatus> py_project_status(py_project_manager, "ProjectStatus", R"(
            Represents the logic value that a Boolean function operates on.
        )");

        py_project_status.value("NONE", ProjectManager::ProjectStatus::NONE, R"(Represents the default state.)")
            .value("OPENED", ProjectManager::ProjectStatus::OPENED, R"(Represents an open project state.)")
            .value("SAVED", ProjectManager::ProjectStatus::SAVED, R"(Represents a saved project state.)")
            .export_values();

        py_project_manager.def_static("instance", &ProjectManager::instance, R"(
            Returns the singleton instance which gets constructed upon first call.

            :returns: The singleton instance.
            :rtype: hal_py.ProjectManager
        )");

        // TODO register_serializer

        // TODO unregister_serializer

        py_project_manager.def("get_project_status", &ProjectManager::get_project_status, R"(
            Returns the current project status.

            :returns: The project status value.
            :rtype: hal_py.ProjectManager.ProjectStatus
        )");

        py_project_manager.def("set_project_status", &ProjectManager::set_project_status, py::arg("status"), R"(
            Set the current project status to a new value.
            Must be called when a project is closed.

            :param hal_py.ProjectManager.ProjectStatus status: The new project status value.
        )");

        py_project_manager.def("get_filename", &ProjectManager::get_filename, py::arg("serializer_name"), R"(
            Returns the relative path of the file to be parsed by an external serializer.

            :param str serializer_name: The unique name of the serializer.
            :returns: The relative file path.
            :rtype: str
        )");

        py_project_manager.def("set_gate_library_path", &ProjectManager::set_gate_library_path, py::arg("gl_path"), R"(
            Set the path to the gate library file.

            :param str gl_path: The path to the gate library file.
        )");

        py_project_manager.def("serialize_project", &ProjectManager::serialize_project, py::arg("netlist"), py::arg("shadow") = false, R"(
            Serialize the netlist and all dependent data to the project directory.

            :param hal_py.Netlist netlist: The netlist.
            :param bool shadow: Set to True if function is called from autosave procedure, False otherwise. Defaults to False.
            :returns: True if serialization of the netlist was successful, False otherwise.
            :rtype: bool
        )");

        py_project_manager.def("open_project", &ProjectManager::open_project, py::arg("path") = std::string(), R"(
            Open the project specified by the provided directory path.

            :param str path: The path to the project directory. Can be omitted if the path was previously set using `ProjectManager::set_project_directory`.
            :returns: True on success, False otherwise.
            :rtype: bool
        )");

        // TODO get_project_directory

        py_project_manager.def("set_project_directory", &ProjectManager::set_project_directory, py::arg("path"), R"(
            Set path to the project directory.

            :param str path: The path to the project directory.
        )");

        // TODO restore_project_file_from_autosave

        py_project_manager.def("create_project_directory", &ProjectManager::create_project_directory, py::arg("path"), R"(
            Create an empty project directory at the specified location. 
            The project directory must not exist.

            :param str path: The path to the new project directory.
            :returns: True on success, False otherwise.
            :rtype: bool
        )");

        py_project_manager.def("remove_project_directory", &ProjectManager::remove_project_directory, R"(
            Remove the existing project directory and clear the path member variable.

            :returns: True on success, False otherwise.
            :rtype: bool
        )");

        py_project_manager.def("get_netlist_filename", &ProjectManager::get_netlist_filename, R"(
            Returns the path to the netlist file.

            :returns: The netlist file path.
            :rtype: str
        )");
    }
}    // namespace hal