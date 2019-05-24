#ifndef HAL_PLUGIN_QUINE_MCCLUSKEY_H
#define HAL_PLUGIN_QUINE_MCCLUSKEY_H

#include "core/interface_base.h"

#include <hal_bdd.h>
#include <string>

/* forward declaration */
class gate;

/* maximum number of input literals */
#define PLUGIN_QUINE_MCCLUSKEY_MAX_LITERALS 7
/* maximum size for internal computation */
#define PLUGIN_QUINE_MCCLUSKEY_MAX 1024

/**
 * Struct to store simplified Boolean function
 * function - stores encoded Boolean function as string of bytes (size = num_of_clauses * num_of_literals)
 *  function[x] = 0 means literal is false in clause
 *  function[x] = 1 means literal is true in clause
 *  function[x] = -1 means literal is not relevant in clause
 * num_of_clauses - stores number of clauses
 * num_of_literals - stores number of literals (= different variables)
 */
struct boolean_function_t
{
    std::vector<u8> function;
    u32 num_of_clauses;
    u32 num_of_literals;
};

class plugin_quine_mccluskey : virtual public i_base
{
public:
    /** constructor (= default) */
    plugin_quine_mccluskey() = default;
    /** destructor (= default) */
    ~plugin_quine_mccluskey() = default;

    /*
     *      interface implementations
     */

    /** interface implementation: i_base */
    std::string get_name() override;
    /** interface implementation: i_base */
    std::string get_version() override;

    /*
     *      plugin specific functions
     */

    /**
     * Returns simplified Boolean function
     *
     * @param[in] table - Boolean function as std::vector<u8> (table[0] refers to input for all zero input)
     * @return Tuple of simplified table as Boolean vector, number of clauses, and number of literals
     */
    boolean_function_t get_boolean_function(const std::vector<bool>& table);

    /**
     * Returns Boolean logic function string for given table
     *
     * @param[in] bf - Boolean function struct
     * @return Boolean function as string.
     */
    std::string get_boolean_function_str(const boolean_function_t& bf);

    /**
     * Returns Boolean logic function string for each output pin for given gate.
     *
     * @param[in] g - Gate
     * @param[in] css_beautified - Switch to enable/disable richtext and CSS in Boolean function string (default = false)
     * @returns Boolean function as string for each output pin.
     */
    std::map<std::string, std::string> get_boolean_function_str(std::shared_ptr<gate> const g, bool css_beautified = false);

    // TODO: declare private later
    /** returns true if the formula of the function table can be computed */
    bool simplify(const bool* table, u32 num_of_literals, u8** formula, u32* num_of_clauses);

private:
    int minterm[PLUGIN_QUINE_MCCLUSKEY_MAX][PLUGIN_QUINE_MCCLUSKEY_MAX];

    int32_t mask[PLUGIN_QUINE_MCCLUSKEY_MAX][PLUGIN_QUINE_MCCLUSKEY_MAX];    // mask of minterm
    int used[PLUGIN_QUINE_MCCLUSKEY_MAX][PLUGIN_QUINE_MCCLUSKEY_MAX];        // minterm used
    int result[PLUGIN_QUINE_MCCLUSKEY_MAX];                                  // results
    int primmask[PLUGIN_QUINE_MCCLUSKEY_MAX];                                // mask for prime implicants
    int prim[PLUGIN_QUINE_MCCLUSKEY_MAX];                                    // prime implicant
    int wprim[PLUGIN_QUINE_MCCLUSKEY_MAX];                                   // essential prime implicant (true/false)
    int nwprim[PLUGIN_QUINE_MCCLUSKEY_MAX];                                  // needed not essential prime implicant


    std::string formula_to_string(u8* formula, u32 num_of_clauses, u32 num_of_literals);
    void clear_member_variables();

    std::map<std::string, std::string> get_boolean_function_str_bdd(std::shared_ptr<gate> const g);
};

#endif /* HAL_PLUGIN_QUINE_MCCLUSKEY_H */
