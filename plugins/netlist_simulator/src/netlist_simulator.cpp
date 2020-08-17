#include "plugin_netlist_simulator/netlist_simulator.h"

#include "core/log.h"
#include "netlist/gate_library/gate_type/gate_type_sequential.h"

#include <algorithm>

template<typename T>
static T toggle(T v)
{
    if (v == 0 || v == 1)
    {
        return static_cast<T>(1 - v);
    }
    return v;
}

template<typename T>
std::string signal_to_string(T v)
{
    if (v >= 0)
    {
        return std::to_string(v);
    }
    return "X";
}

namespace hal
{
#define seconds_since(X) ((double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - (X)).count() / 1000)

#define measure_block_time(X) measure_block_time_t UNIQUE_NAME(X);

    class measure_block_time_t
    {
    public:
        measure_block_time_t(const std::string& section_name)
        {
            m_name       = section_name;
            m_begin_time = std::chrono::high_resolution_clock::now();
        }

        ~measure_block_time_t()
        {
            log_info("test", "{} took {:3.2f}s", m_name, seconds_since(m_begin_time));
        }

    private:
        std::string m_name;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_begin_time;
    };

    NetlistSimulator::NetlistSimulator()
    {
        reset();
    }

    void NetlistSimulator::add_gates(const std::vector<Gate*>& gates)
    {
        m_simulation_set.insert(gates.begin(), gates.end());

        compute_input_nets();
        compute_output_nets();

        m_needs_initialization = true;
    }

    void NetlistSimulator::add_clock_hertz(Net* clock_net, u64 hertz, bool start_at_zero)
    {
        u64 period = 1'000'000'000'000ul / hertz;
        add_clock_period(clock_net, period, start_at_zero);
    }

    void NetlistSimulator::add_clock_period(Net* clock_net, u64 period, bool start_at_zero)
    {
        Clock c;
        c.clock_net     = clock_net;
        c.switch_time   = period / 2;
        c.start_at_zero = start_at_zero;
        m_clocks.push_back(c);
    }

    std::vector<Gate*> NetlistSimulator::get_gates() const
    {
        return std::vector<Gate*>(m_simulation_set.begin(), m_simulation_set.end());
    }

    std::vector<Net*> NetlistSimulator::get_input_nets() const
    {
        return m_input_nets;
    }

    std::vector<Net*> NetlistSimulator::get_output_nets() const
    {
        return m_output_nets;
    }

    void NetlistSimulator::set_input(Net* net, SignalValue value)
    {
        if (auto it = m_simulation.m_events.find(net); it != m_simulation.m_events.end())
        {
            if (value == it->second.back().new_value)
            {
                return;
            }
        }

        Event e;
        e.affected_net = net;
        e.time         = m_current_time;
        e.new_value    = value;
        m_event_queue.push_back(e);
    }

    void NetlistSimulator::load_initial_values()
    {
        for (auto gate : m_simulation_set)
        {
            if (gate->get_type()->get_base_type() == GateType::BaseType::ff)
            {
                auto gate_type = dynamic_cast<const GateTypeSequential*>(gate->get_type());
                auto init_str  = std::get<1>(gate->get_data_by_key(gate_type->get_init_data_category(), gate_type->get_init_data_identifier()));
                if (!init_str.empty())
                {
                    SignalValue value = SignalValue::X;
                    if (init_str == "1")
                    {
                        value = SignalValue::ONE;
                    }
                    else if (init_str == "0")
                    {
                        value = SignalValue::ZERO;
                    }
                    else
                    {
                        log_error("netlist simulator", "init value of sequential gate '{}' (type '{}') is neither '1' or '0'", gate->get_name(), gate_type->get_name());
                    }

                    SignalValue inv_value = toggle(value);

                    for (const auto& pin : gate_type->get_state_output_pins())
                    {
                        Event e;
                        e.affected_net = gate->get_fan_out_net(pin);
                        e.new_value    = value;
                        e.time         = m_current_time;
                        m_event_queue.push_back(e);
                    }
                    for (const auto& pin : gate_type->get_inverted_state_output_pins())
                    {
                        Event e;
                        e.affected_net = gate->get_fan_out_net(pin);
                        e.new_value    = inv_value;
                        e.time         = m_current_time;
                        m_event_queue.push_back(e);
                    }
                }
            }
        }
    }

    void NetlistSimulator::simulate(u64 picoseconds)
    {
        if (m_needs_initialization)
        {
            initialize();
            m_needs_initialization = false;
        }

        prepare_clock_events(picoseconds);

        process_events(m_current_time + picoseconds);
    }

