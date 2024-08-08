#include "hal_core/python_bindings/python_bindings.h"

#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"
#include "sequential_symbolic_execution/plugin_sequential_symbolic_execution.h"
#include "sequential_symbolic_execution/sequential_symbolic_execution.h"
#include "z3_utils/simplification.h"

namespace py = pybind11;

namespace hal
{

    // the name in PYBIND11_MODULE/PYBIND11_PLUGIN *MUST* match the filename of the output library (without extension),
    // otherwise you will get "ImportError: dynamic module does not define module export function" when importing the module

#ifdef PYBIND11_MODULE
    PYBIND11_MODULE(sequential_symbolic_execution, m)
    {
        m.doc() = "hal SequentialSymbolicExecutionPlugin python bindings";
#else
    PYBIND11_PLUGIN(sequential_symbolic_execution)
    {
        py::module m("sequential_symbolic_execution", "hal SequentialSymbolicExecutionPlugin python bindings");
#endif    // ifdef PYBIND11_MODULE

        py::class_<SequentialSymbolicExecutionPlugin, RawPtrWrapper<SequentialSymbolicExecutionPlugin>, BasePluginInterface> py_sequential_symbolic_execution_plugin(
            m, "SequentialSymbolicExecutionPlugin", R"(This class provides an interface to integrate the HAWKEYE tool as a plugin within the HAL framework.)");

        py_sequential_symbolic_execution_plugin.def_property_readonly("name", &SequentialSymbolicExecutionPlugin::get_name, R"(
            The name of the plugin.

            :type: str
        )");

        py_sequential_symbolic_execution_plugin.def("get_name", &SequentialSymbolicExecutionPlugin::get_name, R"(
            Get the name of the plugin.

            :returns: The name of the plugin.
            :rtype: str
        )");

        py_sequential_symbolic_execution_plugin.def_property_readonly("version", &SequentialSymbolicExecutionPlugin::get_version, R"(
            The version of the plugin.

            :type: str
        )");

        py_sequential_symbolic_execution_plugin.def("get_version", &SequentialSymbolicExecutionPlugin::get_version, R"(
            Get the version of the plugin.

            :returns: The version of the plugin.
            :rtype: str
        )");

        py_sequential_symbolic_execution_plugin.def_property_readonly("description", &SequentialSymbolicExecutionPlugin::get_description, R"(
            The description of the plugin.

            :type: str
        )");

        py_sequential_symbolic_execution_plugin.def("get_description", &SequentialSymbolicExecutionPlugin::get_description, R"(
            Get the description of the plugin.

            :returns: The description of the plugin.
            :rtype: str
        )");

        py_sequential_symbolic_execution_plugin.def_property_readonly("dependencies", &SequentialSymbolicExecutionPlugin::get_dependencies, R"(
            A set of plugin names that this plugin depends on.

            :type: set[str]
        )");

        py_sequential_symbolic_execution_plugin.def("get_dependencies", &SequentialSymbolicExecutionPlugin::get_dependencies, R"(
            Get a set of plugin names that this plugin depends on.

            :returns: A set of plugin names that this plugin depends on.
            :rtype: set[str]
        )");

        m.def("get_pg_word_values_at_z3",
              [](const std::vector<std::pair<Module*, PinGroup<ModulePin>*>>& words,
                 const std::vector<u32>& time_indices,
                 const std::vector<bool>& subgraph_gates_byte_map,
                 const std::vector<std::map<u32, bool>>& known_inputs,
                 const std::map<PinGroup<ModulePin>*, BooleanFunction>& word_level_calculations,
                 const bool substitute_endpoints) {
                  z3::context ctx;

                  const auto word_values_res = sse::get_word_values_at_z3(words, time_indices, subgraph_gates_byte_map, known_inputs, word_level_calculations, substitute_endpoints, ctx);

                  if (word_values_res.is_error())
                  {
                      log_error("sequential_symbolic_execution", "{}", word_values_res.get_error().get());
                      return;
                  }

                  const auto word_values = word_values_res.get();

                  for (u32 idx = 0; idx < word_values.size(); idx++)
                  {
                      const auto& [mod, pg] = words.at(idx);
                      const auto& w         = word_values.at(idx);
                      const auto t          = time_indices.at(idx);

                      if (idx == 0 || t != time_indices.at(idx - 1))
                      {
                          std::cout << "T[" << t << "]" << std::endl;
                      }

                      //std::cout << "\tWORD:     " << w << std::endl;
                      std::cout << "\t" << mod->get_name() << " - " << pg->get_name() << ":  " << std::endl;
                      // std::cout << "\t\t" << z3_utils::to_bf(w).get().to_string() << std::endl;

                      const auto w_z3_simplified = z3_utils::simplify_local(w.simplify());
                      if (w_z3_simplified.is_error())
                      {
                          std::cout << w_z3_simplified.get_error().get() << std::endl;
                      }

                      const auto w_bfz_simplified = z3_utils::to_bf(w_z3_simplified.get());
                      if (w_bfz_simplified.is_error())
                      {
                          std::cout << w_bfz_simplified.get_error().get() << std::endl;
                      }
                      else
                      {
                          const std::string s = w_bfz_simplified.get().to_string();
                          std::cout << "\t\t" << s << std::endl;
                      }
                  }

                  return;
              });

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
