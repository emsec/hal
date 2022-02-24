#include "netlist_simulator/netlist_simulator.h"

#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/init_component.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"
#include "netlist_simulator/simulation_utils.h"
#include "netlist_simulator_controller/simulation_input.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <unordered_map>

namespace hal
{
#define measure_block_time(X)

    NetlistSimulator::NetlistSimulator(const std::string &nam)
        : SimulationEngineEventDriven(nam)
    {;}

    void NetlistSimulator::set_input(const Net* net, BooleanFunction::Value value)
    {
        if (net == nullptr)
        {
            log_error("netlist_simulator", "net is a nullptr.");
            return;
        }

        if (auto it = m_simulation.m_events.find(net); it != m_simulation.m_events.end())
        {
            if (value == it->second.back().new_value)
            {
                return;
            }
        }

        WaveEvent e;
        e.affected_net = net;
        e.time         = m_current_time;
        e.new_value    = value;
        m_event_queue.push_back(e);
    }

    void NetlistSimulator::initialize_sequential_gates(const std::function<bool(const Gate*)>& filter)
    {
        if (!m_is_initialized)
        {
            m_init_seq_gates.push_back(std::make_tuple(true, BooleanFunction::Value::X, filter));
        }
        else
        {
            log_error("netlist_simulator", "cannot initialize sequential gates after the simulation was started.");
            return;
        }
    }

    void NetlistSimulator::initialize_sequential_gates(BooleanFunction::Value value, const std::function<bool(const Gate*)>& filter)
    {
        if (!m_is_initialized)
        {
            m_init_seq_gates.push_back(std::make_tuple(false, value, filter));
        }
        else
        {
            log_error("netlist_simulator", "cannot initialize sequential gates after the simulation was started.");
            return;
        }
    }

    void NetlistSimulator::load_initial_values(BooleanFunction::Value value)
    {
        // has to work even if the simulation was not started, i.e., initialize was not called yet
        // so we cannot use the SimulationGateFF type

        for (const Gate* gate : mSimulationInput->get_gates())
        {
            if (gate->get_type()->has_property(GateTypeProperty::ff))
            {
                GateType* gate_type                                       = gate->get_type();
                const std::unordered_map<std::string, PinType>& pin_types = gate_type->get_pin_types();

                BooleanFunction::Value inv_value = simulation_utils::toggle(value);

                // generate events
                for (Endpoint* ep : gate->get_fan_out_endpoints())
                {
                    if (pin_types.at(ep->get_pin()) == PinType::state)
                    {
                        WaveEvent e;
                        e.affected_net = ep->get_net();
                        e.new_value    = value;
                        e.time         = m_current_time;
                        m_event_queue.push_back(e);
                    }
                    else if (pin_types.at(ep->get_pin()) == PinType::neg_state)
                    {
                        WaveEvent e;
                        e.affected_net = ep->get_net();
                        e.new_value    = inv_value;
                        e.time         = m_current_time;
                        m_event_queue.push_back(e);
                    }
                }
            }
        }
    }

    void NetlistSimulator::load_initial_values_from_netlist()
    {
        // has to work even if the simulation was not started, i.e., initialize was not called yet
        // so we cannot use the SimulationGateFF type

        for (const Gate* gate : mSimulationInput->get_gates())
        {
            if (gate->get_type()->has_property(GateTypeProperty::ff))
            {
                GateType* gate_type                                       = gate->get_type();
                const std::unordered_map<std::string, PinType>& pin_types = gate_type->get_pin_types();

                // extract init string
                const InitComponent* init_component =
                    gate->get_type()->get_component_as<InitComponent>([](const GateTypeComponent* component) { return component->get_type() == GateTypeComponent::ComponentType::init; });
                if (init_component == nullptr)
                {
                    continue;
                }
                const std::string& init_str = std::get<1>(gate->get_data(init_component->get_init_category(), init_component->get_init_identifiers().front()));

                if (!init_str.empty())
                {
                    // parse init string

                    BooleanFunction::Value value = BooleanFunction::Value::X;
                    if (init_str == "1")
                    {
                        value = BooleanFunction::Value::ONE;
                    }
                    else if (init_str == "0")
                    {
                        value = BooleanFunction::Value::ZERO;
                    }
                    else
                    {
                        log_error("netlist_simulator", "init value of sequential gate '{}' (type '{}') is neither '1' or '0'.", gate->get_name(), gate_type->get_name());
                    }

                    BooleanFunction::Value inv_value = simulation_utils::toggle(value);

                    // generate events
                    for (Endpoint* ep : gate->get_fan_out_endpoints())
                    {
                        if (pin_types.at(ep->get_pin()) == PinType::state)
                        {
                            WaveEvent e;
                            e.affected_net = ep->get_net();
                            e.new_value    = value;
                            e.time         = m_current_time;
                            m_event_queue.push_back(e);
                        }
                        else if (pin_types.at(ep->get_pin()) == PinType::neg_state)
                        {
                            WaveEvent e;
                            e.affected_net = ep->get_net();
                            e.new_value    = inv_value;
                            e.time         = m_current_time;
                            m_event_queue.push_back(e);
                        }
                    }
                }
            }
        }
    }

