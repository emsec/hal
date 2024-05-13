#include "hawkeye/candidate_search.h"

#include "hal_core/netlist/decorators/netlist_traversal_decorator.h"

namespace hal
{
    namespace hawkeye
    {
        Result<std::vector<Candidate>> detect_candidates(Netlist* nl, const std::vector<DetectionConfiguration>& configs, u32 min_state_size, const std::vector<Gate*>& start_ffs)
        {
            if (nl == nullptr)
            {
                return ERR("netlist is a nullptr");
            }

            const auto nl_dec = NetlistTraversalDecorator(*nl);
            const auto res    = nl_dec.get_next_sequential_gates_map(true, {PinType::enable, PinType::reset, PinType::set, PinType::clock});
            if (res.is_error())
            {
                return ERR(res.get_error());
            }
            const auto seq_map = res.get();

            for (const auto& config : configs)
            {
            }

            // TODO for each Detection configuration, create a NetlistGraph

            // TODO run neighborhood stuff and potentiall SCC detection on NetlistGraph
            // neighborhood: feed all currently considered FFs as start vertices; after every iteration, sort out start vertices of saturating neighborhoods as well as equivalent ones
            // SCCs: compute SCCs within neighborhoods

            return ERR("not implemented");
        }
    }    // namespace hawkeye
}    // namespace hal