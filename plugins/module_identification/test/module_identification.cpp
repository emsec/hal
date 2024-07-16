// #include "module_identification/api/module_identification.h"

// #include "hal_core/defines.h"
// #include "hal_core/netlist/gate.h"
// #include "hal_core/netlist/gate_library/gate_library_manager.h"
// #include "hal_core/netlist/net.h"
// #include "hal_core/netlist/netlist.h"
// #include "hal_core/netlist/netlist_factory.h"
// #include "hal_core/netlist/netlist_parser/netlist_parser_manager.h"
// #include "hal_core/netlist/persistent/netlist_serializer.h"
// #include "hal_core/plugin_system/plugin_manager.h"
// #include "hal_core/utilities/log.h"
// #include "module_identification/api/configuration.h"
// #include "module_identification/api/result.h"
// #include "module_identification/candidates/verified_candidate.h"
// #include "module_identification/plugin_module_identification.h"
// #include "netlist_test_utils.h"

// #include <fstream>
// #include <string>

// namespace hal
// {
//     namespace
//     {
//         std::vector<module_identification::VerifiedCandidate> test_all(const std::vector<Gate*>& gates)
//         {
//             if (gates.empty())
//             {
//                 return {};
//             }
//             auto config = module_identification::Configuration(gates[0]->get_netlist());
//             // config.with_multithreding_handling(MultithreadingHandling::none);
//             config.with_multithreading_priority(module_identification::MultithreadingPriority::memory_priority);

//             auto res = module_identification::execute_on_gates(gates, config).get();
//             auto map = res.get_verified_candidates();

//             std::vector<module_identification::VerifiedCandidate> retval;
//             for (auto item : map)
//             {
//                 if (item.second.is_verified())
//                 {
//                     retval.push_back(item.second);
//                 }
//             }

//             log_info("module_identification", "test_all done, found {} candidates", retval.size());

//             return retval;
//         }
//     }    // namespace

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
//                 nl = netlist_parser_manager::parse(netlist_path, lib);
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
//      * Test verification of ADDER_1.v for Xilinx
//      *
//      * Functions: module_identification
//      */
//     TEST_F(ModuelIdentificationTest,
//            test_xilinx_adder_1){TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/test_lib_xilinx/ADDER_1.v";

//     std::string file_name = std::filesystem::path(netlist_path).filename().string();
//     std::string type      = file_name.substr(0, file_name.find("_"));

//     std::unique_ptr<Netlist> nl = parse_netlist(netlist_path, xilinx_lib);
//     if (nl == nullptr)
//     {
//         FAIL() << "netlist could not be parsed";
//     }

//     auto candidates = test_all(nl->get_gates());

//     std::cout << "check done, found " << candidates.size() << " verified candidates" << std::endl;

//     EXPECT_FALSE(candidates.empty());
//     std::cout << std::endl;
// }    // namespace hal
// TEST_END
// }
// ;

// /**
//      * Test verification of ADDER_CARRY_1.v for Xilinx
//      *
//      * Functions: module_identification
//      */
// TEST_F(ModuelIdentificationTest,
//        test_xilinx_adder_carry_1){TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/test_lib_xilinx/ADDER_CARRY_1.v";

// std::string file_name = std::filesystem::path(netlist_path).filename().string();
// std::string type      = file_name.substr(0, file_name.find("_"));

// std::unique_ptr<Netlist> nl = parse_netlist(netlist_path, xilinx_lib);
// if (nl == nullptr)
// {
//     FAIL() << "netlist could not be parsed";
// }

// auto candidates = test_all(nl->get_gates());

// std::cout << "check done, found " << candidates.size() << " verified candidates" << std::endl;

// EXPECT_FALSE(candidates.empty());
// std::cout << std::endl;
// }    // namespace hal
// TEST_END
// }
// ;

