#include "hal_core/netlist/boolean_function/simplification.h"

#include "hal_core/netlist/boolean_function/symbolic_execution.h"
#include "hal_core/utilities/log.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>

#include <boost/fusion/include/at_c.hpp>
#include <boost/fusion/sequence/intrinsic/at_c.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

extern "C" {
    /// # Developer Note
    /// Since ABC is implemented in C (and we link towards the libabc.so within
    /// the HAL core), we have to forward declare the extern data types and 
    /// functions we use from ABC.

    void Abc_Start();
    void Abc_Stop();

    typedef struct Abc_Frame_t_ Abc_Frame_t;

    Abc_Frame_t* Abc_FrameGetGlobalFrame();
    int Cmd_CommandExecute(Abc_Frame_t*, const char*);
}

namespace hal {
namespace {
	/** 
	 * Short-hand function to access a sliced-index of the Boolean function.
	 * 
	 * @param[in] function - Boolean function.
	 * @param[in] index - Bit index of the Boolean function.
	 * @returns Sliced and simplified Boolean function on success, error otherwise.
	 */
	Result<BooleanFunction> slice_at(const BooleanFunction& function, const u16 index)
	{
		// (1) setup the sliced Boolean function
		auto status = BooleanFunction::Slice(
			function.clone(),
			BooleanFunction::Index(index, function.size()),
			BooleanFunction::Index(index, function.size()),
			1
		);
    
    	if (std::get_if<std::string>(&status) != nullptr) {
            return ERR("Cannot slice " + function.to_string() + " at bit " + std::to_string(index) + "(= " + std::get<std::string>(status) + ").");
        }

        // (2) perform a local simplification in order to remove unnecessary slices
        return Simplification::local_simplification(std::get<0>(status));
	}

	/**
	 * Short-hand function to translate a Boolean function node to Verilog.
	 * 
	 * @param[in] node - Boolean function node.
	 * @param[in] p - Boolean function node parameters in Verilog representation.
	 * @returns Verilog string on success, error otherwise.
	 */
	Result<std::string> node2verilog(const BooleanFunction::Node& node, std::vector<std::string>&& p)
	{
        if (node.get_arity() != p.size())
        {
            return ERR("Cannot translate " + node.to_string() + " as arity does not match inputs.");
        }

        switch (node.type)
        {
            case BooleanFunction::NodeType::Variable:
                return OK(node.to_string());
            case BooleanFunction::NodeType::And:
                return OK("(" + p[0] + " & " + p[1] + ")");
            case BooleanFunction::NodeType::Not:
                return OK("(~ " + p[0] + ")");
            case BooleanFunction::NodeType::Or:
                return OK("(" + p[0] + " | " + p[1] + ")");
            case BooleanFunction::NodeType::Xor:
                return OK("(" + p[0] + " ^ " + p[1] + ")");

            default:
                return ERR("Cannot translate " + node.to_string() + " to Verilog.");
        }
    }

