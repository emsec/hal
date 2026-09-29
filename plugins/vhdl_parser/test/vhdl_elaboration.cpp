#include "vhdl_parser/vhdl_elaboration.h"

#include "netlist_test_utils.h"
#include "vhdl_parser/vhdl_syntax.h"

#include "gtest/gtest.h"

namespace hal
{
    using namespace vhdl;
    using namespace netlist_ir;

    class VHDLElaborationTest : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            NO_COUT_BLOCK;
            test_utils::init_log_channels();
        }

        virtual void TearDown()
        {
        }

        Design elaborate_text(const std::string& text)
        {
            auto parsed = parse_string(text, "test.vhd");
            if (parsed.is_error())
            {
                ADD_FAILURE() << parsed.get_error().get();
                return {};
            }
            auto res = elaborate(parsed.get());
            if (res.is_error())
            {
                ADD_FAILURE() << res.get_error().get();
                return {};
            }
            return res.get();
        }

        static bool elaboration_fails(const std::string& text, const std::string& needle)
        {
            auto parsed = parse_string(text, "test.vhd");
            if (parsed.is_error())
            {
                ADD_FAILURE() << "did not even parse: " << parsed.get_error().get();
                return false;
            }
            auto res = elaborate(parsed.get());
            if (res.is_ok())
            {
                return false;
            }
            const std::string msg = res.get_error().get();
            if (msg.find(needle) == std::string::npos)
            {
                ADD_FAILURE() << "the error does not mention '" << needle << "': " << msg;
                return false;
            }
            return true;
        }

        static std::tuple<std::string, std::string> typed(const std::vector<TypedValue>& values, const std::string& name)
        {
            for (const TypedValue& v : values)
            {
                if (v.declaration.get_name() == name)
                {
                    return std::make_tuple(enum_to_string(v.declaration.get_type()), v.value);
                }
            }
            return std::make_tuple("", "");
        }

        static u16 typed_size(const std::vector<TypedValue>& values, const std::string& name)
        {
            for (const TypedValue& v : values)
            {
                if (v.declaration.get_name() == name)
                {
                    return v.declaration.get_size();
                }
            }
            return 0;
        }
    };

    /**
     * Integer and bit string literals.
     */
    TEST_F(VHDLElaborationTest, check_literals)
    {
        TEST_START
        EXPECT_EQ(parse_integer("12").get(), 12);
        EXPECT_EQ(parse_integer("1_000").get(), 1000);
        EXPECT_EQ(parse_integer("16#FF#").get(), 255);
        EXPECT_EQ(parse_integer("2#1010#E1").get(), 20);
        EXPECT_EQ(parse_integer("1e3").get(), 1000);
        EXPECT_EQ(parse_integer("1E+2").get(), 100);
        EXPECT_TRUE(parse_integer("1.5").is_error());
        EXPECT_TRUE(parse_integer("16#FG#").is_error());
        EXPECT_TRUE(parse_integer("1e-1").is_error());

        EXPECT_EQ(expand_bit_string("X\"AB\"").get(), "10101011");
        EXPECT_EQ(expand_bit_string("x\"a_b\"").get(), "10101011");
        EXPECT_EQ(expand_bit_string("B\"01_1\"").get(), "011");
        EXPECT_EQ(expand_bit_string("O\"17\"").get(), "001111");
        EXPECT_EQ(expand_bit_string("D\"10\"").get(), "1010");
        EXPECT_EQ(expand_bit_string("D\"2748\"").get(), "101010111100");
        EXPECT_EQ(expand_bit_string("D\"0\"").get(), "0");
        EXPECT_EQ(expand_bit_string("8X\"F\"").get(), "00001111");
        EXPECT_EQ(expand_bit_string("4X\"0F\"").get(), "1111");
        EXPECT_EQ(expand_bit_string("6SX\"F\"").get(), "111111");
        EXPECT_EQ(expand_bit_string("X\"Z-\"").get(), "zzzz----");
        EXPECT_EQ(expand_bit_string("B\"1X0\"").get(), "1x0");
        EXPECT_TRUE(expand_bit_string("3X\"F\"").is_error());
        EXPECT_TRUE(expand_bit_string("X\"G\"").is_error());
        TEST_END
    }

    /**
     * Ports, signals and types: scalar and vector standard types, the HAL pseudo types, user array types and
     * subtypes, generics in ranges, and declared spelling with case-insensitive lookup.
     */
    TEST_F(VHDLElaborationTest, check_ports_signals_types)
    {
        TEST_START
        {
            Design d = elaborate_text("package p is\n"
                                      "  type t_word is array (3 downto 0) of std_logic;\n"
                                      "  constant K : integer := 2;\n"
                                      "end p;\n"
                                      "entity Top is\n"
                                      "  generic ( N : integer := 4; INIT : std_logic_vector(3 downto 0) := X\"A\"; M : string := \"fast\" );\n"
                                      "  port ( a : in std_logic;\n"
                                      "         B : in STD_LOGIC_VECTOR(N - 1 downto 0);\n"
                                      "         c : out std_ulogic_vector(0 to K * 2);\n"
                                      "         q : inout bit;\n"
                                      "         w : out t_word );\n"
                                      "end Top;\n"
                                      "architecture rtl of top is\n"
                                      "  type t_mem is array (0 to 1) of std_logic_vector(1 downto 0);\n"
                                      "  type t_unc is array (natural range <>) of std_logic;\n"
                                      "  subtype t_nib is std_logic_vector(3 downto 0);\n"
                                      "  signal s1, S2 : std_logic;\n"
                                      "  signal v2 : std_logic_vector2(0 to 1, 1 downto 0);\n"
                                      "  signal mem : t_mem;\n"
                                      "  signal u : t_unc(7 downto 0);\n"
                                      "  signal nib : t_nib;\n"
                                      "  signal sig : signed(B'length + 1 downto 0);\n"
                                      "begin\n"
                                      "end rtl;\n");
            ASSERT_EQ(d.modules.size(), 1);
            const netlist_ir::Module& m = d.modules.front();
            EXPECT_EQ(m.name, "Top");
            ASSERT_EQ(m.ports.size(), 5);
            EXPECT_EQ(m.ports.at(0).name, "a");
            EXPECT_EQ(m.ports.at(0).direction, PinDirection::input);
            EXPECT_TRUE(m.ports.at(0).dims.empty());
            EXPECT_EQ(m.ports.at(1).name, "B");
            EXPECT_EQ(m.ports.at(1).dims, (std::vector<Range>{{3, 0}}));
            EXPECT_EQ(m.ports.at(2).direction, PinDirection::output);
            EXPECT_EQ(m.ports.at(2).dims, (std::vector<Range>{{0, 4}}));
            EXPECT_EQ(m.ports.at(3).direction, PinDirection::inout);
            EXPECT_EQ(m.ports.at(4).dims, (std::vector<Range>{{3, 0}}));

            ASSERT_EQ(m.signals.size(), 7);
            EXPECT_EQ(m.signals.at(0).name, "s1");
            EXPECT_EQ(m.signals.at(1).name, "S2");
            EXPECT_EQ(m.signals.at(2).dims, (std::vector<Range>{{0, 1}, {1, 0}}));
            EXPECT_EQ(m.signals.at(3).dims, (std::vector<Range>{{0, 1}, {1, 0}}));
            EXPECT_EQ(m.signals.at(4).dims, (std::vector<Range>{{7, 0}}));
            EXPECT_EQ(m.signals.at(5).dims, (std::vector<Range>{{3, 0}}));
            EXPECT_EQ(m.signals.at(6).name, "sig");
            EXPECT_EQ(m.signals.at(6).dims, (std::vector<Range>{{5, 0}}));

            EXPECT_EQ(typed(m.parameters, "N"), std::make_tuple("integer", "4"));
            EXPECT_EQ(typed(m.parameters, "INIT"), std::make_tuple("bit_vector", "0xA"));
            EXPECT_EQ(typed_size(m.parameters, "INIT"), 4);
            EXPECT_EQ(typed(m.parameters, "M"), std::make_tuple("string", "fast"));
        }
        EXPECT_TRUE(elaboration_fails("entity e is port ( a : in std_logic_vector ); end e; architecture a of e is begin end a;", "needs 1 index range"));
        EXPECT_TRUE(elaboration_fails("entity e is port ( a : in integer ); end e; architecture a of e is begin end a;", "not a bit type"));
        EXPECT_TRUE(elaboration_fails("entity e is port ( a : in t_nope ); end e; architecture a of e is begin end a;", "unknown type 't_nope'"));
        EXPECT_TRUE(elaboration_fails("entity e is port ( a : in std_logic ); end e; architecture a of e is signal A : std_logic; begin end a;", "shadows a port"));
        EXPECT_TRUE(elaboration_fails("entity e is port ( a : in std_logic_vector(0 downto 3) ); end e; architecture a of e is begin end a;", "null range"));
        EXPECT_TRUE(elaboration_fails("entity e is port ( a : in std_logic_vector(W - 1 downto 0) ); end e; architecture a of e is begin end a;", "'W' is not a generic"));
        EXPECT_TRUE(elaboration_fails("entity e is end e;", "no entity has an architecture"));
        EXPECT_TRUE(elaboration_fails("entity e is port ( a : in std_logic; b : out std_logic_vector(a'length downto 0) ); end e; architecture a of e is begin end a;", "no index range"));
        TEST_END
    }

    /**
     * Wiring: literals, indices, slices, concatenation, aggregates, constants, assignments, and open.
     */
    TEST_F(VHDLElaborationTest, check_wiring_and_aliases)
    {
        TEST_START
        {
            Design d = elaborate_text("entity top is\n"
                                      "  port ( a : in std_logic_vector(3 downto 0); b : in std_logic_vector(0 to 1); y : out std_logic_vector(7 downto 0); z : out std_logic; m : in std_logic_vector2(0 to 1, 1 downto 0) );\n"
                                      "end top;\n"
                                      "architecture rtl of top is\n"
                                      "  constant C : std_logic_vector(1 downto 0) := \"10\";\n"
                                      "  constant U : std_logic_vector := X\"F\";\n"
                                      "  signal s : std_logic_vector(3 downto 0);\n"
                                      "  signal t : std_logic;\n"
                                      "begin\n"
                                      "  s <= A(3 downto 2) & b(0) & '1';\n"
                                      "  y <= (s, \"0\", others => '0');\n"
                                      "  z <= C(1);\n"
                                      "  t <= m(1, 0);\n"
                                      "  g1 : BUF port map ( I => U(3), O => open );\n"
                                      "  g2 : BUF port map ( I => a(0), O => y(0) );\n"
                                      "end rtl;\n");
            ASSERT_EQ(d.modules.size(), 1);
            const netlist_ir::Module& m = d.modules.front();
            const Port& a   = m.ports.at(0);
            const Port& b   = m.ports.at(1);
            const Port& y   = m.ports.at(2);
            const Port& z   = m.ports.at(3);
            const Port& mm  = m.ports.at(4);
            const Signal* s = m.find_signal("s");
            const Signal* t = m.find_signal("t");
            ASSERT_NE(s, nullptr);
            ASSERT_NE(t, nullptr);

            // s(3) <= a(3), s(2) <= a(2), s(1) <= b(0), s(0) <= '1'
            std::vector<std::pair<BitId, BitId>> expected = {
                {s->bits.at(0), a.bits.at(0)}, {s->bits.at(1), a.bits.at(1)}, {s->bits.at(2), b.bits.at(0)}, {s->bits.at(3), ONE},
            };
            // y <= (s, "0", others => '0'): y(7..4) <= s, y(3) <= '0', y(2..0) <= '0'
            for (u32 i = 0; i < 4; i++)
            {
                expected.emplace_back(y.bits.at(i), s->bits.at(i));
            }
            for (u32 i = 4; i < 8; i++)
            {
                expected.emplace_back(y.bits.at(i), ZERO);
            }
            expected.emplace_back(z.bits.at(0), ONE);    // C(1) of "10" is '1'
            expected.emplace_back(t->bits.at(0), mm.bit_at({1, 0}).get());
            for (const auto& e : expected)
            {
                EXPECT_TRUE(std::find(m.aliases.begin(), m.aliases.end(), e) != m.aliases.end()) << e.first << " <= " << e.second;
            }
            ASSERT_EQ(m.instances.size(), 2);
            ASSERT_EQ(m.instances.at(0).connections.size(), 1);    // O => open makes no connection
            EXPECT_EQ(m.instances.at(0).connections.at(0).port, "I");
            EXPECT_EQ(m.instances.at(0).connections.at(0).bits, (std::vector<BitId>{ONE}));
            EXPECT_EQ(m.instances.at(1).connections.at(1).bits, (std::vector<BitId>{y.bits.at(7)}));
        }
        const std::string head = "entity top is port ( a : in std_logic; y : out std_logic_vector(1 downto 0) ); end top;\narchitecture rtl of top is\nbegin\n";
        EXPECT_TRUE(elaboration_fails(head + "  y <= a and a;\nend rtl;", "logic expression"));
        EXPECT_TRUE(elaboration_fails(head + "  y <= 3;\nend rtl;", "not a wiring value"));
        EXPECT_TRUE(elaboration_fails(head + "  y <= nope;\nend rtl;", "not a declared signal"));
        EXPECT_TRUE(elaboration_fails(head + "  y(5) <= a;\nend rtl;", "invalid index"));
        EXPECT_TRUE(elaboration_fails(head + "  y <= a & (others => '0');\nend rtl;", "known width"));
        TEST_END
    }

    /**
     * Instances: gates and modules by component, entity and configuration; positional and named maps with slices;
     * generics typed by declaration or by literal; attributes on signals, labels and the entity.
     */
    TEST_F(VHDLElaborationTest, check_instances_generics_attributes)
    {
        TEST_START
        {
            Design d = elaborate_text("library ieee;\n"
                                      "use ieee.std_logic_1164.all;\n"
                                      "package comps is\n"
                                      "  component AND2 port ( I0, I1 : in std_logic; O : out std_logic ); end component;\n"
                                      "  attribute KEEP : string;\n"
                                      "end comps;\n"
                                      "entity sub is\n"
                                      "  generic ( WIDTH : integer := 3; MODE : string := \"a\"; FLAG : boolean := false; DELAY : time := 1 ns );\n"
                                      "  port ( i : in std_logic_vector(WIDTH - 1 downto 0); o : out std_logic );\n"
                                      "end sub;\n"
                                      "architecture rtl of sub is\n"
                                      "begin\n"
                                      "  o <= i(0);\n"
                                      "end rtl;\n"
                                      "architecture other of sub is\n"
                                      "begin\n"
                                      "end other;\n"
                                      "entity top is\n"
                                      "  port ( a, b : in std_logic; y : out std_logic_vector(3 downto 0) );\n"
                                      "  attribute KEEP of top : entity is \"yes\";\n"
                                      "end top;\n"
                                      "architecture rtl of top is\n"
                                      "  component sub generic ( WIDTH : integer := 2 ); port ( i : in std_logic_vector(WIDTH - 1 downto 0); o : out std_logic ); end component;\n"
                                      "  component wrapper port ( x : in std_logic; y : out std_logic ); end component;\n"
                                      "  signal n : std_logic;\n"
                                      "  attribute KEEP of n : signal is \"true\";\n"
                                      "  attribute LOC of g1 : label is \"X0Y0\";\n"
                                      "  attribute DONT : string;\n"
                                      "  attribute DONT of g1 : label is 1;\n"
                                      "  attribute NUM of n : signal is 42;\n"
                                      "  attribute FLAGGY of g1 : label is true;\n"
                                      "begin\n"
                                      "  g1 : and2 port map ( i0 => a, I1 => b, o => n );\n"
                                      "  g2 : AND2 port map ( a, b, y(3) );\n"
                                      "  g3 : LUT2 generic map ( INIT => X\"E\", NEG => -1, R => 1.5, T => 10 ns, D => 16#FF#, B => '1' ) port map ( I0 => a, I1 => b, O => y(2) );\n"
                                      "  s1 : sub generic map ( WIDTH => 2 ) port map ( i => a & b, o => y(1) );\n"
                                      "  s2 : entity work.sub(rtl) generic map ( 3, \"b\", true, 2 ps ) port map ( i(2 downto 1) => a & b, i(0) => '0', o => y(0) );\n"
                                      "  s3 : component sub port map ( i => (a, b), o => open );\n"
                                      "  w : wrapper port map ( x => a, y => open );\n"
                                      "  \\Odd/Name\\ : work.MyLib.BUF port map ( I => a, O => open );\n"
                                      "end rtl;\n");
            ASSERT_EQ(d.modules.size(), 3);
            const netlist_ir::Module* sub = d.find_module("sub");
            const netlist_ir::Module* sub_rtl = d.find_module("sub(rtl)");
            const netlist_ir::Module* top = d.find_module("top");
            ASSERT_NE(sub, nullptr);
            ASSERT_NE(sub_rtl, nullptr);
            ASSERT_NE(top, nullptr);
            EXPECT_EQ(sub->aliases.size(), 0);        // the default architecture is the last one declared, `other`
            EXPECT_EQ(sub_rtl->aliases.size(), 1);    // the one the direct instantiation names
            EXPECT_EQ(typed(sub->parameters, "WIDTH"), std::make_tuple("integer", "3"));
            EXPECT_EQ(typed(sub->parameters, "MODE"), std::make_tuple("string", "a"));
            EXPECT_EQ(typed(sub->parameters, "FLAG"), std::make_tuple("boolean", "false"));
            EXPECT_EQ(typed(sub->parameters, "DELAY"), std::make_tuple("time", "1ns"));
            EXPECT_EQ(typed(top->parameters, "KEEP"), std::make_tuple("string", "yes"));

            ASSERT_EQ(top->instances.size(), 8);
            const Instance& g1 = top->instances.at(0);
            EXPECT_EQ(g1.name, "g1");
            EXPECT_EQ(g1.type, "AND2");    // the component's declared spelling
            EXPECT_EQ(g1.kind, InstanceKind::Gate);
            ASSERT_EQ(g1.connections.size(), 3);
            EXPECT_EQ(g1.connections.at(0).port, "I0");    // the declared spelling of the formal
            EXPECT_EQ(g1.connections.at(2).port, "O");
            EXPECT_EQ(typed(g1.parameters, "LOC"), std::make_tuple("string", "X0Y0"));
            EXPECT_EQ(typed(g1.parameters, "DONT"), std::make_tuple("string", "1"));
            EXPECT_EQ(typed(g1.parameters, "FLAGGY"), std::make_tuple("boolean", "true"));
            EXPECT_TRUE(std::all_of(g1.parameters.begin(), g1.parameters.end(), [](const TypedValue& v) { return v.declaration.get_source() == Parameter::Source::Attribute; }));
            const Instance& g2 = top->instances.at(1);
            ASSERT_EQ(g2.connections.size(), 3);
            EXPECT_FALSE(g2.connections.at(0).replicate);
            EXPECT_TRUE(g2.connections.at(0).port.empty());
            EXPECT_EQ(g2.connections.at(2).bits, (std::vector<BitId>{top->ports.at(2).bits.at(0)}));
            const Instance& g3 = top->instances.at(2);
            EXPECT_EQ(g3.type, "LUT2");
            EXPECT_EQ(typed(g3.parameters, "INIT"), std::make_tuple("bit_vector", "0xE"));
            EXPECT_EQ(typed_size(g3.parameters, "INIT"), 4);
            EXPECT_EQ(typed(g3.parameters, "NEG"), std::make_tuple("integer", "-1"));
            EXPECT_EQ(typed(g3.parameters, "R"), std::make_tuple("float", "1.5"));
            EXPECT_EQ(typed(g3.parameters, "T"), std::make_tuple("time", "10ns"));
            EXPECT_EQ(typed(g3.parameters, "D"), std::make_tuple("integer", "255"));
            EXPECT_EQ(typed(g3.parameters, "B"), std::make_tuple("bit_vector", "0x1"));
            const Instance& s1 = top->instances.at(3);
            EXPECT_EQ(s1.kind, InstanceKind::Module);
            EXPECT_EQ(s1.type, "sub");
            EXPECT_EQ(typed(s1.parameters, "WIDTH"), std::make_tuple("integer", "2"));
            ASSERT_EQ(s1.connections.size(), 2);
            EXPECT_EQ(s1.connections.at(0).bits, (std::vector<BitId>{top->ports.at(0).bits.at(0), top->ports.at(1).bits.at(0)}));
            const Instance& s2 = top->instances.at(4);
            EXPECT_EQ(s2.kind, InstanceKind::Module);
            EXPECT_EQ(s2.type, "sub(rtl)");
            EXPECT_EQ(typed(s2.parameters, "WIDTH"), std::make_tuple("integer", "3"));
            EXPECT_EQ(typed(s2.parameters, "MODE"), std::make_tuple("string", "b"));
            EXPECT_EQ(typed(s2.parameters, "FLAG"), std::make_tuple("boolean", "true"));
            EXPECT_EQ(typed(s2.parameters, "DELAY"), std::make_tuple("time", "2ps"));
            ASSERT_EQ(s2.connections.size(), 3);
            EXPECT_EQ(s2.connections.at(0).port, "i");
            EXPECT_EQ(s2.connections.at(0).port_slice, (Range{2, 1}));
            EXPECT_EQ(s2.connections.at(1).port_slice, (Range{0, 0}));
            EXPECT_EQ(s2.connections.at(1).bits, (std::vector<BitId>{ZERO}));
            const Instance& s3 = top->instances.at(5);
            EXPECT_EQ(s3.kind, InstanceKind::Module);
            ASSERT_EQ(s3.connections.size(), 1);
            EXPECT_EQ(s3.connections.at(0).bits.size(), 2);
            const Instance& w = top->instances.at(6);
            EXPECT_EQ(w.kind, InstanceKind::Gate);    // a component without an entity is a gate type
            EXPECT_EQ(w.type, "wrapper");
            const Instance& odd = top->instances.at(7);
            EXPECT_EQ(odd.name, "Odd/Name");
            EXPECT_EQ(odd.type, "BUF");    // the library prefix is stripped
            EXPECT_EQ(typed(top->find_signal("n")->parameters, "KEEP"), std::make_tuple("string", "true"));
            EXPECT_EQ(typed(top->find_signal("n")->parameters, "NUM"), std::make_tuple("integer", "42"));
        }
        {
            // a configuration binds a component to an entity of the file
            Design d = elaborate_text("entity leaf is port ( i : in std_logic; o : out std_logic ); end leaf;\n"
                                      "architecture rtl of leaf is begin o <= i; end rtl;\n"
                                      "entity top is port ( a : in std_logic; y : out std_logic ); end top;\n"
                                      "architecture rtl of top is\n"
                                      "  component blackbox port ( i : in std_logic; o : out std_logic ); end component;\n"
                                      "begin\n"
                                      "  u : blackbox port map ( i => a, o => y );\n"
                                      "end rtl;\n"
                                      "configuration cfg of top is\n"
                                      "  for rtl\n"
                                      "    for all : blackbox use entity work.leaf(rtl); end for;\n"
                                      "  end for;\n"
                                      "end cfg;\n");
            const netlist_ir::Module* top = d.find_module("top");
            ASSERT_NE(top, nullptr);
            ASSERT_EQ(top->instances.size(), 1);
            EXPECT_EQ(top->instances.at(0).kind, InstanceKind::Module);
            EXPECT_EQ(top->instances.at(0).type, "leaf");
        }
        const std::string head = "entity sub is port ( i : in std_logic ); end sub;\narchitecture rtl of sub is begin end rtl;\narchitecture x of sub is begin end x;\n"
                                 "entity top is port ( a : in std_logic ); end top;\narchitecture rtl of top is\nbegin\n";
        {
            // two architectures in use: one module each, the default one carries the entity's name
            Design d = elaborate_text(head + "  s1 : entity work.sub(rtl) port map ( i => a );\n  s2 : entity work.sub(x) port map ( i => a );\nend rtl;");
            ASSERT_EQ(d.modules.size(), 3);
            EXPECT_NE(d.find_module("sub(rtl)"), nullptr);
            EXPECT_NE(d.find_module("sub"), nullptr);
            EXPECT_EQ(d.find_module("top")->instances.at(0).type, "sub(rtl)");
            EXPECT_EQ(d.find_module("top")->instances.at(1).type, "sub");
        }
        EXPECT_TRUE(elaboration_fails(head + "  s1 : entity work.sub(nope) port map ( i => a );\nend rtl;", "no architecture 'nope'"));
        EXPECT_TRUE(elaboration_fails(head + "  s1 : sub port map ( q => a );\nend rtl;", "has no port 'q'"));
        EXPECT_TRUE(elaboration_fails(head + "  s1 : sub port map ( i => a, i => a );\nend rtl;", "connected twice"));
        EXPECT_TRUE(elaboration_fails(head + "  s1 : sub port map ( i => a );\n  S1 : sub port map ( i => a );\nend rtl;", "used twice"));
        EXPECT_TRUE(elaboration_fails(head + "  s1 : sub generic map ( 1 ) port map ( i => a );\nend rtl;", "more positional generic"));
        EXPECT_TRUE(elaboration_fails(head + "  g : BUF generic map ( K => Inv4lid ) port map ( I => a );\nend rtl;", "not a generic or constant"));
        {
            // `(others => '1')` on a gate port of unknown width is a replicated single bit
            Design d = elaborate_text(head + "  g : RAM port map ( DATA_IN => (others => '1'), ADDR => (a, a) );\nend rtl;");
            const Instance& g = d.find_module("top")->instances.at(0);
            ASSERT_EQ(g.connections.size(), 2);
            EXPECT_TRUE(g.connections.at(0).replicate);
            EXPECT_EQ(g.connections.at(0).bits, (std::vector<BitId>{ONE}));
            EXPECT_FALSE(g.connections.at(1).replicate);
            EXPECT_EQ(g.connections.at(1).bits.size(), 2);
        }
        EXPECT_TRUE(elaboration_fails("entity top is port ( a : in std_logic ); end top;\narchitecture rtl of top is\n  attribute K of nope : signal is 1;\nbegin\nend rtl;", "no signal or port 'nope'"));
        TEST_END
    }
}    // namespace hal
