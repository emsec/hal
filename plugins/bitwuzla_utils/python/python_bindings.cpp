#include "hal_core/python_bindings/python_bindings.h"

#include "bitwuzla/cpp/parser.h"
#include "bitwuzla_utils/bitwuzla_utils.h"
#include "bitwuzla_utils/plugin_bitwuzla_utils.h"
#include "hal_core/netlist/boolean_function/solver.h"
#include "hal_core/netlist/decorators/subgraph_netlist_decorator.h"
#include "hal_core/netlist/endpoint.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/result.h"
#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11/stl_bind.h"
#include "z3_utils/include/z3_utils.h"

#include <chrono>

namespace py = pybind11;

namespace hal
{

    // the name in PYBIND11_MODULE/PYBIND11_PLUGIN *MUST* match the filename of the output library (without extension),
    // otherwise you will get "ImportError: dynamic module does not define module export function" when importing the module

#ifdef PYBIND11_MODULE
    PYBIND11_MODULE(bitwuzla_utils, m)
    {
        m.doc() = "hal BitwuzlaUtilsPlugin python bindings";
#else
    PYBIND11_PLUGIN(bitwuzla_utils)
    {
        py::module m("bitwuzla_utils", "hal BitwuzlaUtilsPlugin python bindings");
#endif    // ifdef PYBIND11_MODULE

        py::class_<BitwuzlaUtilsPlugin, RawPtrWrapper<BitwuzlaUtilsPlugin>, BasePluginInterface> py_bitwuzla_utils(m, "BitwuzlaUtilsPlugin");
        py_bitwuzla_utils.def_property_readonly("name", &BitwuzlaUtilsPlugin::get_name);
        py_bitwuzla_utils.def("get_name", &BitwuzlaUtilsPlugin::get_name);
        py_bitwuzla_utils.def_property_readonly("version", &BitwuzlaUtilsPlugin::get_version);
        py_bitwuzla_utils.def("get_version", &BitwuzlaUtilsPlugin::get_version);

        py_bitwuzla_utils.def_static(
            "test_on_net",
            [](const Netlist* nl, const Net* n) {
                const auto comb_gates = nl->get_gates([](const auto& g) { return g->get_type()->has_property(GateTypeProperty::combinational); });
                const auto bf_res     = bitwuzla_utils::get_subgraph_bitwuzla_function(comb_gates, n);
                if (bf_res.is_error())
                {
                    log_error("bitwuzla_utils", "{}", bf_res.get_error().get());
                    return;
                }

                const auto bf = bf_res.get();

                auto vars = bitwuzla_utils::get_variable_names(bf);
                std::cout << "vars:" << std::endl;
                for (const auto& var : vars)
                {
                    std::cout << "\t" << var << std::endl;
                }
                std::cout << "BF: " << bf << std::endl;
            },

            R"(

        )");

        py_bitwuzla_utils.def_static(
            "test_on_netlist",
            [](const Netlist* nl) {
                const auto comb_gates = nl->get_gates([](const auto& g) { return g->get_type()->has_property(GateTypeProperty::combinational); });

                for (const auto& n : nl->get_nets())
                {
                    const auto bf_res = bitwuzla_utils::get_subgraph_bitwuzla_function(comb_gates, n);
                    if (bf_res.is_error())
                    {
                        log_error("bitwuzla_utils", "{}", bf_res.get_error().get());
                        return;
                    }

                    const auto bf = bf_res.get();

                    std::cout << "BF: " << bf << std::endl;
                }
            },

            R"(

        )");

#define DURATION_UNIT std::chrono::milliseconds
#define UNIT_STRING " [ms]"

        py_bitwuzla_utils.def_static(
            "minimal_substitution",
            []() {
                // bitwuzla
                {
                    const bitwuzla::Term a = bitwuzla::mk_var(bitwuzla::mk_bv_sort(1), "a");
                    const bitwuzla::Term b = bitwuzla::mk_var(bitwuzla::mk_bv_sort(1), "b");

                    const bitwuzla::Term bf = bitwuzla::mk_term(bitwuzla::Kind::BV_AND, {a, b});

                    const std::unordered_map<std::string, bitwuzla::Term> sub_one_map  = {{"a", bitwuzla::mk_bv_one(bitwuzla::mk_bv_sort(1))}};
                    const std::unordered_map<std::string, bitwuzla::Term> sub_zero_map = {{"a", bitwuzla::mk_bv_zero(bitwuzla::mk_bv_sort(1))}};

                    const auto bf_sub_one  = bitwuzla::substitute_term(bf, sub_one_map);
                    const auto bf_sub_zero = bitwuzla::substitute_term(bf, sub_zero_map);

                    std::cout << "Vanilla: " << std::endl;
                    std::cout << bf << std::endl;
                    bitwuzla_utils::simplify(bf);

                    std::cout << "Sub One: " << std::endl;
                    std::cout << bf_sub_one << std::endl;
                    bitwuzla_utils::simplify(bf_sub_one);

                    std::cout << "Sub Zero: " << std::endl;
                    std::cout << bf_sub_zero << std::endl;
                    bitwuzla_utils::simplify(bf_sub_zero);
                }
            },
            R"(

        )");

        py_bitwuzla_utils.def_static(
            "benchmark_simplification",
            [](const Netlist* nl, const std::vector<Net*>& nets) {
                const auto comb_gates = nl->get_gates([](const auto& g) { return g->get_type()->has_property(GateTypeProperty::combinational); });

                // time creation of Boolean functions

                // bitwuzla
                {
                    auto start_bitwuzla_creation = std::chrono::steady_clock::now();

                    const auto res = bitwuzla_utils::get_subgraph_bitwuzla_functions(comb_gates, nets);
                    if (res.is_error())
                    {
                        log_error("bitwuzla_utils", "{}", res.get_error().get());
                        return;
                    }
                    const std::vector<bitwuzla::Term> bitwuzla_functions = res.get();

                    auto end_bitwuzla_creation = std::chrono::steady_clock::now();

                    std::cout << "bitwuzla cached function generation: " << std::chrono::duration_cast<DURATION_UNIT>(end_bitwuzla_creation - start_bitwuzla_creation).count() << UNIT_STRING
                              << std::endl;

                    auto start_substitution = std::chrono::steady_clock::now();
                    for (const auto& bf : bitwuzla_functions)
                    {
                        // const auto vars = bitwuzla_utils::get_variable_names(bf);

                        // if (vars.empty())
                        // {
                        //     std::cout << "Empty..." << std::endl;
                        //     continue;
                        // }

                        // const auto var = *(vars.begin());

                        const std::string var = "net_23914";
                        const auto sub_term   = bitwuzla::mk_var(bitwuzla::mk_bv_sort(1), var);

                        const std::unordered_map<bitwuzla::Term, bitwuzla::Term> sub_one_map  = {{sub_term, bitwuzla::mk_bv_one(bitwuzla::mk_bv_sort(1))}};
                        const std::unordered_map<bitwuzla::Term, bitwuzla::Term> sub_zero_map = {{sub_term, bitwuzla::mk_bv_zero(bitwuzla::mk_bv_sort(1))}};

                        const auto bf_sub_one  = bitwuzla::substitute_term(bf, sub_one_map);
                        const auto bf_sub_zero = bitwuzla::substitute_term(bf, sub_zero_map);

                        std::cout << "Vanilla: " << std::endl;
                        std::cout << bf << std::endl;

                        std::cout << "Sub Var: " << var << std::endl;
                        std::cout << "Sub Term: " << sub_term << std::endl;

                        std::cout << "Sub One: " << std::endl;
                        std::cout << bf_sub_one << std::endl;

                        std::cout << "Sub Zero: " << std::endl;
                        std::cout << bf_sub_zero << std::endl;

                        // bitwuzla::Options options;
                        // options.set(bitwuzla::Option::SAT_SOLVER, "cadical");
                        // bitwuzla::Bitwuzla bitwuzla(options);

                        // bitwuzla.assert_formula(bitwuzla::mk_term(bitwuzla::Kind::NOT, {bitwuzla::mk_term(bitwuzla::Kind::EQUAL, {bf, bf})}));
                        // bitwuzla::Result result = bitwuzla.check_sat();
                    }
                    auto end_substitution = std::chrono::steady_clock::now();

                    std::cout << "bitwuzla substitution:        " << std::chrono::duration_cast<DURATION_UNIT>(end_substitution - start_substitution).count() << UNIT_STRING << std::endl;
                }
            },
            R"(

        )");

        py_bitwuzla_utils.def_static(
            "benchmark_threaded_creation",
            [](const Netlist* nl, const std::vector<Net*>& nets, const u32 thread_num) {
                const auto comb_gates = nl->get_gates([](const auto& g) { return g->get_type()->has_property(GateTypeProperty::combinational); });

                // time creation of Boolean functions

                // bitwuzla
                {
                    auto start_bitwuzla_creation = std::chrono::steady_clock::now();

                    std::vector<bitwuzla::Term> bitwuzla_functions(nets.size());

                    const u32 correction  = (nets.size() % thread_num) ? 1 : 0;
                    const u32 packet_size = (nets.size() / thread_num) + correction;

                    std::vector<std::thread> workers;
                    for (u32 i = 0; i < thread_num; i++)
                    {
                        const u32 start = std::min(i * packet_size, u32(nets.size() - 1));
                        const u32 end   = std::min(start + packet_size, u32(nets.size()));

                        workers.emplace_back([&nets, &comb_gates, &bitwuzla_functions, start, end]() {
                            std::map<u32, bitwuzla::Term> thread_net_cache;
                            std::map<std::pair<u32, const GatePin*>, BooleanFunction> thread_gate_cache;

                            for (u32 idx = start; idx < end; idx++)
                            {
                                const bitwuzla::Term t = bitwuzla_utils::get_subgraph_bitwuzla_function(comb_gates, nets.at(idx), thread_net_cache, thread_gate_cache).get();
                                // bitwuzla_functions.at(idx) = t;
                            }
                        });
                    }

                    // wait for threads to finish
                    for (auto& worker : workers)
                    {
                        worker.join();
                    }

                    auto end_bitwuzla_creation = std::chrono::steady_clock::now();

                    std::cout << "bitwuzla threaded function creation: " << std::chrono::duration_cast<DURATION_UNIT>(end_bitwuzla_creation - start_bitwuzla_creation).count() << UNIT_STRING
                              << std::endl;
                }
            },
            R"(

        )");

        py_bitwuzla_utils.def_static(
            "benchmark_threaded_solving",
            [](const Netlist* nl, const std::vector<Net*>& nets, const u32 thread_num) {
                const auto comb_gates = nl->get_gates([](const auto& g) { return g->get_type()->has_property(GateTypeProperty::combinational); });

                // time creation of Boolean functions

                // bitwuzla
                {
                    auto start_bitwuzla_creation = std::chrono::steady_clock::now();

                    const auto res = bitwuzla_utils::get_subgraph_bitwuzla_functions(comb_gates, nets);
                    if (res.is_error())
                    {
                        log_error("bitwuzla_utils", "{}", res.get_error().get());
                        return;
                    }
                    const std::vector<bitwuzla::Term> bitwuzla_functions = res.get();

                    auto end_bitwuzla_creation = std::chrono::steady_clock::now();

                    std::cout << "bitwuzla cached function generation: " << std::chrono::duration_cast<DURATION_UNIT>(end_bitwuzla_creation - start_bitwuzla_creation).count() << UNIT_STRING
                              << std::endl;

                    auto start_static_cadical_bitwuzla_solving = std::chrono::steady_clock::now();

                    const u32 correction  = (nets.size() % thread_num) ? 1 : 0;
                    const u32 packet_size = (nets.size() / thread_num) + correction;

                    std::vector<std::thread> static_workers;
                    for (u32 i = 0; i < thread_num; i++)
                    {
                        const u32 start = std::min(i * packet_size, u32(nets.size() - 1));
                        const u32 end   = std::min(start + packet_size, u32(nets.size()));

                        static_workers.emplace_back([&bitwuzla_functions, start, end]() {
                            for (u32 idx = start; idx < end; idx++)
                            {
                                bitwuzla::Options options;
                                options.set(bitwuzla::Option::PRODUCE_MODELS, true);
                                options.set(bitwuzla::Option::SAT_SOLVER, "cadical");
                                bitwuzla::Bitwuzla bitwuzla(options);

                                const auto& bf = bitwuzla_functions.at(idx);

                                bitwuzla.assert_formula(bitwuzla::mk_term(bitwuzla::Kind::NOT, {bitwuzla::mk_term(bitwuzla::Kind::EQUAL, {bf, bf})}));
                                bitwuzla::Result result = bitwuzla.check_sat();
                            }
                        });
                    }

                    // wait for threads to finish
                    for (auto& worker : static_workers)
                    {
                        worker.join();
                    }

                    auto end_static_cadical_bitwuzla_solving = std::chrono::steady_clock::now();

                    std::cout << "bitwuzla static cadical smt solving: "
                              << std::chrono::duration_cast<DURATION_UNIT>(end_static_cadical_bitwuzla_solving - start_static_cadical_bitwuzla_solving).count() << UNIT_STRING << std::endl;

                    auto start_dynamic_cadical_bitwuzla_solving = std::chrono::steady_clock::now();

                    std::atomic<u32> curr_idx = 0;

                    std::vector<std::thread> dynamic_workers;
                    for (u32 i = 0; i < thread_num; i++)
                    {
                        dynamic_workers.emplace_back([&bitwuzla_functions, &curr_idx]() {
                            while (true)
                            {
                                const u32 idx = curr_idx++;
                                if (idx >= bitwuzla_functions.size())
                                {
                                    return;
                                }

                                bitwuzla::Options options;
                                options.set(bitwuzla::Option::PRODUCE_MODELS, true);
                                options.set(bitwuzla::Option::SAT_SOLVER, "cadical");
                                bitwuzla::Bitwuzla bitwuzla(options);

                                const auto& bf = bitwuzla_functions.at(idx);

                                bitwuzla.assert_formula(bitwuzla::mk_term(bitwuzla::Kind::NOT, {bitwuzla::mk_term(bitwuzla::Kind::EQUAL, {bf, bf})}));
                                bitwuzla::Result result = bitwuzla.check_sat();
                            }
                        });
                    }

                    // wait for threads to finish
                    for (auto& worker : dynamic_workers)
                    {
                        worker.join();
                    }

                    auto end_dynamic_cadical_bitwuzla_solving = std::chrono::steady_clock::now();

                    std::cout << "bitwuzla dynamic cadical smt solving: "
                              << std::chrono::duration_cast<DURATION_UNIT>(end_dynamic_cadical_bitwuzla_solving - start_dynamic_cadical_bitwuzla_solving).count() << UNIT_STRING << std::endl;
                }
            },
            R"(

        )");

        py_bitwuzla_utils.def_static(
            "benchmark",
            [](const Netlist* nl, const std::vector<Net*>& nets) {
                const auto comb_gates = nl->get_gates([](const auto& g) { return g->get_type()->has_property(GateTypeProperty::combinational); });

                // time creation of Boolean functions

                // TODO
                // time variable names, simplification
                // measure simplification quality?

                // // z3
                // {
                //     std::vector<z3::expr> z3_functions;
                //     z3::context ctx;
                //     auto start_z3_creation = std::chrono::steady_clock::now();
                //     for (const auto& n : nets)
                //     {
                //         const auto bf_res = z3_utils::get_subgraph_z3_function(comb_gates, n, ctx);
                //         if (bf_res.is_error())
                //         {
                //             log_error("bitwuzla_utils", "{}", bf_res.get_error().get());
                //             return;
                //         }
                //         const auto bf = bf_res.get();
                //         z3_functions.push_back(bf);
                //     }
                //     auto end_z3_creation = std::chrono::steady_clock::now();

                //     std::cout << "z3 function generation:              " << std::chrono::duration_cast<DURATION_UNIT>(end_z3_creation - start_z3_creation).count() << UNIT_STRING << std::endl;
                // }

                // // z3
                // {
                //     z3::context ctx;
                //     auto start_z3_creation = std::chrono::steady_clock::now();

                //     const auto res = z3_utils::get_subgraph_z3_functions(comb_gates, nets, ctx);
                //     if (res.is_error())
                //     {
                //         log_error("bitwuzla_utils", "{}", res.get_error().get());
                //         return;
                //     }
                //     const std::vector<z3::expr> z3_functions = res.get();

                //     auto end_z3_creation = std::chrono::steady_clock::now();

                //     std::cout << "z3 cached function generation:       " << std::chrono::duration_cast<DURATION_UNIT>(end_z3_creation - start_z3_creation).count() << UNIT_STRING << std::endl;

                //     auto start_z3_solving = std::chrono::steady_clock::now();
                //     for (const auto& bf : z3_functions)
                //     {
                //         z3::solver s(ctx);
                //         s.add(bf != bf);
                //         auto res = s.check();
                //     }
                //     auto end_z3_solving = std::chrono::steady_clock::now();

                //     std::cout << "z3 smt solving:                      " << std::chrono::duration_cast<DURATION_UNIT>(end_z3_solving - start_z3_solving).count() << UNIT_STRING << std::endl;
                // }

                // // bitwuzla
                // {
                //     std::vector<bitwuzla::Term> bitwuzla_functions;
                //     auto start_bitwuzla_creation = std::chrono::steady_clock::now();
                //     for (const auto& n : nets)
                //     {
                //         const auto bf_res = bitwuzla_utils::get_subgraph_bitwuzla_function(comb_gates, n);
                //         if (bf_res.is_error())
                //         {
                //             log_error("bitwuzla_utils", "{}", bf_res.get_error().get());
                //             return;
                //         }
                //         const auto bf = bf_res.get();
                //         bitwuzla_functions.push_back(bf);
                //     }
                //     auto end_bitwuzla_creation = std::chrono::steady_clock::now();

                //     std::cout << "bitwuzla function generation:        " << std::chrono::duration_cast<DURATION_UNIT>(end_bitwuzla_creation - start_bitwuzla_creation).count() << UNIT_STRING
                //               << std::endl;
                // }

                // bitwuzla
                {
                    auto start_bitwuzla_creation = std::chrono::steady_clock::now();

                    const auto res = bitwuzla_utils::get_subgraph_bitwuzla_functions(comb_gates, nets);
                    if (res.is_error())
                    {
                        log_error("bitwuzla_utils", "{}", res.get_error().get());
                        return;
                    }
                    const std::vector<bitwuzla::Term> bitwuzla_functions = res.get();

                    auto end_bitwuzla_creation = std::chrono::steady_clock::now();

                    std::cout << "bitwuzla cached function generation: " << std::chrono::duration_cast<DURATION_UNIT>(end_bitwuzla_creation - start_bitwuzla_creation).count() << UNIT_STRING
                              << std::endl;

                    auto start_cadical_bitwuzla_solving = std::chrono::steady_clock::now();
                    for (const auto& bf : bitwuzla_functions)
                    {
                        bitwuzla::Options options;
                        options.set(bitwuzla::Option::PRODUCE_MODELS, true);
                        options.set(bitwuzla::Option::SAT_SOLVER, "cadical");
                        bitwuzla::Bitwuzla bitwuzla(options);

                        bitwuzla.assert_formula(bitwuzla::mk_term(bitwuzla::Kind::NOT, {bitwuzla::mk_term(bitwuzla::Kind::EQUAL, {bf, bf})}));
                        bitwuzla::Result result = bitwuzla.check_sat();
                    }
                    auto end_cadical_bitwuzla_solving = std::chrono::steady_clock::now();

                    std::cout << "bitwuzla cadical smt solving:        " << std::chrono::duration_cast<DURATION_UNIT>(end_cadical_bitwuzla_solving - start_cadical_bitwuzla_solving).count()
                              << UNIT_STRING << std::endl;

                    auto start_kissat_bitwuzla_solving = std::chrono::steady_clock::now();
                    for (const auto& bf : bitwuzla_functions)
                    {
                        bitwuzla::Options options;
                        options.set(bitwuzla::Option::PRODUCE_MODELS, true);
                        options.set(bitwuzla::Option::SAT_SOLVER, "kissat");
                        bitwuzla::Bitwuzla bitwuzla(options);

                        bitwuzla.assert_formula(bitwuzla::mk_term(bitwuzla::Kind::NOT, {bitwuzla::mk_term(bitwuzla::Kind::EQUAL, {bf, bf})}));
                        bitwuzla::Result result = bitwuzla.check_sat();
                    }
                    auto end_kissat_bitwuzla_solving = std::chrono::steady_clock::now();

                    std::cout << "bitwuzla kissat smt solving:         " << std::chrono::duration_cast<DURATION_UNIT>(end_kissat_bitwuzla_solving - start_kissat_bitwuzla_solving).count()
                              << UNIT_STRING << std::endl;
                }

                // if (nets.size() > 100)
                // {
                //     std::cout << "NO DATA - HAL is prone to cause a RAM overflow when benchmarking for more than a few nets" << std::endl;
                //     return;
                // }

                // // hal
                // {
                //     auto start_hal_creation = std::chrono::steady_clock::now();
                //     std::vector<BooleanFunction> hal_functions;
                //     for (const auto& n : nets)
                //     {
                //         const auto bf_res = SubgraphNetlistDecorator(*nl).get_subgraph_function(comb_gates, n);
                //         if (bf_res.is_error())
                //         {
                //             log_error("bitwuzla_utils", "{}", bf_res.get_error().get());
                //             return;
                //         }
                //         const auto bf = bf_res.get();
                //         hal_functions.push_back(bf);
                //     }
                //     auto end_hal_creation = std::chrono::steady_clock::now();

                //     std::cout << "hal function generation:             " << std::chrono::duration_cast<DURATION_UNIT>(end_hal_creation - start_hal_creation).count() << UNIT_STRING << std::endl;
                // }

                // // hal
                // {
                //     auto start_hal_creation = std::chrono::steady_clock::now();
                //     std::vector<BooleanFunction> hal_functions;
                //     std::map<std::pair<u32, const GatePin*>, BooleanFunction> gate_cache;
                //     for (const auto& n : nets)
                //     {
                //         const auto bf_res = SubgraphNetlistDecorator(*nl).get_subgraph_function(comb_gates, n, gate_cache);
                //         if (bf_res.is_error())
                //         {
                //             log_error("bitwuzla_utils", "{}", bf_res.get_error().get());
                //             return;
                //         }
                //         const auto bf = bf_res.get();
                //         hal_functions.push_back(bf);
                //     }
                //     auto end_hal_creation = std::chrono::steady_clock::now();

                //     std::cout << "hal cached function generation:      " << std::chrono::duration_cast<DURATION_UNIT>(end_hal_creation - start_hal_creation).count() << UNIT_STRING << std::endl;

                //     auto start_hal_smt2_generation = std::chrono::steady_clock::now();
                //     std::vector<std::string> smt2_strings;
                //     for (const auto& bf : hal_functions)
                //     {
                //         auto cfg = SMT::QueryConfig().with_call(SMT::SolverCall::Library).with_solver(SMT::SolverType::Z3).with_local_solver().with_model_generation();
                //         auto s   = SMT::Solver().with_constraint(SMT::Constraint(BooleanFunction::Not(BooleanFunction::Eq(bf.clone(), bf.clone(), 1).get(), 1).get()));
                //         smt2_strings.push_back(s.to_smt2(cfg).get());
                //     }
                //     auto end_hal_smt2_generation = std::chrono::steady_clock::now();

                //     std::cout << "hal smt2 generation:                 " << std::chrono::duration_cast<DURATION_UNIT>(end_hal_smt2_generation - start_hal_smt2_generation).count() << UNIT_STRING
                //               << std::endl;

                //     auto start_hal_bitwuzla_smt2 = std::chrono::steady_clock::now();
                //     for (const auto& s : smt2_strings)
                //     {
                //         bitwuzla::Options options;
                //         options.set(bitwuzla::Option::PRODUCE_MODELS, true);

                //         const char* smt2_char_string = s.c_str();

                //         auto in_stream = fmemopen((void*)smt2_char_string, strlen(smt2_char_string), "r");
                //         std::stringbuf result_string;
                //         std::ostream output_stream(&result_string);

                //         bitwuzla::parser::Parser p(options, "VIRTUAL_FILE", in_stream, "smt2", &output_stream);
                //         std::string err_msg = p.parse(false);

                //         if (!err_msg.empty())
                //         {
                //             log_error("bitwuzla_utils", "{}", err_msg);
                //         }

                //         fclose(in_stream);

                //         // std::string output(result_string.str());

                //         // return OK({false, output});
                //     }
                //     auto end_hal_bitwuzla_smt2 = std::chrono::steady_clock::now();

                //     std::cout << "hal bitwuzla smt2 solving:           " << std::chrono::duration_cast<DURATION_UNIT>(end_hal_bitwuzla_smt2 - start_hal_bitwuzla_smt2).count() << UNIT_STRING
                //               << std::endl;

                //     auto start_hal_z3_library_solving = std::chrono::steady_clock::now();
                //     for (const auto& bf : hal_functions)
                //     {
                //         auto cfg = SMT::QueryConfig().with_call(SMT::SolverCall::Library).with_solver(SMT::SolverType::Z3).with_local_solver().with_model_generation();
                //         auto s   = SMT::Solver().with_constraint(SMT::Constraint(BooleanFunction::Not(BooleanFunction::Eq(bf.clone(), bf.clone(), 1).get(), 1).get()));
                //         auto res = s.query(cfg);
                //     }
                //     auto end_hal_z3_library_solving = std::chrono::steady_clock::now();

                //     std::cout << "hal-z3-library smt solving:          " << std::chrono::duration_cast<DURATION_UNIT>(end_hal_z3_library_solving - start_hal_z3_library_solving).count() << UNIT_STRING
                //               << std::endl;

                //     auto start_hal_bitwuzla_library_solving = std::chrono::steady_clock::now();
                //     for (const auto& bf : hal_functions)
                //     {
                //         auto cfg = SMT::QueryConfig().with_call(SMT::SolverCall::Library).with_solver(SMT::SolverType::Bitwuzla).with_local_solver().with_model_generation();
                //         auto s   = SMT::Solver().with_constraint(SMT::Constraint(BooleanFunction::Not(BooleanFunction::Eq(bf.clone(), bf.clone(), 1).get(), 1).get()));
                //         auto res = s.query(cfg);
                //     }
                //     auto end_hal_bitwuzla_library_solving = std::chrono::steady_clock::now();

                //     std::cout << "hal-bitwuzla-library smt solving:    " << std::chrono::duration_cast<DURATION_UNIT>(end_hal_bitwuzla_library_solving - start_hal_bitwuzla_library_solving).count()
                //               << UNIT_STRING << std::endl;
                // }
            },
            R"(

        )");

#ifndef PYBIND11_MODULE
        return m.ptr();
#endif    // PYBIND11_MODULE
    }
}    // namespace hal
