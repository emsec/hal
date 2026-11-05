#include "hal_core/netlist/gate_library/gate_library_parser/gate_library_parser.h"

#include <atomic>

namespace hal
{
    namespace
    {
        // parsers are created for a single parse each, so the outcome must outlive them
        std::atomic<GateLibraryParser::ParseStatus> s_parse_status(GateLibraryParser::ParseStatus::NoError);
        std::atomic<bool> s_upgrade_requested(false);
    }    // namespace

    GateLibraryParser::GateLibraryParser()
    {
        s_parse_status = ParseStatus::UnknownError;
    }

    GateLibraryParser::~GateLibraryParser()
    {
        s_upgrade_requested = false;
    }

    GateLibraryParser::ParseStatus GateLibraryParser::get_parse_status()
    {
        return s_parse_status;
    }

    void GateLibraryParser::request_upgrade()
    {
        s_upgrade_requested = true;
    }

    void GateLibraryParser::set_parse_status(ParseStatus status)
    {
        s_parse_status = status;
    }

    bool GateLibraryParser::is_upgrade_requested()
    {
        return s_upgrade_requested;
    }
}    // namespace hal
