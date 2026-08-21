// MIT License
//
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include "hal_core/defines.h"
#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/boolean_function/solver.h"
#include "hal_core/netlist/boolean_function/symbolic_execution.h"
#include "hal_core/netlist/boolean_function/symbolic_state.h"
#include "hal_core/netlist/boolean_function/types.h"
#include "hal_core/netlist/decorators/boolean_function_decorator.h"
#include "hal_core/netlist/decorators/boolean_function_net_decorator.h"
#include "hal_core/netlist/decorators/netlist_modification_decorator.h"
#include "hal_core/netlist/decorators/netlist_traversal_decorator.h"
#include "hal_core/netlist/decorators/subgraph_netlist_decorator.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/enums/async_set_reset_behavior.h"
#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/netlist/gate_library/gate_library_parser/gate_library_parser_manager.h"
#include "hal_core/netlist/gate_library/gate_library_writer/gate_library_writer_manager.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/grouping.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_factory.h"
#include "hal_core/netlist/netlist_utils.h"
#include "hal_core/netlist/netlist_writer/netlist_writer_manager.h"
#include "hal_core/netlist/persistent/netlist_serializer.h"
#include "hal_core/netlist/pins/base_pin.h"
#include "hal_core/netlist/pins/gate_pin.h"
#include "hal_core/netlist/pins/module_pin.h"
#include "hal_core/netlist/pins/pin_group.h"
#include "hal_core/netlist/project_manager.h"
#include "hal_core/plugin_system/fac_extension_interface.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/program_arguments.h"
#include "hal_core/utilities/program_options.h"
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
#include "pybind11/stl/filesystem.h"
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
     * Call policy for a binding that hands out an object owned by something else, such as a gate of
     * a netlist or a gate type of a gate library.
     *
     * Use it wherever `py::return_value_policy::reference_internal` was used before. It hands the
     * object over without ownership, exactly as that policy did, but decides differently what has
     * to stay alive and when that decision is made.
     *
     * It requests `py::return_value_policy::reference`, which creates a non-owning wrapper and links
     * nothing, and then establishes every link itself. `reference_internal` cannot be used
     * underneath: it calls `keep_alive_impl(instance, parent)` while the wrapper is being created,
     * which would add a link to the receiver on top of the one added here.
     *
     * Two things it does that the policy does not:
     *
     * - It links **unconditionally**. `reference_internal` is only honoured while a wrapper is being
     *   created; when pybind11 finds that the object already has one it returns that wrapper
     *   immediately and applies neither the policy nor any keep_alive. Whether an object was
     *   protected therefore depended on which binding happened to hand it over first, and one
     *   unprotected path was enough to leave it unprotected everywhere.
     *
     * - It keeps the **owner** alive rather than the object the binding was called on. A module read
     *   from a gate stays valid because the netlist stays alive, not because that gate does, and the
     *   netlist is also what owns it. Linking to the receiver instead would tie the module to the
     *   gate while the gate can be read back from the module, closing a reference cycle; pybind11
     *   instances are not tracked by the garbage collector, so such a cycle is never collected, and
     *   making the link unconditional would have turned an order dependence into a permanent leak.
     *
     * The owner is only ever resolved through a wrapper that already exists. Constructing one is
     * deliberately avoided, as that would hand a borrowed pointer to a holder and could open a
     * second ownership group over an object that is already owned elsewhere.
     *
     * @ingroup pybind
     */
    struct borrowed
    {
    };

    namespace python_bindings_detail
    {
        /**
         * Get the Python wrapper of an object, without ever creating one.
         *
         * @returns The wrapper if the object has one, an empty handle otherwise.
         */
        template<typename T>
        py::handle existing_wrapper(const T* object)
        {
            if (object == nullptr)
            {
                return py::handle();
            }

            const auto* type = py::detail::get_type_info(typeid(T));
            if (type == nullptr)
            {
                return py::handle();
            }

            // returns a borrowed reference and does not create a wrapper
            return py::detail::get_object_handle(object, type);
        }

        /**
         * Get the object that has to outlive the given one.
         *
         * @returns The owner if it has a wrapper already, an empty handle otherwise.
         */
        inline py::handle owner_of(py::handle object)
        {
            if (py::isinstance<Gate>(object))
            {
                return existing_wrapper(py::cast<const Gate*>(object)->get_netlist());
            }
            if (py::isinstance<Net>(object))
            {
                return existing_wrapper(py::cast<const Net*>(object)->get_netlist());
            }
            if (py::isinstance<Module>(object))
            {
                return existing_wrapper(py::cast<const Module*>(object)->get_netlist());
            }
            if (py::isinstance<Grouping>(object))
            {
                return existing_wrapper(py::cast<const Grouping*>(object)->get_netlist());
            }
            if (py::isinstance<Endpoint>(object))
            {
                const auto* gate = py::cast<const Endpoint*>(object)->get_gate();
                return gate == nullptr ? py::handle() : existing_wrapper(gate->get_netlist());
            }
            if (py::isinstance<GateType>(object))
            {
                return existing_wrapper(py::cast<const GateType*>(object)->get_gate_library());
            }

            // A pin, a pin group and a gate type component do not know what they belong to, so
            // there is nothing to look up and the caller falls back to the receiver.
            return py::handle();
        }

        /**
         * Keep the owner of a returned object alive for as long as Python refers to the object.
         *
         * Applies to each element of a returned container. The container itself cannot be linked to
         * anything, as a Python list is not a pybind11 type and has no slot to hold a reference in.
         * This is also why the work cannot be left to `py::keep_alive`, which fails on a container.
         */
        inline void keep_owner_alive(py::detail::function_call& call, py::handle value, py::handle receiver)
        {
            if (!value || value.is_none())
            {
                return;
            }

            if (py::isinstance<py::list>(value) || py::isinstance<py::tuple>(value) || py::isinstance<py::set>(value))
            {
                for (py::handle element : py::reinterpret_borrow<py::object>(value))
                {
                    keep_owner_alive(call, element, receiver);
                }
                return;
            }

            if (py::isinstance<py::dict>(value))
            {
                for (auto item : py::reinterpret_borrow<py::dict>(value))
                {
                    keep_owner_alive(call, item.second, receiver);
                }
                return;
            }

            // Only a pybind11 instance can hold a reference to its owner. Anything else, such as the
            // index paired with the pin group returned by GatePin.get_group, would send keep_alive
            // down its weak reference fallback, which fails outright on an int or a string.
            if (py::detail::all_type_info(Py_TYPE(value.ptr())).empty())
            {
                return;
            }

            const py::handle owner = owner_of(value);
            const py::handle patient = owner ? owner : receiver;

            if (!patient || patient.is_none() || patient.ptr() == value.ptr())
            {
                return;
            }

            py::detail::keep_alive_impl(value, patient);
        }
    }    // namespace python_bindings_detail
}    // namespace hal

