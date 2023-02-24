#pragma once

#include "hal_core/defines.h"
#include "hal_core/plugin_system/plugin_interface_base.h"
#include "hal_core/utilities/result.h"
#include "hal_core/netlist/netlist.h"

namespace hal
{
    class ModulePin;

    class PLUGIN_API BitorderPropagationPlugin : public BasePluginInterface
    {
    public:
        std::string get_name() const override;
        std::string get_version() const override;

        void initialize() override;

        /**
         * Propagates known bit order information to module pin groups with unknown bit order.
         * Afterwards the algorithm tries to reconstrcut valid bit orders from the propagated information.
         *
         * @param[in] known_bitorders - The known indices for the nets belonging to module pin groups. 
         * @param[in] unknown_bitorders - The module pin groups with yet unknown bit order.
         * @param[in] strict_consens_finding - When set to true this option only allows for complete and continous bitorders, while false would allow for such orders to be formed.
         * @returns A mapping of all the known bit orders consisting of the new and old.
         */
        static Result<std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>>>
            propagate_module_pingroup_bitorder(const std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>>& known_bitorders,
                                               const std::set<std::pair<Module*, PinGroup<ModulePin>*>>& unknown_bitorders,
                                               const bool strict_consens_finding = true);

        /**
         * This function reorders and renames the pins of the pin groups according to the corresponding bit order information. 
         *
         * @param[in] ordered_module_pin_groups - A mapping from all the modules and pin groups with known bit order information to this bit order information mapping every net to its coressponding index.
         * @returns OK if everything worked, Error otherwise.
         */
        static Result<std::monostate> reorder_module_pin_groups(const std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>>& ordered_module_pin_groups);

        /**
         * Propagates known bit order information to module pin groups with unknown bit order.
         * Afterwards the algorithm tries to reconstrcut valid bit orders from the propagated information.
         * The valid bit orders are then annotated to the module pin groups.
         *
         * @param[in] src - The module / pin group pair with known bit order. (The pins of the pin group have to be in the right order already) 
         * @param[in] unknown_bitorders - The module / pin group pair with unknown bit order.
         * @returns OK and a map containing all known bitorders consisting of the new and already known ones or an Error.
         */
        static Result<std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>>>
            propagate_bitorder(Netlist* nl, const std::pair<u32, std::string>& src, const std::pair<u32, std::string>& dst);

        /**
         * Propagates known bit order information to module pin groups with unknown bit order.
         * Afterwards the algorithm tries to reconstrcut valid bit orders from the propagated information.
         * The valid bit orders are then annotated to the module pin groups.
         *
         * @param[in] src - The module / pin group pair with known bit order. (The pins of the pin group have to be in the right order already) 
         * @param[in] unknown_bitorders - The module / pin group pair with unknown bit order.
         * @returns OK and a map containing all known bitorders consisting of the new and already known ones or an Error.
         */
        static Result<std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>>> propagate_bitorder(const std::pair<Module*, PinGroup<ModulePin>*>& src,
                                                                                                                  const std::pair<Module*, PinGroup<ModulePin>*>& dst);

        /**
         * Propagates known bit order information to module pin groups with unknown bit order.
         * Afterwards the algorithm tries to reconstrcut valid bit orders from the propagated information.
         * The valid bit orders are then annotated to the module pin groups.
         *
         * @param[in] src - The module / pin group pairs with known bit order. (The pins of the pin group have to be in the right order already) 
         * @param[in] unknown_bitorders - The module / pin group pairs with unknown bit order.
         * @returns OK and a map containing all known bitorders consisting of the new and already known ones or an Error.
         */
        static Result<std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>>>
            propagate_bitorder(Netlist* nl, const std::vector<std::pair<u32, std::string>>& src, const std::vector<std::pair<u32, std::string>>& dst);

        /**
         * Propagates known bit order information to module pin groups with unknown bit order.
         * Afterwards the algorithm tries to reconstrcut valid bit orders from the propagated information.
         * The valid bit orders are then annotated to the module pin groups.
         *
         * @param[in] src - The module / pin group pairs with known bit order. (The pins of the pin group have to be in the right order already) 
         * @param[in] unknown_bitorders - The module / pin group pairs with unknown bit order.
         * @returns OK and a map containing all known bitorders consisting of the new and already known ones or an Error.
         */
        static Result<std::map<std::pair<Module*, PinGroup<ModulePin>*>, std::map<Net*, u32>>> propagate_bitorder(const std::vector<std::pair<Module*, PinGroup<ModulePin>*>>& src,
                                                                                                                  const std::vector<std::pair<Module*, PinGroup<ModulePin>*>>& dst);
    };
}    // namespace hal
