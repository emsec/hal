#include "plugin_z3_utils.h"

#include "converter/cpp_converter.h"
#include "converter/verilog_converter.h"

#include "hal_core/netlist/decorators/boolean_function_net_decorator.h"

namespace hal
{
    namespace z3_utils
    {
        z3::expr to_z3(const BooleanFunction& bf, z3::context& context, const std::map<std::string, z3::expr>& var2expr)
        {
            /// Helper function to reduce a abstract syntax subtree to z3 expressions
            ///
            /// @param[in] node - Boolean function node.
            /// @param[in] p - Boolean function node parameters.
            /// @returns (1) status (true on success, false otherwise),
            ///          (2) SMT-LIB string representation of node and operands.
            auto reduce_to_z3 = [&context, &var2expr](const auto& node, auto&& p) -> std::tuple<bool, z3::expr> {
                if (node.get_arity() != p.size())
                {
                    return {false, z3::expr(context)};
                }

                switch (node.type)
                {
                    case BooleanFunction::NodeType::Index:
                        return {true, context.bv_val(node.index, node.size)};
                    case BooleanFunction::NodeType::Constant: {
                        // since our constants are defined as arbitrary bit-vectors,
                        // we have to concat each bit just to be on the safe side
                        auto constant = context.bv_val(node.constant.front(), 1);
                        for (u32 i = 1; i < node.constant.size(); i++)
                        {
                            const auto bit = node.constant.at(i);
                            constant       = z3::concat(context.bv_val(bit, 1), constant);
                        }
                        return {true, constant};
                    }
                    case BooleanFunction::NodeType::Variable: {
                        if (auto it = var2expr.find(node.variable); it != var2expr.end())
                        {
                            return {true, it->second};
                        }
                        return {true, context.bv_const(node.variable.c_str(), node.size)};
                    }

                    case BooleanFunction::NodeType::And:
                        return {true, p[0] & p[1]};
                    case BooleanFunction::NodeType::Or:
                        return {true, p[0] | p[1]};
                    case BooleanFunction::NodeType::Not:
                        return {true, ~p[0]};
                    case BooleanFunction::NodeType::Xor:
                        return {true, p[0] ^ p[1]};
                    case BooleanFunction::NodeType::Slice:
                        return {true, p[0].extract(p[2].get_numeral_uint(), p[1].get_numeral_uint())};
                    case BooleanFunction::NodeType::Concat:
                        return {true, z3::concat(p[0], p[1])};
                    case BooleanFunction::NodeType::Sext:
                        return {true, z3::sext(p[0], p[1].get_numeral_uint())};

                    default:
                        log_error("netlist", "Not implemented reached for nodetype {} in z3 conversion", node.type);
                        return {false, z3::expr(context)};
                }
            };

            std::vector<z3::expr> stack;
            for (const auto& node : bf.get_nodes())
            {
                std::vector<z3::expr> operands;
                std::move(stack.end() - static_cast<i64>(node.get_arity()), stack.end(), std::back_inserter(operands));
                stack.erase(stack.end() - static_cast<i64>(node.get_arity()), stack.end());

                if (auto [ok, reduction] = reduce_to_z3(node, std::move(operands)); ok)
                {
                    stack.emplace_back(reduction);
                }
                else
                {
                    return z3::expr(context);
                }
            }

            switch (stack.size())
            {
                case 1:
                    return stack.back();
                default:
                    return z3::expr(context);
            }
        }

