#include "vhdl_parser/vhdl_parser.h"

#include "hal_core/netlist/gate_library/gate_library.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"
#include "vhdl_parser/vhdl_elaboration.h"
#include "vhdl_parser/vhdl_syntax.h"

#include <chrono>

namespace hal
{
    namespace vhdl
    {
        Result<netlist_ir::Design> parse_to_ir(const std::filesystem::path& file)
        {
            auto parsed = parse_file(file);
            if (parsed.is_error())
            {
                return ERR(parsed.get_error());
            }
            return elaborate(parsed.get());
        }

        Result<netlist_ir::Design> parse_text_to_ir(const std::string& text, const std::filesystem::path& file)
        {
            auto parsed = parse_string(text, file);
            if (parsed.is_error())
            {
                return ERR(parsed.get_error());
            }
            return elaborate(parsed.get());
        }
    }    // namespace vhdl

    Result<std::monostate> VHDLParser::parse(const std::filesystem::path& file_path)
    {
        const auto begin = std::chrono::steady_clock::now();
        m_design.reset();

        auto res = vhdl::parse_to_ir(file_path);
        if (res.is_error())
        {
            return ERR_APPEND(res.get_error(), "could not parse VHDL file '" + file_path.string() + "'");
        }
        m_design = res.get();

        log_info("vhdl_parser", "parsed '{}' in {:2.2f} seconds.", file_path.string(), std::chrono::duration<double>(std::chrono::steady_clock::now() - begin).count());
        return OK({});
    }

    Result<std::unique_ptr<Netlist>> VHDLParser::instantiate(const GateLibrary* gate_library)
    {
        if (!m_design.has_value())
        {
            return ERR("could not instantiate VHDL netlist: no file has been parsed");
        }
        const auto begin = std::chrono::steady_clock::now();

        auto res = netlist_ir::instantiate(m_design.value(), gate_library, m_options);
        if (res.is_error())
        {
            return ERR_APPEND(res.get_error(), "could not instantiate VHDL netlist '" + m_design->source + "' with gate library '" + (gate_library ? gate_library->get_name() : "") + "'");
        }

        log_info("vhdl_parser", "instantiated '{}' in {:2.2f} seconds.", m_design->source, std::chrono::duration<double>(std::chrono::steady_clock::now() - begin).count());
        return res;
    }

    const std::optional<netlist_ir::Design>& VHDLParser::get_design() const
    {
        return m_design;
    }

    void VHDLParser::set_instantiation_options(const netlist_ir::InstantiationOptions& options)
    {
        m_options = options;
    }
}    // namespace hal