    void NetlistSimulator::reset()
    {
        m_current_time = 0;
        m_id_counter   = 0;
        m_simulation   = Simulation();
        m_event_queue.clear();
        m_needs_initialization = true;
    }

    void NetlistSimulator::set_state(const Simulation& state)
    {
        log_error("netlist simulator", "not implemented!");
    }

    Simulation NetlistSimulator::get_current_state() const
    {
        return m_simulation;
    }

    void NetlistSimulator::compute_input_nets()
    {
        m_input_nets.clear();
        for (auto gate : m_simulation_set)
        {
            for (auto net : gate->get_fan_in_nets())
            {
                if (net->is_global_input_net())
                {
                    m_input_nets.push_back(net);
                }
                else
                {
                    for (auto src : net->get_sources())
                    {
                        if (m_simulation_set.find(src.get_gate()) == m_simulation_set.end())
                        {
                            m_input_nets.push_back(net);
                            break;
                        }
                    }
                }
            }
        }
    }

    void NetlistSimulator::compute_output_nets()
    {
        m_output_nets.clear();
        for (auto gate : m_simulation_set)
        {
            for (auto net : gate->get_fan_out_nets())
            {
                if (net->is_global_output_net())
                {
                    m_output_nets.push_back(net);
                }
                else
                {
                    for (auto dst : net->get_destinations())
                    {
                        if (m_simulation_set.find(dst.get_gate()) == m_simulation_set.end())
                        {
                            m_output_nets.push_back(net);
                            break;
                        }
                    }
                }
            }
        }
    }

    void NetlistSimulator::initialize()
    {
        measure_block_time("NetlistSimulator::initialize()");
        m_successors.clear();
        m_sim_gates.clear();

        std::unordered_map<Gate*, SimulationGate*> sim_gates_map;
        std::unordered_set<Net*> all_nets;

        for (auto gate : m_simulation_set)
        {
            std::vector<std::string> input_pins;
            std::vector<Net*> input_nets;

            for (auto pin : gate->get_input_pins())
            {
                input_pins.push_back(pin);
                input_nets.push_back(gate->get_fan_in_net(pin));
            }

            SimulationGate* sim_gate_base = nullptr;

            if (gate->get_type()->get_base_type() == GateType::BaseType::ff)
            {
                auto sim_gate_owner = std::make_unique<SimulationGateFF>();
                auto sim_gate       = sim_gate_owner.get();
                sim_gate_base       = sim_gate;
                m_sim_gates.push_back(std::move(sim_gate_owner));
                sim_gate->is_flip_flop = true;
                sim_gate->input_pins   = input_pins;
                sim_gate->input_nets   = input_nets;

                auto gate_type            = static_cast<const GateTypeSequential*>(gate->get_type());
                sim_gate->clock_func      = gate->get_boolean_function("clock");
                sim_gate->preset_func     = gate->get_boolean_function("preset");
                sim_gate->clear_func      = gate->get_boolean_function("clear");
                sim_gate->next_state_func = gate->get_boolean_function("next_state");
                for (auto pin : gate_type->get_state_output_pins())
                {
                    sim_gate->state_output_nets.push_back(gate->get_fan_out_net(pin));
                }
                for (auto pin : gate_type->get_inverted_state_output_pins())
                {
                    sim_gate->state_inverted_output_nets.push_back(gate->get_fan_out_net(pin));
                }
                for (auto pin : gate_type->get_clock_pins())
                {
                    sim_gate->clock_nets.push_back(gate->get_fan_in_net(pin));
                }
                auto behavior                      = gate_type->get_set_reset_behavior();
                sim_gate->sr_behavior_out          = behavior.first;
                sim_gate->sr_behavior_out_inverted = behavior.second;
            }
            else
            {
                auto sim_gate_owner = std::make_unique<SimulationGateCombinational>();
                auto sim_gate       = sim_gate_owner.get();
                sim_gate_base       = sim_gate;
                m_sim_gates.push_back(std::move(sim_gate_owner));
                sim_gate->is_flip_flop = false;
                sim_gate->input_pins   = input_pins;
                sim_gate->input_nets   = input_nets;

                auto all_functions = gate->get_boolean_functions();

                sim_gate->output_pins = gate->get_output_pins();

                for (auto pin : sim_gate->output_pins)
                {
                    auto out_net = gate->get_fan_out_net(pin);
                    sim_gate->output_nets.push_back(out_net);

                    auto func = all_functions.at(pin);

                    while (true)
                    {
                        auto vars = func.get_variables();
                        bool exit = true;
                        for (auto other_pin : sim_gate->output_pins)
                        {
                            if (vars.find(other_pin) != vars.end())
                            {
                                func = func.substitute(other_pin, all_functions.at(other_pin));
                                exit = false;
                            }
                        }
                        if (exit)
                        {
                            break;
                        }
                    }

                    sim_gate->functions.emplace(out_net, func);
                }
            }

            sim_gate_base->gate = gate;

            sim_gates_map.emplace(gate, sim_gate_base);

            auto out_nets = gate->get_fan_out_nets();
            all_nets.insert(out_nets.begin(), out_nets.end());
        }

        all_nets.insert(m_input_nets.begin(), m_input_nets.end());

        for (auto net : all_nets)
        {
            if (auto it = m_successors.find(net); it != m_successors.end())
            {
                continue;
            }
            auto endpoints  = net->get_destinations();
            auto& dst_gates = m_successors[net];
            for (auto ep : endpoints)
            {
                auto gate = ep.get_gate();
                if (m_simulation_set.find(gate) == m_simulation_set.end())
                {
                    continue;
                }
                dst_gates.push_back(sim_gates_map.at(gate));
            }
        }

        for (auto g : m_simulation_set)
        {
            if (g->is_gnd_gate())
            {
                for (auto n : g->get_fan_out_nets())
                {
                    Event e;
                    e.affected_net = n;
                    e.new_value    = SignalValue::ZERO;
                    e.time         = m_current_time;
                    m_event_queue.push_back(e);
                }
            }
            else if (g->is_vcc_gate())
            {
                for (auto n : g->get_fan_out_nets())
                {
                    Event e;
                    e.affected_net = n;
                    e.new_value    = SignalValue::ONE;
                    e.time         = m_current_time;
                    m_event_queue.push_back(e);
                }
            }
        }

        // log_info("netlist simulator", "initialization done");
    }

