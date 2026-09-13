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
            ASSERT_EQ(without_selection.size(), 3);
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

            // with only nets selected, the entry removing no load wires of the selection is offered and acts on those nets only
            Gate* inv           = nl->create_gate(m_gl->get_gate_type_by_name("INV"), "inv");
            Net* selected_nlw   = nl->create_net("NLW_inv_O_UNCONNECTED");
            Net* unselected_nlw = nl->create_net("NLW_other_O_UNCONNECTED");
            selected_nlw->add_source(inv, "O");
            auto with_nets = gui->get_context_contribution(nl.get(), {}, {}, {selected_nlw->get_id()});
            ASSERT_EQ(with_nets.size(), 1);
            EXPECT_EQ(with_nets.front().mTagname, "remove_no_load_wires_selection");
            gui->execute_function("remove_no_load_wires_selection", nl.get(), {}, {}, {selected_nlw->get_id()});
            EXPECT_EQ(nl->get_net_by_id(selected_nlw->get_id()), nullptr);
            EXPECT_TRUE(inv->get_fan_out_nets().empty());
            EXPECT_EQ(nl->get_net_by_id(unselected_nlw->get_id()), unselected_nlw);

            gui->execute_function("remove_no_load_wires_netlist", nl.get(), {}, {}, {});
            EXPECT_EQ(nl->get_net_by_id(unselected_nlw->get_id()), nullptr);
        }
        TEST_END
    }

    /**
     * Testing the removal of the no load wires that Vivado writes for unused output pins.
     *
     * Functions: remove_no_load_wires
     */
    TEST_F(XilinxToolboxTest, check_remove_no_load_wires)
    {
        TEST_START
        {
            Net* gnd_net = nullptr;
            Net* vcc_net = nullptr;
            auto nl      = create_netlist(&gnd_net, &vcc_net);
            ASSERT_NE(nl, nullptr);

            auto* fd_type = m_gl->get_gate_type_by_name("FDRE");
            ASSERT_NE(fd_type, nullptr);

            // no load wires as Vivado writes them: for a single-bit pin, for a flattened instance path, with the path in
            // front of the prefix, and for one bit of a multi-bit pin in both index styles
            Gate* ff_a     = nl->create_gate(fd_type, "ff_a");
            Net* nlw_plain = nl->create_net("NLW_ff_a_Q_UNCONNECTED");
            nlw_plain->add_source(ff_a, "Q");
            Gate* ff_b    = nl->create_gate(fd_type, "sub/ff_b");
            Net* nlw_flat = nl->create_net("NLW_sub/ff_b_Q_UNCONNECTED");
            nlw_flat->add_source(ff_b, "Q");
            Gate* ff_g         = nl->create_gate(fd_type, "sub/ff_g");
            Net* nlw_flat_path = nl->create_net("sub/NLW_ff_g_Q_UNCONNECTED");
            nlw_flat_path->add_source(ff_g, "Q");
            Gate* carry     = nl->create_gate(m_gl->get_gate_type_by_name("CARRY4"), "carry");
            Net* nlw_bit    = nl->create_net("NLW_carry_CO_UNCONNECTED(2)");
            nlw_bit->add_source(carry, "CO(2)");
            Net* nlw_bit_sq = nl->create_net("NLW_carry_CO_UNCONNECTED[3]");
            nlw_bit_sq->add_source(carry, "CO(3)");

            // an ordinary net without destinations is not a no load wire, whatever its name looks like
            Gate* ff_c  = nl->create_gate(fd_type, "ff_c");
            Net* unread = nl->create_net("unread");
            unread->add_source(ff_c, "Q");
            Gate* ff_d       = nl->create_gate(fd_type, "ff_d");
            Net* nlw_in_path = nl->create_net("NLW_ff_d_Q_UNCONNECTED/inner");
            nlw_in_path->add_source(ff_d, "Q");
            Gate* ff_h        = nl->create_gate(fd_type, "ff_h");
            Net* nlw_in_token = nl->create_net("xNLW_ff_h_Q_UNCONNECTED");
            nlw_in_token->add_source(ff_h, "Q");

            // a no load wire by name that is actually read stays, as does one that leaves the netlist
            Gate* ff_e     = nl->create_gate(fd_type, "ff_e");
            Net* nlw_read  = nl->create_net("NLW_ff_e_Q_UNCONNECTED");
            nlw_read->add_source(ff_e, "Q");
            nlw_read->add_destination(nl->create_gate(m_gl->get_gate_type_by_name("INV"), "inv"), "I");
            Gate* ff_f      = nl->create_gate(fd_type, "ff_f");
            Net* nlw_output = nl->create_net("NLW_ff_f_Q_UNCONNECTED");
            nlw_output->add_source(ff_f, "Q");
            nl->mark_global_output_net(nlw_output);

            // a scope containing a net of another netlist is refused
            auto other = netlist_factory::create_netlist(m_gl);
            Net* foreign = other->create_net("NLW_x_O_UNCONNECTED");
            EXPECT_TRUE(xilinx_toolbox::remove_no_load_wires(nl.get(), {foreign}).is_error());
            EXPECT_TRUE(xilinx_toolbox::remove_no_load_wires(nullptr).is_error());

            // a scope only removes the no load wires within it
            auto scoped = xilinx_toolbox::remove_no_load_wires(nl.get(), {nlw_flat, unread});
            ASSERT_TRUE(scoped.is_ok());
            EXPECT_EQ(scoped.get(), 1);
            EXPECT_EQ(nl->get_net_by_id(nlw_flat->get_id()), nullptr);
            EXPECT_TRUE(ff_b->get_fan_out_nets().empty());
            EXPECT_EQ(nl->get_net_by_id(nlw_plain->get_id()), nlw_plain);

            const u32 nets_before = nl->get_nets().size();
            auto all              = xilinx_toolbox::remove_no_load_wires(nl.get());
            ASSERT_TRUE(all.is_ok());
            EXPECT_EQ(all.get(), 4);
            EXPECT_EQ(nl->get_nets().size(), nets_before - 4);
            EXPECT_EQ(nl->get_net_by_id(nlw_plain->get_id()), nullptr);
            EXPECT_EQ(nl->get_net_by_id(nlw_flat_path->get_id()), nullptr);
            EXPECT_EQ(nl->get_net_by_id(nlw_bit->get_id()), nullptr);
            EXPECT_EQ(nl->get_net_by_id(nlw_bit_sq->get_id()), nullptr);
            EXPECT_TRUE(ff_a->get_fan_out_nets().empty());
            EXPECT_TRUE(ff_g->get_fan_out_nets().empty());
            EXPECT_TRUE(carry->get_fan_out_nets().empty());
            EXPECT_EQ(ff_c->get_fan_out_net("Q"), unread);
            EXPECT_EQ(ff_d->get_fan_out_net("Q"), nlw_in_path);
            EXPECT_EQ(ff_h->get_fan_out_net("Q"), nlw_in_token);
            EXPECT_EQ(ff_e->get_fan_out_net("Q"), nlw_read);
            EXPECT_EQ(ff_f->get_fan_out_net("Q"), nlw_output);

            // running again finds nothing left to do
            auto again = xilinx_toolbox::remove_no_load_wires(nl.get());
            ASSERT_TRUE(again.is_ok());
            EXPECT_EQ(again.get(), 0);
        }
        TEST_END
    }
}    // namespace hal
