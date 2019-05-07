#include "plugin_quine_mccluskey.h"

#include "core/log.h"
#include "core/plugin_manager.h"
#include "core/utils.h"

#include "netlist/gate.h"
#include "netlist/netlist.h"

#include "gate_decorator_system/decorators/gate_decorator_bdd.h"
#include "gate_decorator_system/decorators/gate_decorator_lut.h"
#include "gate_decorator_system/gate_decorator_system.h"

#include <string.h>

namespace plugin_quine_mccluskey_helper
{
    // print upper part of term
    void upper_term(int bitfield, int mask, int num)
    {
        if (mask)
        {
            int z;
            for (z = 0; z < num; z++)
            {
                if (mask & (1 << z))
                {
                    if (bitfield & (1 << z))
                        printf("_");
                    else
                        printf(" ");
                }
            }
        }
    }

    // print lower part of term
    void lower_term(int mask, int num)
    {
        if (mask)
        {
            int z;
            for (z = 0; z < num; z++)
            {
                if (mask & (1 << z))
                {
                    printf("%c", 'z' - (num - 1) + z);
                }
            }
        }
    }

    void write_output(int bitfield, int mask, u32 num, u8* out)
    {
        if (mask)
        {
            u32 z;
            for (z = 0; z < num; z++)
            {
                if (mask & (1 << z))
                {
                    if (bitfield & (1 << z))
                        out[z] = 0;
                    else
                        out[z] = 1;
                }
                else
                {
                    out[z] = -1;
                }
            }
        }
    }

    // determines whether "value" contains "part"
    bool contains(int value, int32_t mask, int part, int partmask)
    {
        if ((value & partmask) == (part & partmask))
        {
            if ((mask & partmask) == partmask)
                return true;
        }
        return false;
    }

    u8 hamming_weight(const u64 value)
    {
        u8 hw = 0;
        for (u8 i = 0; i < 64; i++)
        {
            if (get_bit(value, i) == 1)
                hw++;
        }
        return hw;
    }

    u8 hamming_distance(const u64 lvalue, const u64 rvalue)
    {
        return hamming_weight(lvalue ^ rvalue);
    }

} /* end of namespace plugin_quine_mccluskey_helper */

std::string plugin_quine_mccluskey::get_name()
{
    return std::string("plugin_quine_mccluskey");
}

std::string plugin_quine_mccluskey::get_version()
{
    return std::string("0.1");
}

void plugin_quine_mccluskey::clear_member_variables()
{
    memset(minterm, 0, PLUGIN_QUINE_MCCLUSKEY_MAX * PLUGIN_QUINE_MCCLUSKEY_MAX * sizeof(int));
    memset(mask, 0, PLUGIN_QUINE_MCCLUSKEY_MAX * PLUGIN_QUINE_MCCLUSKEY_MAX * sizeof(int));
    memset(used, false, PLUGIN_QUINE_MCCLUSKEY_MAX * PLUGIN_QUINE_MCCLUSKEY_MAX * sizeof(int));

    memset(primmask, 0, PLUGIN_QUINE_MCCLUSKEY_MAX * sizeof(int));
    memset(prim, 0, PLUGIN_QUINE_MCCLUSKEY_MAX * sizeof(int));
    memset(wprim, false, PLUGIN_QUINE_MCCLUSKEY_MAX * sizeof(int));
    memset(nwprim, true, PLUGIN_QUINE_MCCLUSKEY_MAX * sizeof(int));
    memset(result, false, PLUGIN_QUINE_MCCLUSKEY_MAX * sizeof(int));
}

