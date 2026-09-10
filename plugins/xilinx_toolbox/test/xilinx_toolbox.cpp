#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/netlist/netlist_factory.h"
#include "hgl_parser/hgl_parser.h"
#include "netlist_test_utils.h"
#include "xilinx_toolbox/plugin_xilinx_toolbox.h"
#include "xilinx_toolbox/preprocessing.h"

namespace hal
{
    class XilinxToolboxTest : public ::testing::Test
    {
    protected:
        std::unique_ptr<GateLibrary> m_gl_owner;
        GateLibrary* m_gl = nullptr;

        virtual void SetUp()
        {
            NO_COUT_BLOCK;
            test_utils::init_log_channels();
            test_utils::create_sandbox_directory();

            // the gate library manager relies on the parser plugins being registered, so parse the library directly
            const std::string path = utils::get_base_directory().string() + "/share/hal/gate_libraries/XILINX_UNISIM.hgl";

            HGLParser parser;
            if (auto res = parser.parse(path); res.is_ok())
            {
                m_gl_owner = res.get();
                m_gl       = m_gl_owner.get();
            }
        }

        virtual void TearDown()
        {
            test_utils::remove_sandbox_directory();
        }

        /**
         * A netlist using the Xilinx UNISIM gate library, with GND and VCC gates already marked.
         */
        std::unique_ptr<Netlist> create_netlist(Net** gnd_net, Net** vcc_net)
        {
            if (m_gl == nullptr)
            {
                return nullptr;
            }

            auto nl = netlist_factory::create_netlist(m_gl);
            if (nl == nullptr)
            {
                return nullptr;
            }

            Gate* gnd_gate = nl->create_gate(m_gl->get_gate_type_by_name("GND"), "gnd");
            nl->mark_gnd_gate(gnd_gate);
            *gnd_net = nl->create_net("gnd_net");
            (*gnd_net)->add_source(gnd_gate, "G");

            Gate* vcc_gate = nl->create_gate(m_gl->get_gate_type_by_name("VCC"), "vcc");
            nl->mark_vcc_gate(vcc_gate);
            *vcc_net = nl->create_net("vcc_net");
            (*vcc_net)->add_source(vcc_gate, "P");

            return nl;
        }
    };

