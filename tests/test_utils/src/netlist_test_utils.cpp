
#include "netlist_test_utils.h"
#include <core/utils.h>

std::shared_ptr<netlist> test_utils::create_empty_netlist(const int id)
{
    NO_COUT_BLOCK;
    std::shared_ptr<gate_library> gl = gate_library_manager::get_gate_library(g_lib_name);
    std::shared_ptr<netlist> g_obj   = std::make_shared<netlist>(gl);

    if (id >= 0)
    {
        g_obj->set_id(id);
    }
    return g_obj;
}

endpoint test_utils::get_endpoint(const std::shared_ptr<gate> g, const std::string pin_type)
{
    endpoint ep;
    ep.gate     = g;
    ep.pin_type = pin_type;
    return ep;
}

endpoint test_utils::get_endpoint(const std::shared_ptr<netlist> nl, const int gate_id, const std::string pin_type)
{
    std::shared_ptr<gate> g = nl->get_gate_by_id(gate_id);
    if (g != nullptr)
        return get_endpoint(g, pin_type);
    else
        return get_endpoint(nullptr, "");
}

bool test_utils::is_empty(const endpoint ep)
{
    return ((ep.gate == nullptr) && (ep.pin_type == ""));
}

bool test_utils::string_contains_substring(const std::string str, const std::string sub_str)
{
    return (str.find(sub_str) != std::string::npos);
}

std::shared_ptr<netlist> test_utils::create_example_netlist(const int id)
{
    NO_COUT_BLOCK;
    std::shared_ptr<gate_library> gl = gate_library_manager::get_gate_library(g_lib_name);
    std::shared_ptr<netlist> nl      = std::make_shared<netlist>(gl);
    if (id >= 0)
    {
        nl->set_id(id);
    }

    // Create the gates
    std::shared_ptr<gate> gate_0 = nl->create_gate(MIN_GATE_ID+0, "AND2", "gate_0");
    std::shared_ptr<gate> gate_1 = nl->create_gate(MIN_GATE_ID+1, "GND", "gate_1");
    std::shared_ptr<gate> gate_2 = nl->create_gate(MIN_GATE_ID+2, "VCC", "gate_2");
    std::shared_ptr<gate> gate_3 = nl->create_gate(MIN_GATE_ID+3, "INV", "gate_3");
    std::shared_ptr<gate> gate_4 = nl->create_gate(MIN_GATE_ID+4, "INV", "gate_4");
    std::shared_ptr<gate> gate_5 = nl->create_gate(MIN_GATE_ID+5, "AND2", "gate_5");
    std::shared_ptr<gate> gate_6 = nl->create_gate(MIN_GATE_ID+6, "BUF", "gate_6");
    std::shared_ptr<gate> gate_7 = nl->create_gate(MIN_GATE_ID+7, "OR2", "gate_7");
    std::shared_ptr<gate> gate_8 = nl->create_gate(MIN_GATE_ID+8, "OR2", "gate_8");

    // Add the nets (net_x_y1_y2... := net between the gate with id x and the gates y1,y2,...)
    std::shared_ptr<net> net_1_3 = nl->create_net(MIN_NET_ID+13, "net_1_3");
    net_1_3->set_src(gate_1, "O");
    net_1_3->add_dst(gate_3, "I");

    std::shared_ptr<net> net_3_0 = nl->create_net(MIN_NET_ID+30, "net_3_0");
    net_3_0->set_src(gate_3, "O");
    net_3_0->add_dst(gate_0, "I0");

    std::shared_ptr<net> net_2_0 = nl->create_net(MIN_NET_ID+20, "net_2_0");
    net_2_0->set_src(gate_2, "O");
    net_2_0->add_dst(gate_0, "I1");

    std::shared_ptr<net> net_0_4_5 = nl->create_net(MIN_NET_ID+045, "net_0_4_5");
    net_0_4_5->set_src(gate_0, "O");
    net_0_4_5->add_dst(gate_4, "I");
    net_0_4_5->add_dst(gate_5, "I0");

    std::shared_ptr<net> net_7_8 = nl->create_net(MIN_NET_ID+78, "net_7_8");
    net_7_8->set_src(gate_7, "O");
    net_7_8->add_dst(gate_8, "I0");

    return nl;
}

