#include "hal_core/netlist/boolean_function/types.h"

#include <boost/spirit/home/x3.hpp>
#include <mutex>
#include <numeric>

namespace hal
{
    namespace SMT
    {
        namespace ModelParser
        {
            namespace x3 = boost::spirit::x3;

            /// 'ParserContext' refers to the data structure that stores parser results.
            struct ParserContext
            {
                /// stores mapping from variables to its (1) value and (2) bit-size
                std::map<std::string, std::tuple<u64, u16>> model;

                /// stores the current name of a variable assignment
                std::string current_name;
                /// stores the current size of a variable assignment
                std::string current_size;
                /// stores the current value of a variable assignment
                std::string current_value;
            };

            // Parser context to handle
            ParserContext parser_context;

            // Action to store a translated name in the parser context.
            const auto NameAction = [](const auto& ctx) { parser_context.current_name = _attr(ctx); };
            // Action to store a translated size in the parser context.
            const auto SortAction = [](const auto& ctx) { parser_context.current_size = boost::fusion::at_c<0>(_attr(ctx)); };
            // Action to store a translated binary value in the parser context.
            const auto BinaryValueAction = [](const auto& ctx) { parser_context.current_value = "0b" + _attr(ctx); };
            // Action to store a translated hexadecimal value in the parser context.
            const auto HexValueAction = [](const auto& ctx) { parser_context.current_value = "0x" + _attr(ctx); };

            // Action to store a translated signal assignment in the parser context.
            const auto SignalAssigmentAction = [](const auto& /* ctx */) {
                const auto value = (parser_context.current_value.at(1) == 'b') ? strtoull(parser_context.current_value.substr(2, parser_context.current_value.length() - 2).c_str(), nullptr, 2)
                                                                               : strtoull(parser_context.current_value.c_str(), nullptr, 0);

                const auto size = strtoul(parser_context.current_size.c_str(), nullptr, 0);

                parser_context.model.insert({parser_context.current_name, {value, size}});
            };

            // Parser rule to translate a signal assignment name.
            //
            // Example: "A"
            const auto NameRule = (+x3::char_("0-9a-zA-Z_"))[NameAction];
            // Parser rule to translate a signal assignment sort.
            //
            // Example: "(_ BitVec 1)"
            const auto SortRule = (x3::lit("(_ BitVec ") >> +x3::digit >> x3::lit(")"))[SortAction];

            // Parser rule to translate a signal assignment binary constant value.
            //
            // Example: "#b0"
            const auto BinaryValueRule = (x3::lit("#b") >> +x3::char_("0-1"))[BinaryValueAction];
            // Parser rule to translate a signal assignment hexadecimal constant value.
            //
            // Example: "#x1337c0de"
            const auto HexValueRule = (x3::lit("#x") >> +x3::xdigit)[HexValueAction];
            // General parser rule to translate signal assignment constant values.
            const auto ValueRule = BinaryValueRule | HexValueRule;

            // Parser rule to translate a given signal assignment into its signal
            // name, its bit-size and the constant value.
            //
            // Example: "(define-fun A () (_ BitVec 1) #b0)"
            const auto SignalAssignmentRule = (x3::lit("(define-fun") >> NameRule >> x3::lit("()") >> SortRule >> ValueRule >> x3::lit(")"))[SignalAssigmentAction];

            // Boolector grammar to parse a model for a satisfiable formula.
            //
            // Example: "(model (define-fun A () (_ BitVec 1) #b0))"
            const auto BOOLECTOR_MODEL_GRAMMAR = +(x3::lit("(model") >> +SignalAssignmentRule >> x3::lit(")"));

            // Z3 grammar to parse a model for a satisfiable formula.
            //
            // Example: "((define-fun A () (_ BitVec 1) #b0))"
            const auto Z3_MODEL_GRAMMAR = (x3::lit("(") >> (+SignalAssignmentRule) >> x3::lit(")")) | +(x3::lit("(model") >> +SignalAssignmentRule >> x3::lit(")"));
        }    // namespace ModelParser

        QueryConfig& QueryConfig::with_solver(SolverType _solver)
        {
            this->solver = _solver;
            return *this;
        }