// /**
//      * Test verification of ADDER_CARRY_ENABLE_1.v for Xilinx
//      *
//      * Functions: module_identification
//      */
// TEST_F(ModuelIdentificationTest, test_xilinx_adder_carry_enable_1){
//     TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/test_lib_xilinx/ADDER_CARRY_ENABLE_1.v";

// std::string file_name = std::filesystem::path(netlist_path).filename().string();
// std::string type      = file_name.substr(0, file_name.find("_"));

// std::unique_ptr<Netlist> nl = parse_netlist(netlist_path, xilinx_lib);
// if (nl == nullptr)
// {
//     FAIL() << "netlist could not be parsed";
// }

// auto candidates = test_all(nl->get_gates());

// std::cout << "check done, found " << candidates.size() << " verified candidates" << std::endl;

// EXPECT_FALSE(candidates.empty());
// std::cout << std::endl;
// }    // namespace hal
// TEST_END
// }
// ;

// /**
//      * Test verification of SUBTRACT_1.v for Xilinx
//      *
//      * Functions: module_identification
//      */
// TEST_F(ModuelIdentificationTest,
//        test_xilinx_subtract_1){TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/test_lib_xilinx/SUBTRACT_1.v";

// std::string file_name = std::filesystem::path(netlist_path).filename().string();
// std::string type      = file_name.substr(0, file_name.find("_"));

// std::unique_ptr<Netlist> nl = parse_netlist(netlist_path, xilinx_lib);
// if (nl == nullptr)
// {
//     FAIL() << "netlist could not be parsed";
// }

// auto candidates = test_all(nl->get_gates());

// std::cout << "check done, found " << candidates.size() << " verified candidates" << std::endl;

// EXPECT_FALSE(candidates.empty());
// std::cout << std::endl;
// }
// TEST_END
// }
// ;

// /**
//      * Test verification of SUBTRACT_2.v for Xilinx
//      *
//      * Functions: module_identification
//      */
// TEST_F(ModuelIdentificationTest,
//        test_xilinx_subtract_2){TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/test_lib_xilinx/SUBTRACT_2.v";

// std::string file_name = std::filesystem::path(netlist_path).filename().string();
// std::string type      = file_name.substr(0, file_name.find("_"));

// std::unique_ptr<Netlist> nl = parse_netlist(netlist_path, xilinx_lib);
// if (nl == nullptr)
// {
//     FAIL() << "netlist could not be parsed";
// }

// auto candidates = test_all(nl->get_gates());

// std::cout << "check done, found " << candidates.size() << " verified candidates" << std::endl;

// EXPECT_FALSE(candidates.empty());
// std::cout << std::endl;
// }
// TEST_END
// }
// ;

// /**
//      * Test verification of ADDER_WITH_MUX_1.v for Xilinx
//      *
//      * Functions: module_identification
//      */
// TEST_F(ModuelIdentificationTest, test_xilinx_adder_with_mux_1){
//     TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/test_lib_xilinx/ADDER_WITH_MUX_1.v";

// std::string file_name = std::filesystem::path(netlist_path).filename().string();
// std::string type      = file_name.substr(0, file_name.find("_"));

// std::unique_ptr<Netlist> nl = parse_netlist(netlist_path, xilinx_lib);
// if (nl == nullptr)
// {
//     FAIL() << "netlist could not be parsed";
// }

// auto candidates = test_all(nl->get_gates());

// std::cout << "check done, found " << candidates.size() << " verified candidates" << std::endl;

// EXPECT_FALSE(candidates.empty());
// std::cout << std::endl;
// }
// TEST_END
// }
// ;

// /**
//      * Test verification of ADDER_WEIRD_1.v for Xilinx
//      *
//      * Functions: module_identification
//      */
// TEST_F(ModuelIdentificationTest,
//        test_xilinx_adder_weird_1){TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/test_lib_xilinx/ADDER_WEIRD_1.v";

// std::string file_name = std::filesystem::path(netlist_path).filename().string();
// std::string type      = file_name.substr(0, file_name.find("_"));

