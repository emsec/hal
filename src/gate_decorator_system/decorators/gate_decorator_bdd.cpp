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