std::shared_ptr<netlist> test_utils::create_example_netlist_2(const int id)
{
    NO_COUT_BLOCK;
    std::shared_ptr<gate_library> gl = gate_library_manager::get_gate_library(g_lib_name);
    std::shared_ptr<netlist> nl      = std::make_shared<netlist>(gl);
    if (id >= 0)
    {
        nl->set_id(id);
    }

    // Create the gates
    std::shared_ptr<gate> gate_0 = nl->create_gate(MIN_GATE_ID+0, "AND4", "gate_0");
    std::shared_ptr<gate> gate_1 = nl->create_gate(MIN_GATE_ID+1, "AND4", "gate_1");
    std::shared_ptr<gate> gate_2 = nl->create_gate(MIN_GATE_ID+2, "AND4", "gate_2");
    std::shared_ptr<gate> gate_3 = nl->create_gate(MIN_GATE_ID+3, "AND4", "gate_3");

    // Add the nets (net_x_y1_y2... := net between the gate with id x and the gates y1,y2,...)

    std::shared_ptr<net> net_0_1_3 = nl->create_net(MIN_NET_ID+013, "net_0_1_3");
    net_0_1_3->set_src(gate_0, "O");
    net_0_1_3->add_dst(gate_1, "I0");
    net_0_1_3->add_dst(gate_1, "I1");
    net_0_1_3->add_dst(gate_1, "I2");
    net_0_1_3->add_dst(gate_3, "I0");

    std::shared_ptr<net> net_2_1 = nl->create_net(MIN_NET_ID+21, "net_2_1");
    net_2_1->set_src(gate_2, "O");
    net_2_1->add_dst(gate_1, "I3");

    return nl;
}

std::shared_ptr<netlist> test_utils::create_example_netlist_negative(const int id)
{
    NO_COUT_BLOCK;
    std::shared_ptr<gate_library> gl = gate_library_manager::get_gate_library(g_lib_name);
    std::shared_ptr<netlist> nl(new netlist(gl));
    if (id >= 0)
    {
        nl->set_id(id);
    }

    // Create the gate
    std::shared_ptr<gate> gate_0 = nl->create_gate(MIN_GATE_ID+0, "INV", "gate_0");

    // net connected to the input pin
    std::shared_ptr<net> net_X_1 = nl->create_net(MIN_GATE_ID+0, "net_X_1");
    net_X_1->add_dst(gate_0, "I");

    // net connected to the output pin
    std::shared_ptr<net> net_1_X = nl->create_net(MIN_GATE_ID+1, "net_1_X");
    net_1_X->set_src(gate_0, "O");

    return nl;
}

std::shared_ptr<gate> test_utils::create_test_gate(std::shared_ptr<netlist> nl, const u32 id)
{
    std::shared_ptr<gate> res_gate = nl->create_gate(id, "AND3", "gate_" + std::to_string(id));

    return res_gate;
}

endpoint test_utils::get_dst_by_pin_type(const std::vector<endpoint> dsts, const std::string pin_type)
{
    for (auto dst : dsts)
    {
        if (dst.get_pin_type() == pin_type)
        {
            return dst;
        }
    }
    return {nullptr, ""};
}

bool test_utils::nets_are_equal(const std::shared_ptr<net> n0, const std::shared_ptr<net> n1, const bool ignore_id, const bool ignore_name)
{
    if (n0 == nullptr || n1 == nullptr)
    {
        if (n0 == n1)
            return true;
        else
            return false;
    }
    if (!ignore_id && n0->get_id() != n1->get_id())
        return false;
    if (!ignore_name && n0->get_name() != n1->get_name())
        return false;
    if (n0->get_src().get_pin_type() != n1->get_src().get_pin_type())
        return false;
    if (!gates_are_equal(n0->get_src().get_gate(), n1->get_src().get_gate(), ignore_id, ignore_name))
        return false;
    for (auto n0_dst : n0->get_dsts())
    {
        if (!gates_are_equal(n0_dst.get_gate(), get_dst_by_pin_type(n1->get_dsts(), n0_dst.get_pin_type()).get_gate(), ignore_id, ignore_name))
        {
            return false;
        }
    }
    if (n0->get_data() != n1->get_data())
        return false;
    return true;
}

bool test_utils::gates_are_equal(const std::shared_ptr<gate> g0, const std::shared_ptr<gate> g1, const bool ignore_id, const bool ignore_name)
{
    if (g0 == nullptr || g1 == nullptr)
    {
        if (g0 == g1)
            return true;
        else
            return false;
    }
    if (!ignore_id && g0->get_id() != g1->get_id())
        return false;
    if (!ignore_name && g0->get_name() != g1->get_name())
        return false;
    if (g0->get_type() != g1->get_type())
        return false;
    if (g0->get_data() != g1->get_data())
        return false;
    return true;
}

