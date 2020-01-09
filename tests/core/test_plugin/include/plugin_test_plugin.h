
#ifndef HAL_PLUGIN_TEST_PLUGIN_H
#define HAL_PLUGIN_TEST_PLUGIN_H

#include "core/interface_cli.h"
#include "core/interface_base.h"

/* forward declaration */
class netlist;
class gate;
class net;


class PLUGIN_API plugin_test_plugin : virtual public i_cli //i_cli
{
public:
    /** constructor (= default) */
    plugin_test_plugin() = default;

    /** destructor (= default) */
    ~plugin_test_plugin() = default;

    /*
     *      interface implementations
     */

    /** interface implementation: i_base */
    std::string get_name() const override;

    /** interface implementation: i_base */
    std::string get_version() const override;

    /** interface implementation: i_cli */
    program_options get_cli_options() const override;

    /** interface implementation: i_cli */
    bool handle_cli_call(std::shared_ptr<netlist> nl, program_arguments& args) override;

};

#endif //HAL_PLUGIN_TEST_PLUGIN_H
