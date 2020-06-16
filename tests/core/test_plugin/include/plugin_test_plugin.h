
#ifndef HAL_PLUGIN_TEST_PLUGIN_H
#define HAL_PLUGIN_TEST_PLUGIN_H

#include "core/plugin_interface_cli.h"
#include "core/plugin_interface_base.h"

/* forward declaration */
class netlist;
class gate;
class net;


class PLUGIN_API plugin_test_plugin : virtual public CLIPluginInterface //CLIPluginInterface
{
public:
    /** constructor (= default) */
    plugin_test_plugin() = default;

    /** destructor (= default) */
    ~plugin_test_plugin() = default;

    /*
     *      interface implementations
     */

    /** interface implementation: BasePluginInterface */
    std::string get_name() const override;

    /** interface implementation: BasePluginInterface */
    std::string get_version() const override;

    /** interface implementation: CLIPluginInterface */
    ProgramOptions get_cli_options() const override;

    /** interface implementation: CLIPluginInterface */
    bool handle_cli_call(std::shared_ptr<netlist> nl, ProgramArguments& args) override;

};

#endif //HAL_PLUGIN_TEST_PLUGIN_H
