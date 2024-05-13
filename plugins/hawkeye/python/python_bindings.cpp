#include "hal_core/python_bindings/python_bindings.h"

#include "hawkeye/plugin_hawkeye.h"
#include "hawkeye/sbox_database.h"
#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

namespace py = pybind11;

namespace hal
{

    // the name in PYBIND11_MODULE/PYBIND11_PLUGIN *MUST* match the filename of the output library (without extension),
    // otherwise you will get "ImportError: dynamic module does not define module export function" when importing the module

#ifdef PYBIND11_MODULE
    PYBIND11_MODULE(hawkeye, m)
    {
        m.doc() = "hal HawkeyePlugin python bindings";
#else
    PYBIND11_PLUGIN(hawkeye)
    {
        py::module m("hawkeye", "hal HawkeyePlugin python bindings");
#endif    // ifdef PYBIND11_MODULE

        py::class_<HawkeyePlugin, RawPtrWrapper<HawkeyePlugin>, BasePluginInterface> py_hawkeye_plugin(m, "HawkeyePlugin");
        py_hawkeye_plugin.def_property_readonly("name", &HawkeyePlugin::get_name, R"(
            The name of the plugin.

            :type: str
        )");

        py_hawkeye_plugin.def("get_name", &HawkeyePlugin::get_name, R"(
            Get the name of the plugin.

            :returns: Plugin name.
            :rtype: str
        )");

        py_hawkeye_plugin.def_property_readonly("description", &HawkeyePlugin::get_description, R"(
            The description of the plugin.

            :type: str
        )");

        py_hawkeye_plugin.def("get_description", &HawkeyePlugin::get_description, R"(
            Get the description of the plugin.

            :returns: The description of the plugin.
            :rtype: str
        )");

        py_hawkeye_plugin.def_property_readonly("version", &HawkeyePlugin::get_version, R"(
            The version of the plugin.

            :type: str
        )");

        py_hawkeye_plugin.def("get_version", &HawkeyePlugin::get_version, R"(
            Get the version of the plugin.

            :returns: Plugin version.
            :rtype: str
        )");

        py::class_<hawkeye::SBoxDatabase, RawPtrWrapper<hawkeye::SBoxDatabase>> py_hawkeye_sbox_database(m, "SBoxDatabase", R"(
            Database of known S-boxes.
        )");

        py_hawkeye_sbox_database.def(py::init<>(), R"(
            Constructs an empty S-box database.
        )");

        py_hawkeye_sbox_database.def(py::init<const std::map<std::string, std::vector<u8>>&>(), py::arg("sboxes"), R"(
            Constructs an S-box database from the given S-boxes.

            :param dict[str,list[int]] sboxes: A dict from S-box name to the respective S-box.
        )");

        py_hawkeye_sbox_database.def_static(
            "from_file",
            [](const std::filesystem::path& file_path) -> std::optional<hawkeye::SBoxDatabase> {
                auto res = hawkeye::SBoxDatabase::from_file(file_path);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("file_path"),
            R"(
            Constructs an S-box database from file.

            :param pathlib.Path file_path: The path from which to load the S-box database file.
            :returns: The S-box database on success, an error otherwise.
            :rtype: hawkeye.SBoxDatabase or None
        )");

        py_hawkeye_sbox_database.def_static("compute_linear_representative", &hawkeye::SBoxDatabase::compute_linear_representative, py::arg("sbox"), R"(
            Compute the linear representative of the given S-box.

            :param list[int] sbox: The S-box.
            :returns: The linear representative.
            :rtype: list[int]
        )");

        py_hawkeye_sbox_database.def(
            "add",
            [](hawkeye::SBoxDatabase& self, const std::string& name, const std::vector<u8>& sbox) -> bool {
                auto res = self.add(name, sbox);
                if (res.is_ok())
                {
                    return true;
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return false;
                }
            },
            py::arg("name"),
            py::arg("sbox"),
            R"(
            Add multiple S-boxes to the database.

            :param str name: The name of the S-box.
            :patam list[int] sbox: The S-box.
            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool
        )");

        py_hawkeye_sbox_database.def(
            "add",
            [](hawkeye::SBoxDatabase& self, const std::map<std::string, std::vector<u8>>& sboxes) -> bool {
                auto res = self.add(sboxes);
                if (res.is_ok())
                {
                    return true;
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return false;
                }
            },
            py::arg("sboxes"),
            R"(
            Add multiple S-boxes to the database.

            :param dict[str,list[int]] sboxes: A dict from S-box name to the respective S-box.
            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool
        )");

        py_hawkeye_sbox_database.def(
            "load",
            [](hawkeye::SBoxDatabase& self, const std::filesystem::path& file_path, bool overwrite = true) -> bool {
                auto res = self.load(file_path, overwrite);
                if (res.is_ok())
                {
                    return true;
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return false;
                }
            },
            py::arg("file_path"),
            py::arg("overwrite") = true,
            R"(
            Load S-boxes to the database from a file.

            :param pathlib.Path file_path: The path from which to load the S-box database file.
            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool
        )");

        py_hawkeye_sbox_database.def(
            "store",
            [](const hawkeye::SBoxDatabase& self, const std::filesystem::path& file_path) -> bool {
                auto res = self.store(file_path);
                if (res.is_ok())
                {
                    return true;
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return false;
                }
            },
            py::arg("file_path"),
            R"(
            Store the S-box database to a database file.

            :param pathlib.Path file_path: The path to where to store the S-box database file.
            :returns: ``True`` on success, ``False`` otherwise.
            :rtype: bool
        )");

        py_hawkeye_sbox_database.def(
            "lookup",
            [](const hawkeye::SBoxDatabase& self, const std::vector<u8>& sbox) -> std::optional<std::string> {
                auto res = self.lookup(sbox);
                if (res.is_ok())
                {
                    return res.get();
                }
                else
                {
                    log_error("python_context", "{}", res.get_error().get());
                    return std::nullopt;
                }
            },
            py::arg("sbox"),
            R"(
            Attempt to look up an S-box in the database.

            :param list[int] sbox: The S-box to look for.
            :returns: The S-box name on success, ``None`` otherwise.
            :rtype: str or None
        )");

        py_hawkeye_sbox_database.def("print", &hawkeye::SBoxDatabase::print, R"(
            Print the database.
        )");

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
