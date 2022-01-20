#include "hal_core/netlist/boolean_function/simplification.h"

#include "hal_core/netlist/boolean_function/symbolic_execution.h"
#include "hal_core/utilities/log.h"

#include <filesystem>
#include <numeric>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace hal {
namespace {
	/** 
	 * Short-hand function to access a sliced-index of the Boolean function.
	 * 
	 * @param[in] function - Boolean function.
	 * @param[in] index - Bit index of the Boolean function.
	 * @returns Sliced and simplified Boolean function on success, std::string otherwise.
	 */
	std::variant<BooleanFunction, std::string> slice_at(const BooleanFunction& function, const u16 index)
	{
		// (1) setup the sliced Boolean function
		auto status = BooleanFunction::Slice(
			function.clone(),
			BooleanFunction::Index(index, function.size()),
			BooleanFunction::Index(index, function.size()),
			1
		);
    
    	if (std::get_if<std::string>(&status) != nullptr) {
            return "Cannot slice " + function.to_string() + " at bit " + std::to_string(index) + "(= " + std::get<std::string>(status) + ").";
        }
         
        auto slice = std::get<0>(status);

        // (2) perform a local simplification in order to remove unnecessary slices
        status = Simplification::local_simplification(slice);
        if (std::get_if<std::string>(&status) != nullptr) {
            return "Cannot simplify " + slice.to_string() + ".";
        }

        return std::get<0>(status);
	}

	/**
	 * Short-hand function to translate a Boolean function node to Verilog.
	 * 
	 * @param[in] node - Boolean function node.
	 * @param[in] p - Boolean function node parameters in Verilog representation.
	 * @returns `true` and Verilog string on success, `false` and error message otherwise.
	 */
	std::tuple<bool, std::string> node2verilog(const BooleanFunction::Node& node, std::vector<std::string>&& p)
	{
        if (node.get_arity() != p.size())
        {
            return {false, "Cannot translate " + node.to_string() + " as arity does not match inputs."};
        }

        switch (node.type)
        {
            case BooleanFunction::NodeType::Variable:
                return {true, node.to_string()};
            case BooleanFunction::NodeType::And:
                return {true, "(" + p[0] + " & " + p[1] + ")"};
            case BooleanFunction::NodeType::Not:
                return {true, "(~ " + p[0] + ")"};
            case BooleanFunction::NodeType::Or:
                return {true, "(" + p[0] + " | " + p[1] + ")"};
            case BooleanFunction::NodeType::Xor:
                return {true, "(" + p[0] + " ^ " + p[1] + ")"};

            default:
                return {false, "Cannot translate " + node.to_string() + " to Verilog."};
        }
    }

