#ifndef __PLUGIN_IGRAPH_H__
#define __PLUGIN_IGRAPH_H__

#include "core/interface_base.h"

#include "def.h"
#include <igraph/igraph.h>


/* forward declaration */
class netlist;

class PLUGIN_API plugin_igraph : virtual public i_base
{
public:

    /*
     *      interface implementations
     */

    plugin_igraph() = default;
    ~plugin_igraph() = default;

    std::string get_name() override;

    std::string get_version() override;

    std::set<interface_type> get_type() override;


    igraph_t get_igraph_directed(std::shared_ptr<netlist> nl);


};

#endif /* __PLUGIN_IGRAPH_H__ */
