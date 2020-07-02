#include "gtest/gtest.h"

#include "graph/graph.h"
#include "graph/graph_factory.h"

#include "hdl_parser/hdl_parser_vhdl.h"

class PareserTest : public ::testing::Test
{
protected:

    virtual graph *get_graph(std::stringstream &stream)
    {
        graph* g_obj = graph_factory::create_graph("XILINX_ISE_SIMPRIM");
        HDLParserVHDL parser;
        if (parser.parse(g_obj, std::move(stream)) != SUCCESS) {
            delete g_obj;
            return nullptr;
        }
        return g_obj;
    }
};

TEST_F(PareserTest, check_simple_parse)
{
    std::stringstream ss("-- Device\t: 6slx16csg324-3 (PRODUCTION 1.23 2013-10-13)\n"
                             "entity top is\n"
                             "  port ();\n"
                             "end top;\n"
                             "\n"
                             "architecture STRUCTURE of top is\n"
                             "  signal fsm_current_s_FSM_FFd2_37 : STD_LOGIC; \n"
                             "  signal input_IBUF_0 : STD_LOGIC; \n"
                             "  signal fsm_current_s_FSM_FFd2_In : STD_LOGIC;\n"
                             "  signal fsm_current_s_FSM_FFd1_36 : STD_LOGIC;\n"
                             "  signal output_OBUF_25 : STD_LOGIC;\n"
                             "begin\n"
                             "  fsm_current_s_FSM_FFd2_In1 : X_LUT6\n"
                             "    generic map(\n"
                             "      LOC => \"SLICE_X14Y61\",\n"
                             "      INIT => X\"AAAA5555AAAA5555\"\n"
                             "    )\n"
                             "    port map (\n"
                             "      ADR3 => '1',\n"
                             "      ADR1 => '1',\n"
                             "      ADR2 => '1',\n"
                             "      ADR4 => fsm_current_s_FSM_FFd2_37,\n"
                             "      ADR0 => input_IBUF_0,\n"
                             "      ADR5 => '1',\n"
                             "      O => fsm_current_s_FSM_FFd2_In\n"
                             "    );\n"
                             "  fsm_Mmux_output11 : X_LUT5\n"
                             "    generic map(\n"
                             "      LOC => \"SLICE_X14Y61\",\n"
                             "      INIT => X\"CCCC6666\"\n"
                             "    )\n"
                             "    port map (\n"
                             "      ADR3 => '1',\n"
                             "      ADR2 => '1',\n"
                             "      ADR1 => fsm_current_s_FSM_FFd1_36,\n"
                             "      ADR4 => fsm_current_s_FSM_FFd2_37,\n"
                             "      ADR0 => input_IBUF_0,\n"
                             "      O => output_OBUF_25\n"
                             "    );\n"
                             "end STRUCTURE;");
    graph * g_obj = this->get_graph(ss);
    EXPECT_EQ(g_obj->get_nodes().size(), (u32) 4);
}