    /**
     * Translates a given Boolean function into structured Verilog.
     * 
     * @param[in] function - Boolean function to translate.
     * @returns Verilog string with output variables on success, error otherwise.
     */
    Result<std::tuple<std::string, std::vector<std::string>>> translate_to_verilog(const BooleanFunction& function) {
        const auto inputs = function.get_variable_names();
        const auto output_prefix = std::string("out_");

        // (1) check that the "out" is not a prefix in the inputs 
        for (const auto& input: inputs) {
            if (std::mismatch(output_prefix.begin(), output_prefix.end(), input.begin()).first
                == output_prefix.end()) {
                return ERR("Cannot translate to Verilog as \"out\" is an input variable.");
            }
        }

        // (2) assemble the interface variables for the module declaration
        auto interface_input_variables = std::accumulate(inputs.begin(), inputs.end(), std::string{},
            [] (auto acc, auto input) { return acc + input + ", "; });
        auto interface_input_declaration = std::accumulate(inputs.begin(), inputs.end(), std::string{},
            [] (auto acc, auto input) { return acc + "\tinput " + input + ";\n"; });

        std::string interface_output_variables, interface_output_declaration;
        std::vector<std::string> output_variables; output_variables.reserve(function.size());
        for (auto i = 0u; i < function.size(); i++) {
            auto variable = output_prefix + std::to_string(i);
            interface_output_variables += variable;
            interface_output_declaration += "\toutput " + variable + ";\n";

            output_variables.emplace_back(variable);
        }

        // (3) assemble the Verilog textual representation
        auto translate = [] (const auto& function, const auto index) -> Result<std::string> {
           	auto simplified = slice_at(function, index);
           	if (simplified.is_error()) {
                return ERR("Cannot compute simplified slice of " + function.to_string() + " at bit " + std::to_string(index) + "(= " + simplified.get_error().get() + ").");
           	}

            std::vector<std::string> stack;
	        for (const auto& node : simplified.get().get_nodes())
	        {
	            std::vector<std::string> operands;
	            if (stack.size() < node.get_arity())
	            {
	                return ERR("Cannot fetch " + std::to_string(node.get_arity()) + " nodes from the stack (= imbalanced stack with " + std::to_string(stack.size()) + " parts).");
	            }

	            std::move(stack.end() - static_cast<u64>(node.get_arity()), stack.end(), std::back_inserter(operands));
	            stack.erase(stack.end() - static_cast<u64>(node.get_arity()), stack.end());

	            if (auto reduction = node2verilog(node, std::move(operands)); reduction.is_ok())
	            {
	                stack.emplace_back(reduction.get());
	            }
	            else
	            {
	            	return ERR("Cannot translate BooleanFunction::Node '" + node.to_string() + "' to a Verilog string.");
	            }
	        }

	        switch (stack.size())
	        {
	            case 1:
	                return OK(stack.back());
	            default:
	            	return ERR("Cannot translate BooleanFunction (= imbalanced stack with " + std::to_string(stack.size()) + " remaining parts).");
	        }
        };

        std::stringstream ss;

        ss << "module top(" << interface_input_variables << interface_output_variables << ");\n";
        ss << interface_input_declaration;
        ss << interface_output_declaration;
        ss << std::endl;
        for (auto index = 0u; index < function.size(); index++) {
            auto expr = translate(function, index);
            if (expr.is_error()) {
                return ERR("Cannot translate " + function.to_string() + " at index " + std::to_string(index) + " to Verilog.");
            }
            ss << "\tassign " << output_prefix << index << " = " << expr.get() << ";\n";
        }
        ss << "endmodule" << std::endl;

        return OK({ss.str(), output_variables});
    }

    /**
     * Generates a temporary uniquely identifiable filename.
     * 
     * @returns Temporary filename.
     */
    std::string get_random_filename() 
    {
    	// we leverage a universally unique identifier for the filename
    	// in order to prevent any thread / randomization clashes
    	boost::uuids::random_generator gen;

    	std::stringstream ss;
    	ss << std::filesystem::temp_directory_path().u8string() << "/" << gen();

    	return ss.str();
    }

    /**
     * Performs a call to ABC based on a path to structured Verilog.
     * 
     * @param[in] filename Filename.
     * @returns OK on success, error otherwise.
     */
    Result<std::monostate> call_abc(const std::string& filename)
    {
        static bool initialized_abc = false;
        if (!initialized_abc) {
            Abc_Start();
            initialized_abc = true;
        }

        std::stringstream command;
        command << "read " << filename << ";";
        // TODO(@nalbartus) Specify why the various parameter combinations are 
        //                  there and why this command selection works best
        command << "fraig; balance; rewrite -l; refactor -l; balance; rewrite -l; rewrite -lz; balance; refactor -lz; rewrite -lz; balance; rewrite -lz;";
        command << "write_verilog " << filename << ";";

        if (auto context = Abc_FrameGetGlobalFrame(); Cmd_CommandExecute(context, command.str().c_str())) 
        {
            return ERR("Cannot execute command '" + command.str() + "'.");
        }

        return OK({});
    }