bool test_utils::modules_are_equal(const std::shared_ptr<module> m_0, const std::shared_ptr<module> m_1, const bool ignore_id, const bool ignore_name)
{
    // Not only one of them may be a nullptr
    if (m_0 == nullptr || m_1 == nullptr)
    {
        if (m_0 == m_1)
            return true;
        else
            return false;
    }
    // The ids should be equal
    if (!ignore_id && m_0->get_id() != m_1->get_id())
        return false;
    // The names should be equal
    if (!ignore_name && m_0->get_name() != m_1->get_name())
        return false;
    // The stored data should be equal
    if (m_0->get_data() != m_1->get_data())
        return false;


    // Check if gates and nets are the same
    if (m_0->get_gates().size() != m_1->get_gates().size())
        return false;
    for (auto g_0 : m_0->get_gates())
    {
        std::shared_ptr<gate> g_1 = m_1->get_netlist()->get_gate_by_id(g_0->get_id());
        if (!gates_are_equal(g_0, g_1, ignore_id, ignore_name))
            return false;
        if (!m_1->contains_gate(g_1))
            return false;
    }

    // The parents and submodules should be equal as well (to test this we only check their id, since
    // their equality will be tested as well)
    if (m_0->get_parent_module() == nullptr || m_1->get_parent_module() == nullptr){
        if (m_0->get_parent_module() != m_1->get_parent_module())
            return false;
    }
    if (m_0->get_submodules(DONT_CARE,true).size() != m_1->get_submodules(DONT_CARE,true).size())
        return false;
    for (auto sm_0 : m_0->get_submodules(DONT_CARE,true)){
        if(sm_0 == nullptr) continue;
        if(m_1->get_netlist()->get_module_by_id(sm_0->get_id()) == nullptr){
            return false;
        }
    }

    return true;
}

