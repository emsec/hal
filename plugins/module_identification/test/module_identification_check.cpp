// #include "hal_core/defines.h"
// #include "hal_core/netlist/decorators/subgraph_netlist_decorator.h"
// #include "hal_core/netlist/gate.h"
// #include "hal_core/netlist/gate_library/gate_library_manager.h"
// #include "hal_core/netlist/net.h"
// #include "hal_core/netlist/netlist.h"
// #include "hal_core/netlist/netlist_factory.h"
// #include "hal_core/netlist/netlist_parser/netlist_parser_manager.h"
// #include "hal_core/netlist/netlist_utils.h"
// #include "hal_core/netlist/persistent/netlist_serializer.h"
// #include "hal_core/plugin_system/plugin_manager.h"
// #include "hal_core/utilities/log.h"
// #include "module_identification/api/module_identification.h"
// #include "module_identification/plugin_module_identification.h"
// #include "netlist_test_utils.h"

// #include <fstream>
// #include <set>
// #include <string>

// namespace hal
// {
//     class ModuelIdentificationTest : public ::testing::Test
//     {
//     protected:
//         ModuleIdentificationPlugin* plugin;
//         GateLibrary* xilinx_lib;
//         GateLibrary* lattice_lib;
//         GateLibrary* nangate_lib;

//         virtual void SetUp()
//         {
//             NO_COUT_BLOCK;
//             plugin_manager::load_all_plugins();
//             //gate_library_manager::get_gate_library("XILINX_UNISIM.hgl");
//             gate_library_manager::get_gate_library("XILINX_UNISIM_hal.hgl");
//             gate_library_manager::get_gate_library("ice40ultra_hal.hgl");
//             gate_library_manager::get_gate_library("NangateOpenCellLibrary.hgl");

//             xilinx_lib  = gate_library_manager::get_gate_library_by_name("XILINX_UNISIM_WITH_HAL_TYPES");
//             lattice_lib = gate_library_manager::get_gate_library_by_name("ICE40ULTRA_WITH_HAL_TYPES");
//             nangate_lib = gate_library_manager::get_gate_library_by_name("NangateOpenCellLibrary");
//             plugin      = plugin_manager::get_plugin_instance<ModuleIdentificationPlugin>("module_identification");
//         }

//         std::unique_ptr<Netlist> parse_netlist(std::string netlist_path, GateLibrary* lib)
//         {
//             if (!utils::file_exists(netlist_path))
//             {
//                 std::cout << "netlist not found: " << netlist_path << std::endl;
//                 return nullptr;
//             }

//             std::cout << "[+] new netlist \nloading " << netlist_path << std::endl;
//             std::unique_ptr<Netlist> nl;
//             {
//                 // nl = netlist_parser_manager::parse(netlist_path, lib);
//                 nl = netlist_factory::load_netlist(netlist_path, lib);
//                 if (nl == nullptr)
//                 {
//                     std::cout << "netlist couldn't be parsed" << std::endl;
//                     return nullptr;
//                 }
//             }
//             std::cout << "[+] loaded_netlist with " << nl->get_gates().size() << std::endl;

//             return std::move(nl);
//         }

//         virtual void TearDown()
//         {
//             NO_COUT_BLOCK;
//             plugin_manager::unload_all_plugins();
//         }
//     };

//     /**
//      * Test verification of test1  for Xilinx
//      *
//      * Functions: module_identification
//      */
//     TEST_F(ModuelIdentificationTest, xilinx_ibex){TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/xilinx/ibex.hal";

//     std::unique_ptr<Netlist> nl;
//     {
//         // NO_COUT_BLOCK;
//         nl = parse_netlist(netlist_path, xilinx_lib);
//     }
//     if (nl == nullptr)
//     {
//         FAIL() << "netlist couldn't be parsed";
//     }

//     auto expected_modules = nl->get_modules([](const Module* module) { return module->has_data("ModuleIdentification", "VERIFIED_TYPES"); });
//     std::cout << "found " << expected_modules.size() << " modules" << std::endl;
//     for (auto cur_module : expected_modules)
//     {
//         //auto nl  = netlist_utils::get_partial_netlist(nl.get(), cur_module->get_gates());
//         auto config = module_identification::Configuration(nl.get()).with_max_control_num(3).with_multithreding_handling(module_identification::MultithreadingHandling::memory_priority);

