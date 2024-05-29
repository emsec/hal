#include "hal_core/netlist/boolean_function.h"
#include "hal_core/utilities/result.h"

#include <bitwuzla/cpp/bitwuzla.h>

namespace hal
{
    class Gate;
    class GatePin;
    class Net;

    namespace bitwuzla_utils
    {
        // namespace bw
        // {
        //     class Config
        //     {
        //     public:
        //         Config()
        //         {
        //             options.set(bitwuzla::Option::PRODUCE_MODELS, true);
        //             options.set(bitwuzla::Option::SAT_SOLVER, "cadical");
        //             wuzla = bitwuzla::Bitwuzla(options);
        //         };

        //         static bw::term bw_const(BooleanFunction::Value value);

        //     private:
        //         bitwuzla::Options options;
        //         bitwuzla::Bitwuzla wuzla;
        //     };

        // }    // namespace bw

        /**
         * Translates a hal Boolean function into an equivalent bitwuzla term in the given context. 
         * Replacement terms for variables can be specified.
         * 
         * @param[in] bf - The Boolean function to translate.
         * @param[in] ctx - The context where the new term is created in.
         * @param[in] var2term - Optional replacements for variables.
         * @returns A bitwuzla term equivalent to the Boolean function.
         */
        Result<bitwuzla::Term> from_bf(const BooleanFunction& bf, const std::map<std::string, bitwuzla::Term>& var2term = {});

        /**
         * Translates a bitwuzla term into an equivalent hal Boolean function.
         * 
         * @param[in] t - The term to translate.
         * @returns A Boolean function equivalent to the  bitwuzla term.
         */
        Result<BooleanFunction> to_bf(const bitwuzla::Term& t);

        /**
         * Translates a bitwuzla term into an equivalent smt2 representation. 
         * 
         * @param[in] t - The term to translate.
         * @returns A string containing the smt2 representation.
         */
        std::string to_smt2(const bitwuzla::Term& t);

        /**
         * Translates a bitwuzla term into a c++ representation that can be used to evalute the function fast and track the influence of the variables. 
         * 
         * @param[in] t - The term to translate.
         * @returns A string containing the c++ representation.
         */
        std::string to_cpp(const bitwuzla::Term& t);

        /**
         * Translates a bitwuzla term into a verilog network representation.
         * 
         * @param[in] t - The term to translate.
         * @param[in] control_mapping - A control mapping that can be applied.
         * @returns A string containing the verilog representation.
         */
        std::string to_verilog(const bitwuzla::Term& t, const std::map<std::string, bool>& control_mapping = {});

        /**
         * Extracts all variable names from a bitwuzla term.
         * 
         * @param[in] t - The term to extract the variable names from.
         * @returns A set containing all the variable names
         */
        std::set<std::string> get_variable_names(const bitwuzla::Term& t);

        
        /**
         * Extracts all variables as term from a bitwuzla term.
         * 
         * @param[in] t - The term to extract the variable names from.
         * @returns A vectir containing all the variables
         */
        Result<std::vector<bitwuzla::Term>> get_variables(const bitwuzla::Term& t);

        /**
         * Extracts all net ids from the variables of a bitwuzla term.
         * 
         * @param[in] t - The term to extract the net ids from.
         * @returns A set containing all the net ids.
         */
        std::set<u32> extract_net_ids(const bitwuzla::Term& t);

        /**
         * Extracts all net ids from a set of variables.
         * 
         * @param[in] variable_names - The set of variable names.
         * @returns A set containing all the net ids.
         */
        std::set<u32> extract_net_ids(const std::set<std::string>& variable_names);

        /**
         * Get the bitwuzla term representation of a combined Boolean function of a subgraph of combinational gates starting at the source of the provided subgraph output net.
         * The variables of the resulting Boolean function are created from the subgraph input nets using `BooleanFunctionNetDecorator::get_boolean_variable`.
         * 
         * @param[in] subgraph_gates - The gates making up the subgraph to consider.
         * @param[in] subgraph_output - The subgraph oputput net for which to generate the Boolean function.
         * @return The the bitwuzla term representation of combined Boolean function of the subgraph on success, an error otherwise.
         */
        Result<bitwuzla::Term> get_subgraph_bitwuzla_function(const std::vector<Gate*>& subgraph_gates, const Net* subgraph_output);

        /**
         * Get the bitwuzla term representation of a combined Boolean function of a subgraph of combinational gates starting at the source of the provided subgraph output net.
         * The variables of the resulting Boolean function are created from the subgraph input nets using `BooleanFunctionNetDecorator::get_boolean_variable`.
         * 
         * @param[in] subgraph_gates - The gates making up the subgraph to consider.
         * @param[in] subgraph_output - The subgraph oputput net for which to generate the Boolean function.
         * @param[in] net_cache - Cache holding the already visited nets and the generated Term.
         * @param[in] gate_cache - Cache holding the resolved gate Boolean function of the gates already visited.
         * @return The the bitwuzla term representation of combined Boolean function of the subgraph on success, an error otherwise.
         */
        Result<bitwuzla::Term> get_subgraph_bitwuzla_function(const std::vector<Gate*>& subgraph_gates,
                                                              const Net* subgraph_output,
                                                              std::map<u32, bitwuzla::Term>& net_cache,
                                                              std::map<std::pair<u32, const GatePin*>, BooleanFunction>& gate_cache);

        /**
         * Get the bitwuzla term representations of combined Boolean functions of a subgraph of combinational gates starting at the sources of the provided subgraph output nets.
         * The variables of the resulting Boolean functions are created from the subgraph input nets using `BooleanFunctionNetDecorator::get_boolean_variable`.
         * 
         * @param[in] subgraph_gates - The gates making up the subgraph to consider.
         * @param[in] subgraph_outputs - The subgraph oputput nets for which to generate the Boolean functions.
         * @return The the bitwuzla term representations of combined Boolean functions of the subgraph on success, an error otherwise.
         */
        Result<std::vector<bitwuzla::Term>> get_subgraph_bitwuzla_functions(const std::vector<Gate*>& subgraph_gates, const std::vector<Net*>& subgraph_outputs);
        /**
         * Simplify the provided Term with optional replacements as much as possible
         * 
         * @param[in] t - The term to simplify.
         * @param[in] id_to_term - Terms with the given ids will be replaced with Terms specified in the map.
         * @return The simplified version of the term t.
         */
        Result<bitwuzla::Term> simplify(const bitwuzla::Term& t);
        Result<bitwuzla::Term> simplify(const bitwuzla::Term& t, std::map<u64, bitwuzla::Term>& id_to_term);

        /**
         * Evalute the provided Term when a value is given for each variable.
         * 
         * @param[in] t - The term to evaluate.
         * @param[in] id_to_value - A map of each variable mapped to a constant value which will be evaluated. This will fail if the value does not fit into size of the given Variable.
         * @return The evaluated version of the term t as Constant.
         */
        
        Result<bitwuzla::Term> evaluate(const bitwuzla::Term& t,std::map<u64,u64>& id_to_value);

        /**
         * Substitue Subterms of the provided Term with other terms and simplify afterwards.
         * 
         * @param[in] t - The term to evaluate.
         * @param[in] term_to_term - A map of Terms with the corresponding Terms to replace them with.
         * @return The substituted version of the term t.
         */
        Result<bitwuzla::Term> substitue(const bitwuzla::Term& t,std::map< u64,bitwuzla::Term>& id_to_term);

    }    // namespace bitwuzla_utils
}    // namespace hal