// std::unique_ptr<Netlist> nl = parse_netlist(netlist_path, xilinx_lib);
// if (nl == nullptr)
// {
//     FAIL() << "netlist could not be parsed";
// }

// auto candidates = test_all(nl->get_gates());

// std::cout << "check done, found " << candidates.size() << " verified candidates" << std::endl;

// EXPECT_FALSE(candidates.empty());
// std::cout << std::endl;
// }
// TEST_END
// }
// ;

// /**
//      * Test verification of ADDER_WITH_DIFFERENT_LENGTHS_OPERANDS_1.v for Xilinx
//      *
//      * Functions: module_identification
//      */
// TEST_F(ModuelIdentificationTest, test_xilinx_add_with_different_lengths_operands_1){
//     TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/test_lib_xilinx/ADDER_WITH_DIFFERENT_LENGTHS_OPERANDS_1.v";

// std::string file_name = std::filesystem::path(netlist_path).filename().string();
// std::string type      = file_name.substr(0, file_name.find("_"));

// std::unique_ptr<Netlist> nl = parse_netlist(netlist_path, xilinx_lib);
// if (nl == nullptr)
// {
//     FAIL() << "netlist could not be parsed";
// }

// auto candidates = test_all(nl->get_gates());

// std::cout << "check done, found " << candidates.size() << " verified candidates" << std::endl;

// EXPECT_FALSE(candidates.empty());
// std::cout << std::endl;
// }
// TEST_END
// }
// ;

// /**
//      * Test verification of COMPARISON_1.v for Xilinx
//      *
//      * Functions: module_identification
//      */
// TEST_F(ModuelIdentificationTest,
//        test_xilinx_comparison_1){TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/test_lib_xilinx/COMPARISON_1.v";

// std::string file_name = std::filesystem::path(netlist_path).filename().string();
// std::string type      = file_name.substr(0, file_name.find("_"));

// std::unique_ptr<Netlist> nl = parse_netlist(netlist_path, xilinx_lib);
// if (nl == nullptr)
// {
//     FAIL() << "netlist could not be parsed";
// }

// std::cout << "nl: #" << nl->get_gates().size() << " gates" << std::endl;

// auto candidates = test_all(nl->get_gates());

// std::cout << "check done, found " << candidates.size() << " verified candidates" << std::endl;

// EXPECT_FALSE(candidates.empty());
// std::cout << std::endl;
// }
// TEST_END
// }
// ;

// /**
//      * Test verification of COMPARISON_2.v for Xilinx
//      *
//      * Functions: module_identification
//      */
// TEST_F(ModuelIdentificationTest,
//        test_xilinx_comparison_2){TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/test_lib_xilinx/COMPARISON_2.v";

// std::string file_name = std::filesystem::path(netlist_path).filename().string();
// std::string type      = file_name.substr(0, file_name.find("_"));

// std::unique_ptr<Netlist> nl = parse_netlist(netlist_path, xilinx_lib);
// if (nl == nullptr)
// {
//     FAIL() << "netlist could not be parsed";
// }

// auto candidates = test_all(nl->get_gates());

// std::cout << "check done, found " << candidates.size() << " verified candidates" << std::endl;

// EXPECT_FALSE(candidates.empty());
// std::cout << std::endl;
// }
// TEST_END
// }
// ;

// /**
//      * Test verification of COMPARISON_LT_1.v for Xilinx
//      *
//      * Functions: module_identification
//      */
// TEST_F(ModuelIdentificationTest, test_xilinx_comparison_let_1){
//     TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/test_lib_xilinx/COMPARISON_LET_1.v";

// std::string file_name = std::filesystem::path(netlist_path).filename().string();
// std::string type      = file_name.substr(0, file_name.find("_"));

// std::unique_ptr<Netlist> nl = parse_netlist(netlist_path, nangate_lib);
// if (nl == nullptr)
// {
//     FAIL() << "netlist could not be parsed";
// }