bool plugin_quine_mccluskey::simplify(const bool* table, u32 num_of_literals, u8** simplified_table, u32* num_of_clauses)
{
    if (table == nullptr)
    {
        log_error(this->get_name(), "parameter 'table' is nullptr");
        return false;
    }
    if (num_of_literals == 0 || num_of_literals > PLUGIN_QUINE_MCCLUSKEY_MAX_LITERALS)
    {
        log_error(this->get_name(), "number of literals must be in between 1 and {} (parameter is '{}')", PLUGIN_QUINE_MCCLUSKEY_MAX_LITERALS, num_of_literals);
        return false;
    }

    /* check if all table entries are equal */
    bool flag_table_entries_equal = true;
    for (u32 x = 1; x < (u32)(1 << num_of_literals); ++x)
    {
        if (table[0] != table[x])
        {
            flag_table_entries_equal = false;
            break;
        }
    }
    if (flag_table_entries_equal == true)
    {
        *num_of_clauses   = 0;
        *simplified_table = (u8*)calloc(1, sizeof(u8));
        if (*simplified_table == nullptr)
        {
            log_error(this->get_name(), "cannot allocate '1' memory byte");
            return false;
        }
        *simplified_table[0] = table[0];
        return 0;
    }

    int pos             = 0;
    int cur             = 0;
    int reduction       = 0;
    int maderedction    = false;
    size_t prim_count   = 0;
    int term            = 0;
    int termmask        = 0;
    int found           = 0;
    size_t x            = 0;
    size_t y            = 0;
    size_t z            = 0;
    int count           = 0;
    int lastprim        = 0;
    int res             = 0;    // actual result  /  Ist-Ausgabe
    size_t table_length = ((size_t)1) << num_of_literals;
    UNUSED(table_length);

    this->clear_member_variables();

    pos = (1 << num_of_literals);
    cur = 0;
    for (x = (size_t)0; x < (size_t)pos; x++)
    {
        if (table[~x & (pos - 1)])
        {
            mask[cur][0]    = ((1 << num_of_literals) - 1);
            minterm[cur][0] = (int)x;
            cur++;
            result[x] = 1;
        }
    }

    for (reduction = 0; reduction < PLUGIN_QUINE_MCCLUSKEY_MAX; reduction++)
    {
        cur          = 0;
        maderedction = false;
        for (y = 0; y < PLUGIN_QUINE_MCCLUSKEY_MAX; y++)
        {
            for (x = 0; x < PLUGIN_QUINE_MCCLUSKEY_MAX; x++)
            {
                if ((mask[x][reduction]) && (mask[y][reduction]))
                {
                    if (plugin_quine_mccluskey_helper::hamming_weight(mask[x][reduction]) > 1)
                    {
                        if ((plugin_quine_mccluskey_helper::hamming_distance(minterm[x][reduction] & mask[x][reduction], minterm[y][reduction] & mask[y][reduction]) == 1)
                            && (mask[x][reduction] == mask[y][reduction]))
                        {    // Simplification only possible if 1 bit differs  /  Vereinfachung nur m�glich, wenn 1 anderst ist
                            term     = minterm[x][reduction];
                            termmask = mask[x][reduction] ^ (minterm[x][reduction] ^ minterm[y][reduction]);
                            term &= termmask;

                            found = false;
                            for (z = 0; z < (size_t)cur; z++)
                            {
                                if ((minterm[z][reduction + 1] == term) && (mask[z][reduction + 1] == termmask))
                                {
                                    found = true;
                                }
                            }

                            if (found == false)
                            {
                                minterm[cur][reduction + 1] = term;
                                mask[cur][reduction + 1]    = termmask;
                                cur++;
                            }
                            used[x][reduction] = true;
                            used[y][reduction] = true;
                            maderedction       = true;
                        }
                    }
                }
            }
        }
        if (maderedction == false)
            break;    //exit loop early (speed optimisation)
    }
    prim_count = 0;
    for (reduction = 0; reduction < PLUGIN_QUINE_MCCLUSKEY_MAX; reduction++)
    {
        for (x = 0; x < PLUGIN_QUINE_MCCLUSKEY_MAX; x++)
        {
            //Determine all not used minterms
            if ((used[x][reduction] == false) && (mask[x][reduction]))
            {
                //Check if the same prime implicant is already in the list
                found = false;
                for (z = 0; z < prim_count; z++)
                {
                    if (((prim[z] & primmask[z]) == (minterm[x][reduction] & mask[x][reduction])) && (primmask[z] == mask[x][reduction]))
                    {
                        found = true;
                    }
                }
                if (found == false)
                {
                    primmask[prim_count] = mask[x][reduction];
                    prim[prim_count]     = minterm[x][reduction];
                    prim_count++;
                }
            }
        }
    }

    //find essential and not essential prime implicants
    //all alle prime implicants are set to "not essential" so far
    for (y = 0; y < (size_t)pos; y++)
    {    //for all minterms  /  alle Minterme durchgehen
        count    = 0;
        lastprim = 0;
        if (mask[y][0])
        {
            for (x = 0; x < prim_count; x++)
            {
                if (primmask[x])
                {
                    // Check if the minterm contains prime implicant  /  the �berpr�fen, ob der Minterm den Primimplikanten beinhaltet
                    if (plugin_quine_mccluskey_helper::contains(minterm[y][0], mask[y][0], prim[x], primmask[x]))
                    {
                        count++;
                        lastprim = x;
                    }
                }
            }
            // If count = 1 then it is a essential prime implicant
            if (count == 1)
            {
                wprim[lastprim] = true;
            }
        }
    }

    // successively testing if it is possible to remove prime implicants from the rest matrix  /  Nacheinander testen, ob es m�gich ist, Primimplikaten der Restmatrix zu entfernen
    for (z = 0; z < prim_count; z++)
    {
        if (primmask[z])
        {
            if (wprim[z] == false)
            {
                nwprim[z] = false;    // mark as "not essential"
                for (y = 0; y < (size_t)pos; y++)
                {
                    res = 0;
                    for (x = 0; x < prim_count; x++)
                    {
                        if ((wprim[x] == true) || (nwprim[x] == true))
                        {    //essential prime implicant or marked as required
                            if ((u32)(y & primmask[x]) == (u32)(prim[x] & primmask[x]))
                            {    //All bits must be 1
                                res = 1;
                                break;
                            }
                        }
                    }
                    if (res == result[y])
                    {
                    }
                    else
                    {
                        nwprim[z] = true;    //prime implicant required
                    }
                }
            }
        }
    }

    /* count number of prime implicants */
    count = 0;
    for (x = 0; x < prim_count; x++)
    {
        if ((wprim[x] == true) || ((wprim[x] == false) && (nwprim[x] == true)))
            count++;
    }
    /* write simplified function to output */
    *simplified_table = (u8*)calloc(sizeof(u8), count * num_of_literals);
    if (*simplified_table == nullptr)
    {
        log_error(this->get_name(), "cannot allocate '{}' memory bytes", sizeof(u8) * count * num_of_literals);
        return false;
    }
    count = 0;
    for (x = 0; x < prim_count; x++)
    {
        if ((wprim[x] == true) || ((wprim[x] == false) && (nwprim[x] == true)))
        {
            plugin_quine_mccluskey_helper::write_output(prim[x], primmask[x], num_of_literals, (*simplified_table) + count * num_of_literals);
            count++;
        }
    }
    *num_of_clauses = count;
    return true;
}

