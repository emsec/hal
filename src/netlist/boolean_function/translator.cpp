#include "hal_core/netlist/boolean_function/translator.h"

#include "hal_core/utilities/log.h"
#include "hal_core/utilities/utils.h"

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
	 * @param[in] node - Boolean function node.
	 * @param[in] p - Boolean function node parameters.
	 * @returns (1) status (true on success, false otherwise),
	 *          (2) SMT-LIB string representation of node and operands.
	 */
 	std::tuple<bool, std::string> reduce_to_smt2(const BooleanFunction::Node& node, std::vector<std::string>&& p) 
 	{
 		if (node.get_arity() != p.size()) {
 			return std::make_tuple(false, "");
 		}

 		switch (node.type) 
 		{
 			case BooleanFunction::NodeType::Constant:
			{
				if (auto [ok, str] = utils::translate_to<u64>(node.to_string()); ok) 
				{
					return {true, "(_ bv" + std::to_string(str) + " " + std::to_string(node.size) + ")"};
				}
				return {false, ""};
			}
 			case BooleanFunction::NodeType::Index:
				return {true, std::to_string(node.index)};
 			case BooleanFunction::NodeType::Variable: 
 				return {true, node.variable};

 			case BooleanFunction::NodeType::And: 
 				return {true, "(bvand " + p[0] + " " + p[1] + ")"};
 			case BooleanFunction::NodeType::Or:
 				return {true, "(bvor " + p[0] + " " + p[1] + ")"};
 			case BooleanFunction::NodeType::Not:
 				return {true, "(bvnot " + p[0] + ")"};	
 			case BooleanFunction::NodeType::Xor:
 				return {true, "(bvxor " + p[0] + " " + p[1] + ")"};

			case BooleanFunction::NodeType::Add:
 				return {true, "(bvadd " + p[0] + " " + p[1] + ")"};

			case BooleanFunction::NodeType::Concat:
				return {true, "(concat " + p[0] + " " + p[1] + ")"};

 			default: {
 				log_error("netlist", "cannot generate SMT-LIB for node-type '{}' (not implemented reached).", node.to_string());
 				return {false, ""};
 			}
 		}
 	}

 	std::tuple<bool, std::string> translate_to_smt2(const BooleanFunction& function) {
		std::vector<std::string> stack;
		for (const auto& node : function.get_nodes()) {
			std::vector<std::string> operands;
			std::move(stack.end() - static_cast<i64>(node.get_arity()), stack.end(), std::back_inserter(operands));
        	stack.erase(stack.end() - static_cast<i64>(node.get_arity()), stack.end());
        	
			if (auto [ok, reduction] = reduce_to_smt2(node, std::move(operands)); ok) {
        		stack.emplace_back(reduction);
			} else {
				return {false, ""};
        	}
		}

		switch (stack.size()) {
			case 1: return {true, stack.back()};
			default:   return {false, ""};
		}
	}
}  // namespace Translator
}  // namespace SMT
}  // namespace hal