// auto candidates = test_all(nl->get_gates());

// std::cout << "check done, found " << candidates.size() << " verified candidates" << std::endl;

// EXPECT_FALSE(candidates.empty());
// std::cout << std::endl;
// }
// TEST_END
// }
// ;
// /**
//      * Test verification of COMPARISON_LT_1.v for Xilinx
//      *
//      * Functions: module_identification
//      */
// TEST_F(ModuelIdentificationTest, test_xilinx_comparison_let_enable_1){
//     TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/test_lib_xilinx/COMPARISON_LET_ENABLE_1.v";

// std::string file_name = std::filesystem::path(netlist_path).filename().string();
// std::string type      = file_name.substr(0, file_name.find("_"));

// std::unique_ptr<Netlist> nl = parse_netlist(netlist_path, nangate_lib);
// if (nl == nullptr)
// {
//     FAIL() << "netlist could not be parsed";
// }

// auto candidates = test_all(nl->get_gates());

// std::cout << "check done, found " << candidates.size() << " verified candidates" << std::endl;

// EXPECT_FALSE(candidates.empty());
// std::cout << std::endl;
// }
// TEST_END
// }
// ;

// /**
//      * Test verification of COMPARISON_EQ_1.v for Xilinx
//      *
//      * Functions: module_identification
//      */
// TEST_F(ModuelIdentificationTest,
//        test_xilinx_comparison_eq_1){TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/test_lib_xilinx/COMPARISON_EQ_1.v";

// if (!utils::file_exists(netlist_path))
//     FAIL() << "netlist for xilinx tests not found: " << netlist_path;

// std::string file_name = std::filesystem::path(netlist_path).filename().string();
// std::string type      = file_name.substr(0, file_name.find("_"));

// std::cout << "[+] new netlist \nloading " << netlist_path << std::endl;
// std::unique_ptr<Netlist> nl;
// {
//     nl = netlist_parser_manager::parse(netlist_path, nangate_lib);
//     if (nl == nullptr)
//     {
//         FAIL() << "netlist couldn't be parsed";
//     }
// }
// std::cout << "[+] loaded_netlist with " << nl->get_gates().size() << std::endl;

// std::vector<module_identification::VerifiedCandidate> candidates = test_all(nl->get_gates());
// EXPECT_FALSE(candidates.empty());
// std::cout << std::endl;
// }
// TEST_END
// }
// ;
// /**
//      * Test verification of CONST_MUL_1.v for Xilinx
//      *
//      * Functions: module_identification
//      */
// TEST_F(ModuelIdentificationTest,
//        test_xilinx_const_mul_1){TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/test_lib_xilinx/CONST_MUL_1.v";

// if (!utils::file_exists(netlist_path))
//     FAIL() << "netlist for xilinx tests not found: " << netlist_path;

// std::string file_name = std::filesystem::path(netlist_path).filename().string();
// std::string type      = file_name.substr(0, file_name.find("_"));

// std::cout << "[+] new netlist \nloading " << netlist_path << std::endl;
// std::unique_ptr<Netlist> nl;
// {
//     nl = netlist_parser_manager::parse(netlist_path, xilinx_lib);
//     if (nl == nullptr)
//     {
//         FAIL() << "netlist couldn't be parsed";
//     }
// }
// std::cout << "[+] loaded_netlist with " << nl->get_gates().size() << std::endl;

// std::vector<module_identification::VerifiedCandidate> candidates = test_all(nl->get_gates());
// EXPECT_FALSE(candidates.empty());
// std::cout << std::endl;
// }
// TEST_END
// }
// ;

// /**
//      * Test verification of CONST_MUL_2.v for Xilinx
//      *
//      * Functions: module_identification
//      */
// TEST_F(ModuelIdentificationTest,
//        test_xilinx_const_mul_2){TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/test_lib_xilinx/CONST_MUL_2.v";

// if (!utils::file_exists(netlist_path))
//     FAIL() << "netlist for xilinx tests not found: " << netlist_path;

