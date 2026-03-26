# Machine Learning Python Bindings Refactor Direction

## Why this file should be split

`python_bindings.cpp` currently has 3263 lines, which makes it the largest Python binding file in the repository by a wide margin. The next-largest binding file is `plugins/graph_algorithm/python/python_bindings.cpp` at 1088 lines.

The current file already contains clear domain boundaries:

- module and plugin registration
- enums and shared types
- `Context`
- `NetlistGraph`
- free functions
- `gate_feature`
- `gate_pair_feature`
- `gate_label`
- `gate_pair_label`
- `subgraph_label`
- `edge_feature`

That means the file is already logically modular, but physically monolithic.

## Recommended target structure

Keep a single entry point that owns the `PYBIND11_MODULE` macro and split the actual registrations into small domain-specific translation units.

Suggested layout:

- `plugins/machine_learning/python/module.cpp`
- `plugins/machine_learning/python/bindings/register.h`
- `plugins/machine_learning/python/bindings/shared.h`
- `plugins/machine_learning/python/bindings/plugin.cpp`
- `plugins/machine_learning/python/bindings/types.cpp`
- `plugins/machine_learning/python/bindings/context.cpp`
- `plugins/machine_learning/python/bindings/netlist_graph.cpp`
- `plugins/machine_learning/python/bindings/functions.cpp`
- `plugins/machine_learning/python/bindings/gate_feature.cpp`
- `plugins/machine_learning/python/bindings/gate_pair_feature.cpp`
- `plugins/machine_learning/python/bindings/gate_label.cpp`
- `plugins/machine_learning/python/bindings/gate_pair_label.cpp`
- `plugins/machine_learning/python/bindings/subgraph_label.cpp`
- `plugins/machine_learning/python/bindings/edge_feature.cpp`

## Responsibilities per file

`module.cpp`

- defines `PYBIND11_MODULE(machine_learning, m)`
- sets the top-level module docstring
- creates submodules such as `gate_feature` and `gate_pair_label`
- calls the registration functions in a stable order

`register.h`

- declares one `bind_*` function per binding area
- exposes a minimal interface such as:

```cpp
namespace hal::machine_learning::python
{
    void bind_plugin(pybind11::module_& m);
    void bind_types(pybind11::module_& m);
    void bind_context(pybind11::module_& m);
    void bind_netlist_graph(pybind11::module_& m);
    void bind_functions(pybind11::module_& m);
    void bind_gate_features(pybind11::module_& m, pybind11::module_& gate_feature);
    void bind_gate_pair_features(pybind11::module_& m, pybind11::module_& gate_pair_feature);
    void bind_gate_labels(pybind11::module_& m, pybind11::module_& gate_label);
    void bind_gate_pair_labels(pybind11::module_& m, pybind11::module_& gate_pair_label);
    void bind_subgraph_labels(pybind11::module_& m, pybind11::module_& subgraph_label);
    void bind_edge_features(pybind11::module_& m, pybind11::module_& edge_feature);
}
```

`shared.h`

- centralizes common includes
- defines `namespace py = pybind11;`
- can hold tiny binding-only helpers used in multiple files
- should not accumulate full binding implementations

## Split order

The safest incremental refactor is:

1. Extract `module.cpp`, `register.h`, and `shared.h`.
2. Move plugin metadata, enums, and plain structs into `plugin.cpp` and `types.cpp`.
3. Move `Context`, `NetlistGraph`, and free functions into separate files.
4. Move each feature or label family into its own file.
5. Build and run the existing machine learning Python scripts after each step.

This sequence keeps risk low because the module boundary and public Python API stay stable while the implementation moves behind registration functions.

## Design rules for the split

- Only `module.cpp` should own the `PYBIND11_MODULE` macro.
- Each binding file should register one coherent domain.
- Prefer one base class and its derived classes in the same file.
- Keep submodule creation in `module.cpp`, not scattered across files.
- Put repeated lambda wrappers for `Result<T>` handling behind small local helpers if they repeat three or more times.
- Keep Python-visible names unchanged during the refactor.
- Avoid creating files by line count alone; split by API ownership.

## Why this works with the current build

`plugins/machine_learning/CMakeLists.txt` already uses:

```cmake
file(GLOB_RECURSE MACHINE_LEARNING_PYTHON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/*.cpp)
```

That means additional `.cpp` files under `python/` or `python/bindings/` are picked up automatically. The refactor does not require a new build mechanism, only a cleaner file layout.

## Expected outcome

After the split, each file should be small enough to review in isolation, and future additions should naturally go into an existing domain file instead of extending a single monolith.

As a rough target:

- entry point: under 100 lines
- shared types or helper areas: under 300 lines
- large feature families: usually under 400 to 600 lines each

That will make the machine learning bindings look much closer to a maintainable set of modules than a one-file registry dump.
