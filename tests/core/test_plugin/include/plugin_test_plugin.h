//
// Created by ubuntu on 9/28/19.
//

#ifndef HAL_PLUGIN_TEST_PLUGIN_H
#define HAL_PLUGIN_TEST_PLUGIN_H


#include "core/interface_base.h"

#include <hal_bdd.h>

#include <igraph/igraph.h>

/* forward declaration */
class netlist;
class gate;
class net;

class PLUGIN_API plugin_test_plugin : public i_base
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

};

#endif //HAL_PLUGIN_TEST_PLUGIN_H
