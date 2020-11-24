#include "netlist_simulator/netlist_simulator.h"

#include "hal_core/netlist/gate_library/gate_type/gate_type_sequential.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>

template<typename T>
static T toggle(T v)
{
    if (v == 0 || v == 1)
    {
        return static_cast<T>(1 - v);
    }
    return v;
}

namespace hal
{
#define measure_block_time(X)

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

    void NetlistSimulator::add_clock_frequency(Net* clock_net, u64 frequency, bool start_at_zero)
    {
        u64 period = 1'000'000'000'000ul / frequency;
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

    std::unordered_set<Gate*> NetlistSimulator::get_gates() const
    {
        return m_simulation_set;
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
        // has to work even if the simulation was not started, i.e., initialize was not called yet
        // so we cannot use the SimulationGateFF type

        for (auto gate : m_simulation_set)
        {
            if (gate->get_type()->get_base_type() == GateType::BaseType::ff)
            {
                // extract init string
                auto gate_type = dynamic_cast<const GateTypeSequential*>(gate->get_type());
                auto init_str  = std::get<1>(gate->get_data(gate_type->get_init_data_category(), gate_type->get_init_data_identifier()));

                if (!init_str.empty())
                {
                    // parse init string

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

                    // generate events
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

    void NetlistSimulator::set_simulation_state(const Simulation& state)
    {
        m_simulation = state;
    }

    Simulation NetlistSimulator::get_simulation_state() const
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
                // "input net" is either a global input...
                if (net->is_global_input_net())
                {
                    m_input_nets.push_back(net);
                }
                else    // ... or has a source outside of the simulation set
                {
                    for (auto src : net->get_sources())
                    {
                        if (m_simulation_set.find(src->get_gate()) == m_simulation_set.end())
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
                // "output net" is either a global output...
                if (net->is_global_output_net())
                {
                    m_output_nets.push_back(net);
                }
                else    // ... or has a destination outside of the simulation set
                {
                    for (auto dst : net->get_destinations())
                    {
                        if (m_simulation_set.find(dst->get_gate()) == m_simulation_set.end())
                        {
                            m_output_nets.push_back(net);
                            break;
                        }
                    }
                }
            }
        }
    }

    void NetlistSimulator::set_iteration_timeout(u64 iterations)
    {
        m_timeout_iterations = iterations;
    }

    u64 NetlistSimulator::get_simulation_timeout() const
    {
        return m_timeout_iterations;
    }

    /*
     * This function precomputes all the stuff that shall be cached for simulation.
     */
    void NetlistSimulator::initialize()
    {
        measure_block_time("NetlistSimulator::initialize()");
        m_successors.clear();
        m_sim_gates.clear();

        std::unordered_map<Gate*, SimulationGate*> sim_gates_map;
        std::unordered_set<Net*> all_nets;

        // precompute everything that is gate-related
        for (auto gate : m_simulation_set)
        {
            std::vector<std::string> input_pins;
            std::vector<Net*> input_nets;

            // gather input pins + respective nets at same positions of vectors for fast iteration
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
                for (auto pin : input_pins)
                {
                    sim_gate->input_values[pin] = BooleanFunction::X;
                }

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
            else if (gate->get_type()->get_base_type() == GateType::BaseType::combinatorial || gate->get_type()->get_base_type() == GateType::BaseType::lut)
            {
                auto sim_gate_owner = std::make_unique<SimulationGateCombinational>();
                auto sim_gate       = sim_gate_owner.get();
                sim_gate_base       = sim_gate;
                m_sim_gates.push_back(std::move(sim_gate_owner));
                sim_gate->is_flip_flop = false;
                sim_gate->input_pins   = input_pins;
                sim_gate->input_nets   = input_nets;
                for (auto pin : input_pins)
                {
                    sim_gate->input_values[pin] = BooleanFunction::X;
                }

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
                            if (std::find(vars.begin(), vars.end(), other_pin) != vars.end())
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
            else
            {
                log_error("netlist simulator", "no support for gate type {} of gate {}", gate->get_type()->get_name(), gate->get_name());
                m_successors.clear();
                m_sim_gates.clear();
                return;
            }

            sim_gate_base->gate = gate;

            sim_gates_map.emplace(gate, sim_gate_base);

            auto out_nets = gate->get_fan_out_nets();
            all_nets.insert(out_nets.begin(), out_nets.end());
        }

        all_nets.insert(m_input_nets.begin(), m_input_nets.end());

        // find all successors of nets and transform them to their respective simulation gate instance
        for (auto net : all_nets)
        {
            if (auto it = m_successors.find(net); it != m_successors.end())
            {
                continue;
            }
            auto endpoints = net->get_destinations();
            std::unordered_map<Gate*, std::vector<std::string>> affected_pins;
            for (auto ep : endpoints)
            {
                auto gate = ep->get_gate();
                affected_pins[gate].push_back(ep->get_pin());
            }

            for (auto it : affected_pins)
            {
                auto gate  = it.first;
                auto& pins = it.second;
                if (m_simulation_set.find(gate) == m_simulation_set.end())
                {
                    continue;
                }
                auto sim_gate = sim_gates_map.at(gate);
                m_successors[net].emplace_back(sim_gate, pins);
            }
        }

        // create one-time events for global gnd and vcc gates
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

        // set initialization flag only if this point is reached
        m_needs_initialization = false;
    }

    void NetlistSimulator::prepare_clock_events(u64 picoseconds)
    {
        for (auto& c : m_clocks)
        {
            u64 base_time = m_current_time - (m_current_time % c.switch_time);
            u64 time      = 0;

            // determine next signal state
            // works also if we are in the middle of a cycle
            SignalValue v = static_cast<SignalValue>(base_time & 1);
            if (!c.start_at_zero)
            {
                v = toggle(v);
            }

            // insert the required amount of clock signal switch events
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

        // iteration counter to catch infinite loops
        u64 total_iterations_for_one_timeslot = 0;

        // strategy: propagate all signals at the current time
        // THEN evaluate all FFs that were clocked by these signals
        // hence we need to remember FFs that were clocked
        std::vector<SimulationGateFF*> ffs;
        bool ffs_processed = false;

        while (!m_event_queue.empty() || !ffs.empty())
        {
            std::map<std::pair<Net*, u64>, SignalValue> new_events;

            // sort events by time
            std::sort(m_event_queue.begin(), m_event_queue.end());

            // queue empty or all events of the current point in time processed?
            if (m_event_queue.empty() || m_current_time != m_event_queue[0].time)
            {
                // are there FFs that were clocked? process them now!
                if (!ffs.empty() && !ffs_processed)
                {
                    for (auto ff : ffs)
                    {
                        simulate_ff(ff, new_events);
                    }
                    ffs.clear();
                    ffs_processed = true;
                }
                else if (m_event_queue.empty())
                {
                    // no FFs and queue empty -> simulation is done
                    break;
                }
                else    // no FFs but queue is not empty -> advance point in time
                {
                    m_current_time                    = m_event_queue[0].time;
                    total_iterations_for_one_timeslot = 0;
                    ffs_processed                     = false;
                }
            }

            // note: at this point not all events are processed necessarily!
            // but they are processed when simulation is resumed, so no worries
            if (m_current_time > timeout)
            {
                break;
            }

            // process all events of the current point in time
            u32 processed = 0;
            for (; processed < m_event_queue.size() && m_event_queue[processed].time <= m_current_time; ++processed)
            {
                auto& event = m_event_queue[processed];

                // is there already a value recorded for the net?
                if (auto it = m_simulation.m_events.find(event.affected_net); it != m_simulation.m_events.end())
                {
                    // if the event does not change anything, skip it
                    if (it->second.back().new_value == event.new_value)
                    {
                        continue;
                    }
                    // if the event does change something, but there was already an event for this point in time, we simply update the value
                    else if (it->second.back().time == event.time)
                    {
                        it->second.back().new_value = event.new_value;
                        if (it->second.size() > 1 && it->second[it->second.size() - 2].new_value == event.new_value)
                        {
                            it->second.pop_back();
                        }
                    }
                    else    // new event
                    {
                        m_simulation.m_events[event.affected_net].push_back(event);
                    }
                }
                else    // no value recorded -> new event
                {
                    m_simulation.m_events[event.affected_net].push_back(event);
                }

                // simulate affected gates
                // record all FFs that have to be clocked
                if (auto suc_it = m_successors.find(event.affected_net); suc_it != m_successors.end())
                {
                    for (auto& [gate, pins] : suc_it->second)
                    {
                        for (auto& pin : pins)
                        {
                            gate->input_values[pin] = static_cast<BooleanFunction::Value>(event.new_value);
                        }
                        if (!simulate_gate(gate, event, new_events))
                        {
                            ffs.push_back(static_cast<SimulationGateFF*>(gate));
                        }
                    }
                }
            }

            // check for iteration limit
            total_iterations_for_one_timeslot += processed;
            if (m_timeout_iterations > 0 && total_iterations_for_one_timeslot > m_timeout_iterations)
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
        }

        // adjust point in time
        m_current_time = timeout;
    }

    bool NetlistSimulator::simulate_gate(SimulationGate* gate, Event& event, std::map<std::pair<Net*, u64>, SignalValue>& new_events)
    {
        // compute delay, currently just a placeholder
        u64 delay = 0;

        // compute output for flip flop
        if (gate->is_flip_flop)
        {
            auto ff = static_cast<SimulationGateFF*>(gate);

            // is the event triggering a clock pin?
            // if so remember to process the flipflop later!
            if (std::find(ff->clock_nets.begin(), ff->clock_nets.end(), event.affected_net) != ff->clock_nets.end())
            {
                // return true if the event was completely handled
                // -> true if the gate is NOT clocked at this point
                return (ff->clock_func.evaluate(ff->input_values) != BooleanFunction::ONE);
            }
            else    // not a clock pin -> only check for asynchronous signals
            {
                auto async_set   = ff->preset_func.evaluate(ff->input_values);
                auto async_reset = ff->clear_func.evaluate(ff->input_values);

                // check whether an asynchronous set or reset ist triggered
                if (async_set == BooleanFunction::ONE || async_reset == BooleanFunction::ONE)
                {
                    SignalValue result     = SignalValue::X;
                    SignalValue inv_result = SignalValue::X;

                    if (async_set == BooleanFunction::ONE && async_reset == BooleanFunction::ONE)
                    {
                        // both signals set? -> evaluate special behavior
                        SignalValue old_output = SignalValue::X;
                        if (!ff->state_output_nets.empty())
                        {
                            auto out_net = ff->state_output_nets[0];
                            if (auto it = m_simulation.m_events.find(out_net); it != m_simulation.m_events.end())
                            {
                                old_output = it->second.back().new_value;
                            }
                        }
                        SignalValue old_output_inv = SignalValue::X;
                        if (!ff->state_inverted_output_nets.empty())
                        {
                            auto out_net = ff->state_inverted_output_nets[0];
                            if (auto it = m_simulation.m_events.find(out_net); it != m_simulation.m_events.end())
                            {
                                old_output_inv = it->second.back().new_value;
                            }
                        }
                        result     = process_set_reset_behavior(ff->sr_behavior_out, old_output);
                        inv_result = process_set_reset_behavior(ff->sr_behavior_out_inverted, old_output_inv);
                    }
                    else if (async_set == BooleanFunction::ONE)
                    {
                        // only asynch set is 1
                        result     = SignalValue::ONE;
                        inv_result = toggle(result);
                    }
                    else if (async_reset == BooleanFunction::ONE)
                    {
                        // only asynch reset is 1
                        result     = SignalValue::ZERO;
                        inv_result = toggle(result);
                    }

                    // generate events
                    for (auto out_net : ff->state_output_nets)
                    {
                        new_events[std::make_pair(out_net, m_current_time + delay)] = result;
                    }
                    for (auto out_net : ff->state_inverted_output_nets)
                    {
                        new_events[std::make_pair(out_net, m_current_time + delay)] = inv_result;
                    }
                }
            }
        }
        else    // compute output for combinational gate
        {
            auto comb = static_cast<SimulationGateCombinational*>(gate);
            for (auto out_net : comb->output_nets)
            {
                auto result = comb->functions[out_net](comb->input_values);

                new_events[std::make_pair(out_net, m_current_time + delay)] = static_cast<SignalValue>(result);
            }
        }
        return true;
    }

    void NetlistSimulator::simulate_ff(SimulationGateFF* gate, std::map<std::pair<Net*, u64>, SignalValue>& new_events)
    {
        // compute delay, currently just a placeholder
        u64 delay = 0;

        // compute output
        auto result     = static_cast<SignalValue>(gate->next_state_func.evaluate(gate->input_values));
        auto inv_result = toggle(result);

        // generate events
        for (auto out_net : gate->state_output_nets)
        {
            new_events[std::make_pair(out_net, m_current_time + delay)] = result;
        }
        for (auto out_net : gate->state_inverted_output_nets)
        {
            new_events[std::make_pair(out_net, m_current_time + delay)] = inv_result;
        }
    }

    SignalValue NetlistSimulator::process_set_reset_behavior(GateTypeSequential::SetResetBehavior behavior, SignalValue previous_output)
    {
        if (behavior == GateTypeSequential::SetResetBehavior::U)
        {
            log_warning("netlist simulator", "undefined simultaneous set/reset behavior encountered");
            return SignalValue::X;
        }
        else if (behavior == GateTypeSequential::SetResetBehavior::N)
        {
            return previous_output;
        }
        else if (behavior == GateTypeSequential::SetResetBehavior::X)
        {
            return SignalValue::X;
        }
        else if (behavior == GateTypeSequential::SetResetBehavior::L)
        {
            return SignalValue::ZERO;
        }
        else if (behavior == GateTypeSequential::SetResetBehavior::H)
        {
            return SignalValue::ONE;
        }
        else if (behavior == GateTypeSequential::SetResetBehavior::T)
        {
            return toggle(previous_output);
        }
        log_error("netlist simulator", "unsupported set/reset behavior {}", behavior);
        return SignalValue::X;
    }

    bool NetlistSimulator::generate_vcd(const std::filesystem::path& path, u32 start_time, u32 end_time) const
    {
        if (m_simulation_set.empty())
        {
            log_error("netlist simulator", "no gates have been added to the simulator.");
            return false;
        }

        if (m_simulation.get_events().empty())
        {
            log_error("netlist simulator", "nothing has been simulated, cannot generate VCD.");
            return false;
        }

        if (end_time > m_current_time)
        {
            log_error("netlist simulator", "cannot generate VCD for {} ps, only {} ps have been simulated thus far.", end_time, m_current_time);
            return false;
        }

        // write header
        std::stringstream vcd;
        auto t  = std::time(nullptr);
        auto tm = *std::localtime(&t);
        vcd << "$version Generated by HAL $HAL" << std::endl;
        vcd << "$date " << std::put_time(&tm, "%d-%m-%Y %H-%M-%S") << std::endl;
        vcd << "$timescale 1ps $end" << std::endl;

        //declare variables
        vcd << "$scope module TOP $end" << std::endl;

        auto events = m_simulation.get_events();
        std::vector<Net*> simulated_nets;

        for (auto net_changes : events)
        {
            auto net = net_changes.first;
            // maping net ids to net names
            vcd << "$var wire 1 n" << net->get_id() << " " << net->get_name() << " $end" << std::endl;

            // collect all simulated nets
            simulated_nets.push_back(net);
        }

        vcd << "$upscope $end" << std::endl;
        vcd << "$enddefinitions $end" << std::endl;

        std::unordered_map<Net*, SignalValue> change_tracker;
        vcd << "#" << 0 << std::endl;

        std::map<u32, std::map<Net*, SignalValue>> time_to_changes_map;

        std::unordered_map<Net*, std::vector<Event>> event_tracker = m_simulation.get_events();

        for (const auto& simulated_net : simulated_nets)
        {
            std::vector<Event> net_events = event_tracker.at(simulated_net);
            SignalValue initial_value;
            u32 initial_time = 0;
            for (const auto& event_it : net_events)
            {
                u32 event_time = event_it.time;
                if (initial_time == event_time || ((event_time > initial_time) && (event_time < start_time)))
                {
                    initial_time = event_time;
                    initial_value = event_it.new_value;
                }
                if (event_time > start_time && event_time < end_time)
                {
                    time_to_changes_map[event_it.time][simulated_net] = event_it.new_value;
                }
            }
            time_to_changes_map[start_time][simulated_net] = initial_value;

        }

        u32 traces_count = 0;

        for (const auto& [event_time, changed_nets] : time_to_changes_map)
        {
            traces_count++;
            vcd << "#" << event_time << std::endl;

            for (const auto& [net, value] : changed_nets)
            {
                //  print signal value
                if (value == SignalValue::X)
                {
                    vcd << "xn" << net->get_id() << std::endl;
                }
                else if (value == SignalValue::ONE)
                {
                    vcd << "1n" << net->get_id() << std::endl;
                }
                else if (value == SignalValue::ZERO)
                {
                    vcd << "0n" << net->get_id() << std::endl;
                }
                else if (value == SignalValue::Z)
                {
                    log_error("netlist simulator", "signal value of 'Z' for net with ID {} at {} ps is currently not supported.", net->get_id(), time);
                    return false;
                }
                else
                {
                    log_error("netlist simulator", "signal value for net with ID {} at {} ps is unknown.", net->get_id(), time);
                    return false;
                }
            }
        }
        vcd << "#" << traces_count << std::endl;

        std::ofstream ofs(path);
        if (!ofs.is_open())
        {
            log_error("netlist_simulator", "could not open file '{}' for writing.", path.string());
            return false;
        }

        ofs << vcd.str();

        return true;
    }
}    // namespace hal
