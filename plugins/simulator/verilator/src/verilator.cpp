
#include "verilator/verilator.h"
#include "verilator/templates.h"

#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_type.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_utils.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/utils.h"

#include "hal_core/netlist/netlist_writer/netlist_writer_manager.h"
#include "hal_core/plugin_system/plugin_manager.h"

#include "netlist_simulator_controller/simulation_input.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace hal {
namespace verilator {
    namespace converter {
        std::vector<std::string> get_vector_for_const_char(const char** txt)
        {
            std::vector<std::string> retval;
            for (int i=0; txt[i]; i++)
            {
                retval.push_back(std::string(txt[i]));
            }
            return retval;
        }
    }

    void remove_unwanted_parameters_from_netlist(Netlist* nl)
    {
        for (const auto& gate : nl->get_gates()) {
            gate->delete_data("generic", "X_COORDINATE");
            gate->delete_data("generic", "Y_COORDINATE");

            if (!gate->get_type()->has_property(hal::GateTypeProperty::lut) && gate->get_type()->has_property(hal::GateTypeProperty::combinational)) {
                gate->delete_data("generic", "INIT");
            }
        }
    }

    std::string escape_net_name(std::string net_name){
        std::string new_net_name;
        new_net_name = utils::replace(net_name, std::string("'"), std::string("__027"));
        return new_net_name;
    }

    const int VerilatorEngine::s_command_lines = 3;

    bool VerilatorEngine::setSimulationInput(SimulationInput* simInput)
    {
        const std::vector<const Gate*> simulation_gates(simInput->get_gates().begin(), simInput->get_gates().end());
        m_partial_netlist = netlist_utils::get_partial_netlist(simulation_gates.at(0)->get_netlist(), simulation_gates);

        remove_unwanted_parameters_from_netlist(m_partial_netlist.get());

        m_simulator_dir = directory();
        m_design_name = m_partial_netlist->get_design_name();
        if (m_design_name.empty())
        {
            m_design_name = "dummy";
            log_warning("verilator", "no design name for partial metlist, set to '{}'.", m_design_name);
        }
        else
            std::cerr << "m_design_name = <" << m_design_name << ">" << std::endl;

        std::filesystem::path netlist_verilog = m_simulator_dir / std::string(m_partial_netlist->get_design_name() + ".v");

        std::filesystem::path provided_models;
        auto it = mProperties.find(std::string("provided_models"));
        if (it == mProperties.end())
            log_info("verilator", "the property 'provided_models' has not been set (use set_engine_property method to assign).");
        else
            provided_models = it->second;

        // todo: delete folder if not empty
        if (std::filesystem::exists(m_simulator_dir)) {
            std::filesystem::remove_all(m_simulator_dir);
        }
        std::filesystem::create_directories(m_simulator_dir);

        std::vector<SimulationInput::Clock> clocks = simInput->get_clocks();

        // set inputs in testbench
        std::string testbench_cpp = get_testbench_cpp_template();
        std::stringstream simulation_data;

        log_info("verilator", "simulating {} net events", simInput->get_simulation_net_events().size());
        
        u64 partial_testbenches;

        for (const auto& sim_event : simInput->get_simulation_net_events()) {
            u32 duration = sim_event.get_simulation_duration();
            for (const auto& [net, boolean_value] : sim_event) {
                simulation_data << "\tdut->" << escape_net_name(net->get_name()) << " = ";

                switch (boolean_value) {
                case BooleanFunction::Value::ONE:
                    simulation_data << "0x1";
                    break;

                case BooleanFunction::Value::ZERO:
                    simulation_data << "0x0";
                    break;

                default:
                    log_error("verilator", "cannot assign value '{}' to net '{}' at {}ps, only supporting 1 and 0, aborting...", BooleanFunction::to_string(boolean_value), net->get_name(), duration);
                    return false;
                    break;
                }
                simulation_data << ";" << std::endl;
            }
            simulation_data << "\tdut->eval();" << std::endl;
            simulation_data << "\tm_trace->dump(sim_time);" << std::endl;
            simulation_data << "\tsim_time += " << duration << ";" << std::endl;
            simulation_data << std::endl;
        }
        simulation_data << "\tm_trace->dump(sim_time);" << std::endl;

        testbench_cpp = utils::replace(testbench_cpp, std::string("<insert_trace_here>"), simulation_data.str());
        testbench_cpp = utils::replace(testbench_cpp, std::string("<top_system>"), m_partial_netlist->get_design_name());

        std::ofstream testbench_cpp_file(m_simulator_dir / "testbench.cpp");
        testbench_cpp_file << testbench_cpp;
        testbench_cpp_file.close();

        if (!converter::convert_gate_library_to_verilog(m_partial_netlist.get(), m_simulator_dir, provided_models)) {
            log_error("verilator", "could not create gate definitions in verilog");
            return false;
        };

        netlist_writer_manager::write(m_partial_netlist.get(), netlist_verilog);

        return true; // everything ok
    }

    int VerilatorEngine::numberCommandLines() const
    {
        return s_command_lines;
    }

    std::vector<std::string> VerilatorEngine::commandLine(int lineIndex) const
    {
        // returns commands to be executed
        switch (lineIndex) {
        case 0: {
            const char* cl[] = { "verilator", "-I.", "-Wall", "-Wno-fatal", "--MMD", "-trace", "-y", "gate_definitions/",
                "--Mdir", "obj_dir", "--exe", "-cc", "-DSIM_VERILATOR", "--trace-depth", "2",
                "testbench.cpp", nullptr};
            std::vector<std::string> retval = converter::get_vector_for_const_char(cl);
            retval.push_back(m_design_name + ".v");
            return retval;
        } break;
        case 1: {
            const char* cl[] = { "make", "-j8", "--no-print-directory", "-C", "obj_dir/", "-f", nullptr };
            std::vector<std::string> retval = converter::get_vector_for_const_char(cl);
            retval.push_back("V" + m_design_name + ".mk");
            return retval;
        } break;
        case 2: {
            std::vector<std::string> retval;
            retval.push_back("obj_dir/V" + m_design_name);
            return retval;
        } break;
        default:
            break;
        }

        return std::vector<std::string>();
    }

    VerilatorEngine::VerilatorEngine(const std::string& nam)
        : SimulationEngineScripted(nam)
    {
        mRequireClockEvents = true;
    }

    bool VerilatorEngine::finalize()
    {
        mResultFilename = std::string(m_simulator_dir / "waveform.vcd");
        mState = Done;
        return true;
    }

    SimulationEngine* VerilatorEngineFactory::createEngine() const
    {
        return new VerilatorEngine(mName);
    }

} // namespace verilator
}
