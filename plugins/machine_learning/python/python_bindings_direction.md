# Python Bindings Direction

This document describes the preferred way to build Python bindings for HAL plugins. It is intentionally generic so it can be reused as instruction context for future plugins.

## Goal

Python bindings should be easy to extend, easy to review, and safe to refactor. The binding layout should mirror the plugin's API structure instead of collapsing everything into one large `python_bindings.cpp`.

## Build model

HAL plugin bindings are compiled as part of the plugin target itself.

The current plugin CMake pattern is:

```cmake
file(GLOB_RECURSE <PLUGIN>_PYTHON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/*.cpp)

hal_add_plugin(<plugin_name>
    SHARED
    HEADER ${<PLUGIN>_INC}
    SOURCES ${<PLUGIN>_SRC} ${<PLUGIN>_PYTHON_SRC}
    LINK_LIBRARIES ...
)
```

Because the Python sources are collected recursively, a plugin may safely split bindings across multiple files and subdirectories under `python/`.

## Recommended file layout

Every plugin should prefer this structure:

- `python/module.cpp`
- `python/bindings/register.h`
- `python/bindings/shared.h`
- `python/bindings/<domain>.cpp`

Examples of `<domain>`:

- `plugin.cpp`
- `types.cpp`
- `configuration.cpp`
- `context.cpp`
- `algorithms.cpp`
- `writers.cpp`
- `feature_extractors.cpp`

## Core rule

Only one file should contain the module entry point:

```cpp
PYBIND11_MODULE(<module_name>, m)
```

That file should do only four things:

1. set the module docstring
2. create submodules
3. call `bind_*` registration functions
4. keep registration order stable

It should not contain the full binding implementation for every class and function.

## Registration pattern

Use one registration function per domain.

Example:

```cpp
namespace hal::<plugin>::python
{
    void bind_plugin(pybind11::module_& m);
    void bind_types(pybind11::module_& m);
    void bind_context(pybind11::module_& m);
    void bind_algorithms(pybind11::module_& m);
    void bind_labels(pybind11::module_& m, pybind11::module_& labels);
}
```

This keeps ownership clear and avoids merge conflicts when multiple people touch the bindings.

## What belongs together

Split files by API ownership, not by arbitrary size.

Good grouping:

- one base class with its derived classes
- one namespace or submodule family
- one configuration object and closely related enums
- one feature family

Avoid:

- splitting a single class hierarchy across many files unless it is very large
- mixing unrelated concepts just because they are all short
- hiding module creation in many different translation units

## Shared helpers

Use `shared.h` for common binding-only utilities:

- common includes
- `namespace py = pybind11;`
- small helper wrappers
- repeated conversion or error-handling helpers

Do not turn `shared.h` into a second monolith. If a helper is only used in one file, keep it local to that file.

## Python API stability

Refactors should preserve the external Python surface unless there is a deliberate API change.

Keep stable:

- module names
- submodule names
- class names
- method names
- argument names
- docstring intent

Internal file movement should not force Python users to update scripts.

## Wrapper conventions

Prefer these patterns:

- bind plain C++ methods directly when ownership and return types are already safe
- use small lambdas when converting `Result<T>` to Python-friendly values
- return `None` on failure only when that is already the established plugin behavior
- log failures with a clear binding-specific log channel

If the same wrapper pattern appears several times, extract a tiny helper instead of repeating it everywhere.

## Submodules

If the plugin has multiple conceptual families, create explicit submodules in `module.cpp` and pass them into the relevant registration functions.

Example:

```cpp
auto py_labels = m.def_submodule("labels");
auto py_features = m.def_submodule("features");

bind_labels(m, py_labels);
bind_features(m, py_features);
```

Submodule creation should stay centralized so the module topology is obvious from one file.

## Documentation expectations

Bindings should include concise but useful docstrings:

- what the type or function does
- key parameters
- return shape
- failure behavior when non-obvious

Do not duplicate long C++ implementation details that are irrelevant to Python users.

## Testing expectations

Each plugin should have a lightweight way to exercise its Python bindings after changes.

Prefer one or more of:

- dedicated Python tests under `tests/python_binding/`
- plugin-local Python smoke tests
- small scripts that import the module and instantiate key classes

When refactoring bindings, validate at least:

- the module imports
- submodules exist
- representative classes construct successfully
- representative wrapped functions still return the same Python-visible shapes

## Refactor checklist

When a binding file grows too large:

1. identify the existing domain boundaries in the file
2. create `module.cpp`, `register.h`, and `shared.h` if they do not exist
3. move one domain at a time into `python/bindings/<domain>.cpp`
4. keep Python-visible names unchanged
5. rebuild and run smoke tests after each move

## Prompt-ready instruction block

Use this when asking for future binding work:

```text
Build HAL Python bindings as a modular pybind11 entry point, not a single monolithic file.
Keep exactly one PYBIND11_MODULE file that sets the docstring, creates submodules, and calls bind_* registration functions.
Split bindings by API domain under python/bindings/, keeping one coherent family per file.
Preserve the existing Python-visible API unless an intentional API change is requested.
Use small local helpers for repeated Result<T> to Python conversions, and keep submodule creation centralized in module.cpp.
Assume the plugin CMake already picks up all python/*.cpp files recursively.
```
