#include "genlib_writer/genlib_writer.h"

#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/utilities/log.h"

#include <fstream>
#include <clocale>

namespace hal
{
    namespace
    {
        std::string double_to_string(double d)
        {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(4) << d;
            return oss.str();
        }
    }    // namespace

    bool GenlibWriter::write(const GateLibrary* gate_lib, const std::filesystem::path& file_path)
    {
        std::setlocale(LC_NUMERIC, "en_US");

        // set name
        std::string file_str = "#" + gate_lib->get_name() + "\n";

        // process combinational gates
        for (const auto& [name, gt] : gate_lib->get_gate_types([](const GateType* gt) { return gt->has_property(GateTypeProperty::combinational); }))
        {
            if (gt->get_boolean_functions().empty())
            {
                log_warning("GenlibWriter", "Skipping gate type {} because eventhough it is marked as combinational it does not have any boolean functions.", name);
                continue;
            }

            if (gt->get_boolean_functions().size() > 1)
            {
                log_warning("GenlibWriter",
                            "Skipping gate type {} because it contains {} Boolean functions, but the genlib format only supports single output cells.",
                            name,
                            gt->get_boolean_functions().size());
                continue;
            }

            // TODO make this read out the area from the gate library (currently not implemented)
            const double gate_area = gt->has_property(GateTypeProperty::c_mux) ? 1.2 : (gt->get_input_pins().size() * 0.3 + 1);
            auto [output_pin, bf]  = *(gt->get_boolean_functions().begin());
            // simplify to get rid of XOR which we currently cannot translate to genlib
            bf          = bf.simplify();
            auto bf_str = bf.is_constant() ? (bf.has_constant_value(0) ? "CONST0" : "CONST1") : bf.to_string();

            // TODO: this is the stupid way, but i do not care at the moment
            bf_str = utils::replace(bf_str, std::string("|"), std::string("+"));
            bf_str = utils::replace(bf_str, std::string("&"), std::string("*"));

            std::string gate_str = "GATE " + name + " " + double_to_string(gate_area) + " " + output_pin + "=" + bf_str + ";\n";

            for (const auto& pin : gt->get_input_pins())
            {
                const std::string phase   = (bf.get_top_level_node().is_operation() && (bf.get_top_level_node().type == BooleanFunction::NodeType::Not)) ? "INV" : "NONINV";
                const std::string pin_str = "PIN " + pin->get_name() + " " + phase + " 1 999 1 0 1 0";
                gate_str += pin_str + "\n";
            }

            file_str += gate_str;
        }

        // process sequential gates
        for (const auto& [name, gt] : gate_lib->get_gate_types([](const GateType* gt) { return gt->has_property(GateTypeProperty::sequential); }))
        {
            log_warning("GenlibWriter", "Skipping gate type {} because sequential gates are currently not supported", name);
            continue;
        }

        // write generated string to file
        std::ofstream file(file_path);
        file << file_str;
        file.close();

        return true;
    }

}    // namespace hal