// std::string file_name = std::filesystem::path(netlist_path).filename().string();
// std::string type      = file_name.substr(0, file_name.find("_"));

// std::cout << "[+] new netlist \nloading " << netlist_path << std::endl;
// std::unique_ptr<Netlist> nl;
// {
//     nl = netlist_parser_manager::parse(netlist_path, xilinx_lib);
//     if (nl == nullptr)
//     {
//         FAIL() << "netlist couldn't be parsed";
//     }
// }
// std::cout << "[+] loaded_netlist with " << nl->get_gates().size() << std::endl;

// std::vector<module_identification::VerifiedCandidate> candidates = test_all(nl->get_gates());
// EXPECT_FALSE(candidates.empty());
// std::cout << std::endl;
// }
// TEST_END
// }
// /**
//      * Test verification of COMPARISON_EQ_1.v for Xilinx
//      *
//      * Functions: module_identification
//      */
// TEST_F(ModuelIdentificationTest, test_xilinx_comparison_eq_enable_1){
//     TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/test_lib_xilinx/COMPARISON_EQ_ENABLE_1.v";

// if (!utils::file_exists(netlist_path))
//     FAIL() << "netlist for xilinx tests not found: " << netlist_path;

// std::string file_name = std::filesystem::path(netlist_path).filename().string();
// std::string type      = file_name.substr(0, file_name.find("_"));

// std::cout << "[+] new netlist \nloading " << netlist_path << std::endl;
// std::unique_ptr<Netlist> nl;
// {
//     nl = netlist_parser_manager::parse(netlist_path, nangate_lib);
//     if (nl == nullptr)
//     {
//         FAIL() << "netlist couldn't be parsed";
//     }
// }
// std::cout << "[+] loaded_netlist with " << nl->get_gates().size() << std::endl;

// std::vector<module_identification::VerifiedCandidate> candidates = test_all(nl->get_gates());
// EXPECT_FALSE(candidates.empty());
// std::cout << std::endl;
// }
// TEST_END
// }
// ;

// /**
//      * Test verification of COUNTER_INV_1.v for Xilinx
//      *
//      * Functions: module_identification
//      */
// TEST_F(ModuelIdentificationTest,
//        test_xilinx_counter_inv_1){TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/test_lib_xilinx/COUNTER_INV_1.v";

// if (!utils::file_exists(netlist_path))
//     FAIL() << "netlist for xilinx tests not found: " << netlist_path;

// std::string file_name = std::filesystem::path(netlist_path).filename().string();
// std::string type      = file_name.substr(0, file_name.find("_"));

// std::cout << "[+] new netlist \nloading " << netlist_path << std::endl;
// std::unique_ptr<Netlist> nl;
// {
//     nl = netlist_parser_manager::parse(netlist_path, xilinx_lib);
//     if (nl == nullptr)
//     {
//         FAIL() << "netlist couldn't be parsed";
//     }
// }
// std::cout << "[+] loaded_netlist with " << nl->get_gates().size() << std::endl;

// std::vector<module_identification::VerifiedCandidate> candidates = test_all(nl->get_gates());
// EXPECT_FALSE(candidates.empty());
// std::cout << std::endl;
// }
// TEST_END
// }
// ;

// /**
//      * Test verification of COUNTER_INV_2.v for Xilinx
//      *
//      * Functions: module_identification
//      */
// TEST_F(ModuelIdentificationTest,
//        test_xilinx_counter_inv_2){TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/test_lib_xilinx/COUNTER_INV_2.v";

// if (!utils::file_exists(netlist_path))
//     FAIL() << "netlist for xilinx tests not found: " << netlist_path;

// std::string file_name = std::filesystem::path(netlist_path).filename().string();
// std::string type      = file_name.substr(0, file_name.find("_"));

