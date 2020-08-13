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

namespace hal
{
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
        u64 period = 1'000'000'000 / hertz;
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

    void NetlistSimulator::simulate(u64 nanoseconds)
    {
        if (m_needs_initialization)
        {
            initialize();
            m_needs_initialization = false;
        }

        prepare_clock_events(nanoseconds);

        process_events(m_current_time + nanoseconds);
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
        m_successors.clear();
        for (auto gate : m_simulation_set)
        {
            for (auto net : gate->get_fan_out_nets())
            {
                if (auto it = m_successors.find(net); it != m_successors.end())
                {
                    continue;
                }
                auto endpoints = net->get_destinations();
                std::vector<Gate*> dst_gates;
                std::transform(endpoints.begin(), endpoints.end(), std::back_inserter(dst_gates), [](auto& ep) { return ep.get_gate(); });
                dst_gates.erase(std::unique(dst_gates.begin(), dst_gates.end()), dst_gates.end());
                m_successors.emplace(net, dst_gates);
            }
        }
        for (auto net : m_input_nets)
        {
            if (auto it = m_successors.find(net); it != m_successors.end())
            {
                continue;
            }
            auto endpoints = net->get_destinations();
            std::vector<Gate*> dst_gates;
            std::transform(endpoints.begin(), endpoints.end(), std::back_inserter(dst_gates), [](auto& ep) { return ep.get_gate(); });
            dst_gates.erase(std::unique(dst_gates.begin(), dst_gates.end()), dst_gates.end());
            m_successors.emplace(net, dst_gates);
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

        log_info("netlist simulator", "initialization done");
        for (auto net : m_input_nets)
        {
            log_info("netlist simulator", "input net: {}", net->get_name());
        }
        for (auto net : m_output_nets)
        {
            log_info("netlist simulator", "output net: {}", net->get_name());
        }
    }

    void NetlistSimulator::prepare_clock_events(u64 nanoseconds)
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
            while (time < nanoseconds)
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
        auto sort_by_time = [](auto& a, auto& b) {
            if (a.time != b.time)
            {
                return a.time < b.time;
            }
            return a.id < b.id;
        };

        auto signal_to_string = [](auto v) -> std::string {
            if (v >= 0)
                return std::to_string(v);
            return "X";
        };

        u64 total_iterations_for_one_timeslot = 0;

        while (!m_event_queue.empty())
        {
            std::sort(m_event_queue.begin(), m_event_queue.end(), sort_by_time);

            if (m_current_time != m_event_queue[0].time)
            {
                m_current_time                    = m_event_queue[0].time;
                total_iterations_for_one_timeslot = 0;
                log_info("netlist simulator", "");
            }

            if (m_current_time > timeout)
            {
                break;
            }

            std::map<std::pair<Net*, u64>, SignalValue> new_events;
            u32 processed = 0;
            for (; processed < m_event_queue.size() && m_event_queue[processed].time <= m_current_time; ++processed)
            {
                auto& event = m_event_queue[processed];

                log_info("netlist simulator", "event: {} to {} at {}", event.affected_net->get_name(), signal_to_string(event.new_value), event.time);

                if (auto it = m_simulation.m_events.find(event.affected_net); it != m_simulation.m_events.end())
                {
                    if (it->second.back().new_value == event.new_value)
                    {
                        log_info("netlist simulator", "  no change -> skip");
                        continue;
                    }
                    else if (it->second.back().time == event.time)
                    {
                        it->second.back().new_value = event.new_value;
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
                for (auto successor : m_successors.at(event.affected_net))
                {
                    // compute delay
                    u64 delay = 0;

                    log_info("netlist simulator", "computing gate {}", successor->get_name());

                    // gather inputs
                    std::map<std::string, BooleanFunction::value> input_values;
                    for (auto pin : successor->get_input_pins())
                    {
                        auto net          = successor->get_fan_in_net(pin);
                        SignalValue value = SignalValue::X;
                        if (auto it = m_simulation.m_events.find(net); it != m_simulation.m_events.end())
                        {
                            value = it->second.back().new_value;
                        }
                        input_values.emplace(pin, static_cast<BooleanFunction::value>(value));
                        log_info("netlist simulator", "  pin {} = {}", pin, signal_to_string(value));
                    }

                    // compute output
                    if (successor->get_type()->get_base_type() == GateType::BaseType::ff)
                    {
                        auto gate_type = dynamic_cast<const GateTypeSequential*>(successor->get_type());

                        auto async_set   = successor->get_boolean_function("preset").evaluate(input_values);
                        auto async_reset = successor->get_boolean_function("clear").evaluate(input_values);

                        SignalValue result = SignalValue::X;
                        bool change_state  = false;

                        auto output_pins     = gate_type->get_state_output_pins();
                        auto inv_output_pins = gate_type->get_inverted_state_output_pins();

                        if (async_set == BooleanFunction::value::ONE && async_reset == BooleanFunction::value::ONE)
                        {
                            change_state   = true;
                            auto [q, qnot] = gate_type->get_set_reset_behavior();
                            if (q == GateTypeSequential::SetResetBehavior::U)
                            {
                                log_error("netlist simulator", "simultaneous set/reset behavior is undefined for gate {}", successor->get_name());
                                change_state = false;
                            }
                            else if (q == GateTypeSequential::SetResetBehavior::N)
                            {
                                change_state = false;
                            }
                            else if (q == GateTypeSequential::SetResetBehavior::X)
                            {
                                result = SignalValue::X;
                            }
                            else if (q == GateTypeSequential::SetResetBehavior::L)
                            {
                                result = SignalValue::ZERO;
                            }
                            else if (q == GateTypeSequential::SetResetBehavior::H)
                            {
                                result = SignalValue::ONE;
                            }
                            else if (q == GateTypeSequential::SetResetBehavior::T)
                            {
                                SignalValue value = SignalValue::X;
                                auto out_net      = successor->get_fan_out_net(*output_pins.begin());
                                if (auto it = m_simulation.m_events.find(out_net); it != m_simulation.m_events.end())
                                {
                                    value = it->second.back().new_value;
                                }
                                result = toggle(value);
                            }
                        }
                        else if (async_set == BooleanFunction::ONE)
                        {
                            change_state = true;
                            result       = SignalValue::ONE;
                        }
                        else if (async_reset == BooleanFunction::ONE)
                        {
                            change_state = true;
                            result       = SignalValue::ZERO;
                        }
                        else
                        {
                            auto clocked_on_func    = successor->get_boolean_function("clock");
                            bool is_change_to_clock = false;
                            for (auto pin : clocked_on_func.get_variables())
                            {
                                if (successor->get_fan_in_net(pin) == event.affected_net)
                                {
                                    is_change_to_clock = true;
                                    break;
                                }
                            }
                            if (!is_change_to_clock)
                            {
                                log_info("netlist simulator", "  not connected to a clock pin -> skip");
                                continue;
                            }
                            log_info("netlist simulator", "  clocked on {}", clocked_on_func.to_string());
                            if (clocked_on_func.evaluate(input_values) != BooleanFunction::ONE)
                            {
                                log_info("netlist simulator", "  clock function not satisfied -> skip");
                                continue;
                            }

                            auto f = successor->get_boolean_function("next_state");
                            result = static_cast<SignalValue>(f.evaluate(input_values));
                            log_info("netlist simulator", "  state = {} = {}", f.to_string(), result);
                            change_state = true;
                        }

                        if (change_state)
                        {
                            auto inv_result = toggle(result);

                            for (const auto& pin : output_pins)
                            {
                                auto out_net                                                = successor->get_fan_out_net(pin);
                                new_events[std::make_pair(out_net, m_current_time + delay)] = result;
                            }

                            for (const auto& pin : inv_output_pins)
                            {
                                auto out_net                                                = successor->get_fan_out_net(pin);
                                new_events[std::make_pair(out_net, m_current_time + delay)] = inv_result;
                            }
                        }
                    }
                    else
                    {
                        for (auto pin : successor->get_output_pins())
                        {
                            auto f = successor->get_boolean_function(pin);

                            auto result = f.evaluate(input_values);

                            auto out_net = successor->get_fan_out_net(pin);

                            log_info("netlist simulator", "  {} = {} = {} -> {}", pin, f.to_string(), result, out_net->get_name());

                            new_events[std::make_pair(out_net, m_current_time + delay)] = static_cast<SignalValue>(result);
                        }
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
            log_info("netlist simulator", "-- added new events --");
        }

        m_current_time = timeout;
        log_info("netlist simulator", "");
    }

}    // namespace hal
