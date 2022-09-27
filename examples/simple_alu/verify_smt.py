output_functions = list()

# add output nets in correct order to list
for i in range(3,11):
    net = netlist.get_net_by_id(i)
    output_functions.append(hal_py.NetlistUtils.get_subgraph_function(net, netlist.get_gates()))


# get multibit boolean functions
output_c = output_functions[0]
for i in range(1, 8):
    new_size = output_functions[i].get_size() + output_c.get_size()
    output_c = hal_py.BooleanFunction.Concat(output_functions[i], output_c, new_size)



# substitute variable names with their original input
# important: get rid of brackets, due to smt2 syntax
for i in range(11, 29):
    net = netlist.get_net_by_id(i)
    output_c = output_c.substitute("net_" + str(net.get_id()), net.get_name().replace("(", "").replace(")",""))



# check if adder
var_a = hal_py.BooleanFunction.Var("A0", 1)
var_b = hal_py.BooleanFunction.Var("B0", 1)
for i in range(1,8):
    new_size = 1 + var_a.get_size()
    var_a = hal_py.BooleanFunction.Concat(hal_py.BooleanFunction.Var("A" +str(i), 1), var_a, new_size)
    var_b = hal_py.BooleanFunction.Concat(hal_py.BooleanFunction.Var("B" +str(i), 1), var_b, new_size)



#build constraints
const_one = hal_py.BooleanFunction.Const(hal_py.BooleanFunction.Value.ONE)
const_zero = hal_py.BooleanFunction.Const(hal_py.BooleanFunction.Value.ZERO)

adder = hal_py.SMT.Constraint(hal_py.BooleanFunction.Not(hal_py.BooleanFunction.Eq(hal_py.BooleanFunction.Add(var_a, var_b, 8), output_c, 1), 1))
op_code_0 = hal_py.SMT.Constraint(hal_py.BooleanFunction.Var("op0", 1), const_zero)
op_code_1 = hal_py.SMT.Constraint(hal_py.BooleanFunction.Var("op1", 1), const_zero)


# This uses SMT solvers to find a set of inputs that make the trigger function evaluate to 1.
solver = hal_py.SMT.Solver([adder, op_code_0, op_code_1])
res = solver.query(hal_py.SMT.QueryConfig().with_solver(hal_py.SMT.SolverType.Z3).with_local_solver().with_model_generation().with_timeout(1000))
print(res.type)

    