    void NetlistSimulator::prepare_clock_events(u64 picoseconds)
    {
        for (auto& c : m_clocks)
        {
            u64 base_time = m_current_time - (m_current_time % c.switch_time);
            u64 time      = 0;
            SignalValue v = static_cast<SignalValue>(base_time & 1);
            if (!c.start_at_zero)
            {
                v = toggle(v);
            }
            while (time < picoseconds)
            {
                Event e;
                e.affected_net = c.clock_net;
                e.new_value    = v;
                e.time         = base_time + time;
                m_event_queue.push_back(e);

                v = toggle(v);
                time += c.switch_time;
            }
        }
    }

    void NetlistSimulator::process_events(u64 timeout)
    {
        measure_block_time("NetlistSimulator::process_events(" + std::to_string(timeout) + ")");
        auto sort_by_time = [](auto& a, auto& b) {
            if (a.time != b.time)
            {
                return a.time < b.time;
            }
            return a.id < b.id;
        };

        u64 total_iterations_for_one_timeslot = 0;

        std::vector<SimulationGateFF*> ffs;
        bool ffs_processed = false;

        while (!m_event_queue.empty() || !ffs.empty())
        {
            std::map<std::pair<Net*, u64>, SignalValue> new_events;

            std::sort(m_event_queue.begin(), m_event_queue.end(), sort_by_time);

            if (m_event_queue.empty() || m_current_time != m_event_queue[0].time)
            {
                if (!ffs.empty() && !ffs_processed)
                {
                    // log_info("netlist simulator", "-- NOW PROCESSING FFs --");
                    for (auto ff : ffs)
                    {
                        simulate_ff(ff, new_events);
                    }
                    ffs.clear();
                    ffs_processed = true;
                }
                else if (m_event_queue.empty())
                {
                    break;
                }
                else
                {
                    m_current_time                    = m_event_queue[0].time;
                    total_iterations_for_one_timeslot = 0;
                    ffs_processed                     = false;
                }

                // log_info("netlist simulator", "");
            }

            if (m_current_time > timeout)
            {
                break;
            }

            u32 processed = 0;
            for (; processed < m_event_queue.size() && m_event_queue[processed].time <= m_current_time; ++processed)
            {
                auto& event = m_event_queue[processed];

                // log_info("netlist simulator", "event: {} to {} at {}", event.affected_net->get_name(), signal_to_string(event.new_value), event.time);

                if (auto it = m_simulation.m_events.find(event.affected_net); it != m_simulation.m_events.end())
                {
                    if (it->second.back().new_value == event.new_value)
                    {
                        // log_info("netlist simulator", "  no change -> skip");
                        continue;
                    }
                    else if (it->second.back().time == event.time)
                    {
                        it->second.back().new_value = event.new_value;
                        if (it->second.size() > 1 && it->second[it->second.size() - 2].new_value == event.new_value)
                        {
                            it->second.pop_back();
                        }
                    }
                    else
                    {
                        m_simulation.m_events[event.affected_net].push_back(event);
                    }
                }
                else
                {
                    m_simulation.m_events[event.affected_net].push_back(event);
                }
                for (auto gate : m_successors.at(event.affected_net))
                {
                    if (!simulate(gate, event, new_events))
                    {
                        ffs.push_back(static_cast<SimulationGateFF*>(gate));
                    }
                }
            }

            total_iterations_for_one_timeslot += processed;

            if (total_iterations_for_one_timeslot > m_timeout_iterations)
            {
                log_error("netlist simulator", "reached iteration timeout of {} without advancing in time, aborting simulation. Is there a combinational loop?", m_timeout_iterations);
                return;
            }

            // remove processed events
            m_event_queue.erase(m_event_queue.begin(), m_event_queue.begin() + processed);

            // add new events
            m_event_queue.reserve(m_event_queue.size() + new_events.size());
            for (const auto& event_it : new_events)
            {
                Event e;
                e.affected_net = event_it.first.first;
                e.time         = event_it.first.second;
                e.new_value    = event_it.second;
                e.id           = m_id_counter++;
                m_event_queue.push_back(e);
            }
            // log_info("netlist simulator", "-- added new events --");
        }

        m_current_time = timeout;
        // log_info("netlist simulator", "");
    }