bool test_utils::netlists_are_equal(const std::shared_ptr<netlist> nl_0, const std::shared_ptr<netlist> nl_1, const bool ignore_id, const bool ignore_name)
{
    if (nl_0 == nullptr || nl_1 == nullptr)
    {
        if (nl_0 == nl_1)
            return true;
        else
            return false;
    }
    if (!ignore_id && nl_0->get_id() != nl_1->get_id())
        return false;
    if (!ignore_name && nl_0->get_gate_library()->get_name() != nl_1->get_gate_library()->get_name())
        return false;

    // Check if gates and nets are the same
    if (nl_0->get_gates().size() != nl_1->get_gates().size())
        return false;
    for (auto g_0 : nl_0->get_gates())
    {
        if (!gates_are_equal(g_0, nl_1->get_gate_by_id(g_0->get_id()), ignore_id, ignore_name))
            return false;
    }

    if (nl_0->get_nets().size() != nl_1->get_nets().size())
        return false;
    for (auto n_0 : nl_0->get_nets())
    {
        if (!nets_are_equal(n_0, nl_1->get_net_by_id(n_0->get_id()), ignore_id, ignore_name))
            return false;
    }

    // Check if global gates are the same
    if (nl_0->get_global_gnd_gates().size() != nl_1->get_global_gnd_gates().size())
        return false;
    for (auto gl_gnd_0 : nl_0->get_global_gnd_gates())
    {
        if (!nl_1->is_global_gnd_gate(nl_1->get_gate_by_id(gl_gnd_0->get_id())))
            return false;
    }

    if (nl_0->get_global_vcc_gates().size() != nl_1->get_global_vcc_gates().size())
        return false;
    for (auto gl_vcc_0 : nl_0->get_global_vcc_gates())
    {
        if (!nl_1->is_global_vcc_gate(nl_1->get_gate_by_id(gl_vcc_0->get_id())))
            return false;
    }

    // Check if global nets are the same
    if (nl_0->get_global_input_nets().size() != nl_1->get_global_input_nets().size())
        return false;
    for (auto gl_in_net : nl_0->get_global_input_nets())
    {
        if (!nl_1->is_global_input_net(nl_1->get_net_by_id(gl_in_net->get_id())))
            return false;
    }

    if (nl_0->get_global_output_nets().size() != nl_1->get_global_output_nets().size())
        return false;
    for (auto gl_out_net : nl_0->get_global_output_nets())
    {
        if (!nl_1->is_global_output_net(nl_1->get_net_by_id(gl_out_net->get_id())))
            return false;
    }

    if (nl_0->get_global_inout_nets().size() != nl_1->get_global_inout_nets().size())
        return false;
    for (auto gl_inout_net : nl_0->get_global_inout_nets())
    {
        if (!nl_1->is_global_inout_net(nl_1->get_net_by_id(gl_inout_net->get_id())))
            return false;
    }

    // -- Check if the modules are the same
    if(nl_0->get_modules().size() != nl_1->get_modules().size())
        return false;
    std::set<std::shared_ptr<module>> mods_1 = nl_1->get_modules();
    for(auto m_0 : nl_0->get_modules()){
        if(!modules_are_equal(m_0, nl_1->get_module_by_id(m_0->get_id()), ignore_id, ignore_name))
            return false;
    }

    return true;
}
/* OLD parser_vhdl_old temp gate lib
void test_utils::create_temp_gate_lib()
{
    NO_COUT_BLOCK;

    hal::path lol (core_utils::get_gate_library_directories()[0]);
    hal::path temp_lib_path = (lol) / "temp_lib.json";
    std::ofstream test_lib(temp_lib_path.string());
    test_lib << "{\n"
                "    \"library\": {\n"
                "        \"library_name\": \"TEMP_GATE_LIBRARY\",\n"
                "        \"elements\": {\n"
                "            \"GATE0\" : [[\"I\"], [], [\"O\"]],\n"
                "            \"GATE1\" : [[\"I(0)\",\"I(1)\",\"I(2)\",\"I(3)\",\"I(4)\"], [], [\"O(0)\",\"O(1)\",\"O(2)\",\"O(3)\", \"O(4)\"]],\n"
                "            \"GATE2\" : [[\"I(0, 0)\",\"I(0, 1)\",\"I(1, 0)\",\"I(1, 1)\"], [], [\"O(0, 0)\",\"O(0, 1)\",\"O(1, 0)\",\"O(1, 1)\"]],\n"
                "            \"GATE3\" : [[\"I(0, 0, 0)\",\"I(0, 0, 1)\",\"I(0, 1, 0)\",\"I(0, 1, 1)\",\"I(1, 0, 0)\",\"I(1, 0, 1)\",\"I(1, 1, 0)\",\"I(1, 1, 1)\"], [], [\"O(0, 0, 0)\",\"O(0, 0, 1)\",\"O(0, 1, 0)\",\"O(0, 1, 1)\",\"O(1, 0, 0)\",\"O(1, 0, 1)\",\"O(1, 1, 0)\",\"O(1, 1, 1)\"]],\n"
                "\n"
                "            \"GND\" : [[], [], [\"O\"]],\n"
                "            \"VCC\" : [[], [], [\"O\"]]\n"
                "        },\n"
                "        \"vhdl_includes\": [],\n"
                "        \"global_gnd_nodes\": [\"GND\"],\n"
                "        \"global_vcc_nodes\": [\"VCC\"]\n"
                "    }\n"
                "}";
    test_lib.close();

    gate_library_manager::load_all();
}*/
/*
// Create and load temporarily a custom gate library, which contains gates with input and output vectors up to dimension 3 (this is not the gate_lib of the vhdl_parser test)
void create_temp_gate_lib()
{
    NO_COUT_BLOCK;
    std::ofstream test_lib(temp_lib_path.string());
    test_lib << "{\n"
                "    \"library\": {\n"
                "        \"library_name\": \"TEMP_GATE_LIBRARY\",\n"
                "        \"elements\": {\n"
                "            \"GATE0\" : [[\"I\"], [], [\"O\"]],\n"
                "            \"GATE1\" : [[\"I(0)\",\"I(1)\",\"I(2)\",\"I(3)\"], [], [\"O\"]],\n"
                "            \"GATE2\" : [[\"I\"], [], [\"O(0)\",\"O(1)\",\"O(2)\",\"O(3)\"]],\n"
                "            \"GATE3\" : [[\"I(0)\",\"I(1)\",\"I(2)\",\"I(3)\"], [], [\"O(0)\",\"O(1)\",\"O(2)\",\"O(3)\"]],\n"
                //"            \"GATE4\" : [[\"I(0, 0)\",\"I(0, 1)\",\"I(1, 0)\",\"I(1, 1)\"], [], [\"O(0, 0)\",\"O(0, 1)\",\"O(1, 0)\",\"O(1, 1)\"]],\n"
                //"            \"GATE5\" : [[\"I(0, 0, 0)\",\"I(0, 0, 1)\",\"I(0, 1, 0)\",\"I(0, 1, 1)\",\"I(1, 0, 0)\",\"I(1, 0, 1)\",\"I(1, 1, 0)\",\"I(1, 1, 1)\"], [], [\"O(0, 0, 0)\",\"O(0, 0, 1)\",\"O(0, 1, 0)\",\"O(0, 1, 1)\",\"O(1, 0, 0)\",\"O(1, 0, 1)\",\"O(1, 1, 0)\",\"O(1, 1, 1)\"]],\n"
                "\n"
                "            \"GND\" : [[], [], [\"O\"]],\n"
                "            \"VCC\" : [[], [], [\"O\"]]\n"
                "        },\n"
                "        \"vhdl_includes\": [],\n"
                "        \"global_gnd_nodes\": [\"GND\"],\n"
                "        \"global_vcc_nodes\": [\"VCC\"]\n"
                "    }\n"
                "}";
    test_lib.close();

    gate_library_manager::load_all();
}*/

