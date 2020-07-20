#include "bindings.h"

namespace hal
{
    void core_utils_init(py::module& m)
    {
        auto py_core_utils = m.def_submodule("CoreUtils", R"(
            HAL Core Utility functions.
    )");

        py_core_utils.def("get_binary_directory", &core_utils::get_binary_directory, R"(
            Get the path to the executable of HAL.

            :returns: The path.
            :rtype: hal_py.hal_path
    )");

        py_core_utils.def("get_base_directory", &core_utils::get_base_directory, R"(
            Get the base path to the HAL installation.
            1. Use Environment Variable HAL_BASE_PATH
            2. If current executable is hal (not e.g. python3 interpreter) use it's path to determine base path.
            3. Try to find hal executable in path and use its base path.

            :returns: The path.
            :rtype: hal_py.hal_path
    )");

        py_core_utils.def("get_library_directory", &core_utils::get_library_directory, R"(
            Get the path to the shared and static libraries of HAL.
            Relative to the binary directory.

            :returns: The path.
            :rtype: hal_py.hal_path
    )");

        py_core_utils.def("get_share_directory", &core_utils::get_share_directory, R"(
            Get the path to the sh
            Relative to the binary

            :returns: The path.
            :rtype: hal_py.hal_path
    )");

        py_core_utils.def("get_user_share_directory", &core_utils::get_user_share_directory, R"(
            Get the path to shared objects and files provided by the user.
            home/.local/share for Unix

            :returns: The path.
            :rtype: hal_py.hal_path
    )");

        py_core_utils.def("get_user_config_directory", &core_utils::get_user_config_directory, R"(
            Get the path to the configuration directory of the user.
            home/.config/hal for Unix

            :returns: The path.
            :rtype: hal_py.hal_path
    )");

        py_core_utils.def("get_default_log_directory", &core_utils::get_default_log_directory, py::arg("source_file") = "", R"(
            Get the path to the default directory for log files.
            If an hdl source file is provided, the function returns the parent directory, otherwise get_user_share_directory() / "log".

            :param source_file: The hdl source file.
            :type source_file: hal_py.hal_path
            :returns: The path.
            :rtype: hal_py.hal_path
    )");

        py_core_utils.def("get_gate_library_directories", &core_utils::get_gate_library_directories, R"(
            Get the paths where gate libraries are searched.
            Contains the share and user share directories.

            :returns: A list of paths.
            :rtype: list[hal_py.hal_path]
    )");

        py_core_utils.def("get_plugin_directories", &core_utils::get_plugin_directories, R"(
            Get the paths where plugins are searched.
            Contains the library and user share directories.

            :returns: A vector of paths.
    )");
    }
}    // namespace hal