namespace pybind11
{
    namespace detail
    {
        /// Makes `hal::borrowed` usable as a call policy, like `py::keep_alive` or a return value policy.
        template<>
        struct process_attribute<hal::borrowed> : public process_attribute_default<hal::borrowed>
        {
            static void init(const hal::borrowed&, function_record* record)
            {
                // A non-owning wrapper that links nothing; every link is added in postcall below.
                record->policy = return_value_policy::reference;
            }

            static void postcall(function_call& call, handle ret)
            {
                // Only a method has a receiver worth falling back to. The first argument of a free
                // function is just an argument and need not own anything that is returned.
                // size(), not empty(): pybind11 changed args from a std::vector to a small_vector that has no empty()
                const handle receiver = (call.func.is_method && call.args.size() > 0) ? call.args[0] : handle();
                hal::python_bindings_detail::keep_owner_alive(call, ret, receiver);
            }
        };
    }    // namespace detail
}    // namespace pybind11

namespace hal
{

    // TODO move into own namespace
    /**
     *
     * @ingroup pybind
     * @{
     */

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
     * Initializes Python bindings for the HAL gate type components in a python module.
     *
     * @param[in] m - the python module
     */
    void gate_type_components_init(py::module& m);

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
     * Initializes Python bindings for the HAL project manager in a python module.
     *
     * @param[in] m - the python module
     */
    void project_manager_init(py::module& m);

    /**
     * Initializes Python bindings for the HAL netlist serializer in a python module.
     *
     * @param[in] m - the python module
     */
    void netlist_serializer_init(py::module& m);

    /**
     * Initializes Python bindings for the HAL netlist utils in a python module.
     *
     * @param[in] m - the python module
     */
    void netlist_utils_init(py::module& m);

    /**
     * Initializes Python bindings for the HAL base pins in a python module.
     *
     * @param[in] m - the python module
     */
    void base_pin_init(py::module& m);

    /**
     * Initializes Python bindings for the HAL gate pins in a python module.
     *
     * @param[in] m - the python module
     */
    void gate_pin_init(py::module& m);

    /**
     * Initializes Python bindings for the HAL gate pin groups in a python module.
     *
     * @param[in] m - the python module
     */
    void gate_pin_group_init(py::module& m);

    /**
     * Initializes Python bindings for the HAL module pins in a python module.
     *
     * @param[in] m - the python module
     */
    void module_pin_init(py::module& m);

    /**
     * Initializes Python bindings for the HAL module pin groups in a python module.
     *
     * @param[in] m - the python module
     */
    void module_pin_group_init(py::module& m);

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
     * Initializes Python bindings for the HAL netlist writer manager in a python module.
     *
     * @param[in] m - the python module
     */
    void netlist_writer_manager_init(py::module& m);

    /**
     * Initializes Python bindings for the HAL plugin manager in a python module.
     *
     * @param[in] m - the python module
     */
    void program_options_init(py::module& m);

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
     * Initializes Python bindings for the HAL SMT solver system in a python module.
     *
     * @param[in] m - the python module
     */
    void smt_init(py::module& m);

    /**
     * Initializes Python bindings for the HAL Boolean function net decorator in a python module.
     *
     * @param[in] m - the python module
     */
    void boolean_function_net_decorator_init(py::module& m);

    /**
     * Initializes Python bindings for the HAL subgraph netlist decorator in a python module.
     *
     * @param[in] m - the python module
     */
    void subgraph_netlist_decorator_init(py::module& m);

    /**
     * Initializes Python bindings for the HAL Boolean function decorator in a python module.
     *
     * @param[in] m - the python module
     */
    void boolean_function_decorator_init(py::module& m);

    /**
     * Initializes Python bindings for the HAL netlist modification decorator in a python module.
     *
     * @param[in] m - the python module
     */
    void netlist_modification_decorator_init(py::module& m);

    /**
     * Initializes Python bindings for the HAL netlist traversal decorator in a python module.
     *
     * @param[in] m - the python module
     */
    void netlist_traversal_decorator_init(py::module& m);

    /**
     * Initializes Python bindings for the HAL LogManager in a python module.
     *
     * @param[in] m - the python module
     */
    void log_init(py::module& m);
    /**
     * @}
     */
}    // namespace hal