std::string plugin_quine_mccluskey::formula_to_string(u8* formula, u32 num_of_clauses, u32 num_of_literals)
{
    if (num_of_clauses == 0)
        return (formula[0] == 1) ? "TRUE" : "FALSE";

    std::string formula_str = "";
    for (u32 clause = 0; clause < num_of_clauses; clause++)
    {
        if (clause > 0)
            formula_str += " + ";
        for (u32 literal = 0; literal < num_of_literals; literal++)
        {
            u8 literal_value = formula[clause * num_of_literals + literal];
            if (literal_value == 0)
                formula_str += "~";
            if (literal_value == 0 || literal_value == 1)
                formula_str += "A" + std::to_string(literal);
        }
    }
    return formula_str;
}

boolean_function_t plugin_quine_mccluskey::get_boolean_function(const std::vector<bool>& table)
{
    struct boolean_function_t bf = {{}, 0, 0};
    // check if table has power of two
    auto length = (int)table.size();
    if ((length == 0) || (length & (length - 1)))
    {
        log_error(this->get_name(), "input Boolean logic table input length (= {}) is not a power of 2.", length);
        return bf;
    }

    // transform inputs to raw c-types
    bool* raw_table = (bool*)malloc(length * sizeof(u8));
    for (int i = 0; i < length; i++)
        raw_table[i] = table[i];
    u32 num_of_literals  = (u32)log2(length);
    u8* simplified_table = nullptr;
    u32 num_of_clauses   = 0;

    if (!this->simplify(raw_table, num_of_literals, &simplified_table, &num_of_clauses))
    {
        free(raw_table);
        return bf;
    }
    free(raw_table);

    // assemble return struct
    bf.num_of_clauses  = num_of_clauses;
    bf.num_of_literals = num_of_literals;
    bf.function.resize(num_of_clauses * num_of_literals, false);
    for (int i = 0; i < (int)bf.function.size(); i++)
        bf.function[i] = simplified_table[i];

    return bf;
}