    bool NetlistSimulator::simulate(SimulationGate* gate, Event& event, std::map<std::pair<Net*, u64>, SignalValue>& new_events)
    {
        // compute delay
        u64 delay = 0;

        // log_info("netlist simulator", "computing gate {}", gate->get_name());

        // gather inputs
        std::map<std::string, BooleanFunction::value> input_values;
        for (u32 i = 0; i < gate->input_pins.size(); ++i)
        {
            auto pin          = gate->input_pins[i];
            auto net          = gate->input_nets[i];
            SignalValue value = SignalValue::X;
            if (auto it = m_simulation.m_events.find(net); it != m_simulation.m_events.end())
            {
                value = it->second.back().new_value;
            }
            input_values.emplace(pin, static_cast<BooleanFunction::value>(value));
            // log_info("netlist simulator", "  pin {} = {}", pin, signal_to_string(value));
        }

        // compute output
        if (gate->is_flip_flop)
        {
            auto ff = static_cast<SimulationGateFF*>(gate);

            auto async_set   = ff->preset_func.evaluate(input_values);
            auto async_reset = ff->clear_func.evaluate(input_values);

            SignalValue result     = SignalValue::X;
            SignalValue inv_result = SignalValue::X;
            bool change_state      = false;

            if (async_set == BooleanFunction::value::ONE && async_reset == BooleanFunction::value::ONE)
            {
                SignalValue old_output = SignalValue::X;
                {
                    auto out_net = ff->state_output_nets[0];
                    if (auto it = m_simulation.m_events.find(out_net); it != m_simulation.m_events.end())
                    {
                        old_output = it->second.back().new_value;
                    }
                }

                change_state = true;
                if (ff->sr_behavior_out == GateTypeSequential::SetResetBehavior::U)
                {
                    log_error("netlist simulator", "simultaneous set/reset behavior is undefined for gate {}", ff->gate->get_name());
                    change_state = false;
                }
                else if (ff->sr_behavior_out == GateTypeSequential::SetResetBehavior::N)
                {
                    result = old_output;
                }
                else if (ff->sr_behavior_out == GateTypeSequential::SetResetBehavior::X)
                {
                    result = SignalValue::X;
                }
                else if (ff->sr_behavior_out == GateTypeSequential::SetResetBehavior::L)
                {
                    result = SignalValue::ZERO;
                }
                else if (ff->sr_behavior_out == GateTypeSequential::SetResetBehavior::H)
                {
                    result = SignalValue::ONE;
                }
                else if (ff->sr_behavior_out == GateTypeSequential::SetResetBehavior::T)
                {
                    result = toggle(old_output);
                }

                if (ff->sr_behavior_out_inverted == GateTypeSequential::SetResetBehavior::U)
                {
                    log_error("netlist simulator", "simultaneous set/reset behavior is undefined for gate {}", ff->gate->get_name());
                    change_state = false;
                }
                else if (ff->sr_behavior_out_inverted == GateTypeSequential::SetResetBehavior::N)
                {
                    inv_result = toggle(old_output);
                }
                else if (ff->sr_behavior_out_inverted == GateTypeSequential::SetResetBehavior::X)
                {
                    inv_result = SignalValue::X;
                }
                else if (ff->sr_behavior_out_inverted == GateTypeSequential::SetResetBehavior::L)
                {
                    inv_result = SignalValue::ZERO;
                }
                else if (ff->sr_behavior_out_inverted == GateTypeSequential::SetResetBehavior::H)
                {
                    inv_result = SignalValue::ONE;
                }
                else if (ff->sr_behavior_out_inverted == GateTypeSequential::SetResetBehavior::T)
                {
                    inv_result = old_output;
                }
            }
            else if (async_set == BooleanFunction::ONE)
            {
                change_state = true;
                result       = SignalValue::ONE;
                inv_result   = toggle(result);
            }
            else if (async_reset == BooleanFunction::ONE)
            {
                change_state = true;
                result       = SignalValue::ZERO;
                inv_result   = toggle(result);
            }

            if (change_state)
            {
                for (auto out_net : ff->state_output_nets)
                {
                    new_events[std::make_pair(out_net, m_current_time + delay)] = result;
                    // log_info("netlist simulator", "  Q ->  {}", out_net->get_name());
                }

                for (auto out_net : ff->state_inverted_output_nets)
                {
                    new_events[std::make_pair(out_net, m_current_time + delay)] = inv_result;
                    // log_info("netlist simulator", "  !Q -> {}", out_net->get_name());
                }
            }

            if (std::any_of(ff->clock_nets.begin(), ff->clock_nets.end(), [&event](auto clock_net) { return clock_net == event.affected_net; }))
            {
                // log_info("netlist simulator", "  clocked on {}", clocked_on_func.to_string());
                if (ff->clock_func.evaluate(input_values) != BooleanFunction::ONE)
                {
                    // log_info("netlist simulator", "  clock function not satisfied -> skip");
                    return true;
                }
                return false;
            }
            else
            {
                // log_info("netlist simulator", "  not connected to a clock pin -> skip");
            }
        }
        else
        {
            auto comb = static_cast<SimulationGateCombinational*>(gate);
            for (auto out_net : comb->output_nets)
            {
                auto result                                                 = comb->functions[out_net](input_values);
                new_events[std::make_pair(out_net, m_current_time + delay)] = static_cast<SignalValue>(result);
            }
            // log_info("netlist simulator", "  {} = {} = {} -> {}", pin, f.to_string(), result, out_net->get_name());
        }
        return true;
    }

