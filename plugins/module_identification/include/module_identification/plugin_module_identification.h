#pragma once

#include "hal_core/plugin_system/gui_extension_interface.h"
#include "hal_core/plugin_system/plugin_interface_base.h"
#include "module_identification/types/candidate_types.h"
#include "module_identification/types/multithreading_types.h"

#include <memory.h>

namespace hal
{
    class Gate;
    class Module;

    namespace module_identification
    {
        class BaseCandidate;
        class StructuralCandidate;
    }

    /**
     * @class GuiExtensionModuleIdentification
     * @brief GUI extension interface for the module identification plugin.
     * 
     * This class provides a GUI extension interface for the module identification plugin, allowing for parameter configuration,
     * progress indication, and context menu contributions.
     */
    class GuiExtensionModuleIdentification : public GuiExtensionInterface
    {
        /**
         * @brief Maximum number of threads to be used.
         * Default is 1.
         */
        u32 m_max_thread_count = 1;

        /**
         * @brief Maximum number of control signals.
         * Default is 3.
         */
        u32 m_max_control_signals = 3;

        /**
         * @brief Flag to enable or disable multithreading.
         * Default is false.
         */
        bool m_do_multithreading = false;

        /**
         * @brief Flag to indicate if the button was clicked.
         * Default is false.
         */
        bool m_button_clicked = false;

        /**
         * @brief Check selector value.
         * Default is 0.
         */
        u32 m_check_selector = 0;

        /**
         * @brief Multithreading priority configuration.
         * Default is none.
         */
        module_identification::MultithreadingPriority m_multithreading_priority = module_identification::MultithreadingPriority::memory_priority;

        /**
         * @brief Vector of candidate types to check.
         */
        std::vector<module_identification::CandidateType> m_types_to_check = module_identification::all_checkable_candidate_types;

    public:
        /**
         * @brief Default constructor for `GuiExtensionModuleIdentification`.
         */
        GuiExtensionModuleIdentification() = default;

        /**
         * @brief Get list of configurable parameters.
         *
         * @returns List of parameters.
         */
        std::vector<PluginParameter> get_parameter() const override;

        /**
         * @brief Set configurable parameters to values.
         * 
         * @param[in] params - The parameters with values.
         */
        void set_parameter(const std::vector<PluginParameter>& params) override;

        /**
         * @brief Execute a function with a specific tag on a netlist.
         * 
         * @param[in] tag - The tag of the function to execute.
         * @param[in] nl - The netlist to operate on.
         * @param[in] mods - The modules involved.
         * @param[in] gats - The gates involved.
         * @param[in] nets - The nets involved.
         */
        void execute_function(std::string tag, Netlist* nl, const std::vector<u32>& mods, const std::vector<u32>& gats, const std::vector<u32>& nets) override;

        /**
         * @brief Register function to indicate work progress when busy.
         * 
         * @param[in] pif - Progress Indicator Function to register.
         */
        virtual void register_progress_indicator(std::function<void(int, const std::string&)> pif) override;

        /**
         * @brief Static progress indicator function.
         */
        static std::function<void(int, const std::string&)> s_progress_indicator_function;

        /**
         * @brief Get context menu contributions based on the current netlist and selection.
         * 
         * @param[in] nl - The netlist to operate on.
         * @param[in] mods - The selected modules.
         * @param[in] gats - The selected gates.
         * @param[in] nets - The selected nets.
         * @returns Vector of context menu contributions.
         */
        std::vector<ContextMenuContribution> get_context_contribution(const Netlist* nl, const std::vector<u32>& mods, const std::vector<u32>& gats, const std::vector<u32>& nets);
    };

    /**
     * @class ModuleIdentificationPlugin
     * @brief Plugin for identifying functional modules within a netlist.
     * 
     * This plugin identifies groups of gates in a netlist that fulfill a recognized function, primarily focusing on arithmetic operations.
     */
    class PLUGIN_API ModuleIdentificationPlugin : public BasePluginInterface
    {
    public:
        /**
         * @brief Constructor for `ModuleIdentificationPlugin`.
         */
        ModuleIdentificationPlugin();

        /**
         * @brief Default destructor for `ModuleIdentificationPlugin`.
         */
        ~ModuleIdentificationPlugin() = default;

        /**
         * @brief Get the name of the plugin.
         *
         * @returns The name of the plugin.
         */
        std::string get_name() const override;

        /**
         * @brief Get a short description of the plugin.
         *
         * @returns The short description.
         */
        std::string get_description() const override;

        /**
         * @brief Get the version of the plugin.
         *
         * @returns The version of the plugin.
         */
        std::string get_version() const override;

        /**
         * @brief Initialize the plugin.
         */
        void initialize() override;
    };
}    // namespace hal
