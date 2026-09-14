#include "vhdl_parser/vhdl_syntax.h"

#include "netlist_test_utils.h"

#include "gtest/gtest.h"

namespace hal
{
    using namespace vhdl;
    using namespace vhdl::ast;

    class VHDLSyntaxTest : public ::testing::Test
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

        SourceFile parse(const std::string& text)
        {
            auto res = parse_string(text, "test.vhd");
            if (res.is_error())
            {
                ADD_FAILURE() << res.get_error().get();
                return {};
            }
            return res.get();
        }

        std::string parse_error(const std::string& text)
        {
            auto res = parse_string(text, "test.vhd");
            if (res.is_ok())
            {
                ADD_FAILURE() << "expected an error";
                return "";
            }
            return res.get_error().get();
        }

        static std::string dump(const Expr& e)
        {
            switch (e.kind)
            {
                case Expr::Kind::Empty:
                    return "<>";
                case Expr::Kind::Open:
                    return "open";
                case Expr::Kind::Others:
                    return "others";
                case Expr::Kind::Identifier: {
                    std::string s;
                    for (const Name& p : e.prefix)
                    {
                        s += p.text + ".";
                    }
                    return s + (e.name.extended ? "\\" + e.name.text + "\\" : e.name.text);
                }
                case Expr::Kind::Number:
                    return e.text;
                case Expr::Kind::Character:
                    return "'" + e.text + "'";
                case Expr::Kind::String:
                    return "\"" + e.text + "\"";
                case Expr::Kind::BitString:
                    return e.text;
                case Expr::Kind::Index: {
                    std::string s = dump(e.children.at(0)) + "[";
                    for (u32 i = 1; i < e.children.size(); i++)
                    {
                        s += (i > 1 ? "," : "") + dump(e.children.at(i));
                    }
                    return s + "]";
                }
                case Expr::Kind::Slice:
                    return dump(e.children.at(0)) + "[" + dump(e.children.at(1)) + " " + e.text + " " + dump(e.children.at(2)) + "]";
                case Expr::Kind::Attribute:
                    return dump(e.children.at(0)) + "'" + e.text;
                case Expr::Kind::Concat: {
                    std::string s = "{";
                    for (u32 i = 0; i < e.children.size(); i++)
                    {
                        s += (i > 0 ? " & " : "") + dump(e.children.at(i));
                    }
                    return s + "}";
                }
                case Expr::Kind::Aggregate: {
                    std::string s = "(";
                    for (u32 i = 0; i < e.children.size(); i++)
                    {
                        const Expr& c = e.children.at(i);
                        s += (i > 0 ? ", " : "");
                        for (u32 j = 0; j < c.choices.size(); j++)
                        {
                            s += (j > 0 ? "|" : "") + dump(c.choices.at(j));
                        }
                        s += (c.choices.empty() ? "" : " => ") + dump(c);
                    }
                    return s + ")";
                }
                case Expr::Kind::Unary:
                    return "(" + e.text + " " + dump(e.children.at(0)) + ")";
                case Expr::Kind::Binary:
                    return "(" + dump(e.children.at(0)) + " " + e.text + " " + dump(e.children.at(1)) + ")";
            }
            return "?";
        }