std::string plugin_quine_mccluskey::get_boolean_function_str(const boolean_function_t& bf)
{
    auto tbl = (u8*)malloc(bf.num_of_literals * bf.num_of_clauses * sizeof(u8));
    for (int i = 0; i < (int)(bf.num_of_clauses * bf.num_of_literals); i++)
        tbl[i] = bf.function[i];
    auto str = this->formula_to_string(tbl, bf.num_of_clauses, bf.num_of_literals);
    free(tbl);
    return str;
}

std::map<std::string, std::string> plugin_quine_mccluskey::get_boolean_function_str(std::shared_ptr<gate> const g, bool css_beautified)
{
    if (g == nullptr)
    {
        log_error(this->get_name(), "parameter 'g' is nullptr.");
        return {};
    }

    if (!gate_decorator_system::has_decorator_type(g, gate_decorator_system::decorator_type::LUT))
        return this->get_boolean_function_str_bdd(g);

    std::map<std::string, std::string> pin_to_boolean_function;

    auto lut_decorator = std::dynamic_pointer_cast<gate_decorator_lut>(gate_decorator_system::query_decorator(g, gate_decorator_system::decorator_type::LUT));
    auto str           = this->get_boolean_function_str(this->get_boolean_function(lut_decorator->get_lut_configuration()));

    // replace pin names
    int variable_cnt = 0;
    for (const auto& pin : g->get_input_pin_types())
        str = core_utils::replace(str, "A" + std::to_string(variable_cnt++), pin + " ");

    // add brackets
    str = "(" + str + ")";
    str = core_utils::replace(str, " + ", ") + (");
    str = core_utils::replace(str, " )", ")");

    if (css_beautified)
    {
        // replace commas with AND and OR symbols
        for (const auto& pin : g->get_input_pin_types())
            str = core_utils::replace(str, pin, pin + " " + std::string(u8"\u2227") + " ");
        str = core_utils::replace(str, " " + std::string(u8"\u2227") + " )", ")");
        str = core_utils::replace(str, " + ", " " + std::string(u8"\u2228") + " ");
        // replace ~Axx with overline for each input pin and replace remove single bracket clauses
        for (const auto& pin : g->get_input_pin_types())
        {
            str = core_utils::replace(str, "(~" + pin + ")", "~" + pin);
            str = core_utils::replace(str, "~" + pin, "<span style=\"text-decoration:overline\">" + pin + "</span>");
            str = core_utils::replace(str, "(" + pin + ")", pin);
        }
    }

    pin_to_boolean_function["O"] = str;
    return pin_to_boolean_function;
}

std::map<std::string, std::string> plugin_quine_mccluskey::get_boolean_function_str_bdd(std::shared_ptr<gate> const g)
{
    if (!gate_decorator_system::has_decorator_type(g, gate_decorator_system::decorator_type::BDD))
        return {};

    // assemble boolean function string via gate BDD
    std::map<std::string, int> input_pin_to_bdd_id;
    int cnt = 0;
    for (const auto& pin : g->get_input_pin_types())
        input_pin_to_bdd_id[pin] = cnt++;

    std::map<std::string, std::string> pin_to_boolean_function;
    auto bdd_decorator = std::dynamic_pointer_cast<gate_decorator_bdd>(gate_decorator_system::query_decorator(g, gate_decorator_system::decorator_type::BDD));
    for (const auto& it : bdd_decorator->get_bdd())
    {
        // TODO: use qmc later
        auto str = gate_decorator_bdd::get_bdd_str(it.second);

        // replace brackets
        str = core_utils::replace(str, ">", "), ");
        if (str.find("), ") != std::string::npos)    // remove last ', ' from str
            str = str.substr(0, str.size() - 2);
        str = core_utils::replace(str, "<", "(");

        // replace commas with AND and OR symbols
        str = core_utils::replace(str, "), ", ") " + std::string(u8"\u2228") + " ");
        str = core_utils::replace(str, ", ", " " + std::string(u8"\u2227") + " ");

        // replace :0 with overline for each input pin
        for (const auto& jt : input_pin_to_bdd_id)
            str = core_utils::replace(str, std::to_string(jt.second) + ":0", "<span style=\"text-decoration:overline\">" + std::to_string(jt.second) + "</span>");
        // replace variable indices by real pin names and remove ":1"
        for (const auto& jt : input_pin_to_bdd_id)
        {
            str = core_utils::replace(str, ">" + std::to_string(jt.second) + "<", ">" + jt.first + "<");
            str = core_utils::replace(str, std::to_string(jt.second) + ":1", jt.first);
        }

        pin_to_boolean_function[it.first] = str;
    }

    return pin_to_boolean_function;
}