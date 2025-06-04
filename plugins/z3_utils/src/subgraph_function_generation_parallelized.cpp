#include "hal_core/netlist/decorators/boolean_function_net_decorator.h"
#include "hal_core/netlist/decorators/subgraph_netlist_decorator.h"
#include "hal_core/netlist/endpoint.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/utilities/log.h"
#include "z3_utils/subgraph_function_generation_parallelized.h"
#include "z3_utils/z3_utils.h"

#include <atomic>
#include <functional>
#include <memory>
#include <shared_mutex>
#include <thread>
#include <vector>

namespace hal
{
    namespace z3_utils
    {
        namespace
        {
            struct Z3SharedState
            {
                z3::context& ctx;
                std::shared_mutex ctx_mutex;

                std::vector<std::shared_ptr<z3::expr>> net_cache;

                Z3SharedState(z3::context& c, size_t max_net_id) : ctx(c), net_cache(max_net_id + 1 /* all slots initially nullptr */)
                {
                }
            };

            Result<z3::expr> get_subgraph_z3_function_parallelized_recursive(const std::function<bool(const Gate*)> subgraph_filter, const Net* net, Z3SharedState& shared)
            {
                const u32 net_id = net->get_id();

                // 1) Try lock-free read of net_cache[net_id].
                {
                    std::shared_ptr<z3::expr> existing = std::atomic_load_explicit(&shared.net_cache[net_id], std::memory_order_acquire);
                    if (existing)
                    {
                        // Already computed by another thread.
                        return OK(*existing);
                    }
                }

                const std::vector<Endpoint*> sources = net->get_sources();

                // net is multi driven
                if (sources.size() > 1)
                {
                    return ERR("cannot get Boolean z3 function of net " + net->get_name() + " with ID " + std::to_string(net->get_id()) + ": net is multi driven.");
                }

                // net has no source
                if (sources.empty())
                {
                    z3::expr leaf(shared.ctx);
                    {
                        std::unique_lock<std::shared_mutex> lock(shared.ctx_mutex);
                        leaf = shared.ctx.bv_const(BooleanFunctionNetDecorator(*net).get_boolean_variable_name().c_str(), 1);
                    }
                    // Publish via atomic_compare_exchange
                    auto new_ptr                       = std::make_shared<z3::expr>(std::move(leaf));
                    std::shared_ptr<z3::expr> expected = nullptr;
                    if (std::atomic_compare_exchange_strong_explicit(&shared.net_cache[net_id], &expected, new_ptr, std::memory_order_release, std::memory_order_relaxed))
                    {
                        return OK(*new_ptr);
                    }
                    // If someone beat us, drop new_ptr and use the existing one
                    return OK(*expected);
                }

                const Endpoint* src_ep = sources.front();
                if (src_ep->get_gate() == nullptr)
                {
                    return ERR("cannot get Boolean z3 function of net " + net->get_name() + " with ID " + std::to_string(net->get_id()) + ": net source is null.");
                }

                const Gate* src = src_ep->get_gate();

                // 4) If source-gate is not in subgraph, treat net as leaf variable
                if (!subgraph_filter(src))
                {
                    z3::expr leaf(shared.ctx);
                    {
                        std::unique_lock<std::shared_mutex> lock(shared.ctx_mutex);
                        leaf = shared.ctx.bv_const(BooleanFunctionNetDecorator(*net).get_boolean_variable_name().c_str(), 1);
                    }
                    auto new_ptr                       = std::make_shared<z3::expr>(std::move(leaf));
                    std::shared_ptr<z3::expr> expected = nullptr;
                    if (std::atomic_compare_exchange_strong_explicit(&shared.net_cache[net_id], &expected, new_ptr, std::memory_order_release, std::memory_order_relaxed))
                    {
                        return OK(*new_ptr);
                    }
                    return OK(*expected);
                }

                const auto bf_res = src->get_resolved_boolean_function(src_ep->get_pin());
                if (bf_res.is_error())
                {
                    return ERR_APPEND(bf_res.get_error(),
                                      "cannot get Boolean z3 function of net " + net->get_name() + " with ID " + std::to_string(net->get_id()) + ": failed to get function of gate.");
                }
                const BooleanFunction bf = bf_res.get();

                // 6) Recurse on each input net from bf.get_variable_names()
                std::map<std::string, z3::expr> input_to_expr;
                for (auto const& in_net_str : bf.get_variable_names())
                {
                    auto in_net_res = BooleanFunctionNetDecorator::get_net_from(src->get_netlist(), in_net_str);
                    if (in_net_res.is_error())
                    {
                        return ERR_APPEND(in_net_res.get_error(),
                                          "cannot get Boolean Z3 function of net \"" + net->get_name() + "\" (ID " + std::to_string(net_id) + "): failed to reconstruct input net \"" + in_net_str
                                              + "\".");
                    }
                    const Net* in_net = in_net_res.get();

                    auto in_bf_res = get_subgraph_z3_function_parallelized_recursive(subgraph_filter, in_net, shared);
                    if (in_bf_res.is_error())
                    {
                        return in_bf_res;    // propagate
                    }
                    input_to_expr.emplace(in_net_str, in_bf_res.get());
                }

                // 7) Build & simplify the Z3 expression under ctx_mutex
                z3::expr result(shared.ctx);
                {
                    std::unique_lock<std::shared_mutex> lock(shared.ctx_mutex);
                    result = z3_utils::from_bf(bf, shared.ctx, input_to_expr).simplify();
                }

                // 8) Publish into net_cache via CAS
                auto new_ptr                       = std::make_shared<z3::expr>(std::move(result));
                std::shared_ptr<z3::expr> expected = nullptr;
                if (std::atomic_compare_exchange_strong_explicit(&shared.net_cache[net_id], &expected, new_ptr, std::memory_order_release, std::memory_order_relaxed))
                {
                    return OK(*new_ptr);
                }
                return OK(*expected);
            }