        QueryConfig& QueryConfig::with_call(SolverCall _call)
        {
            this->call = _call;
            return *this;
        }

        QueryConfig& QueryConfig::with_local_solver()
        {
            this->local = true;
            return *this;
        }

        QueryConfig& QueryConfig::with_remote_solver()
        {
            this->local = false;
            return *this;
        }

        QueryConfig& QueryConfig::with_model_generation()
        {
            this->generate_model = true;
            return *this;
        }

        QueryConfig& QueryConfig::without_model_generation()
        {
            this->generate_model = false;
            return *this;
        }

        QueryConfig& QueryConfig::with_timeout(u64 seconds)
        {
            this->timeout_in_seconds = seconds;
            return *this;
        }

        std::ostream& operator<<(std::ostream& out, const QueryConfig& config)
        {
            out << "{";
            out << "solver : " << enum_to_string(config.solver) << ", ";
            out << "call: " << enum_to_string(config.call) << ", ";
            out << "local : " << std::boolalpha << config.local << ", ";
            out << "generate_model : " << std::boolalpha << config.generate_model << ", ";
            out << "timeout : " << std::dec << config.timeout_in_seconds << "s";
            out << "}";
            return out;
        }

        Constraint::Constraint(BooleanFunction&& _constraint) : constraint(std::move(_constraint))
        {
        }

        Constraint::Constraint(BooleanFunction&& _lhs, BooleanFunction&& _rhs) : constraint(std::make_pair(std::move(_lhs), std::move(_rhs)))
        {
        }

        std::ostream& operator<<(std::ostream& out, const Constraint& constraint)
        {
            if (constraint.is_assignment())
            {
                const auto assignment = constraint.get_assignment().get();
                out << assignment->first << " = " << assignment->second;
            }
            else
            {
                out << *constraint.get_function().get();
            }

            return out;
        }

        std::string Constraint::to_string() const
        {
            std::stringstream ss;
            ss << *this;
            return ss.str();
        }

        bool Constraint::is_assignment() const
        {
            return std::get_if<std::pair<BooleanFunction, BooleanFunction>>(&constraint);
        }

        Result<const std::pair<BooleanFunction, BooleanFunction>*> Constraint::get_assignment() const
        {
            if (this->is_assignment())
            {
                return OK(&std::get<std::pair<BooleanFunction, BooleanFunction>>(constraint));
            }
            return ERR("constraint is not an assignment");
        }

        Result<const BooleanFunction*> Constraint::get_function() const
        {
            if (!this->is_assignment())
            {
                return OK(&std::get<BooleanFunction>(constraint));
            }
            return ERR("constraint is not a function");
        }

        Model::Model(const std::map<std::string, std::tuple<u64, u16>>& _model) : model(_model)
        {
        }

        bool Model::operator==(const Model& other) const
        {
            return this->model == other.model;
        }

        bool Model::operator!=(const Model& other) const
        {
            return !(*this == other);
        }

        std::ostream& operator<<(std::ostream& out, const Model& model)
        {
            auto s = std::accumulate(model.model.begin(), model.model.end(), std::string(), [](auto accumulator, auto entry) -> std::string {
                return accumulator + ", " + std::get<0>(entry) + ":" + std::to_string(std::get<0>(std::get<1>(entry)));
            });

            out << "{" << s << "}";
            return out;
        }