    /**
     * Test that the gate scope restricts which 'LUT6_2' gates are split, and that a 'LUT6_2' with an unconnected
     * output is handled gracefully.
     *
     * Functions: split_luts
     */
    TEST_F(XilinxToolboxTest, check_split_luts_scoped)
    {
        TEST_START
        {
            Net *gnd_net = nullptr, *vcc_net = nullptr;
            std::unique_ptr<Netlist> nl = create_netlist(&gnd_net, &vcc_net);
            ASSERT_NE(nl, nullptr);

            GateType* lut6_2 = m_gl->get_gate_type_by_name("LUT6_2");
            ASSERT_NE(lut6_2, nullptr);

            std::vector<Gate*> luts;
            for (const std::string& name : {"l0", "l1"})
            {
                Gate* l = nl->create_gate(lut6_2, name);
                ASSERT_TRUE(l->set_init_data({"ABCDEF0123456789"}).is_ok());

                for (u32 i = 0; i < 6; i++)
                {
                    Net* n = nl->create_net(name + "_i" + std::to_string(i));
                    n->add_destination(l, "I" + std::to_string(i));
                    n->mark_global_input_net();
                }

                for (const std::string& pin : {"O5", "O6"})
                {
                    Net* n = nl->create_net(name + "_" + pin);
                    n->add_source(l, pin);
                    n->mark_global_output_net();
                    // the outputs have to be used, otherwise there is nothing to split off
                    n->add_destination(nl->create_gate(m_gl->get_gate_type_by_name("INV"), name + "_" + pin + "_sink"), "I");
                }

                luts.push_back(l);
            }

            // only the LUT within the scope is split into a 'LUT6' and a 'LUT5'
            auto res = xilinx_toolbox::split_luts(nl.get(), {luts.at(0)});
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get(), 1);

            EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "LUT6_2"; }).size(), 1);
            EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "LUT6"; }).size(), 1);
            EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "LUT5"; }).size(), 1);

            // without a scope the remaining LUT is split as well
            res = xilinx_toolbox::split_luts(nl.get());
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get(), 1);

            EXPECT_TRUE(nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "LUT6_2"; }).empty());
            EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "LUT6"; }).size(), 2);
            EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "LUT5"; }).size(), 2);
        }
        {
            // a 'LUT6_2' that only uses 'O6' is split into a single 'LUT6' instead of crashing
            Net *gnd_net = nullptr, *vcc_net = nullptr;
            std::unique_ptr<Netlist> nl = create_netlist(&gnd_net, &vcc_net);
            ASSERT_NE(nl, nullptr);

            Gate* l = nl->create_gate(m_gl->get_gate_type_by_name("LUT6_2"), "l0");
            ASSERT_TRUE(l->set_init_data({"ABCDEF0123456789"}).is_ok());

            for (u32 i = 0; i < 6; i++)
            {
                Net* n = nl->create_net("i" + std::to_string(i));
                n->add_destination(l, "I" + std::to_string(i));
                n->mark_global_input_net();
            }

            Net* o6 = nl->create_net("o6");
            o6->add_source(l, "O6");
            o6->add_destination(nl->create_gate(m_gl->get_gate_type_by_name("INV"), "sink"), "I");

            auto res = xilinx_toolbox::split_luts(nl.get());
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get(), 1);

            EXPECT_TRUE(nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "LUT6_2"; }).empty());
            EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "LUT6"; }).size(), 1);
            EXPECT_TRUE(nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "LUT5"; }).empty());
        }
        TEST_END
    }

    /**
     * Test that the gate scope restricts which shift registers are split, and that the created flip-flops are assigned
     * to the module of the gate they replace.
     *
     * Functions: split_shift_registers
     */
    TEST_F(XilinxToolboxTest, check_split_shift_registers_scoped)
    {
        // creates an 'SRL16E' gate with all control pins tied to GND, i.e. a shift register of length one
        auto create_srl = [](Netlist* nl, GateLibrary* gl, const std::string& name, Net* gnd_net, Net* vcc_net) -> Gate* {
            Gate* srl = nl->create_gate(gl->get_gate_type_by_name("SRL16E"), name);

            for (u32 i = 0; i < 4; i++)
            {
                gnd_net->add_destination(srl, "A" + std::to_string(i));
            }
            vcc_net->add_destination(srl, "CE");

            Net* clk = nl->create_net(name + "_clk");
            clk->add_destination(srl, "CLK");
            clk->mark_global_input_net();

            Net* d = nl->create_net(name + "_d");
            d->add_destination(srl, "D");
            d->mark_global_input_net();

            Net* q = nl->create_net(name + "_q");
            q->add_source(srl, "Q");
            q->add_destination(nl->create_gate(gl->get_gate_type_by_name("INV"), name + "_sink"), "I");

            return srl;
        };

        TEST_START
        {
            Net *gnd_net = nullptr, *vcc_net = nullptr;
            std::unique_ptr<Netlist> nl = create_netlist(&gnd_net, &vcc_net);
            ASSERT_NE(nl, nullptr);

            Gate* s0 = create_srl(nl.get(), m_gl, "s0", gnd_net, vcc_net);
            Gate* s1 = create_srl(nl.get(), m_gl, "s1", gnd_net, vcc_net);

            // only the shift register within the scope is replaced by flip-flops
            auto res = xilinx_toolbox::split_shift_registers(nl.get(), {s0});
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get(), 1);

            EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "SRL16E"; }).size(), 1);
            EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "FDE"; }).size(), 1);

            // without a scope the remaining shift register is replaced as well
            res = xilinx_toolbox::split_shift_registers(nl.get());
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get(), 1);

            EXPECT_TRUE(nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "SRL16E"; }).empty());
            EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "FDE"; }).size(), 2);
        }
        {
            // the created flip-flops belong to the module of the shift register, not to the top module
            Net *gnd_net = nullptr, *vcc_net = nullptr;
            std::unique_ptr<Netlist> nl = create_netlist(&gnd_net, &vcc_net);
            ASSERT_NE(nl, nullptr);

            Gate* s0 = create_srl(nl.get(), m_gl, "s0", gnd_net, vcc_net);

            Module* mod = nl->create_module("mod", nl->get_top_module(), {s0});
            ASSERT_NE(mod, nullptr);

            auto res = xilinx_toolbox::split_shift_registers(nl.get(), {s0});
            ASSERT_TRUE(res.is_ok());
            EXPECT_EQ(res.get(), 1);

            auto ffs = nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "FDE"; });
            ASSERT_EQ(ffs.size(), 1);
            EXPECT_EQ(ffs.front()->get_module(), mod);
        }
        TEST_END
    }

    /**
     * Test the context menu entries contributed to the GUI.
     *
     * Functions: GuiExtensionXilinxToolbox::get_context_contribution, GuiExtensionXilinxToolbox::execute_function
     */
    TEST_F(XilinxToolboxTest, check_gui_extension)
    {
        TEST_START
        {
            XilinxToolboxPlugin plugin;

            GuiExtensionXilinxToolbox* gui = nullptr;
            for (auto* ext : plugin.get_extensions())
            {
                if (auto* casted = dynamic_cast<GuiExtensionXilinxToolbox*>(ext); casted != nullptr)
                {
                    gui = casted;
                }
            }
            ASSERT_NE(gui, nullptr);

            Net *gnd_net = nullptr, *vcc_net = nullptr;
            std::unique_ptr<Netlist> nl = create_netlist(&gnd_net, &vcc_net);
            ASSERT_NE(nl, nullptr);

            std::vector<Gate*> luts;
            for (const std::string& name : {"l0", "l1"})
            {
                Gate* l = nl->create_gate(m_gl->get_gate_type_by_name("LUT6_2"), name);
                ASSERT_TRUE(l->set_init_data({"ABCDEF0123456789"}).is_ok());

                for (u32 i = 0; i < 6; i++)
                {
                    Net* n = nl->create_net(name + "_i" + std::to_string(i));
                    n->add_destination(l, "I" + std::to_string(i));
                    n->mark_global_input_net();
                }

                Net* o6 = nl->create_net(name + "_o6");
                o6->add_source(l, "O6");
                o6->add_destination(nl->create_gate(m_gl->get_gate_type_by_name("INV"), name + "_sink"), "I");

                luts.push_back(l);
            }

            // without a selection the netlist-wide entries are offered
            auto without_selection = gui->get_context_contribution(nl.get(), {}, {}, {});
            ASSERT_EQ(without_selection.size(), 2);
            for (const auto& cmc : without_selection)
            {
                EXPECT_NE(cmc.mTagname.find("_netlist"), std::string::npos);
            }

            // with a selection only the entries operating on it are offered
            auto with_selection = gui->get_context_contribution(nl.get(), {}, {luts.at(0)->get_id()}, {});
            ASSERT_EQ(with_selection.size(), 2);
            for (const auto& cmc : with_selection)
            {
                EXPECT_EQ(cmc.mContributer, gui);
                EXPECT_FALSE(cmc.mEntry.empty());
                EXPECT_NE(cmc.mTagname.find("_selection"), std::string::npos);
            }

            // running the entry on the selected gate splits only that LUT
            gui->execute_function("split_luts_selection", nl.get(), {}, {luts.at(0)->get_id()}, {});
            EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "LUT6_2"; }).size(), 1);
            EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "LUT6"; }).size(), 1);

            // the netlist-wide entry then splits the remaining one
            gui->execute_function("split_luts_netlist", nl.get(), {}, {}, {});
            EXPECT_TRUE(nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "LUT6_2"; }).empty());
            EXPECT_EQ(nl->get_gates([](const Gate* g) { return g->get_type()->get_name() == "LUT6"; }).size(), 2);
        }
        TEST_END
    }
}    // namespace hal