            Result<z3::expr> get_subgraph_z3_function_parallelized_internal(const std::function<bool(const Gate*)> subgraph_filter, const Net* net, Z3SharedState& shared)
            {
                if (net == nullptr)
                {
                    return ERR("could not get subgraph z3 function: net is a 'nullptr'");
                }
                else if (net->get_num_of_sources() > 1)
                {
                    return ERR("could not get subgraph z3 function of net '" + net->get_name() + "' with ID " + std::to_string(net->get_id()) + ": net has more than one source");
                }
                else if (net->is_global_input_net())
                {
                    z3::expr leaf(shared.ctx);
                    {
                        std::unique_lock<std::shared_mutex> lock(shared.ctx_mutex);
                        leaf = shared.ctx.bv_const(BooleanFunctionNetDecorator(*net).get_boolean_variable_name().c_str(), 1);
                    }
                    return OK(leaf);
                }
                else if (net->get_num_of_sources() == 0)
                {
                    return ERR("could not get subgraph function of net '" + net->get_name() + "' with ID " + std::to_string(net->get_id()) + ": net has no sources");
                }

                return get_subgraph_z3_function_parallelized_recursive(subgraph_filter, net, shared);
            }

        }    // namespace

        Result<std::vector<z3::expr>>
            get_subgraph_z3_functions_parallelized(const std::function<bool(const Gate*)> subgraph_filter, const std::vector<Net*>& subgraph_outputs, z3::context& ctx, const u32 num_threads)
        {
            if (subgraph_outputs.empty())
            {
                return OK({});
            }

            const Netlist* nl = subgraph_outputs.front()->get_netlist();

            u32 max_net_id = 0;
            for (const auto& n : nl->get_nets())
            {
                if (n->get_id() > max_net_id)
                {
                    max_net_id = n->get_id();
                }
            }

            const size_t N = subgraph_outputs.size();
            std::vector<z3::expr> results(N, z3::expr(ctx));

            // Shared Z3 state (context, caches, and their mutexes)
            Z3SharedState shared(ctx, max_net_id);

            // For collecting the first error
            std::mutex error_mutex;
            bool error_flag = false;
            Error error;

            // Atomic index for dynamic task assignment
            std::atomic<size_t> next_index{0};

            // Worker lambda: grab the next index atomically, process until done
            auto worker = [&]() {
                while (true)
                {
                    size_t i = next_index.fetch_add(1, std::memory_order_relaxed);
                    if (i >= N)
                    {
                        break;
                    }

                    const Net* net = subgraph_outputs[i];
                    auto res       = get_subgraph_z3_function_parallelized_internal(subgraph_filter, net, shared);
                    if (res.is_error())
                    {
                        std::lock_guard<std::mutex> lock(error_mutex);
                        if (!error_flag)
                        {
                            error_flag = true;
                            error      = std::move(res.get_error());
                            return;
                        }
                    }
                    else
                    {
                        std::lock_guard<std::mutex> lock(error_mutex);
                        if (!error_flag)
                        {
                            results[i] = res.get();
                        }
                        else
                        {
                            return;
                        }
                    }
                }
            };

            std::vector<std::thread> threads;
            threads.reserve(num_threads);
            for (unsigned t = 0; t < num_threads; ++t)
            {
                threads.emplace_back(worker);
            }

            for (auto& t : threads)
            {
                if (t.joinable())
                {
                    t.join();
                }
            }

            if (error_flag)
            {
                return ERR_APPEND(error, "unable to generate subgraph functions");
            }
            return OK(results);
        }

        Result<std::vector<z3::expr>>
            get_subgraph_z3_functions_parallelized(const std::vector<Gate*>& subgraph_gates, const std::vector<Net*>& subgraph_outputs, z3::context& ctx, const u32 num_threads)
        {
            const std::function<bool(const Gate*)> subgraph_filter = [&subgraph_gates](const Gate* g) { return std::find(subgraph_gates.begin(), subgraph_gates.end(), g) != subgraph_gates.end(); };

            return get_subgraph_z3_functions_parallelized(subgraph_filter, subgraph_outputs, ctx, num_threads);
        }

        Result<std::vector<z3::expr>>
            get_subgraph_z3_functions_parallelized(const GateTypeProperty subgraph_property, const std::vector<Net*>& subgraph_outputs, z3::context& ctx, const u32 num_threads)
        {
            const std::function<bool(const Gate*)> subgraph_filter = [&subgraph_property](const Gate* g) { return g->get_type()->has_property(subgraph_property); };

            return get_subgraph_z3_functions_parallelized(subgraph_filter, subgraph_outputs, ctx, num_threads);
        }

    }    // namespace z3_utils
}    // namespace hal
