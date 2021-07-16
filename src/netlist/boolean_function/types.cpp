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
            const auto Z3_MODEL_GRAMMAR = x3::lit("(") >> (+SignalAssignmentRule) >> x3::lit(")");
        }    // namespace ModelParser

        QueryConfig& QueryConfig::with_solver(SolverType _solver)
        {
            this->solver = _solver;
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
            out << "local : " << std::boolalpha << config.local << ", ";
            out << "generate_model : " << std::boolalpha << config.generate_model << ", ";
            out << "timeout : " << std::dec << config.timeout_in_seconds << "s";
            out << "}";
            return out;
        }

        Constraint::Constraint(BooleanFunction _lhs, BooleanFunction _rhs) : lhs(_lhs), rhs(_rhs)
        {
        }

        std::ostream& operator<<(std::ostream& out, const Constraint& constraint)
        {
            out << constraint.lhs << " = " << constraint.rhs;
            return out;
        }

        Model::Model(std::map<std::string, std::tuple<u64, u16>> _model) : model(_model)
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

        std::tuple<bool, Model> Model::parse(std::string s, const SolverType& type)
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

                    default: return false;
                }
            }();

            switch (ok && (iter == s.end()))
            {
                case true: return {true, Model(ModelParser::parser_context.model)};
                default:   return {false, Model()};
            }
        }

        Result::Result(ResultType _type, std::optional<Model> _model) : type(_type), model(_model)
        {
        }

        Result Result::Sat(const std::optional<Model>& model)
        {
            return Result(ResultType::Sat, model);
        }

        Result Result::UnSat()
        {
            return Result(ResultType::UnSat, {});
        }

        Result Result::Unknown()
        {
            return Result(ResultType::Unknown, {});
        }

        bool Result::is(const ResultType& _type) const
        {
            return this->type == _type;
        }

        bool Result::is_sat() const
        {
            return this->is(ResultType::Sat);
        }

        bool Result::is_unsat() const
        {
            return this->is(ResultType::UnSat);
        }

        bool Result::is_unknown() const
        {
            return this->is(ResultType::Unknown);
        }

        std::ostream& operator<<(std::ostream& out, const Result& result)
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
    std::vector<std::string> EnumStrings<SMT::SolverType>::data = {"Z3", "Boolector", "Unknown"};

    template<>
    std::vector<std::string> EnumStrings<SMT::ResultType>::data = {"sat", "unsat", "unknown"};
}    // namespace hal