// std::cout << "[+] new netlist \nloading " << netlist_path << std::endl;
// std::unique_ptr<Netlist> nl;
// {
//     nl = netlist_parser_manager::parse(netlist_path, xilinx_lib);
//     if (nl == nullptr)
//     {
//         FAIL() << "netlist couldn't be parsed";
//     }
// }
// std::cout << "[+] loaded_netlist with " << nl->get_gates().size() << std::endl;

// std::vector<module_identification::VerifiedCandidate> candidates = test_all(nl->get_gates());
// EXPECT_FALSE(candidates.empty());
// std::cout << std::endl;
// }
// TEST_END
// }
// ;

// /**
//      * Test verification of CONST_MUL_1.v for Lattice iCE40
//      *
//      * Functions: module_identification
//      */
// TEST_F(ModuelIdentificationTest,
//        test_lattice_const_mul_1){TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/test_lib_lattice/CONST_MUL_1.v";

// std::string file_name = std::filesystem::path(netlist_path).filename().string();
// std::string type      = file_name.substr(0, file_name.find("_"));

// std::unique_ptr<Netlist> nl = parse_netlist(netlist_path, lattice_lib);
// if (nl == nullptr)
// {
//     FAIL() << "netlist could not be parsed";
// }

// auto candidates = test_all(nl->get_gates([](const Gate* g) { return g->get_type()->has_property(GateTypeProperty::combinational); }));

// std::cout << "check done, found " << candidates.size() << " verified candidates" << std::endl;

// EXPECT_FALSE(candidates.empty());
// std::cout << std::endl;
// }    // namespace hal
// TEST_END
// }
// ;

// /**
//      * Test verification of COUNTER_1.v for Lattice iCE40
//      *
//      * Functions: module_identification
//      */
// TEST_F(ModuelIdentificationTest,
//        test_lattice_counter_1){TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/test_lib_lattice/COUNTER_1.v";

// std::string file_name = std::filesystem::path(netlist_path).filename().string();
// std::string type      = file_name.substr(0, file_name.find("_"));

// std::unique_ptr<Netlist> nl = parse_netlist(netlist_path, lattice_lib);
// if (nl == nullptr)
// {
//     FAIL() << "netlist could not be parsed";
// }

// auto candidates = test_all(nl->get_gates());

// std::cout << "check done, found " << candidates.size() << " verified candidates" << std::endl;

// EXPECT_FALSE(candidates.empty());
// std::cout << std::endl;
// }    // namespace hal
// TEST_END
// }
// ;

// // ------------ NANGATE ------------
// /**
//      * Test verification of ADDER_1.v for NANGATE
//      *
//      * Functions: module_identification
//      */
// TEST_F(ModuelIdentificationTest,
//        test_nangate_adder_1){TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/test_lib_nangate/ADDER_1.v";

// std::string file_name = std::filesystem::path(netlist_path).filename().string();
// std::string type      = file_name.substr(0, file_name.find("_"));

// std::unique_ptr<Netlist> nl = parse_netlist(netlist_path, nangate_lib);
// if (nl == nullptr)
// {
//     FAIL() << "netlist could not be parsed";
// }

// auto candidates = test_all(nl->get_gates());

// std::cout << "check done, found " << candidates.size() << " verified candidates" << std::endl;

// EXPECT_FALSE(candidates.empty());
// std::cout << std::endl;
// }    // namespace hal
// TEST_END
// }
// ;

// /**
//      * Test verification of ADDER_CARRY_1.v for NANGATE
//      *
//      * Functions: module_identification
//      */
// TEST_F(ModuelIdentificationTest,
//        test_nangate_adder_carry_1){TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/test_lib_nangate/ADDER_CARRY_1.v";

// std::string file_name = std::filesystem::path(netlist_path).filename().string();
// std::string type      = file_name.substr(0, file_name.find("_"));

// std::unique_ptr<Netlist> nl = parse_netlist(netlist_path, nangate_lib);
// if (nl == nullptr)
// {
//     FAIL() << "netlist could not be parsed";
// }