    void NetlistSimulator::simulate_ff(SimulationGateFF* gate, std::map<std::pair<Net*, u64>, SignalValue>& new_events)
    {
        // compute delay
        u64 delay = 0;

        // log_info("netlist simulator", "computing gate {}", gate->get_name());

        // gather inputs
        std::map<std::string, BooleanFunction::value> input_values;
        for (u32 i = 0; i < gate->input_pins.size(); ++i)
        {
            auto pin          = gate->input_pins[i];
            auto net          = gate->input_nets[i];
            SignalValue value = SignalValue::X;
            if (auto it = m_simulation.m_events.find(net); it != m_simulation.m_events.end())
            {
                value = it->second.back().new_value;
            }
            input_values.emplace(pin, static_cast<BooleanFunction::value>(value));
            // log_info("netlist simulator", "  pin {} = {}", pin, signal_to_string(value));
        }

        // compute output
        auto result     = static_cast<SignalValue>(gate->next_state_func.evaluate(input_values));
        auto inv_result = toggle(result);
        // log_info("netlist simulator", "  state = {} = {}", f.to_string(), result);

        for (auto out_net : gate->state_output_nets)
        {
            new_events[std::make_pair(out_net, m_current_time + delay)] = result;
            // log_info("netlist simulator", "  Q ->  {}", out_net->get_name());
        }

        for (auto out_net : gate->state_inverted_output_nets)
        {
            new_events[std::make_pair(out_net, m_current_time + delay)] = inv_result;
            // log_info("netlist simulator", "  !Q -> {}", out_net->get_name());
        }
    }

}    // namespace hal