    /**
     * Translates a given Boolean function into structured Verilog.
     * 
     * @param[in] function - Boolean function to translate.
     * @returns `true` and Verilog string on success, `false` otherwise.
     */
    std::tuple<bool, std::string> translate_to_verilog(const BooleanFunction& function) {
        const auto inputs = function.get_variable_names();
        const auto output_prefix = std::string("out_");

        // (1) check that the "out" is not a prefix in the inputs 
        for (const auto& input: inputs) {
            if (std::mismatch(output_prefix.begin(), output_prefix.end(), input.begin()).first
                == output_prefix.end()) {
                return {false, "Cannot translate to Verilog as \"out\" is an input variable."};
            }
        }

        // (2) assemble the interface variables for the module declaration
        auto interface_input_variables = std::accumulate(inputs.begin(), inputs.end(), std::string{},
            [] (auto acc, auto input) { return acc + input + ", "; });
        auto interface_input_declaration = std::accumulate(inputs.begin(), inputs.end(), std::string{},
            [] (auto acc, auto input) { return acc + "\tinput " + input + ";\n"; });

        std::string interface_output_variables, interface_output_declaration;
        for (auto i = 0u; i < function.size(); i++) {
            auto variable = output_prefix + std::to_string(i);
            interface_output_variables += variable;
            interface_output_declaration += "\toutput " + variable + ";\n";
        }

        // (3) assemble the Verilog textual representation
        auto translate = [] (const auto& function, const auto index) -> std::tuple<bool, std::string> {
           	auto simplified = slice_at(function, index);
           	if (std::get_if<std::string>(&simplified) != nullptr) {
                return {false, "Cannot compute simplified slice of " + function.to_string() + " at bit " + std::to_string(index) + "(= " + std::get<std::string>(simplified) + ")."};
           	}

            std::vector<std::string> stack;
	        for (const auto& node : std::get<0>(simplified).get_nodes())
	        {
	            std::vector<std::string> operands;
	            if (stack.size() < node.get_arity())
	            {
	                return {false, "Cannot fetch " + std::to_string(node.get_arity()) + " nodes from the stack (= imbalanced stack with " + std::to_string(stack.size()) + " parts)."};
	            }

	            std::move(stack.end() - static_cast<u64>(node.get_arity()), stack.end(), std::back_inserter(operands));
	            stack.erase(stack.end() - static_cast<u64>(node.get_arity()), stack.end());

	            if (auto [ok, reduction] = node2verilog(node, std::move(operands)); ok)
	            {
	                stack.emplace_back(reduction);
	            }
	            else
	            {
	            	return {false, "Cannot translate BooleanFunction::Node '" + node.to_string() + "' to a Verilog string."};
	            }
	        }

	        switch (stack.size())
	        {
	            case 1:
	                return {true, stack.back()};
	            default: {
	            	return {false, "Cannot translate BooleanFunction (= imbalanced stack with " + std::to_string(stack.size()) + " remaining parts)."};
	            }
	        }
        };

        std::stringstream ss;

        ss << "module top(" << interface_input_variables << interface_output_variables << ");\n";
        ss << interface_input_declaration;
        ss << interface_output_declaration;
        ss << std::endl;
        for (auto index = 0u; index < function.size(); index++) {
            auto [ok, expr] = translate(function, index);
            if (!ok) {
                return {false, "Cannot translate " + function.to_string() + " at index " + std::to_string(index) + " to Verilog."};
            }
            ss << "\tassign " << output_prefix << index << " = " << expr << ";\n";
        }
        ss << "endmodule" << std::endl;

        return {true, ss.str()};
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
    	ss << std::filesystem::temp_directory_path() << "/" << gen();

    	return ss.str();
    }
}  // namespace

namespace Simplification {
    std::variant<BooleanFunction, std::string> local_simplification(const BooleanFunction& function)
    {
       	auto current = function.clone(),
             before = BooleanFunction();

        do
        {
            before          = current.clone();
            auto simplified = SMT::SymbolicExecution().evaluate(current);
            if (std::get_if<std::string>(&simplified) != nullptr)
            {
                return std::get<std::string>(simplified);
            }
            current = std::get<BooleanFunction>(simplified);
        } while (before != current);

        return current;
    }



    extern "C" {
// procedures to start and stop the ABC framework
        // (should be called before and after the ABC procedures are called)
        void Abc_Start();
        void Abc_Stop();

        // procedures to get the ABC framework and execute commands in it
        typedef struct Abc_Frame_t_ Abc_Frame_t;

        Abc_Frame_t* Abc_FrameGetGlobalFrame();
        int Cmd_CommandExecute(Abc_Frame_t* pAbc, const char* sCommand);
    }

 bool call_abc(const char* file_name)
            {
                    // variables
                char Command[1000];

                // start the ABC framework
                Abc_Start();
                auto pAbc = Abc_FrameGetGlobalFrame();

                // read the file
                sprintf(Command, "read %s; fraig; balance; rewrite -l; refactor -l; balance; rewrite -l; rewrite -lz; balance; refactor -lz; rewrite -lz; balance;", file_name);
                if (Cmd_CommandExecute(pAbc, Command))
                {
                    log_info("verification", "Cannot execute command \"{}\".\n", Command);
                    return 1;
                }

                // write the result in eqn
                sprintf(Command, "write_verilog %s.eqn", file_name);
                if (Cmd_CommandExecute(pAbc, Command))
                {
                    log_info("verification", "Cannot execute command \"{}\".\n", Command);
                    return 1;
                }

                Abc_Stop();

                return 0;
            }

	std::variant<BooleanFunction, std::string> abc_simplification(const BooleanFunction& function) 
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
            return function.clone();
        }
        
        // (2) translate the Boolean function to structured Verilog and write
        //     it to a random temporary file on the filesystem

        // (3) call ABC on the structured Verilog file to optimize the function

        // (4) read-back the optimized structured Verilog file and translate the
        //     optimized Verilog back into a Boolean function

		return function;
	}
}  // namespace Simplification
}  // namespace hal