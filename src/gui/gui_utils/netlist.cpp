#include "gui_utils/netlist.h"
#include "gui/gui_globals.h"

#include "netlist/gate.h"
#include "netlist/module.h"

// TODO Consider these for moving into the core if they are useful

#include <unordered_set>

namespace gui_utility
{
    std::shared_ptr<module> first_common_ancestor(std::shared_ptr<module> m1, std::shared_ptr<module> m2)
    {
        std::unordered_set<u32> parents_m1;
        while (m1 != nullptr)
        {
            parents_m1.insert(m1->get_id());
            m1 = m1->get_parent_module();
        }
        while (m2 != nullptr)
        {
            if (parents_m1.find(m2->get_id()) != parents_m1.end())
            {
                return m2;
            }
            m2 = m2->get_parent_module();
        }
        return nullptr;
    }

    std::shared_ptr<module> first_common_ancestor(std::unordered_set<std::shared_ptr<module>> modules, const std::unordered_set<std::shared_ptr<gate>>& gates)
    {
        if (modules.empty() && gates.empty())
        {
            return nullptr;
        }
        // resolve all gates to their parent modules, since we don't want to work with gates
        for (const auto& g : gates)
        {
            modules.insert(g->get_module());
        }
        // pick two modules and resolve them to their first common ancestor,
        auto module_list = std::vector<std::shared_ptr<module>>(modules.begin(), modules.end());
        auto result      = module_list[0];
        for (u32 i = 1; i < module_list.size(); ++i)
        {
            // if the top module is the first common ancestor at any time, we can stop searching (early exit)
            if (result->get_parent_module() == nullptr)
            {
                break;
            }
            result = first_common_ancestor(result, module_list[i]);
        }
        // the final module is the first common ancestor of all elements
        return result;
    }

    QSet<u32> parent_modules(std::shared_ptr<gate> g)
    {
        assert(g);
        QSet<u32> parents;
        std::shared_ptr<module> m = g->get_module();
        while (m != nullptr)
        {
            parents.insert(m->get_id());
            m = m->get_parent_module();
        }
        return parents;
    }
}    // namespace gui_utility