void test_utils::create_temp_gate_lib()
{
    NO_COUT_BLOCK;

    hal::path lol (core_utils::get_gate_library_directories()[0]);
    hal::path temp_lib_path = (lol) / "temp_lib.json";
    std::ofstream test_lib(temp_lib_path.string());
    test_lib << "{\n"
                "    \"library\": {\n"
                "        \"library_name\": \"TEMP_GATE_LIBRARY\",\n"
                "        \"elements\": {\"GATE_1^0_IN_1^0_OUT\" : [[\"I\"], [], [\"O\"]],\n"
                "            \"GATE_4^1_IN_4^1_OUT\" : [[\"I(0)\",\"I(1)\",\"I(2)\",\"I(3)\"], [], [\"O(0)\",\"O(1)\",\"O(2)\",\"O(3)\"]],\n"
                "            \"GATE_4^1_IN_1^0_OUT\" : [[\"I(0)\",\"I(1)\",\"I(2)\",\"I(3)\"], [], [\"O\"]],\n"
                "            \"GATE_2^2_IN_2^2_OUT\" : [[\"I(0, 0)\",\"I(0, 1)\",\"I(1, 0)\",\"I(1, 1)\"], [], [\"O(0, 0)\",\"O(0, 1)\",\"O(1, 0)\",\"O(1, 1)\"]],\n"
                "            \"GATE_2^3_IN_2^3_OUT\" : [[\"I(0, 0, 0)\",\"I(0, 0, 1)\",\"I(0, 1, 0)\",\"I(0, 1, 1)\",\"I(1, 0, 0)\",\"I(1, 0, 1)\",\"I(1, 1, 0)\",\"I(1, 1, 1)\"], [], [\"O(0, 0, 0)\",\"O(0, 0, 1)\",\"O(0, 1, 0)\",\"O(0, 1, 1)\",\"O(1, 0, 0)\",\"O(1, 0, 1)\",\"O(1, 1, 0)\",\"O(1, 1, 1)\"]],\n"
                "\n"
                "            \"GND\" : [[], [], [\"O\"]],\n"
                "            \"VCC\" : [[], [], [\"O\"]]\n"
                "        },\n"
                "        \"vhdl_includes\": [],\n"
                "        \"global_gnd_nodes\": [\"GND\"],\n"
                "        \"global_vcc_nodes\": [\"VCC\"]\n"
                "    }\n"
                "}";
    test_lib.close();

    gate_library_manager::load_all();
}

void test_utils::remove_temp_gate_lib() {
    boost::filesystem::remove(((core_utils::get_gate_library_directories()[0]) / "temp_lib.json").string());
}

std::shared_ptr<net> test_utils::get_net_by_subname(std::shared_ptr<netlist> nl, const std::string subname){
    if(nl == nullptr)
        return nullptr;
    std::set<std::shared_ptr<net>> nets = nl->get_nets();
    std::shared_ptr<net> res = nullptr;
    for (auto n : nets){
        std::string n_name = n->get_name();
        if (n_name.find(subname) != n_name.npos){
            if (res != nullptr){
                std::cerr << "Multiple gates contains the subtring '" << subname << "'! This should not happen..." << std::endl;
                return nullptr;
            }
            res = n;
        }
    }
    return res;
}

std::shared_ptr<gate> test_utils::get_gate_by_subname(std::shared_ptr<netlist> nl, const std::string subname){
    if(nl == nullptr)
        return nullptr;
    std::set<std::shared_ptr<gate>> gates = nl->get_gates();
    std::shared_ptr<gate> res = nullptr;
    for (auto g : gates){
        std::string g_name = g->get_name();
        if (g_name.find(subname) != g_name.npos){
            if (res != nullptr){
                std::cerr << "Multiple gates contains the subtring '" << subname << "'! This should not happen..." << std::endl;
                return nullptr;
            }
            res = g;
        }
    }
    return res;
}