        Result<Model> Model::parse(const std::string& s, const SolverType& type)
        {
            // TODO:
            // check how to attach a local parser context variable to the x3 parser
            // in order to prevent the necessity of a mutex and thus speed-up
            // parallel processing.

            // to transform the current parser implementation into a thread-safe
            // implementation, we use a static mutex variable that is locked-upon
            // entry in order to prevent any overwrite of the parser_context data
            static std::mutex mutex;

            // (1) reset current context in a thread-safe way
            std::lock_guard<std::mutex> lock(mutex);
            ModelParser::parser_context = ModelParser::ParserContext();

            // (2) parse model using the SMT-LIB grammars for the different solver
            auto iter = s.begin();
            auto ok   = [&]() -> bool {
                switch (type)
                {
                    case SolverType::Z3:
                        return boost::spirit::x3::phrase_parse(iter, s.end(), ModelParser::Z3_MODEL_GRAMMAR, boost::spirit::x3::space);
                    case SolverType::Boolector:
                        return boost::spirit::x3::phrase_parse(iter, s.end(), ModelParser::BOOLECTOR_MODEL_GRAMMAR, boost::spirit::x3::space);
                    case SolverType::Bitwuzla:
                        return boost::spirit::x3::phrase_parse(iter, s.end(), ModelParser::Z3_MODEL_GRAMMAR, boost::spirit::x3::space);

                    default:
                        return false;
                }
            }();

            if (ok && (iter == s.end()))
            {
                return OK(Model(ModelParser::parser_context.model));
            }

            std::cout << "Model: " << s << std::endl; 
            exit(0);
            return ERR("could not parse SMT-Lib model");
        }

        Result<BooleanFunction> Model::evaluate(const BooleanFunction& bf) const
        {
            std::vector<BooleanFunction::Node> new_nodes;

            for (const auto& node : bf.get_nodes())
            {
                if (node.is_variable())
                {
                    const auto var_name = node.variable;
                    if (auto it = model.find(var_name); it != model.end())
                    {
                        const auto constant = BooleanFunction::Const(std::get<0>(it->second), std::get<1>(it->second));
                        new_nodes.insert(new_nodes.end(), constant.get_nodes().begin(), constant.get_nodes().end());
                    }
                    else
                    {
                        new_nodes.push_back(node);
                    }
                }
                else
                {
                    new_nodes.push_back(node);
                }
            }

            auto build_res = BooleanFunction::build(std::move(new_nodes));
            if (build_res.is_error())
            {
                return ERR_APPEND(build_res.get_error(), "failed to evaluate Boolean function for the model: failed to build function after replacing nodes.");
            }

            return OK(build_res.get().simplify_local());
        }

        SolverResult::SolverResult(SolverResultType _type, std::optional<Model> _model) : type(_type), model(_model)
        {
        }

        SolverResult SolverResult::Sat(const std::optional<Model>& model)
        {
            return SolverResult(SolverResultType::Sat, model);
        }

        SolverResult SolverResult::UnSat()
        {
            return SolverResult(SolverResultType::UnSat, {});
        }

        SolverResult SolverResult::Unknown()
        {
            return SolverResult(SolverResultType::Unknown, {});
        }

        bool SolverResult::is(const SolverResultType& _type) const
        {
            return this->type == _type;
        }

        bool SolverResult::is_sat() const
        {
            return this->is(SolverResultType::Sat);
        }

        bool SolverResult::is_unsat() const
        {
            return this->is(SolverResultType::UnSat);
        }

        bool SolverResult::is_unknown() const
        {
            return this->is(SolverResultType::Unknown);
        }

        std::ostream& operator<<(std::ostream& out, const SolverResult& result)
        {
            out << "{";
            out << "type : " << enum_to_string(result.type) << ", ";
            out << "model: ";
            if (result.model.has_value())
            {
                out << *result.model;
            }
            else
            {
                out << "none";
            }
            out << "}";
            return out;
        }
    }    // namespace SMT

    template<>
    std::map<SMT::SolverType, std::string> EnumStrings<SMT::SolverType>::data = {{SMT::SolverType::Z3, "Z3"},
                                                                                 {SMT::SolverType::Boolector, "Boolector"},
                                                                                 {SMT::SolverType::Bitwuzla, "Bitwuzla"},
                                                                                 {SMT::SolverType::Unknown, "Unknown"}};

    template<>
    std::map<SMT::SolverCall, std::string> EnumStrings<SMT::SolverCall>::data = {{SMT::SolverCall::Library, "Library"},
                                                                                 {SMT::SolverCall::Binary, "Binary"}};


    template<>
    std::map<SMT::SolverResultType, std::string> EnumStrings<SMT::SolverResultType>::data = {{SMT::SolverResultType::Sat, "sat"},
                                                                                             {SMT::SolverResultType::UnSat, "unsat"},
                                                                                             {SMT::SolverResultType::Unknown, "unknown"}};
}    // namespace hal