    /** 
     * Translates a structured Verilog file to a Boolean function.
     * 
     * @param[in] verilog Verilog data that represents a simplified Boolean function.
     * @param[in] function Input Boolean function (not simplified).
     * @param[in] output_variables Output Boolean function variables.
     * @returns Simplified Boolean function on success, error otherwise.
     */
    Result<BooleanFunction> translate_from_verilog(const std::string& verilog, const BooleanFunction& function, const std::vector<std::string>& output_variables) 
    {
        /**
         * Translates an Verilog statement such as "assign new_n8_ = ~I1 & ~I3;"
         * into the respective left-hand and right-hand side Boolean functions.
         * 
         * @param[in] assignment - Verilog assignment statement.
         * @returns Left-hand and right-hand Boolean functions on success, error otherwise.
         */
        auto parse_assignment = [] (const auto& assignment) -> Result<std::tuple<BooleanFunction, BooleanFunction>>
        {
            std::variant<BooleanFunction, std::string> lhs, rhs;
            
            namespace x3 = boost::spirit::x3;

            ////////////////////////////////////////////////////////////////////////
            // Actions
            ////////////////////////////////////////////////////////////////////////

            const auto LHSAction = [&lhs] (auto& ctx) {
                lhs = BooleanFunction::from_string(_attr(ctx));
            };
            const auto RHSAction = [&rhs] (auto& ctx) {
                rhs = BooleanFunction::from_string(_attr(ctx));
            };

            ////////////////////////////////////////////////////////////////////////
            // Rules
            ////////////////////////////////////////////////////////////////////////

            const auto EndOfLineRule = x3::lit(";") >> *x3::space;
            const auto RHSRule = x3::lexeme[*x3::char_("a-zA-Z0-9_+*~|&!' ")] [RHSAction]; 
            const auto EqualSignRule = *x3::space >> x3::lit("=");
            const auto LHSRule = x3::lexeme[*x3::char_("a-zA-Z0-9_")] [LHSAction]; 
            const auto AssignRule = *x3::space >> x3::lit("assign") >> *x3::space;

            // (1) parse the assignment to left-hand and right-hand side
            auto iter = assignment.begin();
            const auto ok = x3::phrase_parse(iter, assignment.end(), 
                ////////////////////////////////////////////////////////////////////
                // Parsing Expression Grammar
                ////////////////////////////////////////////////////////////////////
                AssignRule >> LHSRule >> EqualSignRule >> RHSRule >> EndOfLineRule,
                // we use an invalid a.k.a. non-printable ASCII character in order
                // to prevent the skipping of space characters as they are defined
                // as skipper within a Boolean function and operation
                x3::char_(0x00)
            );

            if (!ok || (iter != assignment.end())) {
                return ERR("Unable to parse assignment '" + assignment + "'.");
            }

            if (std::get_if<std::string>(&lhs) != nullptr) {
                return ERR("Cannot translate left-hand side of '" + assignment + "' into a Boolean function (= " + std::get<std::string>(lhs) + ".");
            }
            if (std::get_if<std::string>(&rhs) != nullptr) {
                return ERR("Cannot translate right-hand side of '" + assignment + "' into a Boolean function (= " + std::get<std::string>(rhs) + ".");
            }

            return OK(std::make_tuple(std::get<0>(lhs), std::get<0>(rhs)));
        };

        std::map<BooleanFunction, BooleanFunction> assignments;

        // (1) translate each Verilog assignment of the form "assign ... = ...;"
        std::istringstream data(verilog);
        std::string line;
        while (std::getline(data, line)) {
            if (auto assignment = parse_assignment(line); assignment.is_ok()) {
                auto [lhs, rhs] = assignment.get();
                assignments[lhs] = rhs;
            }
        }

        // (2) check that each output variable is defined within the assignments
        for (const auto& output_variable: output_variables) {
            if (assignments.find(BooleanFunction::Var(output_variable)) == assignments.end()) {
                return ERR("Cannot simplify '" + function.to_string() + "' as output variable is not defined in Verilog.");
            }
        }

        // (3) recursively replace the variables for each output Boolean function
        //     until only input variables of the original function are present in
        //     the function
        const auto inputs = function.get_variable_names();
        for (const auto& output_variable: output_variables) {
            auto output = BooleanFunction::Var(output_variable);
              
            // in order to prevent infinite-loops by unsupported inputs such as 
            // cyclic replacement dependencies, we only replace |assignments| times
            auto counter = 0u;
            while ((assignments[output].get_variable_names() != inputs)
                && (counter++ < assignments.size())) {
                for (const auto& tmp : assignments[output].get_variable_names()) 
                {
                    if (inputs.find(tmp) != inputs.end()) {
                        continue;
                    }
                    auto simplified = assignments[output].substitute(tmp, assignments[BooleanFunction::Var(tmp)]);
                    if (std::get_if<std::string>(&simplified) != nullptr) {
                        return ERR(std::get<std::string>(simplified));
                    }

                    assignments[output] = std::get<0>(simplified);
                }
            }
        }

        // (4) validate the all outputs only contain input variables 
        for (const auto& output_variable: output_variables) {
            for (const auto& input: assignments[BooleanFunction::Var(output_variable)].get_variable_names()) {
                if (inputs.find(input) == inputs.end()) {
                    return ERR("Cannot replace '" + output_variable + "' as it contains a temporary variable '" + input + "'.");
                }
            }
        }

        // (5) concatenate all output Boolean functions into a single function
        auto state = assignments[BooleanFunction::Var(output_variables[0])];
        for (auto i = 1u; i < function.size(); i++) {
            auto concat = BooleanFunction::Concat(
                assignments[BooleanFunction::Var(output_variables[i])].clone(),
                state.clone(),
                state.size() + 1
            );

            if (std::get_if<std::string>(&concat) != nullptr) {
                return ERR(std::get<std::string>(concat));
            } else {
                state = std::get<0>(concat);
            }
        }
        return OK(state);
    }
}  // namespace

namespace Simplification {
    Result<BooleanFunction> local_simplification(const BooleanFunction& function)
    {
       	auto current = function.clone(),
             before = BooleanFunction();

        do
        {
            before          = current.clone();
            auto simplified = SMT::SymbolicExecution().evaluate(current);
            if (std::get_if<std::string>(&simplified) != nullptr)
            {
                return ERR(std::get<std::string>(simplified));
            }
            current = std::get<BooleanFunction>(simplified);
        } while (before != current);

        return OK(current);
    }