//         auto execution_res = module_identification::execute_on_gates(cur_module->get_gates(), config);
//     if (execution_res.is_error())
//     {
//         FAIL() << "failed plugin execution: \n" << res.get_error().get(); 
//     }
//     const auto res = execution_res.get();


//         //grabbing expected types
//         std::string types_string = std::get<1>(cur_module->get_data("ModuleIdentification", "VERIFIED_TYPES"));
//         std::stringstream ss(types_string);
//         std::string cur_substring;
//         std::set<std::string> expected_types;
//         while (getline(ss, cur_substring, '\n'))
//         {
//             expected_types.insert(cur_substring);
//         }

//         bool found_fitting_module = false;
//         std::cout << res.get_verified_candidates().size() << std::endl;
//         for (auto [_, cur_candidate] : res.get_verified_candidates())
//         {
//             auto created_types_string = cur_candidate.get_all_additional_data()["VERIFIED_TYPES"];
//             std::stringstream ss2(created_types_string);
//             std::string cur_substring;
//             std::set<std::string> created_types;
//             while (getline(ss2, cur_substring, '\n'))
//             {
//                 created_types.insert(cur_substring);
//             }
//             if (created_types != expected_types)
//             {
//                 std::cout << "created " << created_types.size() << " types" << std::endl;
//                 continue;
//             }
//             if (cur_candidate.m_gates.size() != cur_module->get_gates().size())
//             {
//                 continue;
//             }
//             found_fitting_module = true;
//             break;
//         }
//         if (!found_fitting_module)
//         {
//             std::cout << "expected: " << types_string << " of size " << std::to_string(cur_module->get_gates().size()) << std::endl;
//             for (auto [_, cur_candidate] : res.get_verified_candidates())
//             {
//                 std::cout << "got: " << cur_candidate.get_all_additional_data()["VERIFIED_TYPES"] << " of size " << std::to_string(cur_candidate.m_gates.size()) << std::endl;
//             }
//             FAIL() << "couldnt find fitting module";
//         }
//     }
//     std::cout << std::endl;

// }    // namespace hal
// TEST_END
// }
// ;
// /**
//      * Test verification of test2  for Xilinx
//      *
//      * Functions: module_identification
//      */
// TEST_F(ModuelIdentificationTest, xilinx_icicle){TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/xilinx/icicle.hal";

// std::unique_ptr<Netlist> nl;
// {
//     // NO_COUT_BLOCK;
//     nl = parse_netlist(netlist_path, xilinx_lib);
// }
// if (nl == nullptr)
// {
//     FAIL() << "netlist couldn't be parsed";
// }

// auto expected_modules = nl->get_modules([](const Module* module) { return module->has_data("ModuleIdentification", "VERIFIED_TYPES"); });
// std::cout << "found " << expected_modules.size() << " modules" << std::endl;
// for (auto cur_module : expected_modules)
// {
//     //auto nl  = netlist_utils::get_partial_netlist(nl.get(), cur_module->get_gates());
//     auto config       = module_identification::Configuration(nl.get()).with_max_control_num(3).with_multithreding_handling(module_identification::MultithreadingHandling::memory_priority);
//     auto execution_res = module_identification::execute_on_gates(cur_module->get_gates(), config);
//     if (execution_res.is_error())
//     {
//         FAIL() << "failed plugin execution: \n" << res.get_error().get(); 
//     }
//     const auto res = execution_res.get();


//     //grabbing expected types
//     std::string types_string = std::get<1>(cur_module->get_data("ModuleIdentification", "VERIFIED_TYPES"));
//     std::stringstream ss(types_string);
//     std::string cur_substring;
//     std::set<std::string> expected_types;
//     while (getline(ss, cur_substring, '\n'))
//     {
//         expected_types.insert(cur_substring);
//     }

