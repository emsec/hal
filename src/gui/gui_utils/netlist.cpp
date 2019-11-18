#include "gui_utils/netlist.h"
#include "gui/gui_globals.h"

#include "netlist/gate.h"
#include "netlist/module.h"

// TODO Consider these for moving into the core if they are useful

#include <unordered_set>

namespace gui_utility
{

    std::shared_ptr<module> lowest_common_ancestor(std::shared_ptr<module> m1, std::shared_ptr<module> m2)
    {
        std::unordered_set<std::shared_ptr<module>> common;
        common.insert(m1);
        common.insert(m2);
        while (true)
        {
            std::shared_ptr<module> p1 = m1->get_parent_module();
            if (common.find(p1) != common.end())
                return p1;
            common.insert(p1);
            m1 = p1;
            std::shared_ptr<module> p2 = m2->get_parent_module();
            if (common.find(p2) != common.end())
                return p2;
            common.insert(p2);
            m2 = p2;
        }
    }

    std::shared_ptr<module> lowest_common_ancestor(std::unordered_set<std::shared_ptr<module>> modules, std::unordered_set<std::shared_ptr<gate>> gates)
    {
        if (modules.empty() && gates.empty())
        {
            return nullptr;
        }
        // resolve all gates to their parent modules, since we don't want to work with gates
        while(!gates.empty())
        {
            std::shared_ptr<gate> g = *gates.begin();
            gates.erase(g);
            std::shared_ptr<module> m = g->get_module();
            modules.insert(m);
        }
        // pick two modules and resolve them to their lowest common ancestor,
        // insert that back into the set and loop until only 1 module remains
        while(modules.size() >= 2)
        {
            std::shared_ptr<module> m1 = *modules.begin();
            modules.erase(m1);
            std::shared_ptr<module> m2 = *modules.begin();
            modules.erase(m2);
            std::shared_ptr<module> common = lowest_common_ancestor(m1, m2);
            modules.insert(common);
        }
        // the remaining module is the lowest common ancestor of all elements
        return *modules.begin();
    }

} // namespace gui_utility