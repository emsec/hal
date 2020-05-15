
#include "netlist_test_utils.h"
#include <core/utils.h>

//NOTE: Has to be applied to the new gate_library usage

std::shared_ptr<netlist> test_utils::create_empty_netlist(const int id)
{
    NO_COUT_BLOCK;
    std::shared_ptr<gate_library> gl = gate_library_manager::get_gate_library(g_lib_name);
    std::shared_ptr<netlist> nl   = std::make_shared<netlist>(gl);

    if (id >= 0)
    {
        nl->set_id(id);
    }
    nl->set_device_name("device_name");
    nl->set_design_name("design_name");

    return nl;
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

std::shared_ptr<const gate_type> test_utils::get_gate_type_by_name(std::string name, std::string gate_library_name)
{
    std::shared_ptr<gate_library> gl;
    if (gate_library_name == ""){
        gl = gate_library_manager::get_gate_library(g_lib_name);
    }
    else {
        gl = gate_library_manager::get_gate_library(gate_library_name);
    }
    // If the gl can't be found, return a nullptr
    if (gl == nullptr){
        std::cerr << "gate library \'"<< gl->get_name() <<"\'" << " couldn't be found" << std::endl;
        return nullptr;
    }
    auto names_to_type = gl->get_gate_types();
    // If the gate type isn't found in the gate library
    if (names_to_type.find(name) == names_to_type.end()) {
        std::cerr << "gate type: \'" << name << "\' can't be found in the gate library \'"<< gl->get_name() <<"\'" << std::endl;
        return nullptr;
    }
    else {
        return names_to_type.at(name);
    }

}

bool test_utils::string_contains_substring(const std::string str, const std::string sub_str)
{
    return (str.find(sub_str) != std::string::npos);
}

std::shared_ptr<net> test_utils::get_net_by_subname(std::shared_ptr<netlist> nl, const std::string subname){
    if(nl == nullptr)
        return nullptr;
    std::unordered_set<std::shared_ptr<net>> nets = nl->get_nets();
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

void test_utils::create_temp_gate_lib()
{
    //NO_COUT_BLOCK;

    hal::path dir (core_utils::get_gate_library_directories()[0]);
    hal::path temp_lib_path = (dir) / "TEMP_GATE_LIBRARY.lib";
    std::ofstream test_lib(temp_lib_path.string());
    //std::stringstream test_lib(temp_lib_path.string());
    test_lib << "library (TEMP_GATE_LIBRARY) {\n"
                "    define(cell);\n"
                "    cell(GATE_1^0_IN_1^0_OUT) {\n"
                "        pin(I) { direction: input; }\n"
                "        pin(O) {\n"
                "            direction: output;\n"
                "            function: \"I\";\n"
                "        }\n"
                "    }\n"
                "    \n"
                "    cell(GATE_4^1_IN_4^1_OUT) {\n"
                "        pin(\"I(0)\") { direction: input; }\n"
                "        pin(\"I(1)\") { direction: input; }\n"
                "        pin(\"I(2)\") { direction: input; }\n"
                "        pin(\"I(3)\") { direction: input; }\n"
                "        pin(\"O(0)\") {\n"
                "            direction: output;\n"
                "            function: \"I(0)\";\n"
                "        }\n"
                "        pin(\"O(1)\") {\n"
                "            direction: output;\n"
                "            function: \"I(1)\";\n"
                "        }\n"
                "        pin(\"O(2)\") {\n"
                "            direction: output;\n"
                "            function: \"I(2)\";\n"
                "        }\n"
                "        pin(\"O(3)\") {\n"
                "            direction: output;\n"
                "            function: \"I(3)\";\n"
                "        }\n"
                "    }\n"
                "    \n"
                "    cell(GATE_4^1_IN_1^0_OUT) {\n"
                "        pin(\"I(0)\") { direction: input; }\n"
                "        pin(\"I(1)\") { direction: input; }\n"
                "        pin(\"I(2)\") { direction: input; }\n"
                "        pin(\"I(3)\") { direction: input; }\n"
                "        pin(O) {\n"
                "            direction: output;\n"
                "            function: \"I(0)\";\n"
                "        }\n"
                "    }\n"
                "    \n"
                "    cell(GATE_1^0_IN_4^1_OUT) {\n"
                "        pin(I) { direction: input; }\n"
                "        pin(\"O(0)\") {\n"
                "            direction: output;\n"
                "            function: \"I\";\n"
                "        }\n"
                "        pin(\"O(1)\") {\n"
                "            direction: output;\n"
                "            function: \"I\";\n"
                "        }\n"
                "        pin(\"O(2)\") {\n"
                "            direction: output;\n"
                "            function: \"I\";\n"
                "        }\n"
                "        pin(\"O(3)\") {\n"
                "            direction: output;\n"
                "            function: \"I\";\n"
                "        }\n"
                "    }\n"
                "    \n"
                "    cell(GATE_2^2_IN_2^2_OUT) {\n"
                "        pin(\"I(0,0)\") { direction: input; }\n"
                "        pin(\"I(0,1)\") { direction: input; }\n"
                "        pin(\"I(1,0)\") { direction: input; }\n"
                "        pin(\"I(1,1)\") { direction: input; }\n"
                "        pin(\"O(0,0)\") {\n"
                "            direction: output;\n"
                "            function: \"I(0,0)\";\n"
                "        }\n"
                "        pin(\"O(0,1)\") {\n"
                "            direction: output;\n"
                "            function: \"I(0,1)\";\n"
                "        }\n"
                "        pin(\"O(1,0)\") {\n"
                "            direction: output;\n"
                "            function: \"I(1,0)\";\n"
                "        }\n"
                "        pin(\"O(1,1)\") {\n"
                "            direction: output;\n"
                "            function: \"I(1,1)\";\n"
                "        }\n"
                "    }\n"
                "    \n"
                "    cell(GATE_2^3_IN_2^3_OUT) {\n"
                "        pin(\"I(0,0,0)\") { direction: input; }\n"
                "        pin(\"I(0,0,1)\") { direction: input; }\n"
                "        pin(\"I(0,1,0)\") { direction: input; }\n"
                "        pin(\"I(0,1,1)\") { direction: input; }\n"
                "        pin(\"I(1,0,0)\") { direction: input; }\n"
                "        pin(\"I(1,0,1)\") { direction: input; }\n"
                "        pin(\"I(1,1,0)\") { direction: input; }\n"
                "        pin(\"I(1,1,1)\") { direction: input; }\n"
                "        pin(\"O(0,0,0)\") {\n"
                "            direction: output;\n"
                "            function: \"I(0,0,0)\";\n"
                "        }\n"
                "        pin(\"O(0,0,1)\") {\n"
                "            direction: output;\n"
                "            function: \"I(0,0,1)\";\n"
                "        }\n"
                "        pin(\"O(0,1,0)\") {\n"
                "            direction: output;\n"
                "            function: \"I(0,1,0)\";\n"
                "        }\n"
                "        pin(\"O(0,1,1)\") {\n"
                "            direction: output;\n"
                "            function: \"I(0,1,1)\";\n"
                "        }\n"
                "        pin(\"O(1,0,0)\") {\n"
                "            direction: output;\n"
                "            function: \"I(1,0,0)\";\n"
                "        }\n"
                "        pin(\"O(1,0,1)\") {\n"
                "            direction: output;\n"
                "            function: \"I(1,0,1)\";\n"
                "        }\n"
                "        pin(\"O(1,1,0)\") {\n"
                "            direction: output;\n"
                "            function: \"I(1,1,0)\";\n"
                "        }\n"
                "        pin(\"O(1,1,1)\") {\n"
                "            direction: output;\n"
                "            function: \"I(1,1,1)\";\n"
                "        }\n"
                "    }\n"
                "    \n"
                "    cell(GATE_2^3_IN_1^0_OUT) {\n"
                "        pin(\"I(0,0,0)\") { direction: input; }\n"
                "        pin(\"I(0,0,1)\") { direction: input; }\n"
                "        pin(\"I(0,1,0)\") { direction: input; }\n"
                "        pin(\"I(0,1,1)\") { direction: input; }\n"
                "        pin(\"I(1,0,0)\") { direction: input; }\n"
                "        pin(\"I(1,0,1)\") { direction: input; }\n"
                "        pin(\"I(1,1,0)\") { direction: input; }\n"
                "        pin(\"I(1,1,1)\") { direction: input; }\n"
                "        pin(O) {\n"
                "            direction: output;\n"
                "            function: \"I(0,0,0)\";\n"
                "        }\n"
                "    }\n"
                "    \n"
                "    cell(GATE_1^0_IN_2^3_OUT) {\n"
                "        pin(I) { direction: input; }\n"
                "        pin(\"O(0,0,0)\") {\n"
                "            direction: output;\n"
                "            function: \"I\";\n"
                "        }\n"
                "        pin(\"O(0,0,1)\") {\n"
                "            direction: output;\n"
                "            function: \"I\";\n"
                "        }\n"
                "        pin(\"O(0,1,0)\") {\n"
                "            direction: output;\n"
                "            function: \"I\";\n"
                "        }\n"
                "        pin(\"O(0,1,1)\") {\n"
                "            direction: output;\n"
                "            function: \"I\";\n"
                "        }\n"
                "        pin(\"O(1,0,0)\") {\n"
                "            direction: output;\n"
                "            function: \"I\";\n"
                "        }\n"
                "        pin(\"O(1,0,1)\") {\n"
                "            direction: output;\n"
                "            function: \"I\";\n"
                "        }\n"
                "        pin(\"O(1,1,0)\") {\n"
                "            direction: output;\n"
                "            function: \"I\";\n"
                "        }\n"
                "        pin(\"O(1,1,1)\") {\n"
                "            direction: output;\n"
                "            function: \"I\";\n"
                "        }\n"
                "    }\n"
                "    \n"
                "    cell(GND) {\n"
                "        pin(O) {\n"
                "            direction: output;\n"
                "            function: \"0\";\n"
                "        }\n"
                "    }\n"
                "    \n"
                "    cell(VCC) {\n"
                "        pin(O) {\n"
                "            direction: output;\n"
                "            function: \"1\";\n"
                "        }\n"
                "    }\n"
                "    \n"
                "}";
    test_lib.close();
    //std::cout << "=============" << test_lib.str() << "=============" << std::endl;

    //gate_library_manager::load_all();
}

void test_utils::remove_temp_gate_lib() {
    fs::remove(((core_utils::get_gate_library_directories()[0]) / "temp_lib.json").string());
}

std::shared_ptr<netlist> test_utils::create_example_netlist(const int id)
{
    NO_COUT_BLOCK;
    std::shared_ptr<gate_library> gl = gate_library_manager::get_gate_library(g_lib_name);
    std::shared_ptr<netlist> nl      = create_empty_netlist(id);
    if (id >= 0)
    {
        nl->set_id(id);
    }

    // Create the gates
    std::shared_ptr<gate> gate_0 = nl->create_gate(MIN_GATE_ID+0, gl->get_gate_types().at("AND2"), "gate_0");
    std::shared_ptr<gate> gate_1 = nl->create_gate(MIN_GATE_ID+1, gl->get_gate_types().at("GND"), "gate_1");
    std::shared_ptr<gate> gate_2 = nl->create_gate(MIN_GATE_ID+2, gl->get_gate_types().at("VCC"), "gate_2");
    std::shared_ptr<gate> gate_3 = nl->create_gate(MIN_GATE_ID+3, gl->get_gate_types().at("INV"), "gate_3");
    std::shared_ptr<gate> gate_4 = nl->create_gate(MIN_GATE_ID+4, gl->get_gate_types().at("INV"), "gate_4");
    std::shared_ptr<gate> gate_5 = nl->create_gate(MIN_GATE_ID+5, gl->get_gate_types().at("AND2"), "gate_5");
    std::shared_ptr<gate> gate_6 = nl->create_gate(MIN_GATE_ID+6, gl->get_gate_types().at("BUF"), "gate_6");
    std::shared_ptr<gate> gate_7 = nl->create_gate(MIN_GATE_ID+7, gl->get_gate_types().at("OR2"), "gate_7");
    std::shared_ptr<gate> gate_8 = nl->create_gate(MIN_GATE_ID+8, gl->get_gate_types().at("OR2"), "gate_8");


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
    std::shared_ptr<netlist> nl      = create_empty_netlist(id);

    // Create the gates
    std::shared_ptr<gate> gate_0 = nl->create_gate(MIN_GATE_ID+0, gl->get_gate_types().at("AND4"), "gate_0");
    std::shared_ptr<gate> gate_1 = nl->create_gate(MIN_GATE_ID+1, gl->get_gate_types().at("AND4"), "gate_1");
    std::shared_ptr<gate> gate_2 = nl->create_gate(MIN_GATE_ID+2, gl->get_gate_types().at("AND4"), "gate_2");
    std::shared_ptr<gate> gate_3 = nl->create_gate(MIN_GATE_ID+3, gl->get_gate_types().at("AND4"), "gate_3");

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
    std::shared_ptr<netlist> nl = create_empty_netlist(id);

    // Create the gate
    std::shared_ptr<gate> gate_0 = nl->create_gate(MIN_GATE_ID+0, gl->get_gate_types().at("INV"), "gate_0");

    // net connected to the input pin
    std::shared_ptr<net> net_X_1 = nl->create_net(MIN_GATE_ID+0, "net_X_1");
    net_X_1->add_dst(gate_0, "I");

    // net connected to the output pin
    std::shared_ptr<net> net_1_X = nl->create_net(MIN_GATE_ID+1, "net_1_X");
    net_1_X->set_src(gate_0, "O");

    return nl;
}

std::shared_ptr<netlist> test_utils::create_example_parse_netlist(int id)
{
    NO_COUT_BLOCK;
    std::shared_ptr<gate_library> gl = gate_library_manager::get_gate_library(g_lib_name);
    std::shared_ptr<netlist> nl      = create_empty_netlist(id);


    // Create the gates
    std::shared_ptr<gate> gate_0 = nl->create_gate(MIN_GATE_ID+0, gl->get_gate_types().at("AND2"), "gate_0");
    std::shared_ptr<gate> gate_1 = nl->create_gate(MIN_GATE_ID+1, gl->get_gate_types().at("GND"), "gate_1");
    std::shared_ptr<gate> gate_2 = nl->create_gate(MIN_GATE_ID+2, gl->get_gate_types().at("VCC"), "gate_2");
    std::shared_ptr<gate> gate_3 = nl->create_gate(MIN_GATE_ID+3, gl->get_gate_types().at("INV"), "gate_3");
    std::shared_ptr<gate> gate_4 = nl->create_gate(MIN_GATE_ID+4, gl->get_gate_types().at("INV"), "gate_4");
    std::shared_ptr<gate> gate_5 = nl->create_gate(MIN_GATE_ID+5, gl->get_gate_types().at("AND2"), "gate_5");
    std::shared_ptr<gate> gate_6 = nl->create_gate(MIN_GATE_ID+6, gl->get_gate_types().at("OR2"), "gate_6");
    std::shared_ptr<gate> gate_7 = nl->create_gate(MIN_GATE_ID+7, gl->get_gate_types().at("OR2"), "gate_7");

    // Add the nets (net_x_y1_y2... := net between the gate with id x and the gates y1,y2,...)
    std::shared_ptr<net> net_1_3 = nl->create_net(MIN_NET_ID+13, "0_net");
    net_1_3->set_src(gate_1, "O");
    net_1_3->add_dst(gate_3, "I");

    std::shared_ptr<net> net_3_0 = nl->create_net(MIN_NET_ID+30, "net_3_0");
    net_3_0->set_src(gate_3, "O");
    net_3_0->add_dst(gate_0, "I0");

    std::shared_ptr<net> net_2_0 = nl->create_net(MIN_NET_ID+20, "1_net");
    net_2_0->set_src(gate_2, "O");
    net_2_0->add_dst(gate_0, "I1");

    std::shared_ptr<net> net_0_4_5 = nl->create_net(MIN_NET_ID+045, "net_0_4_5");
    net_0_4_5->set_src(gate_0, "O");
    net_0_4_5->add_dst(gate_4, "I");
    net_0_4_5->add_dst(gate_5, "I0");

    std::shared_ptr<net> net_6_7 = nl->create_net(MIN_NET_ID+67, "net_6_7");
    net_6_7->set_src(gate_6, "O");
    net_6_7->add_dst(gate_7, "I0");

    std::shared_ptr<net> net_4_out = nl->create_net(MIN_NET_ID+400, "net_4_out");
    net_4_out->set_src(gate_4, "O");

    std::shared_ptr<net> net_5_out = nl->create_net(MIN_NET_ID+500, "net_5_out");
    net_5_out->set_src(gate_5, "O");

    std::shared_ptr<net> net_7_out = nl->create_net(MIN_NET_ID+700, "net_7_out");
    net_7_out->set_src(gate_7, "O");

    return nl;
}

std::shared_ptr<gate> test_utils::create_test_gate(std::shared_ptr<netlist> nl, const u32 id)
{
    std::shared_ptr<gate_library> gl = gate_library_manager::get_gate_library(g_lib_name);
    std::shared_ptr<gate> res_gate = nl->create_gate(id, gl->get_gate_types().at("AND3"), "gate_" + std::to_string(id));

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
    if (n0->is_global_input_net() != n1->is_global_input_net())
        return false;
    if (n0->is_global_output_net() != n1->is_global_output_net())
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
    if (g0->is_gnd_gate() != g1->is_gnd_gate())
        return false;
    if (g0->is_vcc_gate() != g1->is_vcc_gate())
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
    if (m_0->get_submodules(nullptr,true).size() != m_1->get_submodules(nullptr,true).size())
        return false;
    for (auto sm_0 : m_0->get_submodules(nullptr,true)){
        if(sm_0 == nullptr) continue;
        if(m_1->get_netlist()->get_module_by_id(sm_0->get_id()) == nullptr){
            return false;
        }
    }

    return true;
}

bool test_utils::netlists_are_equal(const std::shared_ptr<netlist> nl_0, const std::shared_ptr<netlist> nl_1, const bool ignore_id)
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
    if (nl_0->get_gate_library()->get_name() != nl_1->get_gate_library()->get_name())
        return false;

    // Check if gates and nets are the same
    if (nl_0->get_gates().size() != nl_1->get_gates().size())
        return false;
    for (auto g_0 : nl_0->get_gates())
    {
        if (ignore_id) {
            auto g_1_list = nl_1->get_gates(gate_name_filter(g_0->get_name()));
            if (g_1_list.size() != 1)
                return false;
            if (!gates_are_equal(g_0, *g_1_list.begin(), ignore_id))
                return false;
        }
        else {
            if (!gates_are_equal(g_0, nl_1->get_gate_by_id(g_0->get_id()), ignore_id))
                return false;
        }
    }

    if (nl_0->get_nets().size() != nl_1->get_nets().size())
        return false;
    for (auto n_0 : nl_0->get_nets())
    {
        if (ignore_id) {
            auto n_1_list = nl_1->get_nets(net_name_filter(n_0->get_name()));
            if (n_1_list.size() != 1)
                return false;
            if (!nets_are_equal(n_0, *n_1_list.begin(), ignore_id))
                return false;
        }
        else {
            if (!nets_are_equal(n_0, nl_1->get_net_by_id(n_0->get_id()), ignore_id))
                return false;
        }
    }

    // -- Check if the modules are the same
    if(nl_0->get_modules().size() != nl_1->get_modules().size())
        return false;
    std::set<std::shared_ptr<module>> mods_1 = nl_1->get_modules();
    for(auto m_0 : nl_0->get_modules()){
        if (ignore_id) {
            auto m_1_all = nl_1->get_modules();
            auto m_1 = std::find_if(m_1_all.begin(), m_1_all.end(),
                    [m_0](const std::shared_ptr<module> m){ return m->get_name() == m_0->get_name();});
            if (m_1 == m_1_all.end())
                return false;
            if (!modules_are_equal(m_0, *m_1, ignore_id))
                return false;
        }
        else {
            if (!modules_are_equal(m_0, nl_1->get_module_by_id(m_0->get_id()), ignore_id))
                return false;
        }
    }

    return true;
}

// Filter Functions

std::function<bool(const std::shared_ptr<module>&)> test_utils::module_name_filter(const std::string& name){
    return [name](auto& m){return m->get_name() == name;};
}

std::function<bool(const std::shared_ptr<gate>&)> test_utils::gate_filter(const std::string& type, const std::string& name){
    return [name, type](auto& g){return g->get_name() == name && g->get_type()->get_name() == type;};
}

std::function<bool(const std::shared_ptr<gate>&)> test_utils::gate_name_filter(const std::string& name){
    return [name](auto& g){return g->get_name() == name;};
}

std::function<bool(const std::shared_ptr<gate>&)> test_utils::gate_type_filter(const std::string& type){
    return [type](auto& g){return g->get_type()->get_name() == type;};
}

std::function<bool(const std::shared_ptr<net>&)> test_utils::net_name_filter(const std::string& name){
    return [name](auto& n){return n->get_name() == name;};
}

std::function<bool(const endpoint&)> test_utils::endpoint_type_filter(const std::string& type){
    return [type](auto& ep){return ep.gate->get_type()->get_name() == type;};
}

std::function<bool(const std::string&, const endpoint&)> test_utils::endpoint_pin_filter(const std::string& pin){
    return [pin](auto&, auto& ep){return ep.pin_type == pin;};
}
std::function<bool(const std::string&, const endpoint&)> test_utils::starting_pin_filter(const std::string& pin){
    return [pin](auto& starting_pin, auto&){return starting_pin == pin;};
}

std::function<bool(const std::string&, const endpoint&)> test_utils::type_filter(const std::string& type){
    return [type](auto&, auto& ep){return ep.gate->get_type()->get_name() == type;};
}