//     bool found_fitting_module = false;
//     std::cout << res.get_verified_candidates().size() << std::endl;
//     for (auto [_, cur_candidate] : res.get_verified_candidates())
//     {
//         auto created_types_string = cur_candidate.get_all_additional_data()["VERIFIED_TYPES"];
//         std::stringstream ss2(created_types_string);
//         std::string cur_substring;
//         std::set<std::string> created_types;
//         while (getline(ss2, cur_substring, '\n'))
//         {
//             created_types.insert(cur_substring);
//         }
//         if (created_types != expected_types)
//         {
//             std::cout << "created " << created_types.size() << " types" << std::endl;
//             continue;
//         }
//         if (cur_candidate.m_gates.size() != cur_module->get_gates().size())
//         {
//             continue;
//         }
//         found_fitting_module = true;
//         break;
//     }
//     if (!found_fitting_module)
//     {
//         std::cout << "expected: " << types_string << " of size " << std::to_string(cur_module->get_gates().size()) << std::endl;
//         for (auto [_, cur_candidate] : res.get_verified_candidates())
//         {
//             std::cout << "got: " << cur_candidate.get_all_additional_data()["VERIFIED_TYPES"] << " of size " << std::to_string(cur_candidate.m_gates.size()) << std::endl;
//         }
//         FAIL() << "couldnt find fitting module";
//     }
// }
// std::cout << std::endl;

// }    // namespace hal
// TEST_END
// }
// ;
// /**
//      * Test verification of test3  for Xilinx
//      *
//      * Functions: module_identification
//      */
// TEST_F(ModuelIdentificationTest,
//        xilinx_simple_risc_v){TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/xilinx/simple_risc_v.hal";

// std::unique_ptr<Netlist> nl;
// {
//     // NO_COUT_BLOCK;
//     nl = parse_netlist(netlist_path, xilinx_lib);
// }
// if (nl == nullptr)
// {
//     FAIL() << "netlist couldn't be parsed";
// }

// auto expected_modules = nl->get_modules([](const Module* module) { return module->has_data("ModuleIdentification", "VERIFIED_TYPES"); });
// std::cout << "found " << expected_modules.size() << " modules" << std::endl;
// for (auto cur_module : expected_modules)
// {
//     //auto nl  = netlist_utils::get_partial_netlist(nl.get(), cur_module->get_gates());
//     auto config       = module_identification::Configuration(nl.get()).with_max_control_num(3).with_multithreding_handling(module_identification::MultithreadingHandling::memory_priority);
//     auto execution_res = module_identification::execute_on_gates(cur_module->get_gates(), config);
//     if (execution_res.is_error())
//     {
//         FAIL() << "failed plugin execution: \n" << res.get_error().get(); 
//     }
//     const auto res = execution_res.get();


//     //grabbing expected types
//     std::string types_string = std::get<1>(cur_module->get_data("ModuleIdentification", "VERIFIED_TYPES"));
//     std::stringstream ss(types_string);
//     std::string cur_substring;
//     std::set<std::string> expected_types;
//     while (getline(ss, cur_substring, '\n'))
//     {
//         expected_types.insert(cur_substring);
//     }

//     bool found_fitting_module = false;
//     std::cout << res.get_verified_candidates().size() << std::endl;
//     for (auto [_, cur_candidate] : res.get_verified_candidates())
//     {
//         auto created_types_string = cur_candidate.get_all_additional_data()["VERIFIED_TYPES"];
//         std::stringstream ss2(created_types_string);
//         std::string cur_substring;
//         std::set<std::string> created_types;
//         while (getline(ss2, cur_substring, '\n'))
//         {
//             created_types.insert(cur_substring);
//         }
//         if (created_types != expected_types)
//         {
//             std::cout << "created " << created_types.size() << " types" << std::endl;
//             continue;
//         }
//         if (cur_candidate.m_gates.size() != cur_module->get_gates().size())
//         {
//             continue;
//         }
//         found_fitting_module = true;
//         break;
//     }
//     if (!found_fitting_module)
//     {
//         std::cout << "expected: " << types_string << " of size " << std::to_string(cur_module->get_gates().size()) << std::endl;
//         for (auto [_, cur_candidate] : res.get_verified_candidates())
//         {
//             std::cout << "got: " << cur_candidate.get_all_additional_data()["VERIFIED_TYPES"] << " of size " << std::to_string(cur_candidate.m_gates.size()) << std::endl;
//         }
//         FAIL() << "couldnt find fitting module";
//     }
// }
// std::cout << std::endl;

