#include "gate_decorator_system/decorators/gate_decorator_bdd.h"

#include "core/log.h"
#include "core/utils.h"

#include "netlist/gate.h"
#include "netlist/netlist.h"

#include <sstream>

gate_decorator_bdd::gate_decorator_bdd(const std::shared_ptr<gate> gate, gate_decorator_system::bdd_decorator_generator generator) : gate_decorator(gate), m_generator(generator)
{
}

gate_decorator_system::decorator_type gate_decorator_bdd::get_type()
{
    return gate_decorator_system::decorator_type::BDD;
}

std::map<std::string, std::shared_ptr<bdd>> gate_decorator_bdd::get_bdd()
{
    std::map<std::string, std::shared_ptr<bdd>> input_pin_type_to_bdd;
    int cnt = 0;
    for (const auto& input_type : this->get_gate()->get_input_pin_types())
    {
        auto var = bdd_ithvar(cnt);
        if (var == bdd_false())
        {
            log_error("netlist.decorator", "cannot create the {}-th's BDD variable (maybe increase the gate_size and cache_size?!)", cnt);
            return {};
        }

        input_pin_type_to_bdd[input_type] = std::make_shared<bdd>(var);
        cnt++;
    }
    return this->get_bdd(input_pin_type_to_bdd);
}

std::map<std::string, std::shared_ptr<bdd>> gate_decorator_bdd::get_bdd(std::map<std::string, int> input_pin_type_to_bdd_identifier)
{
    std::map<std::string, std::shared_ptr<bdd>> input_pin_type_to_bdd;
    for (const auto& it : input_pin_type_to_bdd_identifier)
    {
        auto var = bdd_ithvar(it.second);
        if (var == bdd_false())
        {
            log_error("netlist.decorator", "cannot create the {}-th's BDD variable (maybe increase the gate_size and cache_size?!)", it.second);
            return {};
        }
        input_pin_type_to_bdd[it.first] = std::make_shared<bdd>(var);
    }
    return this->get_bdd(input_pin_type_to_bdd);
}

std::map<std::string, std::shared_ptr<bdd>> gate_decorator_bdd::get_bdd(std::map<std::string, std::tuple<std::shared_ptr<bdd>, int>> input_pin_type_to_bdd_or_identifier)
{
    std::map<std::string, std::shared_ptr<bdd>> input_pin_type_to_bdd;
    for (const auto& it : input_pin_type_to_bdd_or_identifier)
    {
        if (std::get<0>(it.second) == nullptr)
        {
            auto var = bdd_ithvar(std::get<1>(it.second));
            if (var == bdd_false())
            {
                log_error("netlist.decorator", "cannot create the {}-th's BDD variable (maybe increase the gate_size and cache_size?!)", std::get<1>(it.second));
                return {};
            }
            input_pin_type_to_bdd[it.first] = std::make_shared<bdd>(var);
        }
        else
        {
            input_pin_type_to_bdd[it.first] = std::get<0>(it.second);
        }
    }
    return this->get_bdd(input_pin_type_to_bdd);
}

std::map<std::string, std::shared_ptr<bdd>> gate_decorator_bdd::get_bdd(std::map<std::string, std::shared_ptr<bdd>> input_pin_type_to_bdd)
{
    std::set<std::string> input_pin_types;
    for (const auto& it : input_pin_type_to_bdd)
        input_pin_types.insert(it.first);

    auto input_pin_vector = this->get_gate()->get_input_pin_types();

    std::set<std::string> compare(input_pin_vector.begin(), input_pin_vector.end());
    if (input_pin_types != compare)
    {
        log_error("netlist.decorator", "{}", "input pin types do no match input pin types of gate.");
        return {};
    }

    return m_generator(get_gate(), input_pin_type_to_bdd);
}

bool gate_decorator_bdd::is_tautology(std::shared_ptr<bdd> bdd_ptr)
{
    if (bdd_ptr == nullptr)
    {
        log_error("netlist.decorator", "{}", "parameter 'bdd_ptr' is nullptr.");
        return false;
    }
    return (*bdd_ptr == bdd_true());
}

bool gate_decorator_bdd::is_contradiction(std::shared_ptr<bdd> bdd_ptr)
{
    if (bdd_ptr == nullptr)
    {
        log_error("netlist.decorator", "{}", "parameter 'bdd_ptr' is nullptr.");
        return false;
    }
    return (*bdd_ptr == bdd_false());
}

std::string gate_decorator_bdd::get_bdd_str(std::shared_ptr<bdd> bdd_ptr)
{
    if (bdd_ptr == nullptr)
    {
        log_error("netlist.decorator", "{}", "parameter 'bdd_ptr' is nullptr.");
        return std::string();
    }
    std::stringstream ss;
    ss << *bdd_ptr;
    return ss.str();
}

