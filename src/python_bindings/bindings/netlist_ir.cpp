#include "hal_core/netlist/netlist_ir/netlist_ir.h"

#include "hal_core/netlist/netlist_ir/instantiate.h"
#include "hal_core/python_bindings/python_bindings.h"

namespace hal
{
    void netlist_ir_init(py::module& m)
    {
        auto py_ir = m.def_submodule("netlist_ir", R"(
            The netlist intermediate representation (IR).

            A parser for a netlist format builds a ``Design`` from a file; every module of the design owns its ports,
            signals, instances, and aliases. Ports and signals are expanded to bits, and every bit has a module-local
            handle that connections and aliases refer to. The IR does not depend on the format it came from or on a
            gate library; the gate library enters when the design is instantiated into a netlist.

            Containers such as ``Module.ports`` are handed to Python as copies. Build a design through ``add_module``,
            ``add_port``, ``add_signal``, ``add_instance``, and ``add_connection``, which return the objects that live
            inside the design, or assign a whole list to a container attribute.
        )");

        py_ir.attr("ZERO")           = netlist_ir::ZERO;
        py_ir.attr("ONE")            = netlist_ir::ONE;
        py_ir.attr("FIRST_USER_BIT") = netlist_ir::FIRST_USER_BIT;
        py_ir.attr("OPEN")           = netlist_ir::OPEN;

        py::class_<netlist_ir::Range> py_range(py_ir, "Range", R"(
            One index range of a port or signal, as declared: Verilog ``[7:0]`` is ``Range(7, 0)``, VHDL ``(0 to 3)`` is ``Range(0, 3)``.
        )");
        py_range.def(py::init<>());
        py_range.def(py::init([](i32 left, i32 right) {
                         netlist_ir::Range r;
                         r.left  = left;
                         r.right = right;
                         return r;
                     }),
                     py::arg("left"),
                     py::arg("right"),
                     R"(
            Construct a range.

            :param int left: The index written first.
            :param int right: The index written last.
        )");
        py_range.def_readwrite("left", &netlist_ir::Range::left, R"(
            The index written first.

            :type: int
        )");
        py_range.def_readwrite("right", &netlist_ir::Range::right, R"(
            The index written last.

            :type: int
        )");
        py_range.def_property_readonly("size", &netlist_ir::Range::size, R"(
            The number of indices in the range.

            :type: int
        )");
        py_range.def("is_descending", &netlist_ir::Range::is_descending, R"(
            Check whether the range runs from a high index down to a low one.

            :returns: ``True`` for a descending range such as ``[7:0]``, ``False`` otherwise.
            :rtype: bool
        )");
        py_range.def("index_at", &netlist_ir::Range::index_at, py::arg("offset"), R"(
            Get the index at a position, counted from the left end of the range.

            :param int offset: The position, ``0`` for the left end.
            :returns: The index at that position.
            :rtype: int
        )");
        py_range.def("offset_of", &netlist_ir::Range::offset_of, py::arg("index"), R"(
            Get the position of an index, counted from the left end of the range.

            :param int index: The index.
            :returns: The position, or ``None`` if the index is not in the range.
            :rtype: int or None
        )");
        py_range.def(py::self == py::self, R"(
            Check whether two ranges are equal.

            :returns: ``True`` if both ranges are equal, ``False`` otherwise.
            :rtype: bool
        )");
        py_range.def(py::self != py::self, R"(
            Check whether two ranges are unequal.

            :returns: ``True`` if both ranges are unequal, ``False`` otherwise.
            :rtype: bool
        )");
        py_range.def("__repr__", [](const netlist_ir::Range& r) { return "Range(" + std::to_string(r.left) + ", " + std::to_string(r.right) + ")"; });

        py::class_<netlist_ir::Location> py_location(py_ir, "Location", R"(
            Where something was read from: a line and a column of the design's source file, both ``0`` when unknown.
        )");
        py_location.def(py::init<>());
        py_location.def(py::init([](u32 line, u32 column) {
                            netlist_ir::Location l;
                            l.line   = line;
                            l.column = column;
                            return l;
                        }),
                        py::arg("line"),
                        py::arg("column") = 0,
                        R"(
            Construct a location.

            :param int line: The line, starting at 1.
            :param int column: The column, starting at 1; ``0`` when unknown.
        )");
        py_location.def_readwrite("line", &netlist_ir::Location::line, R"(
            The line, ``0`` when unknown.

            :type: int
        )");
        py_location.def_readwrite("column", &netlist_ir::Location::column, R"(
            The column, ``0`` when unknown.

            :type: int
        )");
        py_location.def("__str__", &netlist_ir::Location::to_string);

        py::class_<netlist_ir::TypedValue> py_typed_value(py_ir, "TypedValue", R"(
            A named, typed value: a parameter or generic, or an attribute such as a Verilog ``(* keep = "true" *)``; the
            declaration's source tells which. The declaration carries the name, the type and the source, the type inferred
            from the literal form in the file. When the gate type of an instance declares a parameter of the same name, the
            gate type's declaration wins at instantiation; attributes are never declared by gate types. A generic and an
            attribute may share a name.
        )");
        py_typed_value.def(py::init<>());
        py_typed_value.def(py::init([](const Parameter& declaration, const std::string& value) {
                               netlist_ir::TypedValue v;
                               v.declaration = declaration;
                               v.value       = value;
                               return v;
                           }),
                           py::arg("declaration"),
                           py::arg("value"),
                           R"(
            Construct a typed value.

            :param hal_py.Parameter declaration: The declaration, carrying the name and the type.
            :param str value: The value.
        )");
        py_typed_value.def_readwrite("declaration", &netlist_ir::TypedValue::declaration, R"(
            The declaration, carrying the name and the type.

            :type: hal_py.Parameter
        )");
        py_typed_value.def_readwrite("value", &netlist_ir::TypedValue::value, R"(
            The value.

            :type: str
        )");

        py::class_<netlist_ir::Signal> py_signal(py_ir, "Signal", R"(
            A signal of a module, expanded to bits. ``bits`` lists one handle per bit in declaration order: the index
            written first in the outermost dimension comes first, and the innermost dimension runs fastest. A scalar
            has no dimensions and one bit.
        )");
        py_signal.def(py::init<>());
        py_signal.def_readwrite("name", &netlist_ir::Signal::name, R"(
            The name.

            :type: str
        )");
        py_signal.def_readwrite("dims", &netlist_ir::Signal::dims, R"(
            The index ranges as declared, outermost first; empty for a scalar.

            :type: list[hal_py.netlist_ir.Range]
        )");
        py_signal.def_readwrite("bits", &netlist_ir::Signal::bits, R"(
            One handle per bit, in declaration order.

            :type: list[int]
        )");
        py_signal.def_readwrite("parameters", &netlist_ir::Signal::parameters, R"(
            The typed values, in practice attributes.

            :type: list[hal_py.netlist_ir.TypedValue]
        )");
        py_signal.def_readwrite("location", &netlist_ir::Signal::location, R"(
            Where the declaration was read from.

            :type: hal_py.netlist_ir.Location
        )");
        py_signal.def_property_readonly("width", &netlist_ir::Signal::width, R"(
            The number of bits.

            :type: int
        )");
        py_signal.def(
            "bit_at",
            [](const netlist_ir::Signal& s, const std::vector<i32>& indices) -> std::optional<netlist_ir::BitId> {
                auto res = s.bit_at(indices);
                if (res.is_ok())
                {
                    return res.get();
                }
                log_error("python_context", "{}", res.get_error().get());
                return std::nullopt;
            },
            py::arg("indices"),
            R"(
            Get the bit at the given indices, one per dimension.

            :param list[int] indices: One declared index per dimension, outermost first.
            :returns: The bit on success, ``None`` otherwise.
            :rtype: int or None
        )");
        py_signal.def(
            "slice",
            [](const netlist_ir::Signal& s, const netlist_ir::Range& range) -> std::optional<std::vector<netlist_ir::BitId>> {
                auto res = s.slice(range);
                if (res.is_ok())
                {
                    return res.get();
                }
                log_error("python_context", "{}", res.get_error().get());
                return std::nullopt;
            },
            py::arg("range"),
            R"(
            Get the bits of a part of a one-dimensional signal, in the order the given range lists them.

            :param hal_py.netlist_ir.Range range: The indices to select.
            :returns: The bits on success, ``None`` otherwise.
            :rtype: list[int] or None
        )");
        py_signal.def("bit_name", &netlist_ir::Signal::bit_name, py::arg("position"), R"(
            Get the name of one bit as the netlist will show it: ``name`` for a scalar, ``name(3)`` for a vector bit.

            :param int position: The position of the bit in ``bits``.
            :returns: The name.
            :rtype: str
        )");

        py::class_<netlist_ir::Port, netlist_ir::Signal> py_port(py_ir, "Port", R"(
            A port of a module: a signal with a direction. A port is not repeated in the module's signal list.
        )");
        py_port.def(py::init<>());
        py_port.def_readwrite("direction", &netlist_ir::Port::direction, R"(
            The direction.

            :type: hal_py.PinDirection
        )");

        py::class_<netlist_ir::Connection> py_connection(py_ir, "Connection", R"(
            The bits connected to one port of an instance. ``bits`` are in expression order, left to right as written,
            so the last bit is bit 0 of the port. A connection to a constant refers to ``ZERO`` or ``ONE``; a pin that
            is left open or connected to ``x``/``z`` has no connection.
        )");
        py_connection.def(py::init<>());
        py_connection.def_readwrite("port", &netlist_ir::Connection::port, R"(
            The pin group of a gate or the port of a module; empty for a positional connection.

            :type: str
        )");
        py_connection.def_readwrite("port_slice", &netlist_ir::Connection::port_slice, R"(
            The part of the port that is connected; ``None`` for the whole port.

            :type: hal_py.netlist_ir.Range or None
        )");
        py_connection.def_readwrite("bits", &netlist_ir::Connection::bits, R"(
            The connected bits in expression order.

            :type: list[int]
        )");
        py_connection.def_readwrite("replicate", &netlist_ir::Connection::replicate, R"(
            Whether the single bit in ``bits`` connects to every bit of the port, for VHDL ``(others => '0')`` on a port of unknown width.

            :type: bool
        )");

        py::enum_<netlist_ir::InstanceKind>(py_ir, "InstanceKind", R"(
            Whether an instance refers to a gate type of the library or to a module of the design.
        )")
            .value("Gate", netlist_ir::InstanceKind::Gate, R"(The type is a gate type of the gate library.)")
            .value("Module", netlist_ir::InstanceKind::Module, R"(The type is a module of the design.)");

        py::class_<netlist_ir::Instance> py_instance(py_ir, "Instance", R"(
            An instance of a gate type or of a module inside a module.
        )");
        py_instance.def(py::init<>());
        py_instance.def_readwrite("name", &netlist_ir::Instance::name, R"(
            The instance name.

            :type: str
        )");
        py_instance.def_readwrite("type", &netlist_ir::Instance::type, R"(
            The gate type name or the module name.

            :type: str
        )");
        py_instance.def_readwrite("kind", &netlist_ir::Instance::kind, R"(
            Whether the type is a gate type or a module.

            :type: hal_py.netlist_ir.InstanceKind
        )");
        py_instance.def_readwrite("connections", &netlist_ir::Instance::connections, R"(
            The connections; either all named or all positional.

            :type: list[hal_py.netlist_ir.Connection]
        )");
        py_instance.def_readwrite("parameters", &netlist_ir::Instance::parameters, R"(
            The generics set on the instance and its attributes.

            :type: list[hal_py.netlist_ir.TypedValue]
        )");
        py_instance.def_readwrite("location", &netlist_ir::Instance::location, R"(
            Where the instance was read from.

            :type: hal_py.netlist_ir.Location
        )");
        py_instance.def("add_connection",
                        &netlist_ir::Instance::add_connection,
                        py::arg("port"),
                        py::arg("bits"),
                        py::arg("port_slice") = std::nullopt,
                        py::return_value_policy::reference_internal,
                        R"(
            Add a connection.

            :param str port: The port name, empty for a positional connection.
            :param list[int] bits: The connected bits in expression order.
            :param hal_py.netlist_ir.Range port_slice: The part of the port that is connected, if not the whole port.
            :returns: The new connection.
            :rtype: hal_py.netlist_ir.Connection
        )");
        py_instance.def("find_connection", &netlist_ir::Instance::find_connection, py::arg("port"), py::return_value_policy::reference_internal, R"(
            Find a connection by port name.

            :param str port: The port name.
            :returns: The connection, or ``None`` if there is none.
            :rtype: hal_py.netlist_ir.Connection or None
        )");

        py::class_<netlist_ir::Module> py_module(py_ir, "Module", R"(
            A module: the unit a file declares, i.e., a Verilog module, a VHDL entity with its architecture, an EDIF cell.
        )");
        py_module.def(py::init<>());
        py_module.def_readwrite("name", &netlist_ir::Module::name, R"(
            The module name.

            :type: str
        )");
        py_module.def_readwrite("ports", &netlist_ir::Module::ports, R"(
            The ports in declaration order.

            :type: list[hal_py.netlist_ir.Port]
        )");
        py_module.def_readwrite("signals", &netlist_ir::Module::signals, R"(
            The signals that are not ports.

            :type: list[hal_py.netlist_ir.Signal]
        )");
        py_module.def_readwrite("instances", &netlist_ir::Module::instances, R"(
            The instances.

            :type: list[hal_py.netlist_ir.Instance]
        )");
        py_module.def_readwrite("aliases", &netlist_ir::Module::aliases, R"(
            Pairs of bits that are the same net.

            :type: list[tuple(int,int)]
        )");
        py_module.def_readwrite("parameters", &netlist_ir::Module::parameters, R"(
            The declared parameters or generics with their default values, and the module's attributes.

            :type: list[hal_py.netlist_ir.TypedValue]
        )");
        py_module.def_readwrite("location", &netlist_ir::Module::location, R"(
            Where the module was read from.

            :type: hal_py.netlist_ir.Location
        )");
        py_module.def("new_bit", &netlist_ir::Module::new_bit, R"(
            Allocate a new bit handle.

            :returns: The handle.
            :rtype: int
        )");
        py_module.def("new_bits", &netlist_ir::Module::new_bits, py::arg("count"), R"(
            Allocate consecutive bit handles.

            :param int count: How many.
            :returns: The handles in allocation order.
            :rtype: list[int]
        )");
        py_module.def_property_readonly("next_bit", &netlist_ir::Module::next_bit, R"(
            The next handle that ``new_bit`` would hand out; every handle of the module is below it.

            :type: int
        )");
        py_module.def("add_port",
                      &netlist_ir::Module::add_port,
                      py::arg("name"),
                      py::arg("direction"),
                      py::arg("dims") = std::vector<netlist_ir::Range>(),
                      py::return_value_policy::reference_internal,
                      R"(
            Add a port with freshly allocated bits.

            :param str name: The port name.
            :param hal_py.PinDirection direction: The direction.
            :param list[hal_py.netlist_ir.Range] dims: The index ranges, outermost first; empty for a scalar.
            :returns: The new port.
            :rtype: hal_py.netlist_ir.Port
        )");
        py_module.def("add_signal", &netlist_ir::Module::add_signal, py::arg("name"), py::arg("dims") = std::vector<netlist_ir::Range>(), py::return_value_policy::reference_internal, R"(
            Add a signal with freshly allocated bits.

            :param str name: The signal name.
            :param list[hal_py.netlist_ir.Range] dims: The index ranges, outermost first; empty for a scalar.
            :returns: The new signal.
            :rtype: hal_py.netlist_ir.Signal
        )");
        py_module.def("add_instance", &netlist_ir::Module::add_instance, py::arg("name"), py::arg("type"), py::arg("kind"), py::return_value_policy::reference_internal, R"(
            Add an instance.

            :param str name: The instance name.
            :param str type: The gate type name or module name.
            :param hal_py.netlist_ir.InstanceKind kind: Whether the type is a gate type or a module.
            :returns: The new instance.
            :rtype: hal_py.netlist_ir.Instance
        )");
        py_module.def("add_alias", &netlist_ir::Module::add_alias, py::arg("a"), py::arg("b"), R"(
            Record that two bits are the same net.

            :param int a: One bit.
            :param int b: The other bit.
        )");
        py_module.def("find_port", &netlist_ir::Module::find_port, py::arg("name"), py::return_value_policy::reference_internal, R"(
            Find a port by name.

            :param str name: The port name.
            :returns: The port, or ``None`` if there is none.
            :rtype: hal_py.netlist_ir.Port or None
        )");
        py_module.def("find_signal", &netlist_ir::Module::find_signal, py::arg("name"), py::return_value_policy::reference_internal, R"(
            Find a signal or a port by name.

            :param str name: The name.
            :returns: The signal, or ``None`` if there is none.
            :rtype: hal_py.netlist_ir.Signal or None
        )");
        py_module.def("find_instance",
                      py::overload_cast<const std::string&>(&netlist_ir::Module::find_instance),
                      py::arg("name"),
                      py::return_value_policy::reference_internal,
                      R"(
            Find an instance by name.

            :param str name: The instance name.
            :returns: The instance, or ``None`` if there is none.
            :rtype: hal_py.netlist_ir.Instance or None
        )");

        py::class_<netlist_ir::Design> py_design(py_ir, "Design", R"(
            A design: every module a file declares, and which one is the top.
        )");
        py_design.def(py::init<>());
        py_design.def(py::init([](const std::string& source) {
                          netlist_ir::Design d;
                          d.source = source;
                          return d;
                      }),
                      py::arg("source"),
                      R"(
            Construct an empty design.

            :param str source: The file the design is read from, for messages.
        )");
        py_design.def_readwrite("modules", &netlist_ir::Design::modules, R"(
            The modules.

            :type: list[hal_py.netlist_ir.Module]
        )");
        py_design.def_readwrite("top", &netlist_ir::Design::top, R"(
            The top module if the file names one; ``None`` otherwise.

            :type: str or None
        )");
        py_design.def_readwrite("source", &netlist_ir::Design::source, R"(
            The file the design was read from, for messages.

            :type: str
        )");
        py_design.def("add_module", &netlist_ir::Design::add_module, py::arg("name"), py::return_value_policy::reference_internal, R"(
            Add a module.

            :param str name: The module name.
            :returns: The new module.
            :rtype: hal_py.netlist_ir.Module
        )");
        py_design.def("find_module",
                      py::overload_cast<const std::string&>(&netlist_ir::Design::find_module),
                      py::arg("name"),
                      py::return_value_policy::reference_internal,
                      R"(
            Find a module by name.

            :param str name: The module name.
            :returns: The module, or ``None`` if there is none.
            :rtype: hal_py.netlist_ir.Module or None
        )");
        py_design.def(
            "find_top",
            [](const netlist_ir::Design& d) -> std::optional<std::string> {
                auto res = d.find_top();
                if (res.is_ok())
                {
                    return res.get();
                }
                log_error("python_context", "{}", res.get_error().get());
                return std::nullopt;
            },
            R"(
            Determine the top module: the one named by ``top`` if set, otherwise the single module that no other module
            instantiates.

            :returns: The name of the top module, or ``None`` if there is not exactly one candidate.
            :rtype: str or None
        )");
        py_design.def(
            "validate",
            [](const netlist_ir::Design& d) -> bool {
                auto res = d.validate();
                if (res.is_ok())
                {
                    return true;
                }
                log_error("python_context", "{}", res.get_error().get());
                return false;
            },
            R"(
            Check everything the instantiation relies on: unique names, valid bit handles, consistent widths, existing
            module types, named or positional connections but not both, and connection widths against module ports.

            :returns: ``True`` if the design is valid, ``False`` otherwise (the problem is logged).
            :rtype: bool
        )");

        py::class_<netlist_ir::InstantiationOptions> py_options(py_ir, "InstantiationOptions", R"(
            Settings for the instantiation of a design.
        )");
        py_options.def(py::init<>());
        py_options.def_readwrite("keep_unconnected_signals", &netlist_ir::InstantiationOptions::keep_unconnected_signals, R"(
            Create a net for every signal, even for one that nothing drives or reads.

            :type: bool
        )");
        py_options.def_readwrite("instance_name_separator", &netlist_ir::InstantiationOptions::instance_name_separator, R"(
            What separates the instance path from the name when a name has to be prefixed to stay unique.

            :type: str
        )");
        py_options.def_readwrite("gnd_gate_type", &netlist_ir::InstantiationOptions::gnd_gate_type, R"(
            The gate type to drive the constant ``0`` with; empty selects the first GND type of the gate library.

            :type: str
        )");
        py_options.def_readwrite("vcc_gate_type", &netlist_ir::InstantiationOptions::vcc_gate_type, R"(
            The gate type to drive the constant ``1`` with; empty selects the first VCC type of the gate library.

            :type: str
        )");
        py_options.def_readwrite("top_module_name", &netlist_ir::InstantiationOptions::top_module_name, R"(
            The name of the top module of the netlist; the design name is the top module's type regardless.

            :type: str
        )");

        py_ir.def(
            "instantiate",
            [](const netlist_ir::Design& design, const GateLibrary* gate_library, const netlist_ir::InstantiationOptions& options) -> std::shared_ptr<Netlist> {
                auto res = netlist_ir::instantiate(design, gate_library, options);
                if (res.is_ok())
                {
                    return std::shared_ptr<Netlist>(res.get().release());
                }
                log_error("python_context", "{}", res.get_error().get());
                return nullptr;
            },
            py::arg("design"),
            py::arg("gate_library"),
            py::arg("options") = netlist_ir::InstantiationOptions(),
            R"(
            Instantiate a design against a gate library.

            The design is validated first. Every alias, every connection to a module port and every constant is resolved
            before anything is created, so each net is created exactly once. A net is created for every class of bits
            that connects to a gate pin, is a port of the top module, or carries an attribute. Gate types and pins are
            resolved by exact name first and then by a unique case-insensitive match. Parameters and attributes land in
            the typed store of the created objects; a generic that the gate type declares takes the gate type's
            declaration.

            :param hal_py.netlist_ir.Design design: The design.
            :param hal_py.GateLibrary gate_library: The gate library to resolve gate types against.
            :param hal_py.netlist_ir.InstantiationOptions options: The settings.
            :returns: The netlist on success, ``None`` otherwise (the error is logged).
            :rtype: hal_py.Netlist or None
        )");
    }
}    // namespace hal