    Result<BooleanFunction> abc_simplification(const BooleanFunction& function) 
	{
        // # Developer Note
        // In order to apply a global optimization to the Boolean function, we 
        // leverage the rich-featured and robust implementations of ABC [1].
        // To this end, we first translate the Boolean function into a subset of 
        // structured Verilog (e.g., ABC cannot handle arithmetic such as '+' 
        // and we have to translate multi-bit vectors into single bit-vectors).
        // We then use the rewriting, balancing and refactoring to minimize the 
        // Boolean function and then read-back the subset of structured Verilog 
        // back into our Boolean function representation.
        //
        // [1] https://people.eecs.berkeley.edu/~alanmi/abc/

        // (1) check whether the Boolean function is valid for ABC
        using NodeType = BooleanFunction::NodeType;

        static const std::set<u16> valid_abc_node_types({
            NodeType::And, NodeType::Not, NodeType::Or, NodeType::Xor,
            NodeType::Constant, NodeType::Variable,
        });

        if (auto nodes = function.get_nodes(); std::any_of(nodes.begin(), nodes.end(), 
                [] (auto node) { return valid_abc_node_types.find(node.type) == valid_abc_node_types.end(); })
            || function.get_variable_names().empty()) {
            return OK(function.clone());
        }

        // (2) translate the Boolean function to structured Verilog and write
        //     it to a random temporary file on the filesystem
        auto translation = translate_to_verilog(function);
        if (translation.is_error()) {
            return ERR("Cannot translate " + function.to_string() + " to Verilog (= " + translation.get_error().get() + ").");
        }
        auto [verilog, output_variables] = translation.get();

        auto filename = get_random_filename() + ".v";

        std::ofstream file(filename);
        file << verilog;
        file.close();

        // (3) call ABC on the structured Verilog file to optimize the function
        if (auto abc = call_abc(filename); abc.is_error()) {
            std::remove(filename.c_str());
            return ERR(abc.get_error());
        }

        // (4) read-back the optimized structured Verilog file and translate the
        //     optimized Verilog back into a Boolean function
        std::ifstream input_stream(filename);
        std::string input((std::istreambuf_iterator<char>(input_stream)), std::istreambuf_iterator<char>());
        std::remove(filename.c_str());

        return translate_from_verilog(input, function, output_variables);
	}
}  // namespace Simplification
}  // namespace hal