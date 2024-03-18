#include "bitwuzla_utils/symbolic_execution.h"

#include "hal_core/utilities/log.h"

namespace hal
{
    namespace SMT
    {
        namespace ConstantPropagation
        {
            /**
             * Helper function to simplify a constant AND operation.
             * 
             * @param[in] p0 - Boolean function parameter 0.
             * @param[in] p1 - Boolean function parameter 1.
             * @returns Boolean function with a simplified constant value.
             */
            Result<bitwuzla::Term> And(const bitwuzla::Term& p0, const bitwuzla::Term& p1)
            {
                const auto res = utils::wrapped_stoull(p0.value<std::string>(), 2);
                if (res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "cannot tranlate term to hal Boolean function: failed to extract value");
                }
                const auto res2 = utils::wrapped_stoull(p1.value<std::string>(), 2);
                if (res2.is_error())
                {
                    return ERR_APPEND(res.get_error(), "cannot tranlate term to hal Boolean function: failed to extract value");
                }
                auto simplified_val = res.get() & res2.get();
                return OK(bitwuzla::mk_bv_value_uint64(bitwuzla::mk_bv_sort(p0.sort().bv_size()), simplified_val));
            }

            /**
             * Helper function to simplify a constant Equal operation.
             * 
             * @param[in] p0 - Boolean function parameter 0.
             * @param[in] p1 - Boolean function parameter 1.
             * @returns Boolean function with a simplified constant value.
             */
            Result<bitwuzla::Term> Equal(const bitwuzla::Term& p0, const bitwuzla::Term& p1)
            {
                if (p0.sort().is_bv() & p1.sort().is_bv())
                {
                    const auto res = utils::wrapped_stoull(p0.value<std::string>(), 2);
                    if (res.is_error())
                    {
                        return ERR_APPEND(res.get_error(), "cannot tranlate term to hal Boolean function: failed to extract value");
                    }
                    const auto res2 = utils::wrapped_stoull(p1.value<std::string>(), 2);
                    if (res2.is_error())
                    {
                        return ERR_APPEND(res.get_error(), "cannot tranlate term to hal Boolean function: failed to extract value");
                    }
                    auto simplified_val = res.get() == res2.get();
                    if (res.get() == res2.get())
                        return OK(bitwuzla::mk_true());
                    return OK(bitwuzla::mk_false());
                }
                else if (p0.sort().is_bool() & p1.sort().is_bool())
                {
                    if (p0.is_true() == p1.is_true())
                        return OK(bitwuzla::mk_true());
                    return OK(bitwuzla::mk_false());
                }
                return ERR("Trying to compare 2 Values of different Sort in Equal");
            }

            /**
             * Helper function to simplify a constant OR operation.
             * 
             * @param[in] p0 - Boolean function parameter 0.
             * @param[in] p1 - Boolean function parameter 1.
             * @returns Boolean function with a simplified constant value.
             */
            Result<bitwuzla::Term> Or(const bitwuzla::Term& p0, const bitwuzla::Term& p1)
            {
                const auto res = utils::wrapped_stoull(p0.value<std::string>(), 2);
                if (res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "cannot tranlate term to hal Boolean function: failed to extract value");
                }
                const auto res2 = utils::wrapped_stoull(p1.value<std::string>(), 2);
                if (res2.is_error())
                {
                    return ERR_APPEND(res.get_error(), "cannot tranlate term to hal Boolean function: failed to extract value");
                }
                auto simplified_val = res.get() | res2.get();
                return OK(bitwuzla::mk_bv_value_uint64(bitwuzla::mk_bv_sort(p0.sort().bv_size()), simplified_val));
            }