std::vector<std::map<int, bool>> gate_decorator_bdd::get_bdd_clauses(std::shared_ptr<bdd> bdd_ptr)
{
    if (bdd_ptr == nullptr)
    {
        log_error("netlist.decorator", "parameter 'bdd_ptr' is nullptr.");
        return std::vector<std::map<int, bool>>();
    }

    if (is_tautology(bdd_ptr) || is_contradiction(bdd_ptr))
    {
        log_error("netlist.decorator", "no clauses available for tautologies or contradictions.");
        return std::vector<std::map<int, bool>>();
    }

    std::vector<std::map<int, bool>> result;
    auto clauses = core_utils::split(gate_decorator_bdd::get_bdd_str(bdd_ptr), '>');
    /* If last char is '>' skip the last clause as it is empty */
    if (clauses.back().empty())
        clauses.pop_back();
    for (auto clause_str : clauses)
    {
        /* remove the '<' char */
        clause_str = clause_str.substr(1, clause_str.size());

        std::map<int, bool> clause;
        auto variables = core_utils::split(clause_str, ',');
        for (auto var : variables)
        {
            var              = core_utils::trim(var);
            auto assignments = core_utils::split(var, ':');
            auto input       = (int)stoul(core_utils::trim(assignments[0]), 0, 10);
            auto value       = (bool)stoul(core_utils::trim(assignments[1]), 0, 2);
            clause[input]    = value;
        }
        result.push_back(clause);
    }
    return result;
}


bool gate_decorator_bdd::evaluate_bdd(std::shared_ptr<bdd> bdd_ptr, const std::map<int, bool>& input_configuration)
{
    return evaluate_bdd(get_bdd_clauses(bdd_ptr), input_configuration);
}

bool gate_decorator_bdd::evaluate_bdd(std::shared_ptr<gate> const g, std::shared_ptr<bdd> const bdd_ptr, const std::map<std::string, bool>& input_configuration)
{
    std::map<int, bool> _input_configuration;
    int cnt = 0;
    for (const auto& input_type : g->get_input_pin_types())
        _input_configuration[cnt++] = input_configuration.at(input_type);
    return gate_decorator_bdd::evaluate_bdd(bdd_ptr, _input_configuration);
}

bool gate_decorator_bdd::evaluate_bdd(const std::vector<std::map<int, bool>>& clauses, const std::map<int, bool>& input_configuration)
{
    bool clause_true = true;
    for (const auto& clause : clauses)
    {
        clause_true = true;
        for (const auto entry : clause)
        {
            //get id entry from map
            auto it = input_configuration.find(entry.first);
            if (it == input_configuration.end())
            {
                log_critical("netlist.decorator", "bool parameter {} not found in map", entry.first);
                return false;
            }
            clause_true = (entry.second == it->second) and clause_true;
        }

        if (clause_true)
        {
            break;
        }
    }
    return clause_true;
}

std::tuple<std::vector<int>, std::vector<bool>> gate_decorator_bdd::get_truth_table(std::shared_ptr<bdd> const bdd_ptr)
{
    auto ret = std::make_tuple(std::vector<int>(), std::vector<bool>());

    if (bdd_ptr == nullptr) {
        log_error("netlist.decorator", "parameter 'bdd_ptr' is nullptr.");
        return ret;
    }

    // store inputs in a set to prevent double entries and convert to vector afterwards
    std::set<int> __inputs;
    bdd_mark(bdd_ptr.get()->id());
    for (int n = 0; n < bddnodesize; n++) {
        if (!(LEVEL(n) & MARKON))
            continue;
        auto node = &bddnodes[n];
        LEVELp(node) &= MARKOFF;
        __inputs.insert(bddlevel2var[LEVELp(node)]);
    }
    std::vector<int> inputs(__inputs.begin(), __inputs.end());

    // check whether BDD is tautology or contraction
    if (bdd_ptr.get()->id() < 2)
        return std::make_tuple(inputs, std::vector<bool>(1, (bool) bdd_ptr.get()->id()));

    if ((int) inputs.size() > 20) {
        log_error("netlist.decorator", "cannot generate truth table for more than 20 inputs (current inputs = {}).", inputs.size());
        return ret;
    }

    std::vector<bool> truth_table((1 << inputs.size()), false);
    for (int index = 0; index < (1 << inputs.size()); index++)
    {
        std::map<int, bool> input_configuration;
        auto bit_index = 0;
        for (const auto& input : inputs)
            input_configuration[input] = get_bit(index, bit_index++);
        truth_table[index] = gate_decorator_bdd::evaluate_bdd(bdd_ptr, input_configuration);
    }
    return std::make_tuple(inputs, truth_table);
}