// auto candidates = test_all(nl->get_gates());

// std::cout << "check done, found " << candidates.size() << " verified candidates" << std::endl;

// EXPECT_FALSE(candidates.empty());
// std::cout << std::endl;
// }    // namespace hal
// TEST_END
// }
// ;

// /**
//      * Test verification of ADDER_CARRY_ENABLE_1.v for NANGATE
//      *
//      * Functions: module_identification
//      */
// TEST_F(ModuelIdentificationTest, test_nangate_adder_carry_enable_1){
//     TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/test_lib_nangate/ADDER_CARRY_ENABLE_1.v";

// std::string file_name = std::filesystem::path(netlist_path).filename().string();
// std::string type      = file_name.substr(0, file_name.find("_"));

// std::unique_ptr<Netlist> nl = parse_netlist(netlist_path, nangate_lib);
// if (nl == nullptr)
// {
//     FAIL() << "netlist could not be parsed";
// }

// auto candidates = test_all(nl->get_gates());

// std::cout << "check done, found " << candidates.size() << " verified candidates" << std::endl;

// EXPECT_FALSE(candidates.empty());
// std::cout << std::endl;
// }    // namespace hal
// TEST_END
// }
// ;

// /**
//      * Test verification of ADDER_CARRY_SIGN_EXTEND_1.v for NANGATE
//      *
//      * Functions: module_identification
//      */
// TEST_F(ModuelIdentificationTest, test_nangate_adder_carry_sign_extend_1){
//     TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/test_lib_nangate/ADDER_CARRY_SIGN_EXTEND_1.v";

// std::string file_name = std::filesystem::path(netlist_path).filename().string();
// std::string type      = file_name.substr(0, file_name.find("_"));

// std::unique_ptr<Netlist> nl = parse_netlist(netlist_path, nangate_lib);
// if (nl == nullptr)
// {
//     FAIL() << "netlist could not be parsed";
// }

// auto candidates = test_all(nl->get_gates());

// std::cout << "check done, found " << candidates.size() << " verified candidates" << std::endl;

// EXPECT_FALSE(candidates.empty());
// std::cout << std::endl;
// }    // namespace hal
// TEST_END
// }
// ;

// /**
//      * Test verification of ADDER_CARRY_SIGN_EXTEND_TWO_ENABLE_1.v for NANGATE
//      *
//      * Functions: module_identification
//      */
// TEST_F(ModuelIdentificationTest, test_nangate_adder_carry_sign_extend_two_enable_1){
//     TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/test_lib_nangate/ADDER_CARRY_SIGN_EXTEND_TWO_ENABLE_1.v";

// std::string file_name = std::filesystem::path(netlist_path).filename().string();
// std::string type      = file_name.substr(0, file_name.find("_"));

// std::unique_ptr<Netlist> nl = parse_netlist(netlist_path, nangate_lib);
// if (nl == nullptr)
// {
//     FAIL() << "netlist could not be parsed";
// }

// auto candidates = test_all(nl->get_gates());

// std::cout << "check done, found " << candidates.size() << " verified candidates" << std::endl;

// EXPECT_FALSE(candidates.empty());
// std::cout << std::endl;
// }    // namespace hal
// TEST_END
// }
// ;

// /**
//      * Test verification of ADDER_TWO_ENABLE_1.v for NANGATE
//      *
//      * Functions: module_identification
//      */
// TEST_F(ModuelIdentificationTest, test_nangate_adder_two_enable_1){
//     TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/test_lib_nangate/ADDER_TWO_ENABLE_1.v";

// std::string file_name = std::filesystem::path(netlist_path).filename().string();
// std::string type      = file_name.substr(0, file_name.find("_"));

// std::unique_ptr<Netlist> nl = parse_netlist(netlist_path, nangate_lib);
// if (nl == nullptr)
// {
//     FAIL() << "netlist could not be parsed";
// }

// auto candidates = test_all(nl->get_gates());