        BooleanFunction to_hal(const z3::expr& e)
        {
            if (!e.is_bv()) 
            {
                log_error("z3_utils", "Currently only support bit vector expressions! Got {} with sort {}.", e.to_string(), e.get_sort().name().str());
                return BooleanFunction();
            }

            const u64 size = e.get_sort().bv_size();

            if (size > 64)
            {
                log_error("z3_utils", "Can only handle bit vectors with a max size of 64 bits! Got {}.", size);
                return BooleanFunction();
            }

            if (e.is_numeral())
            {
                const u64 numeral = e.get_numeral_uint64();
                return BooleanFunction::Const(numeral, size);
            }

            if (e.is_const())
            {
                auto var = e.to_string();
                return BooleanFunction::Var(var, size);
            }

            const std::string op = e.decl().name().str();
            auto num_args        = e.num_args();

            if (op == "bvnot" || op == "not")
            {
                if (num_args != 1)
                {
                    log_error("z3_utils", "For the NOT operation we only suppport exactly one operand.");
                    return BooleanFunction();
                }

                return BooleanFunction::Not(to_hal(e.arg(0)), size).get();
            }

            if (op == "bvor" || op == "or")
            {
                if (num_args < 2)
                {
                    log_error("z3_utils", "For the OR operation we require at least 2 operands.");
                    return BooleanFunction();
                }

                BooleanFunction ret = BooleanFunction::Or(to_hal(e.arg(0)), to_hal(e.arg(1)), size).get();

                for (u32 i = 2; i < num_args; i++)
                {
                    ret = BooleanFunction::Or(std::move(ret), to_hal(e.arg(i)), size).get();
                }

                return ret;
            }

            if (op == "bvand" || op == "and")
            {
                if (num_args < 2)
                {
                    log_error("z3_utils", "For the AND operation we require at least 2 operands.");
                    return BooleanFunction();
                }

                BooleanFunction ret = BooleanFunction::And(to_hal(e.arg(0)), to_hal(e.arg(1)), size).get();

                for (u32 i = 2; i < num_args; i++)
                {
                    ret = BooleanFunction::And(std::move(ret), to_hal(e.arg(i)), size).get();
                }

                return ret;
            }

            if (op == "bvxor" || op == "xor")
            {
                if (num_args < 2)
                {
                    log_error("z3_utils", "For the XOR operation we require at least 2 operands.");
                    return BooleanFunction();
                }

                BooleanFunction ret = BooleanFunction::Xor(to_hal(e.arg(0)), to_hal(e.arg(1)), size).get();

                for (u32 i = 2; i < num_args; i++)
                {
                    ret = BooleanFunction::Xor(std::move(ret), to_hal(e.arg(i)), size).get();
                }

                return ret;
            }

            /*
            if (op == "if")
            {
                if (num_args != 3)
                {
                    log_error("z3_utils", "For the IF operation we only suppport exactly three operands.");
                    return BooleanFunction();
                }

                return BooleanFunction::Ite(to_hal(e.arg(0)), to_hal(e.arg(1)), to_hal(e.arg(2)), size).get();
            }
            */

            log_error("z3_utils", "found unhandled operation {}", op);
            return BooleanFunction();
        }
    
        std::string to_smt2(const z3::expr& e)
        {
            auto s = z3::solver(e.ctx());
            s.add(e == e.ctx().bv_val(0, 1));
            return s.to_smt2();
        }

        std::string to_cpp(const z3::expr& e)
        {
            const auto converter = Cpp_Converter();
            const auto c_file    = converter.convert_z3_expr_to_func(e);

            return c_file;
        }

        std::string to_verilog(const z3::expr& e, const std::map<std::string, bool>& control_mapping)
        {
            auto converter = VerilogConverter();
            converter.set_control_mapping(control_mapping);

            const auto verilog_file = converter.convert_z3_expr_to_func(e);

            return verilog_file;
        }
        
        std::set<std::string> get_variable_names(const z3::expr& e)
        {
            std::set<std::string> var_names;

            // get inputs from smt2 string, much faster than iterating over z3 things
            const auto smt = to_smt2(e);
            
            std::istringstream iss(smt);
            for (std::string line; std::getline(iss, line);)
            {
                if (line.find("declare-fun") != std::string::npos)
                {
                    auto start_index = line.find_first_of(' ') + 1;    // variable name starts after the first space
                    auto end_index   = line.find_first_of(' ', start_index);

                    if (start_index == std::string::npos + 1 || end_index == std::string::npos)
                    {
                        log_error("z3_utils",
                                  "Some variables in line '{}' do not seem to fit in our handled format!", line);
                        continue;
                    }

                    auto var_name = line.substr(start_index, end_index - start_index);
                    var_names.insert(var_name);
                }
            }

            return var_names;
        }
        
        std::set<u32> extract_net_ids(const z3::expr& e)
        {
            return extract_net_ids(get_variable_names(e));
        }

        std::set<u32> extract_net_ids(const std::set<std::string>& variable_names)
        {
            std::set<u32> net_ids;

            for (const auto& var : variable_names)
            {
                const auto id_res = BooleanFunctionNetDecorator::get_net_id_from(var);
                if (id_res.is_error())
                {
                    log_error("z3_utils", "{}", id_res.get_error().get());
                }
                net_ids.insert(id_res.get());
            }

            return net_ids;
        } 
    
        z3::expr get_expr_in_ctx(const z3::expr& e, z3::context& ctx) 
        {
            auto expr_vec = ctx.parse_string(to_smt2(e).c_str());
            return expr_vec.back().arg(0).simplify();
        }
    }   // namespace z3_utils

    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<Z3UtilsPlugin>();
    }

    std::string Z3UtilsPlugin::get_name() const
    {
        return std::string("z3_utils");
    }

    std::string Z3UtilsPlugin::get_version() const
    {
        return std::string("0.1");
    }

    void Z3UtilsPlugin::initialize()
    {
    }

    BooleanFunction Z3UtilsPlugin::get_subgraph_function_py(const Net* n, const std::vector<Gate*>& sub_graph_gates) const
    {
        z3::context ctx;
        
        const auto res = z3_utils::get_subgraph_z3_function(sub_graph_gates, n, ctx);
        if (res.is_error())
        {
            log_error("z3_utils", "{}", res.get_error().get());
        }

        BooleanFunction bf = z3_utils::to_hal(res.get());
        // std::cout << "Got bf: " << bf.to_string() << std::endl;

        return bf;
    }
    
}    // namespace hal