// }    // namespace hal
// TEST_END
// }
// ;

// //lattice

// /**
//      * Test verification of test1  for lattice
//      *
//      * Functions: module_identification
//      */
// TEST_F(ModuelIdentificationTest, lattice_ibex){TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/lattice/ibex.hal";

// std::unique_ptr<Netlist> nl;
// {
//     // NO_COUT_BLOCK;
//     nl = parse_netlist(netlist_path, lattice_lib);
// }
// if (nl == nullptr)
// {
//     FAIL() << "netlist couldn't be parsed";
// }

// auto expected_modules = nl->get_modules([](const Module* module) { return module->has_data("ModuleIdentification", "VERIFIED_TYPES"); });
// std::cout << "found " << expected_modules.size() << " modules" << std::endl;
// for (auto cur_module : expected_modules)
// {
//     //auto nl  = netlist_utils::get_partial_netlist(nl.get(), cur_module->get_gates());
//     auto config       = module_identification::Configuration(nl.get()).with_max_control_num(3).with_multithreding_handling(module_identification::MultithreadingHandling::memory_priority);
//     auto execution_res = module_identification::execute_on_gates(cur_module->get_gates(), config);
//     if (execution_res.is_error())
//     {
//         FAIL() << "failed plugin execution: \n" << res.get_error().get(); 
//     }
//     const auto res = execution_res.get();

//     //grabbing expected types
//     std::string types_string = std::get<1>(cur_module->get_data("ModuleIdentification", "VERIFIED_TYPES"));
//     std::stringstream ss(types_string);
//     std::string cur_substring;
//     std::set<std::string> expected_types;
//     while (getline(ss, cur_substring, '\n'))
//     {
//         expected_types.insert(cur_substring);
//     }

//     bool found_fitting_module = false;
//     std::cout << res.get_verified_candidates().size() << std::endl;
//     for (auto [_, cur_candidate] : res.get_verified_candidates())
//     {
//         auto created_types_string = cur_candidate.get_all_additional_data()["VERIFIED_TYPES"];
//         std::stringstream ss2(created_types_string);
//         std::string cur_substring;
//         std::set<std::string> created_types;
//         while (getline(ss2, cur_substring, '\n'))
//         {
//             created_types.insert(cur_substring);
//         }
//         if (created_types != expected_types)
//         {
//             std::cout << "created " << created_types.size() << " types" << std::endl;
//             continue;
//         }
//         if (cur_candidate.m_gates.size() != cur_module->get_gates().size())
//         {
//             continue;
//         }
//         found_fitting_module = true;
//         break;
//     }
//     if (!found_fitting_module)
//     {
//         std::cout << "expected: " << types_string << " of size " << std::to_string(cur_module->get_gates().size()) << std::endl;
//         for (auto [_, cur_candidate] : res.get_verified_candidates())
//         {
//             std::cout << "got: " << cur_candidate.get_all_additional_data()["VERIFIED_TYPES"] << " of size " << std::to_string(cur_candidate.m_gates.size()) << std::endl;
//         }
//         FAIL() << "couldnt find fitting module";
//     }
// }
// std::cout << std::endl;

// }    // namespace hal
// TEST_END
// }
// ;
// /**
//      * Test verification of test2  for lattice
//      *
//      * Functions: module_identification
//      */
// TEST_F(ModuelIdentificationTest, lattice_icicle){TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/lattice/icicle.hal";

// std::unique_ptr<Netlist> nl;
// {
//     // NO_COUT_BLOCK;
//     nl = parse_netlist(netlist_path, lattice_lib);
// }
// if (nl == nullptr)
// {
//     FAIL() << "netlist couldn't be parsed";
// }

// auto expected_modules = nl->get_modules([](const Module* module) { return module->has_data("ModuleIdentification", "VERIFIED_TYPES"); });
// std::cout << "found " << expected_modules.size() << " modules" << std::endl;
// for (auto cur_module : expected_modules)
// {
//     //auto nl  = netlist_utils::get_partial_netlist(nl.get(), cur_module->get_gates());
//     auto config       = module_identification::Configuration(nl.get()).with_max_control_num(3).with_multithreding_handling(module_identification::MultithreadingHandling::memory_priority);
//     auto execution_res = module_identification::execute_on_gates(cur_module->get_gates(), config);
//     if (execution_res.is_error())
//     {
//         FAIL() << "failed plugin execution: \n" << res.get_error().get(); 
//     }
//     const auto res = execution_res.get();