// std::cout << "check done, found " << candidates.size() << " verified candidates" << std::endl;

// EXPECT_FALSE(candidates.empty());
// std::cout << std::endl;
// }    // namespace hal
// TEST_END
// }
// ;

// /**
//      * Test verification of COUNTER_1.v for NANGATE
//      *
//      * Functions: module_identification
//      */
// TEST_F(ModuelIdentificationTest,
//        test_nangate_counter_1){TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/test_lib_nangate/COUNTER_1.v";

// std::string file_name = std::filesystem::path(netlist_path).filename().string();
// std::string type      = file_name.substr(0, file_name.find("_"));

// std::unique_ptr<Netlist> nl = parse_netlist(netlist_path, nangate_lib);
// if (nl == nullptr)
// {
//     FAIL() << "netlist could not be parsed";
// }

// auto candidates = test_all(nl->get_gates());

// std::cout << "check done, found " << candidates.size() << " verified candidates" << std::endl;

// EXPECT_FALSE(candidates.empty());
// std::cout << std::endl;
// }    // namespace hal
// TEST_END
// }
// ;

// /**
//      * Test verification of COUNTER_ENABLE_RESET_1.v for NANGATE
//      *
//      * Functions: module_identification
//      */
// TEST_F(ModuelIdentificationTest, test_nangate_counter_enable_reset_1){
//     TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/test_lib_nangate/COUNTER_ENABLE_RESET_1.v";

// std::string file_name = std::filesystem::path(netlist_path).filename().string();
// std::string type      = file_name.substr(0, file_name.find("_"));

// std::unique_ptr<Netlist> nl = parse_netlist(netlist_path, nangate_lib);
// if (nl == nullptr)
// {
//     FAIL() << "netlist could not be parsed";
// }

// auto candidates = test_all(nl->get_gates());

// std::cout << "check done, found " << candidates.size() << " verified candidates" << std::endl;

// EXPECT_FALSE(candidates.empty());
// std::cout << std::endl;
// }    // namespace hal
// TEST_END
// }
// ;

// /**
//      * Test verification of SUBTRACTION_1.v for NANGATE
//      *
//      * Functions: module_identification
//      */
// TEST_F(ModuelIdentificationTest,
//        test_nangate_subtraction_1){TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/test_lib_nangate/SUBTRACTION_1.v";

// std::string file_name = std::filesystem::path(netlist_path).filename().string();
// std::string type      = file_name.substr(0, file_name.find("_"));

// std::unique_ptr<Netlist> nl = parse_netlist(netlist_path, nangate_lib);
// if (nl == nullptr)
// {
//     FAIL() << "netlist could not be parsed";
// }

// auto candidates = test_all(nl->get_gates());

// std::cout << "check done, found " << candidates.size() << " verified candidates" << std::endl;

// EXPECT_FALSE(candidates.empty());
// std::cout << std::endl;
// }    // namespace hal
// TEST_END
// }
// ;

// /**
//      * Test verification of SUBTRACTION_ENABLE_1.v for NANGATE
//      *
//      * Functions: module_identification
//      */
// TEST_F(ModuelIdentificationTest, test_nangate_subtraction_enable_1){
//     TEST_START{std::string netlist_path = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/module_identification/test_lib_nangate/SUBTRACTION_ENABLE_1.v";

// std::string file_name = std::filesystem::path(netlist_path).filename().string();
// std::string type      = file_name.substr(0, file_name.find("_"));

// std::unique_ptr<Netlist> nl = parse_netlist(netlist_path, nangate_lib);
// if (nl == nullptr)
// {
//     FAIL() << "netlist could not be parsed";
// }

// auto candidates = test_all(nl->get_gates());

// std::cout << "check done, found " << candidates.size() << " verified candidates" << std::endl;

// EXPECT_FALSE(candidates.empty());
// std::cout << std::endl;
// }    // namespace hal
// TEST_END
// }
// ;

// }    // namespace hal