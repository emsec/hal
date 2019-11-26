//
// Created by ubuntu on 9/28/19.
//

#ifndef HAL_PLUGIN_TEST_PLUGIN_H
#define HAL_PLUGIN_TEST_PLUGIN_H


#include "core/interface_base.h"

//#include <hal_bdd.h>

#include <igraph/igraph.h>
#include "core/interface_cli.h"
#include "core/interface_base.h"

/* forward declaration */
class netlist;
class gate;
class net;

#ifdef FIXME

class PLUGIN_API plugin_test_plugin : virtual public i_cli
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
    std::string get_name() override;

    /** interface implementation: i_base */
    std::string get_version() override;

    /** interface implementation: i_base */
    std::set<interface_type> get_type() override;


    /** interface implementation: i_cli */
    program_options get_cli_options() override;



    /** interface implementation: i_cli */
    bool handle_cli_call(std::shared_ptr<netlist> nl, program_arguments& args) override;

};
#endif // FIXME

#endif //HAL_PLUGIN_TEST_PLUGIN_H
