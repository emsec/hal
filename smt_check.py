mod_id = 10
a_grp_name = "A"
b_grp_name = "B" 
o_grp_name = "O"

op_net_ids = [12, 11]

mod = netlist.get_module_by_id(mod_id)
a_grp = mod.get_pin_group_by_name(a_grp_name)
b_grp = mod.get_pin_group_by_name(b_grp_name)
o_grp = mod.get_pin_group_by_name(o_grp_name)

op_nets = [netlist.get_net_by_id(nid) for nid in op_net_ids]

a_var = hal_py.BooleanFunctionDecorator.get_boolean_function_from(a_grp)
b_var = hal_py.BooleanFunctionDecorator.get_boolean_function_from(b_grp)
op_var = hal_py.BooleanFunctionDecorator.get_boolean_function_from(op_nets)

o_nets = [p.get_net() for p in o_grp.get_pins()]
o_bf = hal_py.SubgraphNetlistDecorator(netlist).get_subgraph_function(mod, o_nets[0])
for i in range(1, 8):
    o_bf = hal_py.BooleanFunction.Concat(hal_py.SubgraphNetlistDecorator(netlist).get_subgraph_function(mod, o_nets[i]), o_bf, i + 1)
o_bf = hal_py.BooleanFunctionDecorator(o_bf).substitute_power_ground_nets(netlist)

c_add = hal_py.SMT.Constraint(hal_py.BooleanFunction.Not(hal_py.BooleanFunction.Eq(hal_py.BooleanFunction.Add(a_var, b_var, 8), o_bf, 1), 1))
c_sub1 = hal_py.SMT.Constraint(hal_py.BooleanFunction.Not(hal_py.BooleanFunction.Eq(hal_py.BooleanFunction.Sub(a_var, b_var, 8), o_bf, 1), 1))
c_sub2 = hal_py.SMT.Constraint(hal_py.BooleanFunction.Not(hal_py.BooleanFunction.Eq(hal_py.BooleanFunction.Sub(b_var, a_var, 8), o_bf, 1), 1))
c_not1 = hal_py.SMT.Constraint(hal_py.BooleanFunction.Not(hal_py.BooleanFunction.Eq(hal_py.BooleanFunction.Not(a_var, 8), o_bf, 1), 1))
c_not2 = hal_py.SMT.Constraint(hal_py.BooleanFunction.Not(hal_py.BooleanFunction.Eq(hal_py.BooleanFunction.Not(b_var, 8), o_bf, 1), 1))
c_rol = hal_py.SMT.Constraint(hal_py.BooleanFunction.Not(hal_py.BooleanFunction.Eq(hal_py.BooleanFunction.Rol(b_var, hal_py.BooleanFunction.Index(1, 8), 8), o_bf, 1), 1))

for i in range(4):
    c_op = hal_py.SMT.Constraint(hal_py.BooleanFunction.Eq(op_var, hal_py.BooleanFunction.Const(i, 2), 1))
    
    solver = hal_py.SMT.Solver([c_rol, c_op])
    res = solver.query(hal_py.SMT.QueryConfig().with_solver(hal_py.SMT.SolverType.Z3).with_local_solver().with_timeout(1000).with_model_generation())
    print(res.type)