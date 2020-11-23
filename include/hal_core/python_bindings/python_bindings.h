#pragma once

#include "hal_core/defines.h"
#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/netlist/gate_library/gate_library_parser.h"
#include "hal_core/netlist/gate_library/gate_type/gate_type.h"
#include "hal_core/netlist/gate_library/gate_type/gate_type_lut.h"
#include "hal_core/netlist/gate_library/gate_type/gate_type_sequential.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/netlist/grouping.h"
#include "hal_core/netlist/hdl_writer/hdl_writer_manager.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_factory.h"
#include "hal_core/netlist/netlist_utils.h"
#include "hal_core/netlist/persistent/netlist_serializer.h"
#include "hal_core/plugin_system/plugin_interface_gui.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/utils.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#ifdef COMPILER_CLANG
#pragma clang diagnostic ignored "-Wnested-anon-types"
#pragma clang diagnostic ignored "-Wshadow-field-in-constructor-modified"
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif

#include "pybind11/functional.h"
#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"

#pragma GCC diagnostic pop

namespace hal
{
    namespace py = pybind11;

    /**
     * Wrapper class so that PyBind can work with raw pointers to classes that are managed by HAL.
     *
     * @ingroup pybind
     */
    template<class T>
    using RawPtrWrapper = std::unique_ptr<T, py::nodelete>;

    /**
     * TODO move into own namespace
     *
     * @ingroup pybind
     * @{
     */

    /**
     * Initializes Python bindings for the HAL path in a python module.
     *
     * @param[in] m - the python module
     */
    void path_init(py::module& m);

    /**
     * Initializes Python bindings for the HAL data container in a python module.
     *
     * @param[in] m - the python module
     */
    void data_container_init(py::module& m);

    /**
     * Initializes Python bindings for the HAL core utils in a python module.
     *
     * @param[in] m - the python module
     */
    void core_utils_init(py::module& m);

    /**
     * Initializes Python bindings for the HAL gate type in a python module.
     *
     * @param[in] m - the python module
     */
    void gate_type_init(py::module& m);

    /**
     * Initializes Python bindings for the HAL LUT gate type in a python module.
     *
     * @param[in] m - the python module
     */
    void gate_type_lut_init(py::module& m);

    /**
     * Initializes Python bindings for the HAL sequential gate type in a python module.
     *
     * @param[in] m - the python module
     */
    void gate_type_sequential_init(py::module& m);

    /**
     * Initializes Python bindings for the HAL gate library in a python module.
     *
     * @param[in] m - the python module
     */
    void gate_library_init(py::module& m);

    /**
     * Initializes Python bindings for the HAL gate library manager in a python module.
     *
     * @param[in] m - the python module
     */
    void gate_library_manager_init(py::module& m);

    /**
     * Initializes Python bindings for the HAL endpoint in a python module.
     *
     * @param[in] m - the python module
     */
    void endpoint_init(py::module& m);

    /**
     * Initializes Python bindings for the HAL netlist in a python module.
     *
     * @param[in] m - the python module
     */
    void netlist_init(py::module& m);

    /**
     * Initializes Python bindings for the HAL netlist in a python module.
     *
     * @param[in] m - the python module
     */
    void netlist_utils_init(py::module& m);

    /**
     * Initializes Python bindings for the HAL gate in a python module.
     *
     * @param[in] m - the python module
     */
    void gate_init(py::module& m);

    /**
     * Initializes Python bindings for the HAL net in a python module.
     *
     * @param[in] m - the python module
     */
    void net_init(py::module& m);

    /**
     * Initializes Python bindings for the HAL module in a python module.
     *
     * @param[in] m - the python module
     */
    void module_init(py::module& m);

    /**
     * Initializes Python bindings for the HAL grouping in a python module.
     *
     * @param[in] m - the python module
     */
    void grouping_init(py::module& m);

    /**
     * Initializes Python bindings for the HAL netlist factory in a python module.
     *
     * @param[in] m - the python module
     */
    void netlist_factory_init(py::module& m);

    /**
     * Initializes Python bindings for the HAL hdl writer manager in a python module.
     *
     * @param[in] m - the python module
     */
    void hdl_writer_manager_init(py::module& m);

    /**
     * Initializes Python bindings for the HAL plugin manager in a python module.
     *
     * @param[in] m - the python module
     */
    void plugin_manager_init(py::module& m);

    /**
     * Initializes Python bindings for the HAL plugin interfaces in a python module.
     *
     * @param[in] m - the python module
     */
    void plugin_interfaces_init(py::module& m);

    /**
     * Initializes Python bindings for the HAL Boolean function in a python module.
     *
     * @param[in] m - the python module
     */
    void boolean_function_init(py::module& m);

    /**
     * @}
     */
}    // namespace hal