//     //grabbing expected types
//     std::string types_string = std::get<1>(cur_module->get_data("ModuleIdentification", "VERIFIED_TYPES"));
//     std::stringstream ss(types_string);
//     std::string cur_substring;
//     std::set<std::string> expected_types;
//     while (getline(ss, cur_substring, '\n'))
//     {
//         expected_types.insert(cur_substring);
//     }

//     bool found_fitting_module = false;
//     std::cout << res.get_verified_candidates().size() << std::endl;
//     for (auto [_, cur_candidate] : res.get_verified_candidates())
//     {
//         auto created_types_string = cur_candidate.get_all_additional_data()["VERIFIED_TYPES"];
//         std::stringstream ss2(created_types_string);
//         std::string cur_substring;
//         std::set<std::string> created_types;
//         while (getline(ss2, cur_substring, '\n'))
//         {
//             created_types.insert(cur_substring);
//         }
//         if (created_types != expected_types)
//         {
//             std::cout << "created " << created_types.size() << " types" << std::endl;
//             continue;
//         }
//         if (cur_candidate.m_gates.size() != cur_module->get_gates().size())
//         {
//             continue;
//         }
//         found_fitting_module = true;
//         break;
//     }
//     if (!found_fitting_module)
//     {
//         std::cout << "expected: " << types_string << " of size " << std::to_string(cur_module->get_gates().size()) << std::endl;
//         for (auto [_, cur_candidate] : res.get_verified_candidates())
//         {
//             std::cout << "got: " << cur_candidate.get_all_additional_data()["VERIFIED_TYPES"] << " of size " << std::to_string(cur_candidate.m_gates.size()) << std::endl;
//         }
//         FAIL() << "couldnt find fitting module";
//     }
// }
// std::cout << std::endl;

// }    // namespace hal
// TEST_END
// }
// ;
// /**
//      * Test verification of test4  for lattice
//      *
//      * Functions: module_identification
//      */
// TEST_F(ModuelIdentificationTest, lattice_fft64){TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/lattice/fft64.hal";

// std::unique_ptr<Netlist> nl;
// {
//     // NO_COUT_BLOCK;
//     nl = parse_netlist(netlist_path, lattice_lib);
// }
// if (nl == nullptr)
// {
//     FAIL() << "netlist couldn't be parsed";
// }

// auto expected_modules = nl->get_modules([](const Module* module) { return module->has_data("ModuleIdentification", "VERIFIED_TYPES"); });
// std::cout << "found " << expected_modules.size() << " modules" << std::endl;
// for (auto cur_module : expected_modules)
// {
//     //auto nl  = netlist_utils::get_partial_netlist(nl.get(), cur_module->get_gates());
//     auto config       = module_identification::Configuration(nl.get()).with_max_control_num(3).with_multithreding_handling(module_identification::MultithreadingHandling::memory_priority);
//     auto execution_res = module_identification::execute_on_gates(cur_module->get_gates(), config);
//     if (execution_res.is_error())
//     {
//         FAIL() << "failed plugin execution: \n" << res.get_error().get(); 
//     }
//     const auto res = execution_res.get();


//     //grabbing expected types
//     std::string types_string = std::get<1>(cur_module->get_data("ModuleIdentification", "VERIFIED_TYPES"));
//     std::stringstream ss(types_string);
//     std::string cur_substring;
//     std::set<std::string> expected_types;
//     while (getline(ss, cur_substring, '\n'))
//     {
//         expected_types.insert(cur_substring);
//     }