            /**
             * Helper function to simplify a constant NOT operation.
             * 
             * @param[in] p - Boolean function parameter.
             * @returns Boolean function with a simplified constant value.
             */
            Result<bitwuzla::Term> Not(const bitwuzla::Term& p)
            {
                const auto res = utils::wrapped_stoull(p.value<std::string>(), 2);
                if (res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "cannot tranlate term to hal Boolean function: failed to extract value");
                }
                return OK(bitwuzla::mk_bv_value_uint64(bitwuzla::mk_bv_sort(p.sort().bv_size()), !res.get()));
            }

            /**
             * Helper function to simplify a constant XOR operation.
             * 
             * @param[in] p0 - Boolean function parameter 0.
             * @param[in] p1 - Boolean function parameter 1.
             * @returns Boolean function with a simplified constant value.
             */
            Result<bitwuzla::Term> Xor(const bitwuzla::Term& p0, const bitwuzla::Term& p1)
            {
                const auto res = utils::wrapped_stoull(p0.value<std::string>(), 2);
                if (res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "cannot tranlate term to hal Boolean function: failed to extract value");
                }
                const auto res2 = utils::wrapped_stoull(p1.value<std::string>(), 2);
                if (res2.is_error())
                {
                    return ERR_APPEND(res.get_error(), "cannot tranlate term to hal Boolean function: failed to extract value");
                }
                auto simplified_val = res.get() ^ res2.get();
                return OK(bitwuzla::mk_bv_value_uint64(bitwuzla::mk_bv_sort(p0.sort().bv_size()), simplified_val));
            }

            /**
             * Helper function to simplify a constant ADD operation.
             * 
             * @param[in] p0 - Boolean function parameter 0.
             * @param[in] p1 - Boolean function parameter 1.
             * @returns Boolean function with a simplified constant value.
             */
            Result<bitwuzla::Term> Add(const bitwuzla::Term& p0, const bitwuzla::Term& p1)
            {
                const auto res = utils::wrapped_stoull(p0.value<std::string>(), 2);
                if (res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "cannot tranlate term to hal Boolean function: failed to extract value");
                }
                const auto res2 = utils::wrapped_stoull(p1.value<std::string>(), 2);
                if (res2.is_error())
                {
                    return ERR_APPEND(res.get_error(), "cannot tranlate term to hal Boolean function: failed to extract value");
                }
                auto simplified_val = res.get() + res2.get();
                return OK(bitwuzla::mk_bv_value_uint64(bitwuzla::mk_bv_sort(p0.sort().bv_size()), simplified_val));
            }

            /**
             * Helper function to simplify a constant SUB operation.
             * 
             * @param[in] p0 - Boolean function parameter 0.
             * @param[in] p1 - Boolean function parameter 1.
             * @returns Boolean function with a simplified constant value.
             */
            Result<bitwuzla::Term> Sub(const bitwuzla::Term& p0, const bitwuzla::Term& p1)
            {
                const auto res = utils::wrapped_stoull(p0.value<std::string>(), 2);
                if (res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "cannot tranlate term to hal Boolean function: failed to extract value");
                }
                const auto res2 = utils::wrapped_stoull(p1.value<std::string>(), 2);
                if (res2.is_error())
                {
                    return ERR_APPEND(res.get_error(), "cannot tranlate term to hal Boolean function: failed to extract value");
                }
                auto simplified_val = res.get() - res2.get();
                return OK(bitwuzla::mk_bv_value_uint64(bitwuzla::mk_bv_sort(p0.sort().bv_size()), simplified_val));
            }

            /**
             * Helper function to simplify a constant MUL operation.
             * 
             * @param[in] p0 - Boolean function parameter 0.
             * @param[in] p1 - Boolean function parameter 1.
             * @returns Boolean function with a simplified constant value.
             */
            Result<bitwuzla::Term> Mul(const bitwuzla::Term& p0, const bitwuzla::Term& p1)
            {
                const auto res = utils::wrapped_stoull(p0.value<std::string>(), 2);
                if (res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "cannot tranlate term to hal Boolean function: failed to extract value");
                }
                const auto res2 = utils::wrapped_stoull(p1.value<std::string>(), 2);
                if (res2.is_error())
                {
                    return ERR_APPEND(res.get_error(), "cannot tranlate term to hal Boolean function: failed to extract value");
                }
                auto simplified_val = res.get() * res2.get();
                return OK(bitwuzla::mk_bv_value_uint64(bitwuzla::mk_bv_sort(p0.sort().bv_size()), simplified_val));
            }

            /**
             * Helper function to simplify a constant SLE operation.
             * 
             * @param[in] p0 - Boolean function parameter 0.
             * @param[in] p1 - Boolean function parameter 1.
             * @returns Boolean function with a simplified constant value.
             */
            Result<bitwuzla::Term> Sle(const bitwuzla::Term& p0, const bitwuzla::Term& p1)
            {
                u64 res = 0;
                try
                {
                    res = std::stoll(p0.value<std::string>(), nullptr, 2);
                }
                catch (const std::invalid_argument& e)
                {
                    return ERR("could not get u64 from string");
                }
                catch (const std::out_of_range& e)
                {
                    return ERR("could not get u64 from string");
                }
                u64 res2 = 0;
                try
                {
                    res2 = std::stoll(p0.value<std::string>(), nullptr, 2);
                }
                catch (const std::invalid_argument& e)
                {
                    return ERR("could not get u64 from string");
                }
                catch (const std::out_of_range& e)
                {
                    return ERR("could not get u64 from string");
                }

                if (res <= res2)
                {
                    return OK(bitwuzla::mk_bv_one(bitwuzla::mk_bv_sort(1)));
                }
                return OK(bitwuzla::mk_bv_zero(bitwuzla::mk_bv_sort(1)));
            }

            /**
             * Helper function to simplify a constant SLT operation.
             * 
             * @param[in] p0 - Boolean function parameter 0.
             * @param[in] p1 - Boolean function parameter 1.
             * @returns Boolean function with a simplified constant value.
             */
            Result<bitwuzla::Term> Slt(const bitwuzla::Term& p0, const bitwuzla::Term& p1)
            {
                u64 res = 0;
                try
                {
                    res = std::stoll(p0.value<std::string>(), nullptr, 2);
                }
                catch (const std::invalid_argument& e)
                {
                    return ERR("could not get u64 from string");
                }
                catch (const std::out_of_range& e)
                {
                    return ERR("could not get u64 from string");
                }
                u64 res2 = 0;
                try
                {
                    res2 = std::stoll(p0.value<std::string>(), nullptr, 2);
                }
                catch (const std::invalid_argument& e)
                {
                    return ERR("could not get u64 from string");
                }
                catch (const std::out_of_range& e)
                {
                    return ERR("could not get u64 from string");
                }
                if (res < res2)
                {
                    return OK(bitwuzla::mk_bv_one(bitwuzla::mk_bv_sort(1)));
                }
                return OK(bitwuzla::mk_bv_zero(bitwuzla::mk_bv_sort(1)));
            }

            /**
             * Helper function to simplify a constant ULE operation.
             * 
             * @param[in] p0 - Boolean function parameter 0.
             * @param[in] p1 - Boolean function parameter 1.
             * @returns Boolean function with a simplified constant value.
             */
            Result<bitwuzla::Term> Ule(const bitwuzla::Term& p0, const bitwuzla::Term& p1)
            {
                const auto res = utils::wrapped_stoull(p0.value<std::string>(), 2);
                if (res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "cannot tranlate term to hal Boolean function: failed to extract value");
                }
                const auto res2 = utils::wrapped_stoull(p1.value<std::string>(), 2);
                if (res2.is_error())
                {
                    return ERR_APPEND(res.get_error(), "cannot tranlate term to hal Boolean function: failed to extract value");
                }
                if (res.get() <= res2.get())
                {
                    return OK(bitwuzla::mk_bv_one(bitwuzla::mk_bv_sort(1)));
                }
                return OK(bitwuzla::mk_bv_zero(bitwuzla::mk_bv_sort(1)));
                ;
            }

            /**
             * Helper function to simplify a constant ULT operation.
             * 
             * @param[in] p0 - Boolean function parameter 0.
             * @param[in] p1 - Boolean function parameter 1.
             * @returns Boolean function with a simplified constant value.
             */
            Result<bitwuzla::Term> Ult(const bitwuzla::Term& p0, const bitwuzla::Term& p1)
            {
                const auto res = utils::wrapped_stoull(p0.value<std::string>(), 2);
                if (res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "cannot tranlate term to hal Boolean function: failed to extract value");
                }
                const auto res2 = utils::wrapped_stoull(p1.value<std::string>(), 2);
                if (res2.is_error())
                {
                    return ERR_APPEND(res.get_error(), "cannot tranlate term to hal Boolean function: failed to extract value");
                }
                if (res.get() < res2.get())
                {
                    return OK(bitwuzla::mk_bv_one(bitwuzla::mk_bv_sort(1)));
                }
                return OK(bitwuzla::mk_bv_zero(bitwuzla::mk_bv_sort(1)));
            }

            /**
             * Helper function to simplify a constant ITE operation.
             * 
             * @param[in] p0 - Boolean function parameter 0.
             * @param[in] p1 - Boolean function parameter 1.
             * @param[in] p2 - Boolean function parameter 2.
             * @returns Boolean function with a simplified constant value.
             */
            Result<bitwuzla::Term> Ite(const bitwuzla::Term& p0, const bitwuzla::Term& p1, const bitwuzla::Term& p2)
            {
                if (p0.is_true())
                {
                    return OK(p1);
                }
                return OK(p2);
            }
            /**
             * Helper function to simplify a constant CONCAT operation.
             * 
             * @param[in] p0 - Boolean function parameter 0.
             * @param[in] p1 - Boolean function parameter 1.
             * @returns Boolean function with a simplified constant value.
             */
            Result<bitwuzla::Term> CONCAT(const bitwuzla::Term& p0, const bitwuzla::Term& p1)
            {
                const auto res = utils::wrapped_stoull(p0.value<std::string>(), 2);
                if (res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "cannot tranlate term to hal Boolean function: failed to extract value");
                }
                const auto res2 = utils::wrapped_stoull(p1.value<std::string>(), 2);
                if (res2.is_error())
                {
                    return ERR_APPEND(res.get_error(), "cannot tranlate term to hal Boolean function: failed to extract value");
                }
                auto simplified_val = res.get() << p1.sort().bv_size() + res2.get();
                return OK(bitwuzla::mk_bv_value_uint64(bitwuzla::mk_bv_sort(p0.sort().bv_size()), simplified_val));
            }
            Result<bitwuzla::Term> EQUAL(const bitwuzla::Term& p0, const bitwuzla::Term& p1)
            {
                const auto res = utils::wrapped_stoull(p0.value<std::string>(), 2);
                if (res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "cannot tranlate term to hal Boolean function: failed to extract value");
                }
                const auto res2 = utils::wrapped_stoull(p1.value<std::string>(), 2);
                if (res2.is_error())
                {
                    return ERR_APPEND(res.get_error(), "cannot tranlate term to hal Boolean function: failed to extract value");
                }
                auto simplified_val = res.get() << p1.sort().bv_size() + res2.get();
                return OK(bitwuzla::mk_bv_value_uint64(bitwuzla::mk_bv_sort(p0.sort().bv_size()), simplified_val));
            }

            Result<bitwuzla::Term> BV_ZERO_EXTEND(const bitwuzla::Term& p, const u64 size)
            {
                const auto res = utils::wrapped_stoull(p.value<std::string>(), 2);
                if (res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "cannot translate term to hal Boolean function: failed to extract value");
                }
                return OK(bitwuzla::mk_bv_value_uint64(bitwuzla::mk_bv_sort(p.sort().bv_size() + size), res.get()));
            }
            Result<bitwuzla::Term> BV_SIGN_EXTEND(const bitwuzla::Term& p, const u64 size)
            {
                u64 res = 0;
                try
                {
                    res = std::stoll(p.value<std::string>(), nullptr, 2);
                }
                catch (const std::invalid_argument& e)
                {
                    return ERR("could not get u64 from string");
                }
                catch (const std::out_of_range& e)
                {
                    return ERR("could not get u64 from string");
                }
                return OK(bitwuzla::mk_bv_value_uint64(bitwuzla::mk_bv_sort(p.sort().bv_size() + size), res));
            }
            Result<bitwuzla::Term> BV_EXTRACT(const bitwuzla::Term& p, const u64 start, const u64 end)
            {
                std::string new_str = p.value<std::string>();
                new_str.insert(new_str.begin(), p.sort().bv_size() - new_str.size(), '0');
                new_str        = new_str.substr(start, end);
                const auto res = utils::wrapped_stoull(new_str, 2);
                if (res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "cannot translate term to hal Boolean function: failed to extract value");
                }
                return OK(bitwuzla::mk_bv_value_uint64(bitwuzla::mk_bv_sort(end - start), res.get()));
            }

        }    // namespace ConstantPropagation

        // SymbolicExecution::SymbolicExecution(const std::vector<bitwuzla::Term>& variables) : state(SymbolicState(variables))
        // {
        // }

        namespace
        {
            std::vector<bitwuzla::Term> get_terms_dfs(const bitwuzla::Term& function, std::map<u64, bitwuzla::Term>& id_to_term)
            {
                std::vector<bitwuzla::Term> result;
                if (function.num_children() > 0)
                {
                    auto children = function.children();
                    for (const auto cur_child : children)
                    {
                        auto child_res = get_terms_dfs(cur_child, id_to_term);
                        result.insert(result.end(), child_res.begin(), child_res.end());
                    }
                    result.push_back(function);
                }
                else
                {
                    if (id_to_term.find(function.id()) == id_to_term.end())
                    {
                        id_to_term[function.id()] = function;
                    }
                }
                return result;
            }
        }    // namespace
        Result<bitwuzla::Term> SymbolicExecution::evaluate(const bitwuzla::Term& function, std::map<u64, bitwuzla::Term> id_to_term) const
        {
            //std::map<u64, bitwuzla::Term> id_to_term;
            std::vector<bitwuzla::Term> stack = get_terms_dfs(function, id_to_term);
            bitwuzla::Term result;
            for (const auto& node : stack)
            {
                //std::move(stack.end() - static_cast<i64>(node.num_children()), stack.end(), std::back_inserter(parameters));
                //stack.erase(stack.end() - static_cast<i64>(node.num_children()), stack.end());

                if (id_to_term.find(node.id()) == id_to_term.end())
                {
                    //already simplified that term
                    continue;
                }
                else
                {
                    std::vector<bitwuzla::Term> parameters;
                    for (auto cur_child : node.children())
                    {
                        parameters.push_back(id_to_term[cur_child.id()]);
                    }
                    if (auto simplified = this->simplify(node, parameters); simplified.is_ok())
                    {
                        id_to_term[node.id()] = simplified.get();
                        result                = simplified.get();
                    }
                    else
                    {
                        return ERR_APPEND(simplified.get_error(), "could not evaluate Boolean function within symbolic state: simplification failed");
                    }
                }
            }

            return OK(result);
        }

        // Result<std::monostate> SymbolicExecution::evaluate(const Constraint& constraint)
        // {
        //     if (constraint.is_assignment())
        //     {
        //         const auto& assignment = constraint.get_assignment().get();
        //         if (auto res = this->evaluate(assignment->first).map<std::monostate>([&](auto&& rhs) -> Result<std::monostate> {
        //                 this->state.set(assignment->second.clone(), std::move(rhs));
        //                 return OK({});
        //             });
        //             res.is_error())
        //         {
        //             return ERR_APPEND(res.get_error(), "could not to evaluate assignment constraint within the symbolic state: evaluation failed");
        //         }
        //         else
        //         {
        //             return OK({});
        //         }
        //     }
        //     else
        //     {
        //         const auto& function = constraint.get_function().get();
        //         auto node_type       = function->get_top_level_node().type;
        //         if (!(node_type == bitwuzla::Term::Eq || node_type == bitwuzla::Term::Slt || node_type == bitwuzla::Term::Sle
        //               || node_type == bitwuzla::Term::Ult || node_type == bitwuzla::Term::Ule))
        //         {
        //             return ERR("invalid node type in function '" + function->to_string() + "'");
        //         }
        //         if (auto res = this->evaluate(*function); res.is_error())
        //         {
        //             return ERR_APPEND(res.get_error(), "could not to evaluate function constraint within the symbolic state: evaluation failed");
        //         }
        //         else
        //         {
        //             return OK({});
        //         }
        //     }
        // }

        std::vector<bitwuzla::Term> SymbolicExecution::normalize(std::vector<bitwuzla::Term>&& p)
        {
            if (p.size() <= 1ul)
            {
                return std::move(p);
            }

            std::sort(p.begin(), p.end(), [](const auto& lhs, const auto& rhs) {
                if (lhs.kind() == rhs.kind())
                {
                    return lhs < rhs;
                }
                return rhs.is_value() || rhs.is_bool();
            });
            return std::move(p);
        }

        namespace
        {
            /**
             * Helper function to check whether one of the two functions is just the other function negated.
             */
            bool is_x_not_y(const bitwuzla::Term& x, const bitwuzla::Term& y)
            {
                if (x.kind() == bitwuzla::Kind::NOT)
                {
                    if (is_x_y(x.children()[0], y))
                    {
                        return true;
                    }
                }
                if (x.kind() == bitwuzla::Kind::BV_NOT)
                {
                    if (is_x_y(x.children()[0], y))
                    {
                        return true;
                    }
                }
                if (y.kind() == bitwuzla::Kind::NOT)
                {
                    if (is_x_y(y.children()[0], x))
                    {
                        return true;
                    }
                }
                if (y.kind() == bitwuzla::Kind::BV_NOT)
                {
                    if (is_x_y(y.children()[0], x))
                    {
                        return true;
                    }
                }
            }
            bool is_x_y(const bitwuzla::Term& x, const bitwuzla::Term& y)
            {
                //TODO implement this in a more detailed way maybe?
                if (x.id() == y.id())
                {
                    return true;
                }
                return x.str() == y.str();
            }

            bool is_commutative(const bitwuzla::Term& x)
            {
                return (x.kind() == bitwuzla::Kind::BV_AND) || (x.kind() == bitwuzla::Kind::BV_OR) || (x.kind() == bitwuzla::Kind::BV_XOR) || (x.kind() == bitwuzla::Kind::BV_ADD)
                       || (x.kind() == bitwuzla::Kind::BV_MUL) || (x.kind() == bitwuzla::Kind::EQUAL) || (x.kind() == bitwuzla::Kind::AND) || (x.kind() == bitwuzla::Kind::OR)
                       || (x.kind() == bitwuzla::Kind::XOR);
            }

            bool is_smaller(const bitwuzla::Term& x, const bitwuzla::Term& y)
            {
                if (x.str().length() < x.str().length())
                {
                    return true;
                }
                if (x.str().length() > y.str().length())
                {
                    return false;
                }

                return x.str() < y.str();
            }

        }    // namespace

        Result<bitwuzla::Term> SymbolicExecution::simplify(const bitwuzla::Term& node, std::vector<bitwuzla::Term>& p)
        {
            if (!p.empty() && std::all_of(p.begin(), p.end(), [](const auto& function) { return function.is_bool() || function.is_value(); }))
            {
                if (auto res = SymbolicExecution::constant_propagation(node, p); res.is_error())
                {
                    return ERR_APPEND(res.get_error(), "could not simplify sub-expression in abstract syntax tree: constant propagation failed");
                }
                else
                {
                    return res;
                }
            }

            //TODO
            if (is_commutative(node))
            {
                p = SymbolicExecution::normalize(std::move(p));
            }

            /// # Developer Note
            /// Since the simplify() function vists the abstract syntax tree of the
            /// Boolean function, we want to prevent the use of any recursive call
            /// to the simplify() function of a sub-expression tree. Hence, use the
            /// simplify() function with care, as otherwise run-time may explode :)

            switch (node.kind())
            {
                case bitwuzla::Kind::CONSTANT: {
                    return OK(node);
                }
                case bitwuzla::Kind::VALUE: {
                    return OK(node);
                }
                case bitwuzla::Kind::VARIABLE: {
                    return OK(node);
                }
                case bitwuzla::Kind::BV_AND: {
                    // X & 0   =>   0
                    if (p[1].is_bv_value_zero())
                    {
                        return OK(bitwuzla::mk_bv_zero(bitwuzla::mk_bv_sort(p[1].sort().bv_size())));
                    }
                    // X & 1  =>   X
                    if (p[1].is_bv_value_ones())
                    {
                        return OK(p[0]);
                    }
                    // X & X   =>   X
                    if (is_x_y(p[0], p[1]))
                    {
                        return OK(p[0]);
                    }
                    // X & ~X   =>   0
                    if (is_x_not_y(p[0], p[1]))
                    {
                        return OK(bitwuzla::mk_bv_zero(bitwuzla::mk_bv_sort(p[1].sort().bv_size())));
                    }

                    if ((p[0].kind() == bitwuzla::Kind::BV_OR) && (p[1].kind() == bitwuzla::Kind::BV_OR))
                    {
                        auto p0_parameter = p[0].children();
                        auto p1_parameter = p[1].children();

                        // (X | Y) & (X | Z)   =>   X | (Y & Z)
                        if (is_x_y(p0_parameter[0], p1_parameter[0]))
                        {
                            return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_OR, {p0_parameter[0], bitwuzla::mk_term(bitwuzla::Kind::BV_AND, {p0_parameter[1], p1_parameter[1]})}));
                        }
                        // (X | Y) & (Z | X)   =>   X | (Y & Z)
                        if (is_x_y(p0_parameter[0], p1_parameter[1]))
                        {
                            return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_OR, {p0_parameter[0], bitwuzla::mk_term(bitwuzla::Kind::BV_AND, {p0_parameter[1], p1_parameter[0]})}));
                        }

                        // (X | Y) & (Y | Z)   =>   Y | (X & Z)
                        if (is_x_y(p0_parameter[1], p1_parameter[0]))
                        {
                            return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_OR, {p0_parameter[1], bitwuzla::mk_term(bitwuzla::Kind::BV_AND, {p0_parameter[0], p1_parameter[1]})}));
                        }
                        // (X | Y) & (Z | Y)   =>   Y | (X & Z)
                        if (is_x_y(p0_parameter[1], p1_parameter[1]))
                        {
                            return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_OR, {p0_parameter[1], bitwuzla::mk_term(bitwuzla::Kind::BV_AND, {p0_parameter[0], p1_parameter[0]})}));
                        }
                    }

                    if (p[1].kind() == bitwuzla::Kind::BV_AND)
                    {
                        auto p1_parameter = p[1].children();
                        // X & (X & Y)   =>   (X & Y)
                        if (is_x_y(p[0], p1_parameter[1]))
                        {
                            return OK(p[1]);
                        }
                        // X & (Y & X)   =>   (Y & X)
                        if (is_x_y(p[0], p1_parameter[0]))
                        {
                            return OK(p[1]);
                        }

                        // X & (~X & Y)   =>   0
                        if (is_x_not_y(p1_parameter[0], p[0]))
                        {
                            return OK(bitwuzla::mk_bv_zero(bitwuzla::mk_bv_sort(p[1].sort().bv_size())));
                        }
                        // X & (Y & ~X)   =>   0
                        if (is_x_not_y(p1_parameter[1], p[0]))
                        {
                            return OK(bitwuzla::mk_bv_zero(bitwuzla::mk_bv_sort(p[1].sort().bv_size())));
                        }
                    }

                    if (p[1].kind() == bitwuzla::Kind::BV_OR)
                    {
                        auto p1_parameter = p[1].children();

                        // X & (X | Y)   =>   X
                        if (is_x_y(p1_parameter[0], p[0]))
                        {
                            return OK(p[0]);
                        }
                        // X & (Y | X)   =>   X
                        if (is_x_y(p1_parameter[1], p[0]))
                        {
                            return OK(p[0]);
                        }
                        // X & (~X | Y)   =>  X & Y
                        if (is_x_not_y(p1_parameter[0], p[0]))
                        {
                            return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_AND, {p[0], p1_parameter[1]}));
                        }
                        // X & (Y | ~X)   =>  X & Y
                        if (is_x_not_y(p1_parameter[1], p[0]))
                        {
                            return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_AND, {p[0], p1_parameter[0]}));
                        }
                    }

                    if (p[0].kind() == bitwuzla::Kind::BV_AND)
                    {
                        auto p0_parameter = p[0].children();

                        // (X & Y) & X   =>    X & Y
                        if (is_x_y(p0_parameter[0], p[1]))
                        {
                            return OK(p[0]);
                        }

                        // (Y & X) & X   =>    Y & X
                        if (is_x_y(p0_parameter[1], p[1]))
                        {
                            return OK(p[0]);
                        }
                        // (~X & Y) & X   =>   0
                        if (is_x_not_y(p0_parameter[0], p[1]))
                        {
                            return OK(bitwuzla::mk_bv_zero(bitwuzla::mk_bv_sort(p[1].sort().bv_size())));
                        }
                        // (Y & ~X) & X   =>   0
                        if (is_x_not_y(p0_parameter[1], p[1]))
                        {
                            return OK(bitwuzla::mk_bv_zero(bitwuzla::mk_bv_sort(p[1].sort().bv_size())));
                        }
                    }

                    if (p[0].kind() == (bitwuzla::Kind::BV_OR))
                    {
                        auto p0_parameter = p[0].children();

                        // (X | Y) & X    =>   X
                        if (is_x_y(p0_parameter[0], p[1]))
                        {
                            return OK(p[1]);
                        }
                        // (Y | X) & X    =>   X
                        if (is_x_y(p0_parameter[1], p[1]))
                        {
                            return OK(p[1]);
                        }
                        // (~X | Y) & X   =>   X & Y
                        if (is_x_not_y(p0_parameter[0], p[1]))
                        {
                            return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_AND, {p[1], p0_parameter[1]}));
                        }
                        // (Y | ~X) & X   =>   X & Y
                        if (is_x_not_y(p0_parameter[1], p[1]))
                        {
                            return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_AND, {p[1], p0_parameter[0]}));
                        }
                    }
                    return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_AND, {p[0], p[1]}));
                }
                case bitwuzla::Kind::AND: {
                    // X & 0   =>   0
                    if (p[1].is_false())
                    {
                        return OK(bitwuzla::mk_false());
                    }
                    // X & 1  =>   X
                    if (p[1].is_true())
                    {
                        return OK(p[0]);
                    }
                    // X & X   =>   X
                    if (is_x_y(p[0], p[1]))
                    {
                        return OK(p[0]);
                    }
                    // X & ~X   =>   0
                    if (is_x_not_y(p[0], p[1]))
                    {
                        return OK(bitwuzla::mk_false());
                    }

                    if ((p[0].kind() == bitwuzla::Kind::OR) && (p[1].kind() == bitwuzla::Kind::OR))
                    {
                        auto p0_parameter = p[0].children();
                        auto p1_parameter = p[1].children();

                        // (X | Y) & (X | Z)   =>   X | (Y & Z)
                        if (is_x_y(p0_parameter[0], p1_parameter[0]))
                        {
                            return OK(bitwuzla::mk_term(bitwuzla::Kind::OR, {p0_parameter[0], bitwuzla::mk_term(bitwuzla::Kind::AND, {p0_parameter[1], p1_parameter[1]})}));
                        }
                        // (X | Y) & (Z | X)   =>   X | (Y & Z)
                        if (is_x_y(p0_parameter[0], p1_parameter[1]))
                        {
                            return OK(bitwuzla::mk_term(bitwuzla::Kind::OR, {p0_parameter[0], bitwuzla::mk_term(bitwuzla::Kind::AND, {p0_parameter[1], p1_parameter[0]})}));
                        }

                        // (X | Y) & (Y | Z)   =>   Y | (X & Z)
                        if (is_x_y(p0_parameter[1], p1_parameter[0]))
                        {
                            return OK(bitwuzla::mk_term(bitwuzla::Kind::OR, {p0_parameter[1], bitwuzla::mk_term(bitwuzla::Kind::AND, {p0_parameter[0], p1_parameter[1]})}));
                        }
                        // (X | Y) & (Z | Y)   =>   Y | (X & Z)
                        if (is_x_y(p0_parameter[1], p1_parameter[1]))
                        {
                            return OK(bitwuzla::mk_term(bitwuzla::Kind::OR, {p0_parameter[1], bitwuzla::mk_term(bitwuzla::Kind::AND, {p0_parameter[0], p1_parameter[0]})}));
                        }
                    }

                    if (p[1].kind() == bitwuzla::Kind::AND)
                    {
                        auto p1_parameter = p[1].children();
                        // X & (X & Y)   =>   (X & Y)
                        if (is_x_y(p[0], p1_parameter[1]))
                        {
                            return OK(p[1]);
                        }
                        // X & (Y & X)   =>   (Y & X)
                        if (is_x_y(p[0], p1_parameter[0]))
                        {
                            return OK(p[1]);
                        }

                        // X & (~X & Y)   =>   0
                        if (is_x_not_y(p1_parameter[0], p[0]))
                        {
                            return OK(bitwuzla::mk_false());
                        }
                        // X & (Y & ~X)   =>   0
                        if (is_x_not_y(p1_parameter[1], p[0]))
                        {
                            return OK(bitwuzla::mk_false());
                        }
                    }

                    if (p[1].kind() == bitwuzla::Kind::OR)
                    {
                        auto p1_parameter = p[1].children();

                        // X & (X | Y)   =>   X
                        if (is_x_y(p1_parameter[0], p[0]))
                        {
                            return OK(p[0]);
                        }
                        // X & (Y | X)   =>   X
                        if (is_x_y(p1_parameter[1], p[0]))
                        {
                            return OK(p[0]);
                        }
                        // X & (~X | Y)   =>  X & Y
                        if (is_x_not_y(p1_parameter[0], p[0]))
                        {
                            return OK(bitwuzla::mk_term(bitwuzla::Kind::AND, {p[0], p1_parameter[1]}));
                        }
                        // X & (Y | ~X)   =>  X & Y
                        if (is_x_not_y(p1_parameter[1], p[0]))
                        {
                            return OK(bitwuzla::mk_term(bitwuzla::Kind::AND, {p[0], p1_parameter[0]}));
                        }
                    }

                    if (p[0].kind() == bitwuzla::Kind::AND)
                    {
                        auto p0_parameter = p[0].children();

                        // (X & Y) & X   =>    X & Y
                        if (is_x_y(p0_parameter[0], p[1]))
                        {
                            return OK(p[0]);
                        }

                        // (Y & X) & X   =>    Y & X
                        if (is_x_y(p0_parameter[1], p[1]))
                        {
                            return OK(p[0]);
                        }
                        // (~X & Y) & X   =>   0
                        if (is_x_not_y(p0_parameter[0], p[1]))
                        {
                            return OK(bitwuzla::mk_false());
                        }
                        // (Y & ~X) & X   =>   0
                        if (is_x_not_y(p0_parameter[1], p[1]))
                        {
                            return OK(bitwuzla::mk_false());
                        }
                    }

                    if (p[0].kind() == (bitwuzla::Kind::OR))
                    {
                        auto p0_parameter = p[0].children();

                        // (X | Y) & X    =>   X
                        if (is_x_y(p0_parameter[0], p[1]))
                        {
                            return OK(p[1]);
                        }
                        // (Y | X) & X    =>   X
                        if (is_x_y(p0_parameter[1], p[1]))
                        {
                            return OK(p[1]);
                        }
                        // (~X | Y) & X   =>   X & Y
                        if (is_x_not_y(p0_parameter[0], p[1]))
                        {
                            return OK(bitwuzla::mk_term(bitwuzla::Kind::AND, {p[1], p0_parameter[1]}));
                        }
                        // (Y | ~X) & X   =>   X & Y
                        if (is_x_not_y(p0_parameter[1], p[1]))
                        {
                            return OK(bitwuzla::mk_term(bitwuzla::Kind::AND, {p[1], p0_parameter[0]}));
                        }
                    }
                    return OK(bitwuzla::mk_term(bitwuzla::Kind::AND, {p[0], p[1]}));
                }
                case bitwuzla::Kind::NOT: {
                    if (p[0].kind() == (bitwuzla::Kind::NOT))
                    {
                        return OK(p[0].children()[0]);
                    }
                    return OK(bitwuzla::mk_term(bitwuzla::Kind::NOT, {p[0]}));
                }
                case bitwuzla::Kind::BV_NOT: {
                    // ~~X   =>   X
                    if (p[0].kind() == (bitwuzla::Kind::BV_NOT))
                    {
                        return OK(p[0].children()[0]);
                    }

                    // ~(~X & ~Y)   =>   X | Y
                    if (p[0].kind() == (bitwuzla::Kind::BV_AND))
                    {
                        auto p0_parameter = p[0].children();
                        if ((p0_parameter[0].kind() == (bitwuzla::Kind::BV_NOT)) && (p0_parameter[1].kind() == bitwuzla::Kind::BV_NOT))
                        {
                            return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_OR, {p0_parameter[0].children()[0], p0_parameter[1].children()[0]}));
                        }
                    }

                    // ~(~X | ~Y)   =>   X & Y
                    if (p[0].kind() == (bitwuzla::Kind::BV_OR))
                    {
                        auto p0_parameter = p[0].children();
                        if ((p0_parameter[0].kind() == (bitwuzla::Kind::BV_NOT)) && (p0_parameter[1].kind() == bitwuzla::Kind::BV_NOT))
                        {
                            return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_AND, {p0_parameter[0].children()[0], p0_parameter[1].children()[0]}));
                        }
                    }

                    // ~(X | Y)   =>   ~X & ~Y
                    if (p[0].kind() == (bitwuzla::Kind::BV_OR))
                    {
                        auto p0_parameter = p[0].children();

                        return OK(
                            bitwuzla::mk_term(bitwuzla::Kind::BV_AND, {bitwuzla::mk_term(bitwuzla::Kind::BV_NOT, {p0_parameter[0]}), bitwuzla::mk_term(bitwuzla::Kind::BV_NOT, {p0_parameter[1]})}));
                    }

                    // ~(X & Y)   =>   ~X | ~Y
                    if (p[0].kind() == (bitwuzla::Kind::BV_AND))
                    {
                        auto p0_parameter = p[0].children();
                        return OK(
                            bitwuzla::mk_term(bitwuzla::Kind::BV_OR, {bitwuzla::mk_term(bitwuzla::Kind::BV_NOT, {p0_parameter[0]}), bitwuzla::mk_term(bitwuzla::Kind::BV_NOT, {p0_parameter[1]})}));
                    }
                    return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_NOT, {p[0]}));
                }

                case bitwuzla::Kind::BV_OR: {
                    // X | 0   =>   X
                    if (p[1].is_bv_value_zero())
                    {
                        return OK(p[0]);
                    }

                    // X | 1   =>   1
                    if (p[1].is_bv_value_ones())
                    {
                        return OK(p[1]);
                    }

                    // X | X   =>   X
                    if (is_x_y(p[0], p[1]))
                    {
                        return OK(p[0]);
                    }

                    // X | ~X   =>   111...1
                    if (is_x_not_y(p[0], p[1]))
                    {
                        return OK(bitwuzla::mk_bv_ones(bitwuzla::mk_bv_sort(p[1].sort().bv_size())));
                    }

                    if (p[0].kind() == (bitwuzla::Kind::BV_AND) && p[1].kind() == (bitwuzla::Kind::BV_AND))
                    {
                        auto p0_parameter = p[0].children();
                        auto p1_parameter = p[1].children();

                        // (X & Y) | (X & Z)    => X & (Y | Z)
                        if (is_x_y(p0_parameter[0], p1_parameter[0]))
                        {
                            return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_AND, {p0_parameter[0], bitwuzla::mk_term(bitwuzla::Kind::BV_OR, {p0_parameter[1], p1_parameter[1]})}));
                        }
                        // (X & Y) | (Z & X)    => X & (Y | Z)
                        if (is_x_y(p0_parameter[0], p1_parameter[1]))
                        {
                            return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_AND, {p0_parameter[0], bitwuzla::mk_term(bitwuzla::Kind::BV_OR, {p0_parameter[1], p1_parameter[0]})}));
                        }
                        // (X & Y) | (Y & Z)    => Y & (Y | Z)
                        if (is_x_y(p0_parameter[1], p1_parameter[0]))
                        {
                            return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_AND, {p0_parameter[1], bitwuzla::mk_term(bitwuzla::Kind::BV_OR, {p0_parameter[0], p1_parameter[1]})}));
                        }
                        // (X & Y) | (Z & Y)    => Y & (X | Z)
                        if (is_x_y(p0_parameter[1], p1_parameter[1]))
                        {
                            return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_AND, {p0_parameter[1], bitwuzla::mk_term(bitwuzla::Kind::BV_OR, {p0_parameter[0], p1_parameter[0]})}));
                        }
                    }

                    if (p[1].kind() == (bitwuzla::Kind::BV_AND))
                    {
                        auto p1_parameter = p[1].children();
                        // X | (Y & !X)   =>   X | Y
                        if (is_x_not_y(p1_parameter[1], p[0]))
                        {
                            return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_OR, {p[0], p1_parameter[0]}));
                        }

                        // X | (X & Y)    =>   X
                        if ((is_x_y(p1_parameter[0], p[0])) || (is_x_y(p1_parameter[1], p[0])))
                        {
                            return OK(p[0]);
                        }

                        // X | (~X & Y)   =>   X | Y
                        if (is_x_not_y(p1_parameter[0], p[0]))
                        {
                            return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_OR, {p[0], p1_parameter[1]}));
                        }
                        // X | (Y & ~X)   =>   X | Y
                        if (is_x_not_y(p1_parameter[1], p[0]))
                        {
                            return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_OR, {p[0], p1_parameter[0]}));
                        }
                    }

                    if (p[1].kind() == (bitwuzla::Kind::BV_OR))
                    {
                        auto p1_parameter = p[1].children();

                        // X | (X | Y)   =>   (X | Y)
                        if (is_x_y(p1_parameter[0], p[0]))
                        {
                            return OK(p[1]);
                        }
                        // X | (Y | X)   =>   (Y | X)
                        if (is_x_y(p1_parameter[1], p[0]))
                        {
                            return OK(p[1]);
                        }

                        // X | (~X | Y)   =>   1
                        if (is_x_not_y(p1_parameter[0], p[0]))
                        {
                            return OK(bitwuzla::mk_bv_ones(bitwuzla::mk_bv_sort(p[1].sort().bv_size())));
                        }

                        // X | (Y | ~X)   =>   1
                        if (is_x_not_y(p1_parameter[1], p[0]))
                        {
                            return OK(bitwuzla::mk_bv_ones(bitwuzla::mk_bv_sort(p[1].sort().bv_size())));
                        }
                    }

                    if (p[0].kind() == (bitwuzla::Kind::BV_OR))
                    {
                        auto p0_parameter = p[0].children();

                        // (X | Y) | X   =>   (X | Y)
                        if (is_x_y(p0_parameter[0], p[1]))
                        {
                            return OK(p[0]);
                        }
                        // (Y | X) | X   =>   (Y | X)
                        if (is_x_y(p0_parameter[1], p[1]))
                        {
                            return OK(p[0]);
                        }

                        // (~X | Y) | X   =>   1
                        if (is_x_not_y(p0_parameter[0], p[1]))
                        {
                            return OK(bitwuzla::mk_bv_ones(bitwuzla::mk_bv_sort(p[1].sort().bv_size())));
                        }

                        // (Y | ~X) | X =>   1
                        if (is_x_not_y(p0_parameter[1], p[1]))
                        {
                            return OK(bitwuzla::mk_bv_ones(bitwuzla::mk_bv_sort(p[1].sort().bv_size())));
                        }
                    }

                    if (p[0].kind() == (bitwuzla::Kind::BV_AND))
                    {
                        auto p0_parameter = p[0].children();

                        // (X & Y) | X    =>   X
                        if (is_x_y(p0_parameter[0], p[1]))
                        {
                            return OK(p[1]);
                        }
                        // (Y & X) | X    =>   X
                        if (is_x_y(p0_parameter[1], p[1]))
                        {
                            return OK(p[1]);
                        }

                        // (~X & Y) | X   =>   X | Y
                        if (is_x_not_y(p0_parameter[0], p[1]))
                        {
                            return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_OR, {p0_parameter[1], p[1]}));
                        }

                        // (X & ~Y) | Y   =>   X | Y
                        if (is_x_not_y(p0_parameter[1], p[1]))
                        {
                            return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_OR, {p0_parameter[0], p[1]}));
                        }
                    }

                    return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_OR, {p[0], p[1]}));
                }
                case bitwuzla::Kind::OR: {
                    // X | 0   =>   X
                    if (p[1].is_false())
                    {
                        return OK(p[0]);
                    }

                    // X | 1   =>   1
                    if (p[1].is_true())
                    {
                        return OK(p[1]);
                    }

                    // X | X   =>   X
                    if (is_x_y(p[0], p[1]))
                    {
                        return OK(p[0]);
                    }
                    // X | ~X   =>   111...1
                    if (is_x_not_y(p[0], p[1]))
                    {
                        return OK(bitwuzla::mk_true());
                    }

                    if (p[0].kind() == (bitwuzla::Kind::AND) && p[1].kind() == (bitwuzla::Kind::AND))
                    {
                        auto p0_parameter = p[0].children();
                        auto p1_parameter = p[1].children();

                        // (X & Y) | (X & Z)    => X & (Y | Z)
                        if (is_x_y(p0_parameter[0], p1_parameter[0]))
                        {
                            return OK(bitwuzla::mk_term(bitwuzla::Kind::AND, {p0_parameter[0], bitwuzla::mk_term(bitwuzla::Kind::OR, {p0_parameter[1], p1_parameter[1]})}));
                        }
                        // (X & Y) | (Z & X)    => X & (Y | Z)
                        if (is_x_y(p0_parameter[0], p1_parameter[1]))
                        {
                            return OK(bitwuzla::mk_term(bitwuzla::Kind::AND, {p0_parameter[0], bitwuzla::mk_term(bitwuzla::Kind::OR, {p0_parameter[1], p1_parameter[0]})}));
                        }
                        // (X & Y) | (Y & Z)    => Y & (Y | Z)
                        if (is_x_y(p0_parameter[1], p1_parameter[0]))
                        {
                            return OK(bitwuzla::mk_term(bitwuzla::Kind::AND, {p0_parameter[1], bitwuzla::mk_term(bitwuzla::Kind::OR, {p0_parameter[0], p1_parameter[1]})}));
                        }
                        // (X & Y) | (Z & Y)    => Y & (X | Z)
                        if (is_x_y(p0_parameter[1], p1_parameter[1]))
                        {
                            return OK(bitwuzla::mk_term(bitwuzla::Kind::AND, {p0_parameter[1], bitwuzla::mk_term(bitwuzla::Kind::BV_OR, {p0_parameter[0], p1_parameter[0]})}));
                        }
                    }

                    if (p[1].kind() == (bitwuzla::Kind::AND))
                    {
                        auto p1_parameter = p[1].children();
                        // X | (Y & !X)   =>   X | Y
                        if (is_x_not_y(p1_parameter[1], p[0]))
                        {
                            return OK(bitwuzla::mk_term(bitwuzla::Kind::OR, {p[0], p1_parameter[0]}));
                        }

                        // X | (X & Y)    =>   X
                        if ((is_x_y(p1_parameter[0], p[0])) || (is_x_y(p1_parameter[1], p[0])))
                        {
                            return OK(p[0]);
                        }

                        // X | (~X & Y)   =>   X | Y
                        if (is_x_not_y(p1_parameter[0], p[0]))
                        {
                            return OK(bitwuzla::mk_term(bitwuzla::Kind::OR, {p[0], p1_parameter[1]}));
                        }
                        // X | (Y & ~X)   =>   X | Y
                        if (is_x_not_y(p1_parameter[1], p[0]))
                        {
                            return OK(bitwuzla::mk_term(bitwuzla::Kind::OR, {p[0], p1_parameter[0]}));
                        }
                    }

                    if (p[1].kind() == (bitwuzla::Kind::OR))
                    {
                        auto p1_parameter = p[1].children();

                        // X | (X | Y)   =>   (X | Y)
                        if (is_x_y(p1_parameter[0], p[0]))
                        {
                            return OK(p[1]);
                        }
                        // X | (Y | X)   =>   (Y | X)
                        if (is_x_y(p1_parameter[1], p[0]))
                        {
                            return OK(p[1]);
                        }

                        // X | (~X | Y)   =>   1
                        if (is_x_not_y(p1_parameter[0], p[0]))
                        {
                            return OK(bitwuzla::mk_true());
                        }

                        // X | (Y | ~X)   =>   1
                        if (is_x_not_y(p1_parameter[1], p[0]))
                        {
                            return OK(bitwuzla::mk_true());
                        }
                    }

                    if (p[0].kind() == (bitwuzla::Kind::OR))
                    {
                        auto p0_parameter = p[0].children();

                        // (X | Y) | X   =>   (X | Y)
                        if (is_x_y(p0_parameter[0], p[1]))
                        {
                            return OK(p[0]);
                        }
                        // (Y | X) | X   =>   (Y | X)
                        if (is_x_y(p0_parameter[1], p[1]))
                        {
                            return OK(p[0]);
                        }

                        // (~X | Y) | X   =>   1
                        if (is_x_not_y(p0_parameter[0], p[1]))
                        {
                            return OK(bitwuzla::mk_true());
                        }

                        // (Y | ~X) | X =>   1
                        if (is_x_not_y(p0_parameter[1], p[1]))
                        {
                            return OK(bitwuzla::mk_true());
                        }
                    }

                    if (p[0].kind() == (bitwuzla::Kind::AND))
                    {
                        auto p0_parameter = p[0].children();

                        // (X & Y) | X    =>   X
                        if (is_x_y(p0_parameter[0], p[1]))
                        {
                            return OK(p[1]);
                        }
                        // (Y & X) | X    =>   X
                        if (is_x_y(p0_parameter[1], p[1]))
                        {
                            return OK(p[1]);
                        }

                        // (~X & Y) | X   =>   X | Y
                        if (is_x_not_y(p0_parameter[0], p[1]))
                        {
                            return OK(bitwuzla::mk_term(bitwuzla::Kind::OR, {p0_parameter[1], p[1]}));
                        }

                        // (X & ~Y) | Y   =>   X | Y
                        if (is_x_not_y(p0_parameter[1], p[1]))
                        {
                            return OK(bitwuzla::mk_term(bitwuzla::Kind::OR, {p0_parameter[0], p[1]}));
                        }
                    }

                    return OK(bitwuzla::mk_term(bitwuzla::Kind::OR, {p[0], p[1]}));
                }
                case bitwuzla::Kind::BV_XOR: {
                    // X ^ 0   =>   X
                    if (p[1].is_bv_value_zero())
                    {
                        return OK(p[0]);
                    }
                    // X ^ 1  =>   ~X
                    if (p[1].is_bv_value_ones())
                    {
                        return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_NOT, {p[0]}));
                    }
                    // X ^ X   =>   0
                    if (is_x_y(p[0], p[1]))
                    {
                        return OK(bitwuzla::mk_bv_zero(bitwuzla::mk_bv_sort(p[1].sort().bv_size())));
                    }
                    // X ^ ~X   =>   1
                    if (is_x_not_y(p[0], p[1]))
                    {
                        return OK(bitwuzla::mk_bv_ones(bitwuzla::mk_bv_sort(p[1].sort().bv_size())));
                    }

                    return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_XOR, {p[0], p[1]}));
                }
                case bitwuzla::Kind::XOR: {
                    // X ^ 0   =>   X
                    if (p[1].is_false())
                    {
                        return OK(p[0]);
                    }
                    // X ^ 1  =>   ~X
                    if (p[1].is_true())
                    {
                        return OK(bitwuzla::mk_term(bitwuzla::Kind::NOT, {p[0]}));
                    }
                    // X ^ X   =>   0
                    if (is_x_y(p[0], p[1]))
                    {
                        return OK(bitwuzla::mk_false());
                    }
                    // X ^ ~X   =>   1
                    if (is_x_not_y(p[0], p[1]))
                    {
                        return OK(bitwuzla::mk_true());
                    }

                    return OK(bitwuzla::mk_term(bitwuzla::Kind::XOR, {p[0], p[1]}));
                }
                case bitwuzla::Kind::BV_ADD: {
                    // X + 0    =>   X
                    if (p[1].is_bv_value_zero())
                    {
                        return OK(p[0]);
                    }
                    return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_ADD, {p[0], p[1]}));
                }
                case bitwuzla::Kind::BV_SUB: {
                    // X - 0    =>   X
                    if (p[1].is_bv_value_zero())
                    {
                        return OK(p[0]);
                    }
                    // X - X    =>   0
                    if (is_x_y(p[0], p[1]))
                    {
                        return OK(bitwuzla::mk_bv_zero(bitwuzla::mk_bv_sort(p[1].sort().bv_size())));
                    }

                    return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_SUB, {p[0], p[1]}));
                }
                case bitwuzla::Kind::BV_MUL: {
                    // X * 0    =>   0
                    if (p[1].is_bv_value_zero())
                    {
                        return OK(bitwuzla::mk_bv_zero(bitwuzla::mk_bv_sort(node.sort().bv_size())));
                    }
                    // X * 1    =>   X
                    if (p[1].is_bv_value_one())
                    {
                        return OK(p[0]);
                    }

                    return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_MUL, {p[0], p[1]}));
                }
                case bitwuzla::Kind::BV_SDIV: {
                    // X /s 1    =>   X
                    if (p[1].is_bv_value_one())
                    {
                        return OK(p[0]);
                    }
                    // X /s X    =>   1
                    if (is_x_y(p[0], p[1]))
                    {
                        return OK(bitwuzla::mk_bv_one(bitwuzla::mk_bv_sort(node.sort().bv_size())));
                    }
                    return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_SDIV, {p[0], p[1]}));
                }
                case bitwuzla::Kind::BV_UDIV: {
                    // X / 1    =>   X
                    if (p[1].is_bv_value_one())
                    {
                        return OK(p[0]);
                    }
                    // X / X    =>   1
                    if (is_x_y(p[0], p[1]))
                    {
                        return OK(bitwuzla::mk_bv_one(bitwuzla::mk_bv_sort(node.sort().bv_size())));
                    }

                    return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_UDIV, {p[0], p[1]}));
                }
                case bitwuzla::Kind::BV_SREM: {
                    // X %s 1    =>   0
                    if (p[1].is_bv_value_one())
                    {
                        return OK(bitwuzla::mk_bv_zero(bitwuzla::mk_bv_sort(node.sort().bv_size())));
                    }
                    // X %s X    =>   0
                    if (is_x_y(p[0], p[1]))
                    {
                        return OK(bitwuzla::mk_bv_zero(bitwuzla::mk_bv_sort(node.sort().bv_size())));
                    }

                    return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_SREM, {p[0], p[1]}));
                }
                case bitwuzla::Kind::BV_UREM: {
                    // X %s 1    =>   0
                    if (p[1].is_bv_value_one())
                    {
                        return OK(bitwuzla::mk_bv_zero(bitwuzla::mk_bv_sort(node.sort().bv_size())));
                    }
                    // X %s X    =>   0
                    if (is_x_y(p[0], p[1]))
                    {
                        return OK(bitwuzla::mk_bv_zero(bitwuzla::mk_bv_sort(node.sort().bv_size())));
                    }

                    return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_UREM, {p[0], p[1]}));
                }
                case bitwuzla::Kind::BV_EXTRACT: {
                    // SLICE(p, 0, 0)   =>   p (if p is 1-bit wide)
                    if ((p[0].sort().bv_size() == 1) && node.indices()[0] == 0 && node.indices()[1] == 0)
                    {
                        return OK(p[0]);
                    }

                    return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_EXTRACT, {p[0]}, node.indices()));
                }
                case bitwuzla::Kind::BV_CONCAT: {
                    // CONCAT(X, Y) => CONST(X || Y) already covered in constant propagation
                    // if (p[0].is_constant() && p[1].is_constant())
                    // {
                    //     if ((p[0].size() + p[1].size()) <= 64)
                    //     {
                    //         return OK(bitwuzla::Term::Const((p[0].get_constant_value_u64().get() << p[1].size()) + p[1].get_constant_value_u64().get(), p[0].size() + p[1].size()));
                    //     }
                    // }

                    // We intend to group slices into the same concatination, so that they maybe can be merged into one slice. We try to do this from right to left to make succeeding simplifications easier.
                    if ((p[0].kind() == bitwuzla::Kind::BV_EXTRACT) && p[1].kind() == (bitwuzla::Kind::BV_CONCAT))
                    {
                        auto p1_parameter = p[1].children();

                        if (p1_parameter[0].kind() == (bitwuzla::Kind::BV_EXTRACT))
                        {
                            auto p0_parameter  = p[0].children();
                            auto p10_parameter = p1_parameter[0].children();

                            if (p0_parameter[0] == p10_parameter[0])
                            {
                                if (p1_parameter[1].kind() == (bitwuzla::Kind::BV_EXTRACT))
                                {
                                    auto p11_parameter = p1_parameter[1].children();

                                    // CONCAT(SLICE(X, i, j), CONCAT(SLICE(X, k, l), SLICE(Z, m, n))) => CONCAT(CONCAT(SLICE(X, i, j), SLICE(X, k, l)), SLICE(Z, m, n)))
                                    if (p11_parameter[0] != p10_parameter[0])
                                    {
                                        auto concatination = bitwuzla::mk_term(bitwuzla::Kind::BV_CONCAT, {p[0], p1_parameter[0]});
                                        return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_CONCAT, {concatination, p1_parameter[1]}));
                                    }
                                }
                                else if (p1_parameter[1].kind() == (bitwuzla::Kind::BV_CONCAT))
                                {
                                    auto p11_parameter = p1_parameter[1].children();

                                    if (p11_parameter[0].kind() == (bitwuzla::Kind::BV_EXTRACT))
                                    {
                                        auto p110_parameter = p11_parameter[0].children();

                                        // CONCAT(SLICE(X, i, j), CONCAT(SLICE(X, k, l), CONCAT(SLICE(Y, m, n), Z))) => CONCAT(CONCAT(SLICE(X, i, j), SLICE(X, k, l)), CONCAT(SLICE(Y, m, n), Z)))
                                        if (p110_parameter[0] != p10_parameter[0])
                                        {
                                            auto c1 = bitwuzla::mk_term(bitwuzla::Kind::BV_CONCAT, {p[0], p1_parameter[0]});
                                            return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_CONCAT, {c1, p1_parameter[1]}));
                                        }
                                    }
                                }
                                else
                                {
                                    // CONCAT(SLICE(X, i, j), CONCAT(SLICE(X, k, l), Y)) => CONCAT(CONCAT(SLICE(X, i, j), SLICE(X, k, l)), Y))
                                    auto concatination = bitwuzla::mk_term(bitwuzla::Kind::BV_CONCAT, {p[0], p1_parameter[0]});
                                    return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_CONCAT, {concatination, p1_parameter[1]}));
                                }
                            }
                        }
                    }

                    if (p[0].kind() == (bitwuzla::Kind::BV_EXTRACT) && p[1].kind() == (bitwuzla::Kind::BV_EXTRACT))
                    {
                        auto p0_parameter = p[0].children();
                        auto p1_parameter = p[1].children();

                        if (is_x_y(p0_parameter[0], p1_parameter[0]))
                        {
                            // CONCAT(SLICE(X, j+1, k), SLICE(X, i, j)) => SLICE(X, i, k)
                            if ((p[1].indices()[0] == (p[0].indices()[1] - 1)))
                            {
                                return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_EXTRACT, {p0_parameter[0]}, {p[0].indices()[0], p[1].indices()[1]}));
                            }

                            // CONCAT(SLICE(X, j, j), SLICE(X, i, j)) => SEXT(SLICE(X, i, j), j-i+1)
                            if ((p[1].indices()[1] == p[0].indices()[0]) && (p[1].indices()[1] == p[0].indices()[1]))
                            {
                                return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_SIGN_EXTEND, {p[1]}, {1}));
                            }
                        }
                    }

                    // CONCAT(SLICE(X, j, j), SEXT(SLICE(X, i, j), j-i+n)) => SEXT(SLICE(X, i, j), j-i+n+1)
                    if (p[0].kind() == (bitwuzla::Kind::BV_EXTRACT) && p[1].kind() == (bitwuzla::Kind::BV_SIGN_EXTEND))
                    {
                        auto p1_parameter = p[1].children();

                        if (p1_parameter[0].kind() == (bitwuzla::Kind::BV_EXTRACT))
                        {
                            auto p0_parameter  = p[0].children();
                            auto p10_parameter = p1_parameter[0].children();

                            if ((is_x_y(p0_parameter[0], p10_parameter[0])) && (is_x_y(p0_parameter[1], p0_parameter[2])) && (p[0].indices()[0] == p1_parameter[0].indices()[1]))
                            {
                                return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_SIGN_EXTEND, {p1_parameter[0]}, {p[1].indices()[0] + 1}));
                            }
                        }
                    }

                    // CONCAT(SLICE(X, j, j), CONCAT(SEXT(SLICE(X, i, j), j-i+n), Y)) => CONCAT(SEXT(SLICE(X, i, j), j-i+n+1), Y)
                    if (p[0].kind() == (bitwuzla::Kind::BV_EXTRACT) && p[1].kind() == (bitwuzla::Kind::BV_CONCAT))
                    {
                        auto p1_parameter = p[1].children();

                        if (p1_parameter[0].kind() == (bitwuzla::Kind::BV_SIGN_EXTEND))
                        {
                            auto p10_parameter = p1_parameter[0].children();

                            if (p10_parameter[0].kind() == (bitwuzla::Kind::BV_EXTRACT))
                            {
                                auto p0_parameter   = p[0].children();
                                auto p100_parameter = p10_parameter[0].children();

                                if ((is_x_y(p0_parameter[0], p100_parameter[0])) && (is_x_y(p0_parameter[1], p0_parameter[2])) && (p[0].indices()[0] == p10_parameter[0].indices()[1]))
                                {
                                    auto extension = bitwuzla::mk_term(bitwuzla::Kind::BV_SIGN_EXTEND, {p10_parameter[0]}, {p1_parameter[0].sort().bv_size() - p10_parameter[0].sort().bv_size() + 1});
                                    return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_CONCAT, {extension, p1_parameter[1]}));
                                }
                            }
                        }
                    }
                    return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_CONCAT, {p[0], p[1]}));
                }
                case bitwuzla::Kind::BV_ZERO_EXTEND: {
                    return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_ZERO_EXTEND, {p[0]}, {node.indices()[0]}));
                }
                case bitwuzla::Kind::BV_SIGN_EXTEND: {
                    return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_SIGN_EXTEND, {p[0]}, {node.indices()[0]}));
                }
                case bitwuzla::Kind::EQUAL: {
                    // X == X   =>   1
                    if (is_x_y(p[0], p[1]))
                    {
                        return OK(bitwuzla::mk_true());
                    }

                    return OK(bitwuzla::mk_term(bitwuzla::Kind::EQUAL, {p[0], p[1]}));
                }
                case bitwuzla::Kind::BV_SLE: {
                    // X <=s X   =>   1
                    if (is_x_y(p[0], p[1]))
                    {
                        return OK(bitwuzla::mk_bv_one(bitwuzla::mk_bv_sort(node.sort().bv_size())));
                    }

                    return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_SLE, {p[0], p[1]}));
                }
                case bitwuzla::Kind::BV_SLT: {
                    // X <s X   =>   0
                    if (is_x_y(p[0], p[1]))
                    {
                        return OK(bitwuzla::mk_false());
                    }

                    return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_SLT, {p[0], p[1]}));
                }
                case bitwuzla::Kind::BV_ULE: {
                    // X <= X   =>   1
                    if (is_x_y(p[0], p[1]))
                    {
                        return OK(bitwuzla::mk_true());
                    }

                    return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_ULE, {p[0], p[1]}));
                }
                case bitwuzla::Kind::BV_ULT: {
                    // X < 0   =>   0
                    if (p[1].is_bv_value_zero())
                    {
                        return OK(bitwuzla::mk_false());
                    }
                    // X < X   =>   0
                    if (is_x_y(p[0], p[1]))
                    {
                        return OK(bitwuzla::mk_false());
                    }

                    return OK(bitwuzla::mk_term(bitwuzla::Kind::BV_ULT, {p[0], p[1]}));
                }
                case bitwuzla::Kind::ITE: {
                    // ITE(0, a, b)  =>  b
                    if (p[0].is_false())
                    {
                        return OK(p[2]);
                    }
                    // ITE(1, a, b)  =>  a
                    if (p[0].is_true())
                    {
                        return OK(p[1]);
                    }
                    // ITE(a, b, b)  =>  b
                    if (is_x_y(p[1], p[2]))
                    {
                        return OK(p[1]);
                    }

                    return OK(bitwuzla::mk_term(bitwuzla::Kind::ITE, {p[0], p[1], p[2]}));
                }
                default:
                    return ERR("could not simplify sub-expression in abstract syntax tree: not implemented for given node type");
            }
        }

        Result<bitwuzla::Term> SymbolicExecution::constant_propagation(const bitwuzla::Term& node, std::vector<bitwuzla::Term>& values)
        {
            //     if (node.get_arity() != p.size())
            //     {
            //         return ERR("could not propagate constants: arity does not match number of parameters");
            //     }

            switch (node.kind())
            {
                case bitwuzla::Kind::BV_AND: {
                    return ConstantPropagation::And(values[0], values[1]);
                }
                case bitwuzla::Kind::AND: {
                    if (values[0].is_true() & values[1].is_true())
                    {
                        return OK(bitwuzla::mk_true());
                    }
                    return OK(bitwuzla::mk_false());
                }
                case bitwuzla::Kind::BV_OR: {
                    return ConstantPropagation::Or(values[0], values[1]);
                }
                case bitwuzla::Kind::OR: {
                    if (values[0].is_true() | values[1].is_true())
                    {
                        return OK(bitwuzla::mk_true());
                    }
                    return OK(bitwuzla::mk_false());
                }
                case bitwuzla::Kind::BV_NOT: {
                    return ConstantPropagation::Not(values[0]);
                }

                case bitwuzla::Kind::NOT: {
                    if (values[0].is_true())
                    {
                        return OK(bitwuzla::mk_false());
                    }
                    return OK(bitwuzla::mk_true());
                }
                case bitwuzla::Kind::BV_XOR: {
                    return ConstantPropagation::Xor(values[0], values[1]);
                }
                case bitwuzla::Kind::XOR: {
                    if (values[0].is_true() ^ values[1].is_true())
                    {
                        return OK(bitwuzla::mk_true());
                    }
                    return OK(bitwuzla::mk_false());
                }
                case bitwuzla::Kind::BV_ADD: {
                    return ConstantPropagation::Add(values[0], values[1]);
                }
                case bitwuzla::Kind::BV_SUB: {
                    return ConstantPropagation::Sub(values[0], values[1]);
                }
                case bitwuzla::Kind::BV_MUL: {
                    return ConstantPropagation::Mul(values[0], values[1]);
                }
                case bitwuzla::Kind::BV_EXTRACT: {
                    return ConstantPropagation::BV_EXTRACT(values[0], node.indices()[0], node.indices()[1]);
                }
                case bitwuzla::Kind::BV_CONCAT: {
                    return ConstantPropagation::CONCAT(values[0], values[1]);
                }
                case bitwuzla::Kind::BV_ZERO_EXTEND: {
                    return ConstantPropagation::BV_ZERO_EXTEND(values[0], node.indices()[0]);
                }
                case bitwuzla::Kind::BV_SIGN_EXTEND: {
                    return ConstantPropagation::BV_SIGN_EXTEND(values[0], node.indices()[0]);
                }
                case bitwuzla::Kind::EQUAL: {
                    return ConstantPropagation::Equal(values[0], values[1]);
                }
                case bitwuzla::Kind::BV_SLE: {
                    return ConstantPropagation::Sle(values[0], values[1]);
                }
                case bitwuzla::Kind::BV_SLT: {
                    return ConstantPropagation::Slt(values[0], values[1]);
                }
                case bitwuzla::Kind::BV_ULE: {
                    return ConstantPropagation::Ule(values[0], values[1]);
                }
                case bitwuzla::Kind::BV_ULT: {
                    return ConstantPropagation::Ult(values[0], values[1]);
                }
                case bitwuzla::Kind::ITE: {
                    return ConstantPropagation::Ite(values[0], values[1], values[2]);
                }
                case bitwuzla::Kind::BV_SDIV:
                    // TODO implement
                case bitwuzla::Kind::BV_UDIV:
                    // TODO implement
                case bitwuzla::Kind::BV_SREM:
                    // TODO implement
                case bitwuzla::Kind::BV_UREM:
                    // TODO implement
                default:
                    return ERR("could not propagate constants: not implemented for given node type");
            }
        }
    }    // namespace SMT
}    // namespace hal