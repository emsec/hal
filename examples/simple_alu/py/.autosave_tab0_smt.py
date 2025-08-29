### PREPARATIONS

# create 8-bit A and B variables from input pin groups of ALU module
grp_a = netlist.get_top_module().get_pin_group_by_name("A")
var_a = hal_py.BooleanFunctionDecorator.get_boolean_function_from(grp_a)

grp_b = netlist.get_top_module().get_pin_group_by_name("B")
var_b = hal_py.BooleanFunctionDecorator.get_boolean_function_from(grp_b)

grp_op = netlist.get_top_module().get_pin_group_by_name("op")
var_op = hal_py.BooleanFunctionDecorator.get_boolean_function_from(grp_op)


# obtain 8-bit Boolean function describing ALU module output
output_functions = list()
grp_z = netlist.get_top_module().get_pin_group_by_name("Z")
for pin in reversed(grp_z.get_pins()):
    output_functions.append(hal_py.NetlistUtils.get_subgraph_function(pin.get_net(), netlist.get_gates()))

alu_func = hal_py.BooleanFunctionDecorator.get_boolean_function_from(output_functions)


### CHECK IF ADDER

# construct adder model
adder_func = hal_py.BooleanFunction.Add(var_a, var_b, 8)

# build constraints
adder_cstr = hal_py.SMT.Constraint(hal_py.BooleanFunction.Not(hal_py.BooleanFunction.Eq(adder_func, alu_func, 1), 1))
op_code_cstr = hal_py.SMT.Constraint(hal_py.BooleanFunction.Eq(var_op, hal_py.BooleanFunction.Const(0, 2), 1))

# query SMT solver to check satisfiability under given constraints
solver = hal_py.SMT.Solver([adder_cstr, op_code_cstr])
res = solver.query(hal_py.SMT.QueryConfig().with_solver(hal_py.SMT.SolverType.Z3).with_local_solver().with_timeout(1000))

# if output is SAT, then verified adder for given opcode
print(res.type)

    