//     bool found_fitting_module = false;
//     std::cout << res.get_verified_candidates().size() << std::endl;
//     for (auto [_, cur_candidate] : res.get_verified_candidates())
//     {
//         auto created_types_string = cur_candidate.get_all_additional_data()["VERIFIED_TYPES"];
//         std::stringstream ss2(created_types_string);
//         std::string cur_substring;
//         std::set<std::string> created_types;
//         while (getline(ss2, cur_substring, '\n'))
//         {
//             created_types.insert(cur_substring);
//         }
//         if (created_types != expected_types)
//         {
//             std::cout << "created " << created_types.size() << " types" << std::endl;
//             continue;
//         }
//         if (cur_candidate.m_gates.size() != cur_module->get_gates().size())
//         {
//             continue;
//         }
//         found_fitting_module = true;
//         break;
//     }
//     if (!found_fitting_module)
//     {
//         std::cout << "expected: " << types_string << " of size " << std::to_string(cur_module->get_gates().size()) << std::endl;
//         for (auto [_, cur_candidate] : res.get_verified_candidates())
//         {
//             std::cout << "got: " << cur_candidate.get_all_additional_data()["VERIFIED_TYPES"] << " of size " << std::to_string(cur_candidate.m_gates.size()) << std::endl;
//         }
//         FAIL() << "couldnt find fitting module";
//     }
// }
// std::cout << std::endl;

// }    // namespace hal
// TEST_END
// }
// ;
// /**
//      * Test verification of test5  for lattice
//      *
//      * Functions: module_identification
//      */
// TEST_F(ModuelIdentificationTest, lattice_sha256){TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/lattice/sha256.hal";

// std::unique_ptr<Netlist> nl;
// {
//     // NO_COUT_BLOCK;
//     nl = parse_netlist(netlist_path, lattice_lib);
// }
// if (nl == nullptr)
// {
//     FAIL() << "netlist couldn't be parsed";
// }

// auto expected_modules = nl->get_modules([](const Module* module) { return module->has_data("ModuleIdentification", "VERIFIED_TYPES"); });
// std::cout << "found " << expected_modules.size() << " modules" << std::endl;
// for (auto cur_module : expected_modules)
// {
//     //auto nl  = netlist_utils::get_partial_netlist(nl.get(), cur_module->get_gates());
//     auto config       = module_identification::Configuration(nl.get()).with_max_control_num(3).with_multithreding_handling(module_identification::MultithreadingHandling::memory_priority);
//     auto execution_res = module_identification::execute_on_gates(cur_module->get_gates(), config);
//     if (execution_res.is_error())
//     {
//         FAIL() << "failed plugin execution: \n" << res.get_error().get(); 
//     }
//     const auto res = execution_res.get();


//     //grabbing expected types
//     std::string types_string = std::get<1>(cur_module->get_data("ModuleIdentification", "VERIFIED_TYPES"));
//     std::stringstream ss(types_string);
//     std::string cur_substring;
//     std::set<std::string> expected_types;
//     while (getline(ss, cur_substring, '\n'))
//     {
//         expected_types.insert(cur_substring);
//     }

//     bool found_fitting_module = false;
//     std::cout << res.get_verified_candidates().size() << std::endl;
//     for (auto [_, cur_candidate] : res.get_verified_candidates())
//     {
//         auto created_types_string = cur_candidate.get_all_additional_data()["VERIFIED_TYPES"];
//         std::stringstream ss2(created_types_string);
//         std::string cur_substring;
//         std::set<std::string> created_types;
//         while (getline(ss2, cur_substring, '\n'))
//         {
//             created_types.insert(cur_substring);
//         }
//         if (created_types != expected_types)
//         {
//             std::cout << "created " << created_types.size() << " types" << std::endl;
//             continue;
//         }
//         if (cur_candidate.m_gates.size() != cur_module->get_gates().size())
//         {
//             continue;
//         }
//         found_fitting_module = true;
//         break;
//     }
//     if (!found_fitting_module)
//     {
//         std::cout << "expected: " << types_string << " of size " << std::to_string(cur_module->get_gates().size()) << std::endl;
//         for (auto [_, cur_candidate] : res.get_verified_candidates())
//         {
//             std::cout << "got: " << cur_candidate.get_all_additional_data()["VERIFIED_TYPES"] << " of size " << std::to_string(cur_candidate.m_gates.size()) << std::endl;
//         }
//         FAIL() << "couldnt find fitting module";
//     }
// }
// std::cout << std::endl;

// }    // namespace hal
// TEST_END
// }
// ;
// }