    void NetlistSimulator::simulate(u64 picoseconds)
    {
        if (!m_is_initialized)
        {
            initialize();
        }

        prepare_clock_events(picoseconds);

        process_events(m_current_time + picoseconds);
    }

    void NetlistSimulator::reset()
    {
        mSimulationInput->clear();
        m_current_time = 0;
        m_id_counter   = 0;
        m_simulation   = Simulation();
        m_event_queue.clear();
        m_is_initialized = false;
    }

    void NetlistSimulator::set_simulation_state(const Simulation& state)
    {
        m_simulation = state;
    }

    const Simulation& NetlistSimulator::get_simulation_state() const
    {
        return m_simulation;
    }


    void NetlistSimulator::set_iteration_timeout(u64 iterations)
    {
        m_timeout_iterations = iterations;
    }

    u64 NetlistSimulator::get_simulation_timeout() const
    {
        return m_timeout_iterations;
    }

    std::vector<WaveEvent> NetlistSimulator::get_simulation_events(u32 netId) const
    {
        return m_simulation.get_events_by_net_id(netId);
    }

    bool NetlistSimulator::inputEvent(const SimulationInputNetEvent& netEv)
    {
        for (auto it = netEv.begin(); it != netEv.end(); ++it)
        {
            set_input(it->first, it->second);
        }
        simulate(netEv.get_simulation_duration());
        return true;
    }


