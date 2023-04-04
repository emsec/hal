#include "plugin_z3_utils.h"

#include "z3_utils.h"

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<Z3UtilsPlugin>();
    }

    std::string Z3UtilsPlugin::get_name() const
    {
        return std::string("z3_utils");
    }

    std::string Z3UtilsPlugin::get_version() const
    {
        return std::string("0.1");
    }

    void Z3UtilsPlugin::initialize()
    {
    }

    BooleanFunction Z3UtilsPlugin::get_subgraph_function_py(const Net* n, const std::vector<Gate*>& sub_graph_gates) const
    {
        z3::context ctx;

        const auto res = z3_utils::get_subgraph_z3_function(sub_graph_gates, n, ctx);
        if (res.is_error())
        {
            log_error("z3_utils", "{}", res.get_error().get());
        }

        BooleanFunction bf = z3_utils::to_bf(res.get()).get();
        // std::cout << "Got bf: " << bf.to_string() << std::endl;

        return bf;
    }

}    // namespace hal