        static std::string dump_type(const TypeMark& t)
        {
            std::string s;
            for (const Name& p : t.prefix)
            {
                s += p.text + ".";
            }
            s += t.name.text;
            if (!t.constraints.empty())
            {
                s += "(";
                for (u32 i = 0; i < t.constraints.size(); i++)
                {
                    const Range& r = t.constraints.at(i);
                    s += (i > 0 ? ", " : "") + dump(r.left) + (r.descending ? " downto " : " to ") + dump(r.right);
                }
                s += ")";
            }
            return s;
        }
    };

    /**
     * Entities: generics, ports with every mode, defaults, multiple names per declaration, end variants.
     */
    TEST_F(VHDLSyntaxTest, check_entity)
    {
        TEST_START
        {
            SourceFile f = parse("library IEEE;\n"
                                 "use IEEE.STD_LOGIC_1164.ALL;\n"
                                 "entity top is\n"
                                 "  generic (\n"
                                 "    N : integer := 4;\n"
                                 "    INIT : std_logic_vector(3 downto 0) := X\"A\";\n"
                                 "    T : time := 1 ns);\n"
                                 "  port (\n"
                                 "    a, b : in std_logic;\n"
                                 "    q : out std_logic_vector(N - 1 downto 0);\n"
                                 "    io : inout std_logic;\n"
                                 "    buf : buffer std_logic;\n"
                                 "    d : std_logic := '0');\n"
                                 "end top;\n"
                                 "entity E2 is end entity E2;\n"
                                 "entity e3 is end;\n");
            ASSERT_EQ(f.context.size(), 2);
            EXPECT_FALSE(f.context.at(0).is_use);
            EXPECT_EQ(f.context.at(0).parts.size(), 1);
            EXPECT_TRUE(f.context.at(1).is_use);
            ASSERT_EQ(f.context.at(1).parts.size(), 3);
            EXPECT_EQ(f.context.at(1).parts.at(2).text, "ALL");

            ASSERT_EQ(f.entities.size(), 3);
            const Entity& top = f.entities.at(0);
            EXPECT_EQ(top.name.text, "top");
            EXPECT_EQ(top.location.line, 3);
            ASSERT_EQ(top.generics.size(), 3);
            EXPECT_EQ(top.generics.at(0).name.text, "N");
            EXPECT_EQ(dump_type(top.generics.at(0).type), "integer");
            EXPECT_EQ(dump(top.generics.at(0).default_value.value()), "4");
            EXPECT_EQ(dump_type(top.generics.at(1).type), "std_logic_vector(3 downto 0)");
            EXPECT_EQ(dump(top.generics.at(1).default_value.value()), "X\"A\"");
            EXPECT_EQ(dump(top.generics.at(2).default_value.value()), "1 ns");
            ASSERT_EQ(top.ports.size(), 6);
            EXPECT_EQ(top.ports.at(0).name.text, "a");
            EXPECT_EQ(top.ports.at(1).name.text, "b");
            EXPECT_EQ(top.ports.at(0).mode, PinDirection::input);
            EXPECT_EQ(top.ports.at(1).mode, PinDirection::input);
            EXPECT_EQ(top.ports.at(2).mode, PinDirection::output);
            EXPECT_EQ(dump_type(top.ports.at(2).type), "std_logic_vector((N - 1) downto 0)");
            EXPECT_EQ(top.ports.at(3).mode, PinDirection::inout);
            EXPECT_EQ(top.ports.at(4).mode, PinDirection::output);
            EXPECT_EQ(top.ports.at(5).mode, PinDirection::input);
            EXPECT_EQ(dump(top.ports.at(5).default_value.value()), "'0'");
            EXPECT_EQ(top.ports.at(5).location.line, 13);
            EXPECT_EQ(f.entities.at(1).name.text, "E2");
            EXPECT_EQ(f.entities.at(2).name.text, "e3");
        }
        {
            // a name after `end` that does not match is tolerated with a warning
            NO_COUT_TEST_BLOCK;
            SourceFile f = parse("entity a is end b;");
            EXPECT_EQ(f.entities.size(), 1);
        }
        {
            const std::string err = parse_error("entity a is port (x : in std_logic) end a;");
            EXPECT_NE(err.find("line 1"), std::string::npos) << err;
        }
        TEST_END
    }

    /**
     * Architectures: signal, constant, component, type and attribute declarations.
     */
    TEST_F(VHDLSyntaxTest, check_declarations)
    {
        TEST_START
        {
            SourceFile f = parse("entity top is end top;\n"
                                 "architecture STRUCTURE of top is\n"
                                 "  signal n1, n2 : std_logic;\n"
                                 "  signal bus1 : std_logic_vector(7 downto 0) := (others => '0');\n"
                                 "  signal m : std_logic_vector2(0 to 1, 3 downto 0);\n"
                                 "  signal r : work.pkg.t_bus;\n"
                                 "  constant C : integer := 7;\n"
                                 "  type t_arr is array (0 to 3) of std_logic_vector(1 downto 0);\n"
                                 "  type t_unc is array (natural range <>) of std_logic;\n"
                                 "  type t_enum is (idle, run);\n"
                                 "  subtype t_sub is std_logic_vector(3 downto 0);\n"
                                 "  component AND2\n"
                                 "    generic (W : integer := 1);\n"
                                 "    port (A, B : in std_logic; O : out std_logic);\n"
                                 "  end component;\n"
                                 "  component INV is port (I : in std_logic; O : out std_logic); end component INV;\n"
                                 "  attribute KEEP : string;\n"
                                 "  attribute KEEP of n1, n2 : signal is \"true\";\n"
                                 "  attribute LOC of u1 : label is \"SLICE_X0Y0\";\n"
                                 "  attribute X of STRUCTURE : architecture is 3;\n"
                                 "begin\n"
                                 "end STRUCTURE;\n");
            ASSERT_EQ(f.architectures.size(), 1);
            const Architecture& a = f.architectures.at(0);
            EXPECT_EQ(a.name.text, "STRUCTURE");
            EXPECT_EQ(a.entity.text, "top");
            const Declarations& d = a.declarations;

            ASSERT_EQ(d.objects.size(), 6);
            EXPECT_EQ(d.objects.at(0).name.text, "n1");
            EXPECT_EQ(d.objects.at(1).name.text, "n2");
            EXPECT_EQ(dump_type(d.objects.at(1).type), "std_logic");
            EXPECT_EQ(d.objects.at(1).location.line, 3);
            EXPECT_EQ(dump_type(d.objects.at(2).type), "std_logic_vector(7 downto 0)");
            EXPECT_EQ(dump(d.objects.at(2).initializer.value()), "(others => '0')");
            EXPECT_EQ(dump_type(d.objects.at(3).type), "std_logic_vector2(0 to 1, 3 downto 0)");
            EXPECT_EQ(dump_type(d.objects.at(4).type), "work.pkg.t_bus");
            EXPECT_TRUE(d.objects.at(5).is_constant);
            EXPECT_EQ(dump(d.objects.at(5).initializer.value()), "7");

            ASSERT_EQ(d.types.size(), 4);
            EXPECT_TRUE(d.types.at(0).is_array);
            ASSERT_EQ(d.types.at(0).ranges.size(), 1);
            EXPECT_EQ(dump(d.types.at(0).ranges.at(0).right), "3");
            EXPECT_EQ(dump_type(d.types.at(0).element), "std_logic_vector(1 downto 0)");
            EXPECT_TRUE(d.types.at(1).is_array);
            EXPECT_TRUE(d.types.at(1).ranges.empty());
            EXPECT_FALSE(d.types.at(2).is_array);
            EXPECT_EQ(d.types.at(2).name.text, "t_enum");
            EXPECT_EQ(d.types.at(3).name.text, "t_sub");
            EXPECT_EQ(dump_type(d.types.at(3).subtype.value()), "std_logic_vector(3 downto 0)");

            ASSERT_EQ(d.components.size(), 2);
            EXPECT_EQ(d.components.at(0).name.text, "AND2");
            ASSERT_EQ(d.components.at(0).generics.size(), 1);
            ASSERT_EQ(d.components.at(0).ports.size(), 3);
            EXPECT_EQ(d.components.at(0).ports.at(2).name.text, "O");
            EXPECT_EQ(d.components.at(0).ports.at(2).mode, PinDirection::output);
            EXPECT_EQ(d.components.at(1).name.text, "INV");
            EXPECT_EQ(d.components.at(1).ports.size(), 2);

            ASSERT_EQ(d.attribute_decls.size(), 1);
            EXPECT_EQ(d.attribute_decls.at(0).name.text, "KEEP");
            EXPECT_EQ(d.attribute_decls.at(0).type.text, "string");
            ASSERT_EQ(d.attribute_specs.size(), 3);
            EXPECT_EQ(d.attribute_specs.at(0).attribute.text, "KEEP");
            ASSERT_EQ(d.attribute_specs.at(0).targets.size(), 2);
            EXPECT_EQ(d.attribute_specs.at(0).targets.at(1).text, "n2");
            EXPECT_EQ(d.attribute_specs.at(0).entity_class, "signal");
            EXPECT_EQ(dump(d.attribute_specs.at(0).value), "\"true\"");
            EXPECT_EQ(d.attribute_specs.at(1).entity_class, "label");
            EXPECT_EQ(d.attribute_specs.at(2).entity_class, "architecture");
            EXPECT_EQ(dump(d.attribute_specs.at(2).value), "3");
        }
        {
            SourceFile f = parse("entity top is end top;\n"
                                 "architecture a of top is\n"
                                 "  function f (x : integer) return integer;\n"
                                 "  function g (x : integer) return integer is\n"
                                 "    variable v : integer := 0;\n"
                                 "  begin\n"
                                 "    if x > 0 then v := x; end if;\n"
                                 "    return v;\n"
                                 "  end;\n"
                                 "  signal s : std_logic;\n"
                                 "begin\n"
                                 "end a;\n");
            ASSERT_EQ(f.architectures.size(), 1);
            EXPECT_EQ(f.architectures.at(0).declarations.objects.size(), 1);
        }
        {
            const std::string err = parse_error("entity top is end top;\narchitecture a of top is\n  variable v : integer;\nbegin\nend a;");
            EXPECT_NE(err.find("'variable' is not part of a structural netlist"), std::string::npos) << err;
        }
        {
            const std::string err = parse_error("entity top is end top;\narchitecture a of top is\n  attribute K of others : signal is 1;\nbegin\nend a;");
            EXPECT_NE(err.find("'others'"), std::string::npos) << err;
        }
        TEST_END
    }

    /**
     * Statements: instantiations in every form, association lists, and assignments.
     */
    TEST_F(VHDLSyntaxTest, check_statements)
    {
        TEST_START
        {
            const std::string err = parse_error("entity top is end top;\narchitecture a of top is\nbegin\n  w <= n1 when n2 = '1' else n3;\nend a;");
            EXPECT_NE(err.find("only a plain"), std::string::npos) << err;
        }
        {
            SourceFile f = parse("entity top is end top;\n"
                                 "architecture a of top is\n"
                                 "begin\n"
                                 "  u1 : AND2 port map (A => n1, B => n2, O => n3);\n"
                                 "  u2 : component AND2 generic map (W => 2) port map (n1, n2, n3);\n"
                                 "  u3 : entity work.sub(rtl) generic map (4, \"abc\") port map (x => open, y(1) => n1, z(3 downto 0) => bus1(7 downto 4));\n"
                                 "  u4 : configuration work.cfg port map (a => '0', b => B\"01\", c => n1 & n2 & '1', d => (n1, n2), e => (others => '0'));\n"
                                 "  \\U5\\ : \\Odd Name\\ port map (I => \\x\\);\n"
                                 "  u6 : LUT4 generic map (INIT => X\"FFFE\") port map (I0 => n1, I1 => n2, I2 => bus1(0), I3 => m(0, 1), O => q);\n"
                                 "  u7 : NOGEN;\n"
                                 "  n3 <= n1;\n"
                                 "  lbl : bus1(3 downto 0) <= \"0101\";\n"
                                 "  bus1(7) <= not n1;\n"
                                 "  q <= (n1 and n2) or (n3 xor '1');\n"
                                 "  r <= a'length;\n"
                                 "end a;\n");
            ASSERT_EQ(f.architectures.size(), 1);
            const Architecture& a = f.architectures.at(0);
            ASSERT_EQ(a.instantiations.size(), 7);

            const Instantiation& u1 = a.instantiations.at(0);
            EXPECT_EQ(u1.label.text, "u1");
            EXPECT_EQ(u1.kind, Instantiation::Kind::Component);
            EXPECT_EQ(u1.unit.text, "AND2");
            EXPECT_EQ(u1.location.line, 4);
            ASSERT_EQ(u1.port_map.size(), 3);
            EXPECT_EQ(dump(u1.port_map.at(0).formal.value()), "A");
            EXPECT_EQ(dump(u1.port_map.at(0).actual), "n1");

            const Instantiation& u2 = a.instantiations.at(1);
            EXPECT_EQ(u2.kind, Instantiation::Kind::Component);
            ASSERT_EQ(u2.generic_map.size(), 1);
            EXPECT_EQ(dump(u2.generic_map.at(0).formal.value()), "W");
            EXPECT_EQ(dump(u2.generic_map.at(0).actual), "2");
            ASSERT_EQ(u2.port_map.size(), 3);
            EXPECT_FALSE(u2.port_map.at(0).formal.has_value());
            EXPECT_EQ(dump(u2.port_map.at(2).actual), "n3");

            const Instantiation& u3 = a.instantiations.at(2);
            EXPECT_EQ(u3.kind, Instantiation::Kind::Entity);
            ASSERT_EQ(u3.prefix.size(), 1);
            EXPECT_EQ(u3.prefix.at(0).text, "work");
            EXPECT_EQ(u3.unit.text, "sub");
            EXPECT_EQ(u3.architecture.value().text, "rtl");
            ASSERT_EQ(u3.generic_map.size(), 2);
            EXPECT_EQ(dump(u3.generic_map.at(1).actual), "\"abc\"");
            ASSERT_EQ(u3.port_map.size(), 3);
            EXPECT_EQ(dump(u3.port_map.at(0).actual), "open");
            EXPECT_EQ(dump(u3.port_map.at(1).formal.value()), "y[1]");
            EXPECT_EQ(dump(u3.port_map.at(2).formal.value()), "z[3 downto 0]");
            EXPECT_EQ(dump(u3.port_map.at(2).actual), "bus1[7 downto 4]");

            const Instantiation& u4 = a.instantiations.at(3);
            EXPECT_EQ(u4.kind, Instantiation::Kind::Configuration);
            EXPECT_EQ(u4.unit.text, "cfg");
            ASSERT_EQ(u4.port_map.size(), 5);
            EXPECT_EQ(dump(u4.port_map.at(0).actual), "'0'");
            EXPECT_EQ(dump(u4.port_map.at(1).actual), "B\"01\"");
            EXPECT_EQ(dump(u4.port_map.at(2).actual), "{n1 & n2 & '1'}");
            EXPECT_EQ(dump(u4.port_map.at(3).actual), "(n1, n2)");
            EXPECT_EQ(dump(u4.port_map.at(4).actual), "(others => '0')");

            const Instantiation& u5 = a.instantiations.at(4);
            EXPECT_EQ(u5.label.text, "U5");
            EXPECT_TRUE(u5.label.extended);
            EXPECT_EQ(u5.unit.text, "Odd Name");
            EXPECT_TRUE(u5.unit.extended);
            EXPECT_EQ(dump(u5.port_map.at(0).actual), "\\x\\");

            const Instantiation& u6 = a.instantiations.at(5);
            EXPECT_EQ(dump(u6.generic_map.at(0).actual), "X\"FFFE\"");
            EXPECT_EQ(dump(u6.port_map.at(2).actual), "bus1[0]");
            EXPECT_EQ(dump(u6.port_map.at(3).actual), "m[0,1]");

            EXPECT_EQ(a.instantiations.at(6).unit.text, "NOGEN");
            EXPECT_TRUE(a.instantiations.at(6).port_map.empty());

            ASSERT_EQ(a.assignments.size(), 5);
            EXPECT_FALSE(a.assignments.at(0).label.has_value());
            EXPECT_EQ(dump(a.assignments.at(0).target), "n3");
            EXPECT_EQ(dump(a.assignments.at(0).value), "n1");
            EXPECT_EQ(a.assignments.at(1).label.value().text, "lbl");
            EXPECT_EQ(dump(a.assignments.at(1).target), "bus1[3 downto 0]");
            EXPECT_EQ(dump(a.assignments.at(1).value), "\"0101\"");
            EXPECT_EQ(dump(a.assignments.at(2).value), "(not n1)");
            EXPECT_EQ(dump(a.assignments.at(3).value), "((n1 and n2) or (n3 xor '1'))");
            EXPECT_EQ(dump(a.assignments.at(4).value), "a'length");
        }
        {
            const std::string err = parse_error("entity top is end top;\narchitecture a of top is\nbegin\n  p : process (clk) begin end process;\nend a;");
            EXPECT_NE(err.find("'process' is not part of a structural netlist"), std::string::npos) << err;
        }
        {
            const std::string err = parse_error("entity top is end top;\narchitecture a of top is\nbegin\n  g : for i in 0 to 3 generate end generate;\nend a;");
            EXPECT_NE(err.find("not part of a structural netlist"), std::string::npos) << err;
        }
        TEST_END
    }

    /**
     * Packages, package bodies, configurations, and files without an entity.
     */
    TEST_F(VHDLSyntaxTest, check_units)
    {
        TEST_START
        {
            SourceFile f = parse("package comps is\n"
                                 "  component AND2 port (A, B : in std_logic; O : out std_logic); end component;\n"
                                 "  constant K : integer := 3;\n"
                                 "  function f (x : integer) return integer;\n"
                                 "end package comps;\n"
                                 "package body comps is\n"
                                 "  function f (x : integer) return integer is\n"
                                 "    variable v : integer;\n"
                                 "  begin\n"
                                 "    if x > 0 then v := x; else v := -x; end if;\n"
                                 "    for i in 0 to 3 loop v := v + i; end loop;\n"
                                 "    return v;\n"
                                 "  end function f;\n"
                                 "end package body comps;\n"
                                 "entity top is end top;\n"
                                 "configuration cfg of top is\n"
                                 "  for a\n"
                                 "    for all : AND2 use entity work.AND2(rtl); end for;\n"
                                 "    for u1 : INV use entity work.INV; end for;\n"
                                 "  end for;\n"
                                 "end cfg;\n");
            ASSERT_EQ(f.packages.size(), 1);
            EXPECT_EQ(f.packages.at(0).name.text, "comps");
            EXPECT_EQ(f.packages.at(0).declarations.components.size(), 1);
            EXPECT_EQ(f.packages.at(0).declarations.objects.size(), 1);
            ASSERT_EQ(f.entities.size(), 1);
            ASSERT_EQ(f.configurations.size(), 1);
            const Configuration& c = f.configurations.at(0);
            EXPECT_EQ(c.name.text, "cfg");
            EXPECT_EQ(c.entity.text, "top");
            ASSERT_EQ(c.bindings.size(), 2);
            EXPECT_EQ(c.bindings.at(0).component.text, "AND2");
            EXPECT_EQ(c.bindings.at(0).entity.text, "AND2");
            EXPECT_EQ(c.bindings.at(0).architecture.value().text, "rtl");
            EXPECT_EQ(c.bindings.at(1).component.text, "INV");
            EXPECT_FALSE(c.bindings.at(1).architecture.has_value());
        }
        {
            const std::string err = parse_error("package p is end p;");
            EXPECT_NE(err.find("no entity"), std::string::npos) << err;
        }
        {
            const std::string err = parse_error("");
            EXPECT_FALSE(err.empty());
        }
        TEST_END
    }
}    // namespace hal