    /*
     * This function precomputes all the stuff that shall be cached for simulation.
     */
    void NetlistSimulator::initialize()
    {
        measure_block_time("NetlistSimulator::initialize()");
        m_successors.clear();
        m_sim_gates.clear();
        m_sim_gates_raw.clear();

        std::unordered_map<const Gate*, SimulationGate*> sim_gates_map;
        std::unordered_set<const Net*> all_nets;
        std::map<const Net*, BooleanFunction::Value> init_events;

        // precompute everything that is gate-related
        for (const Gate* gate : mSimulationInput->get_gates())
        {
            SimulationGate* sim_gate_base = nullptr;

            if (gate->get_type()->has_property(GateTypeProperty::ff))
            {
                std::unique_ptr<SimulationGateFF> sim_gate_owner = std::make_unique<SimulationGateFF>(gate);
                SimulationGateFF* sim_gate                       = sim_gate_owner.get();
                sim_gate_base                                    = sim_gate;
                m_sim_gates.push_back(std::move(sim_gate_owner));

                for (const auto& [from_netlist, value, filter] : m_init_seq_gates)
                {
                    if (!filter || filter(gate))
                    {
                        sim_gate->initialize(init_events, from_netlist, value);
                    }
                }
            }
            else if (gate->get_type()->has_property(GateTypeProperty::ram))
            {
                std::unique_ptr<SimulationGateRAM> sim_gate_owner = std::make_unique<SimulationGateRAM>(gate);
                SimulationGateRAM* sim_gate                       = sim_gate_owner.get();
                sim_gate_base                                     = sim_gate;
                m_sim_gates.push_back(std::move(sim_gate_owner));

                for (const auto& [from_netlist, value, filter] : m_init_seq_gates)
                {
                    if (!filter || filter(gate))
                    {
                        sim_gate->initialize(init_events, from_netlist, value);
                    }
                }
            }
            else if (gate->get_type()->has_property(GateTypeProperty::combinational))
            {
                std::unique_ptr<SimulationGateCombinational> sim_gate_owner = std::make_unique<SimulationGateCombinational>(gate);
                SimulationGateCombinational* sim_gate                       = sim_gate_owner.get();
                sim_gate_base                                               = sim_gate;
                m_sim_gates.push_back(std::move(sim_gate_owner));
            }
            else
            {
                log_error("netlist_simulator", "no support for gate type {} of gate {}.", gate->get_type()->get_name(), gate->get_name());
                m_successors.clear();
                m_sim_gates.clear();
                return;
            }

            sim_gates_map.emplace(gate, sim_gate_base);
            m_sim_gates_raw.push_back(sim_gate_base);

            std::vector<Net*> out_nets = gate->get_fan_out_nets();
            all_nets.insert(out_nets.begin(), out_nets.end());
        }

        const std::unordered_set<const Net*>& inets = mSimulationInput->get_input_nets();
        all_nets.insert(inets.begin(), inets.end());

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
                if (!mSimulationInput->contains_gate(gate))
                {
                    continue;
                }
                auto sim_gate = sim_gates_map.at(gate);
                m_successors[net].emplace_back(sim_gate, pins);
            }
        }

        // create one-time events for global gnd and vcc gates
        for (auto g : mSimulationInput->get_gates())
        {
            if (g->is_gnd_gate())
            {
                for (auto n : g->get_fan_out_nets())
                {
                    init_events[n] = BooleanFunction::Value::ZERO;
                }
            }
            else if (g->is_vcc_gate())
            {
                for (auto n : g->get_fan_out_nets())
                {
                    init_events[n] = BooleanFunction::Value::ONE;
                }
            }
        }

        // set initial values
        for (const auto& [net, value] : init_events)
        {
            WaveEvent e;
            e.affected_net = net;
            e.new_value    = value;
            e.time         = m_current_time;
            m_event_queue.push_back(e);
        }

        // set initialization flag only if this point is reached
        m_is_initialized = true;
    }

    void NetlistSimulator::prepare_clock_events(u64 picoseconds)
    {
        for (const SimulationInput::Clock& c : mSimulationInput->get_clocks())
        {
            u64 base_time = m_current_time - (m_current_time % c.switch_time);
            u64 time      = 0;

            // determine next signal state
            // works also if we are in the middle of a cycle
            BooleanFunction::Value v = static_cast<BooleanFunction::Value>(base_time & 1);
            if (!c.start_at_zero)
            {
                v = simulation_utils::toggle(v);
            }

            // insert the required amount of clock signal switch events
            while (time < picoseconds)
            {
                WaveEvent e;
                e.affected_net = c.clock_net;
                e.new_value    = v;
                e.time         = base_time + time;
                m_event_queue.push_back(e);

                v = simulation_utils::toggle(v);
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
        std::vector<SimulationGateSequential*> clocked_gates;
        bool clocked_gates_processed = false;

        while (!m_event_queue.empty() || !clocked_gates.empty())
        {
            std::map<std::pair<const Net*, u64>, BooleanFunction::Value> new_events;

            // sort events by time
            std::sort(m_event_queue.begin(), m_event_queue.end());

            // queue empty or all events of the current point in time processed?
            if (m_event_queue.empty() || m_current_time != m_event_queue[0].time)
            {
                // are there FFs that were clocked? process them now!
                if (!clocked_gates.empty() && !clocked_gates_processed)
                {
                    for (SimulationGateSequential* clocked_gate : clocked_gates)
                    {
                        clocked_gate->clock(m_current_time, new_events);
                    }
                    clocked_gates.clear();
                    clocked_gates_processed = true;
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
                    clocked_gates_processed           = false;
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
                            gate->m_input_values[pin] = event.new_value;
                        }
                        if (!gate->simulate(m_simulation, event, new_events))
                        {
                            clocked_gates.push_back(static_cast<SimulationGateSequential*>(gate));
                        }
                    }
                }
            }

            // check for iteration limit
            total_iterations_for_one_timeslot += processed;
            if (m_timeout_iterations > 0 && total_iterations_for_one_timeslot > m_timeout_iterations)
            {
                log_error("netlist_simulator", "reached iteration timeout of {} without advancing in time, aborting simulation. Please check for a combinational loop.", m_timeout_iterations);
                return;
            }

            // remove processed events
            m_event_queue.erase(m_event_queue.begin(), m_event_queue.begin() + processed);

            // add new events
            m_event_queue.reserve(m_event_queue.size() + new_events.size());
            for (const auto& event_it : new_events)
            {
                WaveEvent e;
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

    BooleanFunction::Value NetlistSimulator::process_clear_preset_behavior(AsyncSetResetBehavior behavior, BooleanFunction::Value previous_output)
    {
        if (behavior == AsyncSetResetBehavior::N)
        {
            return previous_output;
        }
        else if (behavior == AsyncSetResetBehavior::X)
        {
            return BooleanFunction::Value::X;
        }
        else if (behavior == AsyncSetResetBehavior::L)
        {
            return BooleanFunction::Value::ZERO;
        }
        else if (behavior == AsyncSetResetBehavior::H)
        {
            return BooleanFunction::Value::ONE;
        }
        else if (behavior == AsyncSetResetBehavior::T)
        {
            return simulation_utils::toggle(previous_output);
        }
        log_error("netlist_simulator", "unsupported set/reset behavior {}.", enum_to_string(behavior));
        return BooleanFunction::Value::X;
    }

    bool NetlistSimulator::generate_vcd(const std::filesystem::path& path, u32 start_time, u32 end_time, std::set<const Net*> nets) const
    {
        if (mSimulationInput->get_gates().empty())
        {
            log_error("netlist_simulator", "no gates have been added to the simulator.");
            return false;
        }

        if (m_simulation.get_events().empty())
        {
            log_error("netlist_simulator", "nothing has been simulated, cannot generate VCD.");
            return false;
        }

        if (end_time > m_current_time)
        {
            log_error("netlist_simulator", "cannot generate VCD for {} ps, only {} ps have been simulated thus far.", end_time, m_current_time);
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

        std::unordered_map<const Net*, std::vector<WaveEvent>> events = m_simulation.get_events();
        std::vector<const Net*> simulated_nets;

        for (auto net_changes : events)
        {
            const Net* net = net_changes.first;
            if ((net != nullptr) && (nets.empty() || nets.find(net) != nets.end()))
            {
                // maping net ids to net names
                vcd << "$var wire 1 n" << net->get_id() << " " << net->get_name() << " $end" << std::endl;

                // collect all simulated nets
                simulated_nets.push_back(net);
            }
        }

        vcd << "$upscope $end" << std::endl;
        vcd << "$enddefinitions $end" << std::endl;

        std::unordered_map<const Net*, BooleanFunction::Value> change_tracker;
        vcd << "#" << 0 << std::endl;

        std::map<u32, std::map<const Net*, BooleanFunction::Value>> time_to_changes_map;

        std::unordered_map<const Net*, std::vector<WaveEvent>> event_tracker = m_simulation.get_events();

        for (const auto& simulated_net : simulated_nets)
        {
            std::vector<WaveEvent> net_events        = event_tracker.at(simulated_net);
            BooleanFunction::Value initial_value = BooleanFunction::Value::X;
            u32 initial_time                     = 0;

            for (const auto& event_it : net_events)
            {
                u32 event_time = event_it.time;
                if (initial_time == event_time || ((event_time > initial_time) && (event_time < start_time)))
                {
                    initial_time  = event_time;
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
                if (value == BooleanFunction::Value::X)
                {
                    vcd << "xn" << net->get_id() << std::endl;
                }
                else if (value == BooleanFunction::Value::ONE)
                {
                    vcd << "1n" << net->get_id() << std::endl;
                }
                else if (value == BooleanFunction::Value::ZERO)
                {
                    vcd << "0n" << net->get_id() << std::endl;
                }
                else if (value == BooleanFunction::Value::Z)
                {
                    log_error("netlist_simulator", "signal value of 'Z' for net with ID {} at {} ps is currently not supported.", net->get_id(), event_time);
                    return false;
                }
                else
                {
                    log_error("netlist_simulator", "signal value for net with ID {} at {} ps is unknown.", net->get_id(), event_time);
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

    SimulationEngine* NetlistSimulatorFactory::createEngine() const
    {
        return new NetlistSimulator(mName);
    }

}    // namespace hal
