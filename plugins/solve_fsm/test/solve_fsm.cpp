#include "solve_fsm/solve_fsm.h"

#include "hal_core/netlist/gate.h"

#include "hal_core/netlist/boolean_function/solver.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/net.h"
#include "hal_core/netlist/netlist.h"
#include "netlist_test_utils.h"

namespace hal
{
    class SolveFsmTest : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            NO_COUT_BLOCK;
            test_utils::init_log_channels();
            test_utils::create_sandbox_directory();
        }

        virtual void TearDown()
        {
            test_utils::remove_sandbox_directory();
        }

        /**
         * The solver is an external dependency that is not present in every environment, so the tests below are
         * skipped rather than failed when it cannot be found.
         */
        static bool solver_available()
        {
            return SMT::Solver::has_local_solver_for(SMT::SolverType::Z3, SMT::SolverCall::Binary);
        }

        /**
         * A two bit state register clocked by a global input, with 'ff0' holding the least significant bit.
         */
        struct StateRegister
        {
            Gate* ff0;
            Gate* ff1;
            Net* q0;
            Net* q1;

            std::vector<Gate*> gates() const
            {
                return {ff0, ff1};
            }
        };

        StateRegister create_state_register(Netlist* nl)
        {
            const GateLibrary* gl = nl->get_gate_library();

            StateRegister sr;
            sr.ff0 = nl->create_gate(gl->get_gate_type_by_name("DFF"), "ff0");
            sr.ff1 = nl->create_gate(gl->get_gate_type_by_name("DFF"), "ff1");

            Net* clk = nl->create_net("clk");
            clk->add_destination(sr.ff0, "CLK");
            clk->add_destination(sr.ff1, "CLK");
            clk->mark_global_input_net();

            sr.q0 = nl->create_net("q0");
            sr.q0->add_source(sr.ff0, "Q");
            sr.q1 = nl->create_net("q1");
            sr.q1->add_source(sr.ff1, "Q");

            return sr;
        }

        /**
         * Transition logic of a two bit counter, i.e. 0 -> 1 -> 2 -> 3 -> 0.
         */
        std::vector<Gate*> create_counter_logic(Netlist* nl, const StateRegister& sr)
        {
            const GateLibrary* gl = nl->get_gate_library();

            // next bit 0 is the inverse of bit 0
            Gate* inv = nl->create_gate(gl->get_gate_type_by_name("INV"), "inv_d0");
            sr.q0->add_destination(inv, "I");
            Net* d0 = nl->create_net("d0");
            d0->add_source(inv, "O");
            d0->add_destination(sr.ff0, "D");

            // next bit 1 flips whenever bit 0 is set
            Gate* xor2 = nl->create_gate(gl->get_gate_type_by_name("XOR2"), "xor_d1");
            sr.q0->add_destination(xor2, "I0");
            sr.q1->add_destination(xor2, "I1");
            Net* d1 = nl->create_net("d1");
            d1->add_source(xor2, "O");
            d1->add_destination(sr.ff1, "D");

            return {inv, xor2};
        }

        /**
         * Transition logic of a saturating counter, i.e. 0 -> 1 -> 2 -> 3 -> 3. Unlike the wrapping counter above,
         * which state the FSM starts in decides which states are reachable at all.
         */
        std::vector<Gate*> create_saturating_counter_logic(Netlist* nl, const StateRegister& sr)
        {
            const GateLibrary* gl = nl->get_gate_library();

            // next bit 0 is '!q0 | q1'
            Gate* inv = nl->create_gate(gl->get_gate_type_by_name("INV"), "inv_q0");
            sr.q0->add_destination(inv, "I");
            Net* not_q0 = nl->create_net("not_q0");
            not_q0->add_source(inv, "O");

            Gate* or_d0 = nl->create_gate(gl->get_gate_type_by_name("OR2"), "or_d0");
            not_q0->add_destination(or_d0, "I0");
            sr.q1->add_destination(or_d0, "I1");
            Net* d0 = nl->create_net("d0");
            d0->add_source(or_d0, "O");
            d0->add_destination(sr.ff0, "D");

            // next bit 1 is 'q0 | q1'
            Gate* or_d1 = nl->create_gate(gl->get_gate_type_by_name("OR2"), "or_d1");
            sr.q0->add_destination(or_d1, "I0");
            sr.q1->add_destination(or_d1, "I1");
            Net* d1 = nl->create_net("d1");
            d1->add_source(or_d1, "O");
            d1->add_destination(sr.ff1, "D");

            return {inv, or_d0, or_d1};
        }

        /**
         * The constant value of an output, or an empty optional if the output still depends on the inputs of the FSM.
         */
        static std::optional<u64> constant_value(const std::vector<std::pair<std::string, BooleanFunction>>& outputs, const std::string& name)
        {
            for (const auto& [output_name, bf] : outputs)
            {
                if (output_name != name)
                {
                    continue;
                }
                if (!bf.is_constant())
                {
                    return std::nullopt;
                }
                if (auto res = bf.get_constant_value_u64(); res.is_ok())
                {
                    return res.get();
                }
            }
            return std::nullopt;
        }
    };

    /**
     * Test that the outputs of a Moore FSM evaluate to a constant in every state, both for an output driven by
     * combinational logic and for one that is driven by a flip-flop directly.
     *
     * Functions: solve_fsm
     */
    TEST_F(SolveFsmTest, check_moore_outputs)
    {
        TEST_START
        if (!solver_available())
        {
            GTEST_SKIP() << "no local SMT solver available";
        }
        {
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);
            const GateLibrary* gl = nl->get_gate_library();

            const StateRegister sr = create_state_register(nl.get());
            const auto transition_logic = create_counter_logic(nl.get(), sr);

            // one output computed from both state bits, one taken straight off a flip-flop
            Gate* out_xor = nl->create_gate(gl->get_gate_type_by_name("XOR2"), "out_xor");
            sr.q0->add_destination(out_xor, "I0");
            sr.q1->add_destination(out_xor, "I1");
            Net* out_b = nl->create_net("out_b");
            out_b->add_source(out_xor, "O");
            out_b->mark_global_output_net();

            const auto config = solve_fsm::Configuration(nl.get()).with_state_register(sr.gates()).with_transition_logic(transition_logic).with_outputs({{"OUT_A", {sr.q0}}, {"OUT_B", {out_b}}});
            const auto res    = solve_fsm::solve_fsm(config);
            ASSERT_TRUE(res.is_ok());
            const auto graph = res.get();

            // the counter walks through all four states
            ASSERT_EQ(graph.transitions.size(), 4);
            for (u64 state = 0; state < 4; state++)
                {
                ASSERT_EQ(graph.transitions.at(state).size(), 1);
                EXPECT_EQ(graph.transitions.at(state).begin()->first, (state + 1) % 4);
            }

            // 'OUT_A' is state bit 0, 'OUT_B' is the XOR of both state bits
            ASSERT_EQ(graph.outputs.size(), 4);
            for (u64 state = 0; state < 4; state++)
            {
                const auto& outputs = graph.outputs.at(state);
                ASSERT_EQ(outputs.size(), 2);

                // the outputs keep the order in which they were passed in
                EXPECT_EQ(outputs.at(0).first, "OUT_A");
                EXPECT_EQ(outputs.at(1).first, "OUT_B");

                EXPECT_EQ(constant_value(outputs, "OUT_A"), std::optional<u64>(state & 0x1));
                EXPECT_EQ(constant_value(outputs, "OUT_B"), std::optional<u64>((state & 0x1) ^ ((state >> 1) & 0x1)));
            }
        }
        TEST_END
    }

    /**
     * Test that an output of a Mealy FSM keeps the input it depends on instead of collapsing to a constant.
     *
     * Functions: solve_fsm
     */
    TEST_F(SolveFsmTest, check_mealy_outputs)
    {
        TEST_START
        if (!solver_available())
        {
            GTEST_SKIP() << "no local SMT solver available";
        }
        {
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);
            const GateLibrary* gl = nl->get_gate_library();

            const StateRegister sr = create_state_register(nl.get());
            const auto transition_logic = create_counter_logic(nl.get(), sr);

            // the output depends on an input of the FSM as well as on the state
            Net* in0 = nl->create_net("in0");
            in0->mark_global_input_net();

            Gate* out_xor = nl->create_gate(gl->get_gate_type_by_name("XOR2"), "out_xor");
            sr.q0->add_destination(out_xor, "I0");
            in0->add_destination(out_xor, "I1");
            Net* out = nl->create_net("out");
            out->add_source(out_xor, "O");
            out->mark_global_output_net();

            const auto config = solve_fsm::Configuration(nl.get()).with_state_register(sr.gates()).with_transition_logic(transition_logic).with_outputs({{"OUT", {out}}});
            const auto res    = solve_fsm::solve_fsm(config);
            ASSERT_TRUE(res.is_ok());
            const auto graph = res.get();

            ASSERT_EQ(graph.outputs.size(), 4);
            for (u64 state = 0; state < 4; state++)
            {
                const auto& outputs = graph.outputs.at(state);
                ASSERT_EQ(outputs.size(), 1);
                const auto& bf = outputs.at(0).second;

                // the state alone does not determine the output, so the input variable has to survive
                EXPECT_FALSE(bf.is_constant());
                ASSERT_EQ(bf.get_variable_names().size(), 1);

                // 'OUT' is 'in0' XOR state bit 0, so evaluating the input reproduces both possible outputs
                const std::string var = *(bf.get_variable_names().begin());
                for (const auto in_val : {BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE})
                {
                    const std::unordered_map<std::string, BooleanFunction::Value> inputs = {{var, in_val}};
                    const auto eval_res = bf.evaluate(inputs);
                    ASSERT_TRUE(eval_res.is_ok());

                    const auto expected = ((in_val == BooleanFunction::Value::ONE) != ((state & 0x1) == 1)) ? BooleanFunction::Value::ONE : BooleanFunction::Value::ZERO;
                    EXPECT_EQ(eval_res.get(), expected);
                }
            }
        }
        TEST_END
    }

    /**
     * Test that a multi-bit output is reported as a single value, with the first net providing the least significant bit.
     *
     * Functions: solve_fsm
     */
    TEST_F(SolveFsmTest, check_multi_bit_output)
    {
        TEST_START
        if (!solver_available())
        {
            GTEST_SKIP() << "no local SMT solver available";
        }
        {
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);

            const StateRegister sr = create_state_register(nl.get());
            const auto transition_logic = create_counter_logic(nl.get(), sr);

            // the FSM publishes its state register, so the value of the output must equal the state itself
            const auto config = solve_fsm::Configuration(nl.get()).with_state_register(sr.gates()).with_transition_logic(transition_logic).with_outputs({{"OUT", {sr.q0, sr.q1}}});
            const auto res    = solve_fsm::solve_fsm(config);
            ASSERT_TRUE(res.is_ok());
            const auto graph = res.get();

            ASSERT_EQ(graph.outputs.size(), 4);
            for (u64 state = 0; state < 4; state++)
            {
                const auto& outputs = graph.outputs.at(state);
                ASSERT_EQ(outputs.size(), 1);
                EXPECT_EQ(outputs.at(0).second.size(), 2);
                EXPECT_EQ(constant_value(outputs, "OUT"), std::optional<u64>(state));
            }
        }
        {
            // reversing the nets of the output reverses its bits
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);

            const StateRegister sr = create_state_register(nl.get());
            const auto transition_logic = create_counter_logic(nl.get(), sr);

            const auto config = solve_fsm::Configuration(nl.get()).with_state_register(sr.gates()).with_transition_logic(transition_logic).with_outputs({{"OUT", {sr.q1, sr.q0}}});
            const auto res    = solve_fsm::solve_fsm(config);
            ASSERT_TRUE(res.is_ok());
            const auto graph = res.get();

            for (u64 state = 0; state < 4; state++)
            {
                const u64 reversed = ((state & 0x1) << 1) | ((state >> 1) & 0x1);
                EXPECT_EQ(constant_value(graph.outputs.at(state), "OUT"), std::optional<u64>(reversed));
            }
        }
        TEST_END
    }

    /**
     * Test that a non-zero initial state is interpreted with the first flip-flop of the state register providing the
     * least significant bit, and that the exploration starts from that state.
     *
     * Functions: solve_fsm
     */
    TEST_F(SolveFsmTest, check_initial_state)
    {
        TEST_START
        if (!solver_available())
        {
            GTEST_SKIP() << "no local SMT solver available";
        }
        {
            // the saturating counter never returns to a lower state, so the set of reachable states pins down which
            // state the solver actually started from
            const std::vector<std::pair<std::map<u32, bool>, std::set<u64>>> cases = {
                {{{0, false}, {1, false}}, {0, 1, 2, 3}},    // state 0
                {{{0, true}, {1, false}}, {1, 2, 3}},        // state 1, only the first flip-flop is set
                {{{0, false}, {1, true}}, {2, 3}},           // state 2, only the second flip-flop is set
                {{{0, true}, {1, true}}, {3}},               // state 3
            };

            for (const auto& [initial_bits, expected_states] : cases)
            {
                std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
                ASSERT_NE(nl, nullptr);

                const StateRegister sr = create_state_register(nl.get());
                const auto transition_logic = create_saturating_counter_logic(nl.get(), sr);

                const std::map<Gate*, bool> initial_state = {{sr.ff0, initial_bits.at(0)}, {sr.ff1, initial_bits.at(1)}};

                const auto config = solve_fsm::Configuration(nl.get())
                                        .with_state_register(sr.gates())
                                        .with_transition_logic(transition_logic)
                                        .with_outputs({{"OUT", {sr.q0, sr.q1}}})
                                        .with_initial_state(initial_state);
                const auto res = solve_fsm::solve_fsm(config);
                ASSERT_TRUE(res.is_ok());
                const auto graph = res.get();

                std::set<u64> reached;
                for (const auto& [state, successors] : graph.transitions)
                {
                    reached.insert(state);
                    for (const auto& [successor, _] : successors)
                    {
                        reached.insert(successor);
                    }
                }

                EXPECT_EQ(reached, expected_states);
            }
        }
        TEST_END
    }

    /**
     * Test that brute forcing and SMT solving produce the same state transition graph, including for a non-zero
     * initial state, where brute forcing has to discard the states that the FSM can never enter.
     *
     * Functions: solve_fsm
     */
    TEST_F(SolveFsmTest, check_brute_force)
    {
        TEST_START
        if (!solver_available())
        {
            GTEST_SKIP() << "no local SMT solver available";
        }
        {
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);

            const StateRegister sr = create_state_register(nl.get());
            const auto transition_logic = create_saturating_counter_logic(nl.get(), sr);

            // starting in state 1 leaves state 0 unreachable, which brute forcing has to notice
            const std::map<Gate*, bool> initial_state = {{sr.ff0, true}, {sr.ff1, false}};

            const auto base_config = solve_fsm::Configuration(nl.get())
                                         .with_state_register(sr.gates())
                                         .with_transition_logic(transition_logic)
                                         .with_outputs({{"OUT", {sr.q0, sr.q1}}})
                                         .with_initial_state(initial_state);

            auto smt_config = base_config;
            const auto smt_res = solve_fsm::solve_fsm(smt_config);
            ASSERT_TRUE(smt_res.is_ok());
            const auto smt = smt_res.get();

            auto brute_config = base_config;
            brute_config.with_brute_force();
            const auto brute_res = solve_fsm::solve_fsm(brute_config);
            ASSERT_TRUE(brute_res.is_ok());
            const auto brute = brute_res.get();

            EXPECT_EQ(smt.get_state_size(), brute.get_state_size());

            // the same states are reached
            ASSERT_EQ(smt.transitions.size(), brute.transitions.size());
            EXPECT_EQ(smt.transitions.size(), 3);

            for (const auto& [state, successors] : smt.transitions)
            {
                ASSERT_NE(brute.transitions.find(state), brute.transitions.end());
                ASSERT_EQ(successors.size(), brute.transitions.at(state).size());
                for (const auto& [successor, _] : successors)
                {
                    EXPECT_NE(brute.transitions.at(state).find(successor), brute.transitions.at(state).end());
                }

                // and report the same outputs
                EXPECT_EQ(constant_value(smt.outputs.at(state), "OUT"), constant_value(brute.outputs.at(state), "OUT"));
            }
        }
        TEST_END
    }

    /**
     * Test that the solver rejects a configuration that does not describe an FSM.
     *
     * Functions: solve_fsm
     */
    TEST_F(SolveFsmTest, check_invalid_configuration)
    {
        TEST_START
        {
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);

            const StateRegister sr = create_state_register(nl.get());
            const auto transition_logic = create_counter_logic(nl.get(), sr);

            EXPECT_TRUE(solve_fsm::solve_fsm(solve_fsm::Configuration(nullptr).with_state_register(sr.gates()).with_transition_logic(transition_logic)).is_error());
            EXPECT_TRUE(solve_fsm::solve_fsm(solve_fsm::Configuration(nl.get()).with_transition_logic(transition_logic)).is_error());
            EXPECT_TRUE(solve_fsm::solve_fsm(solve_fsm::Configuration(nl.get()).with_state_register(sr.gates())).is_error());

            // an initial state that does not cover the whole state register is rejected as well
            const auto incomplete = solve_fsm::Configuration(nl.get())
                                        .with_state_register(sr.gates())
                                        .with_transition_logic(transition_logic)
                                        .with_initial_state({{sr.ff0, true}});
            EXPECT_TRUE(solve_fsm::solve_fsm(incomplete).is_error());
        }
        TEST_END
    }

    /**
     * Test that the text representation contains the legend mapping the state and the outputs back to the netlist,
     * and the full condition of every transition.
     *
     * Functions: StateTransitionGraph::to_string
     */
    TEST_F(SolveFsmTest, check_to_string)
    {
        TEST_START
        if (!solver_available())
        {
            GTEST_SKIP() << "no local SMT solver available";
        }
        {
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);

            const StateRegister sr = create_state_register(nl.get());
            const auto transition_logic = create_counter_logic(nl.get(), sr);

            const auto config = solve_fsm::Configuration(nl.get()).with_state_register(sr.gates()).with_transition_logic(transition_logic).with_outputs({{"OUT", {sr.q0, sr.q1}}});
            const auto res    = solve_fsm::solve_fsm(config);
            ASSERT_TRUE(res.is_ok());
            const auto graph = res.get();

            const auto str_res = graph.to_string();
            ASSERT_TRUE(str_res.is_ok());
            const std::string str = str_res.get();

            // the legend maps every bit of the state and every output back to the netlist
            EXPECT_NE(str.find("FSM with 2 state bits and 4 reachable states"), std::string::npos);
            EXPECT_NE(str.find("bit 0: 'ff0' with ID " + std::to_string(sr.ff0->get_id())), std::string::npos);
            EXPECT_NE(str.find("bit 1: 'ff1' with ID " + std::to_string(sr.ff1->get_id())), std::string::npos);
            EXPECT_NE(str.find("OUT: 'q0' with ID " + std::to_string(sr.q0->get_id()) + ", 'q1' with ID " + std::to_string(sr.q1->get_id())), std::string::npos);

            // every state carries its outputs and its transitions
            for (u64 state = 0; state < 4; state++)
            {
                EXPECT_NE(str.find("state " + std::to_string(state) + "\n"), std::string::npos);
                EXPECT_NE(str.find("OUT = " + std::to_string(state)), std::string::npos);
                EXPECT_NE(str.find("to " + std::to_string((state + 1) % 4) + " if "), std::string::npos);
            }

            // this FSM has no inputs at all, so no Boolean function holds a variable and there is nothing to resolve
            EXPECT_EQ(str.find("nets referenced in the Boolean functions below:"), std::string::npos);

            // nothing is truncated, so the conditions appear in full
            for (const auto& [state, successors] : graph.transitions)
            {
                for (const auto& [successor, condition] : successors)
                {
                    EXPECT_NE(str.find(condition.to_string()), std::string::npos);
                }
            }

            // binary state labels are zero-padded to the size of the state register
            const auto bin_res = graph.to_string(2);
            ASSERT_TRUE(bin_res.is_ok());
            EXPECT_NE(bin_res.get().find("state 10"), std::string::npos);

            EXPECT_TRUE(graph.to_string(16).is_error());
        }
        {
            // the variables inside the Boolean functions are derived from net IDs and match no name in the netlist,
            // so an FSM that actually depends on an input needs the legend to resolve them
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);
            const GateLibrary* gl = nl->get_gate_library();

            const StateRegister sr = create_state_register(nl.get());
            const auto transition_logic = create_counter_logic(nl.get(), sr);

            Net* in0 = nl->create_net("some_input_net");
            in0->mark_global_input_net();

            Gate* out_xor = nl->create_gate(gl->get_gate_type_by_name("XOR2"), "out_xor");
            sr.q0->add_destination(out_xor, "I0");
            in0->add_destination(out_xor, "I1");
            Net* out = nl->create_net("out");
            out->add_source(out_xor, "O");
            out->mark_global_output_net();

            const auto config = solve_fsm::Configuration(nl.get()).with_state_register(sr.gates()).with_transition_logic(transition_logic).with_outputs({{"OUT", {out}}});
            const auto res    = solve_fsm::solve_fsm(config);
            ASSERT_TRUE(res.is_ok());

            const auto str_res = res.get().to_string();
            ASSERT_TRUE(str_res.is_ok());
            const std::string str = str_res.get();

            const std::string variable = "net_" + std::to_string(in0->get_id());
            EXPECT_NE(str.find("nets referenced in the Boolean functions below:"), std::string::npos);
            EXPECT_NE(str.find("  " + variable + ": 'some_input_net' with ID " + std::to_string(in0->get_id())), std::string::npos);

            // the variable really is the net ID rather than the net name, which is what makes the legend necessary
            EXPECT_NE(str.find(variable), std::string::npos);
            EXPECT_EQ(nl->get_nets([&variable](const Net* n) { return n->get_name() == variable; }).size(), 0);
        }
        TEST_END
    }

    /**
     * Test that the DOT graph carries a node for every state, annotated with the outputs of that state.
     *
     * Functions: StateTransitionGraph::generate_dot_graph
     */
    TEST_F(SolveFsmTest, check_generate_dot_graph)
    {
        TEST_START
        if (!solver_available())
        {
            GTEST_SKIP() << "no local SMT solver available";
        }
        {
            std::unique_ptr<Netlist> nl = test_utils::create_empty_netlist();
            ASSERT_NE(nl, nullptr);

            const StateRegister sr = create_state_register(nl.get());
            const auto transition_logic = create_counter_logic(nl.get(), sr);

            const auto config = solve_fsm::Configuration(nl.get()).with_state_register(sr.gates()).with_transition_logic(transition_logic).with_outputs({{"OUT", {sr.q0, sr.q1}}});
            const auto res    = solve_fsm::solve_fsm(config);
            ASSERT_TRUE(res.is_ok());
            const auto graph = res.get();

            const auto dot_res = graph.generate_dot_graph();
            ASSERT_TRUE(dot_res.is_ok());
            const std::string dot = dot_res.get();

            // solving does not write a graph on its own, rendering it is a separate step
            for (u64 state = 0; state < 4; state++)
            {
                const std::string node = std::to_string(state) + " [label=\"" + std::to_string(state) + "\\nOUT = " + std::to_string(state) + "\"];";
                EXPECT_NE(dot.find(node), std::string::npos) << "missing node statement: " << node;
            }

            // without outputs the states carry no annotation
            const auto plain_config = solve_fsm::Configuration(nl.get()).with_state_register(sr.gates()).with_transition_logic(transition_logic);
            const auto plain_res    = solve_fsm::solve_fsm(plain_config);
            ASSERT_TRUE(plain_res.is_ok());

            const auto plain_dot_res = plain_res.get().generate_dot_graph();
            ASSERT_TRUE(plain_dot_res.is_ok());
            EXPECT_EQ(plain_dot_res.get().find("label=\"0\\nOUT"), std::string::npos);
        }
        TEST_END
    }
}    // namespace hal
