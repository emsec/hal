#include  "hal_core/netlist/boolean_function/translator.h"

namespace hal 
{
namespace SMT
{
namespace Translator
{
	/**
	 * Helper function to reduce a view into the abstract syntax tree of a node
	 * and its parameter leaf nodes to an SMT-LIB string.
	 *
	 * @param[in] node - Node.
	 * @param[in] operands - Operands of node.
	 * @returns (1) status (true on success, false otherwise),
	 *          (2) SMT-LIB string representation of node and operands.
	 */
 	std::tuple<bool, std::string> reduce_to_smt2(const BooleanFunction::Node* node, std::vector<std::string>&& operands) 
 	{
 		if (node->get_arity() != operands.size()) {
 			return std::make_tuple(false, "");
 		}

 		switch (node->type) 
 		{
 			case BooleanFunction::NodeType::Constant: 
 				return {true, "(_ bv" + enum_to_string(node->get_as<BooleanFunction::OperandNode>()->constant) + " " + std::to_string(node->size) + ")"};
 			case BooleanFunction::NodeType::Index:
 				return {true, std::to_string(node->get_as<BooleanFunction::OperandNode>()->index)};
 			case BooleanFunction::NodeType::Variable: 
 				return {true, node->get_as<BooleanFunction::OperandNode>()->variable};

 			case BooleanFunction::NodeType::And: 
 				return {true, "(bvand " + operands[0] + " " + operands[1] + ")"};
 			case BooleanFunction::NodeType::Or:
 				return {true, "(bvor " + operands[0] + " " + operands[1] + ")"};
 			case BooleanFunction::NodeType::Not:
 				return {true, "(bvnot " + operands[0] + ")"};	
 			case BooleanFunction::NodeType::Xor:
 				return {true, "(bvxor " + operands[0] + " " + operands[1] + ")"};

			case BooleanFunction::NodeType::Add:
 				return {true, "(bvadd " + operands[0] + " " + operands[1] + ")"};

 			case BooleanFunction::NodeType::Concat:
 				return {true, "(concat " + operands[0] + " " + operands[1] + ")"};

 			// TODO: check correctness of the extract/extension operations once
 			//       the BooleanFunction uses the new data structure / interface

 			/*
 			case BooleanFunction::NodeType::Slice:
 				return {true, "((_ extract " + operands[2] + " " + operands[1] + ") " + operands[0] + ")"};
 			case BooleanFunction::NodeType::Zext: 
 				return {true, "((_ zero_extend " + operands[1] + ") " + operands[0] +  ")"};
			*/

 			default: {
 				std::cerr << "[!] cannot generate SMT-LIB for node-type '" << std::to_string(node->type) << "' (not implemented reached)." << std::endl;
 				return {false, ""};
 			}
 		}
 	}

 	std::tuple<bool, std::string> translate_to_smt2(const BooleanFunction& function) {
		std::vector<std::string> stack;
		for (const auto& node : function.get_reverse_polish_notation()) {
			std::vector<std::string> operands;
			std::move(stack.end() - static_cast<i64>(node->get_arity()), stack.end(), std::back_inserter(operands));
        	stack.erase(stack.end() - static_cast<i64>(node->get_arity()), stack.end());
        	
        	switch (auto [ok, reduction] = reduce_to_smt2(node.get(), std::move(operands)); ok) {
        		case true: stack.emplace_back(reduction); break;
        		default:   return {false, ""};
        	}
		}

		switch (stack.size() == 1) {
			case true: return {true, stack.back()};
			default:   return {false, ""};
		}	
	}
}  // namespace Translator
}  // namespace SMT
}  // namespace hal
