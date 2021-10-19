#include "netlist_test_utils.h"
#include "gtest/gtest.h"
#include "hal_core/netlist/boolean_function.h"
#include "hal_core/netlist/boolean_function/solver.h"
#include "hal_core/netlist/boolean_function/types.h"

#include <iostream>
#include <type_traits>

namespace hal {
    class BooleanFunctionTest : public ::testing::Test {
    protected:
        virtual void SetUp() {
            test_utils::init_log_channels();
        }

        virtual void TearDown() {
        }

        // Test Debug only
        void print_bf(BooleanFunction bf) {
            std::cout << "\n-------------\n" << bf << "\n-------------\n";
        }

        void printTruthTable(BooleanFunction bf, std::vector<std::string> vars) {
            std::cout << std::endl;
            for (auto i : vars) {
                std::cout << i;
            }
            std::cout << "|O" << std::endl;
            std::vector<BooleanFunction::Value> t_table = bf.get_truth_table(vars);
            for (unsigned int i = 0; i < vars.size() + 2; i++) std::cout << "-";
            std::cout << std::endl;
            for (unsigned int i = 0; i < t_table.size(); i++) {
                for (unsigned int j = 0; j < vars.size(); j++) {
                    std::cout << ((((i >> j) & 1) > 0) ? "1" : "0");
                }
                std::cout << "|";
                switch (t_table[i]) {
                    case BooleanFunction::Value::ONE: std::cout << "1";
                        break;
                    case BooleanFunction::Value::ZERO: std::cout << "0";
                        break;
                    default:std::cout << "X";
                        break;
                }
                std::cout << std::endl;
            }

        }

        // Remove the spaces from a string
        std::string no_space(std::string s) {
            return utils::replace<std::string>(s, " ", "");
        }
    };

    /**
     * Testing move and copy constructors
     */
    TEST_F(BooleanFunctionTest, check_constructor_types) {
        TEST_START
            {
                EXPECT_EQ( std::is_copy_constructible<BooleanFunction>::value, true);
                EXPECT_EQ( std::is_copy_assignable<BooleanFunction>::value, true);
                EXPECT_EQ( std::is_move_constructible<BooleanFunction>::value, true);
                EXPECT_EQ( std::is_move_assignable<BooleanFunction>::value, true);
            }
        TEST_END
    }

    /**
     * Testing the different constructors and the main functionality, by implement the following boolean function:
     *
     *  f(A,B,C) = ( (A AND B) OR C ) XOR 1
     *
     * Functions: constructor, evaluate, get_truth_table, AND, XOR, OR
     */
    TEST_F(BooleanFunctionTest, check_main_example) {
        TEST_START
            // TODO(@fyrbiak): replace this function with symbolic execution once implemented
            /*

            {
                const auto   a = BooleanFunction::Var("A"),
                             b = BooleanFunction::Var("B"),
                             c = BooleanFunction::Var("C"),
                            _0 = BooleanFunction::Const(BooleanFunction::Value::ZERO),
                            _1 = BooleanFunction::Const(BooleanFunction::Value::ONE);

                // Combining them
                BooleanFunction r = ((a & b) | c) ^_1;

                EXPECT_EQ(r(create_input_map("ABC", "000")), BooleanFunction::Value::ONE);
                EXPECT_EQ(r(create_input_map("ABC", "001")), BooleanFunction::Value::ZERO);
                EXPECT_EQ(r(create_input_map("ABC", "010")), BooleanFunction::Value::ONE);
                EXPECT_EQ(r(create_input_map("ABC", "011")), BooleanFunction::Value::ZERO);

                EXPECT_EQ(r(create_input_map("ABC", "100")), BooleanFunction::Value::ONE);
                EXPECT_EQ(r(create_input_map("ABC", "101")), BooleanFunction::Value::ZERO);
                EXPECT_EQ(r(create_input_map("ABC", "110")), BooleanFunction::Value::ZERO);
                EXPECT_EQ(r(create_input_map("ABC", "111")), BooleanFunction::Value::ZERO);

                std::vector<BooleanFunction::Value>
                    truth_table = r.get_truth_table(std::vector<std::string>({"C", "B", "A"}));

                EXPECT_EQ(truth_table, std::vector<BooleanFunction::Value>({
                    BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ONE,   BooleanFunction::Value::ZERO, 
                    BooleanFunction::Value::ONE, BooleanFunction::Value::ZERO, BooleanFunction::Value::ZERO,  BooleanFunction::Value::ZERO
                }));
            }
            */
        TEST_END
    }

    /**
     * Testing the to_string function
     *
     * Functions: to_string
     */
    TEST_F(BooleanFunctionTest, check_to_string) {
        TEST_START
            const auto  a = BooleanFunction::Var("A"),
                        b = BooleanFunction::Var("B"),
                        c = BooleanFunction::Var("C"),
                       _0 = BooleanFunction::Const(BooleanFunction::Value::ZERO),
                       _1 = BooleanFunction::Const(BooleanFunction::Value::ONE);

            // Check some bf strings
            std::vector<std::pair<BooleanFunction, std::string>> test_cases =
                {
                    {(a & b), "(A & B)"},
                    {(a & (b | c)), "(A & (B | C))"},
                    {((a & b) ^ (b & c)), "((A & B) ^ (B & C))"},
                    {(a ^ _1), "(A ^ 0b1)"},
                    {(a ^ _0), "(A ^ 0b0)"},
                    {(~a), "(! A)"}
                };

            for (const auto& [function, str] : test_cases) {
                EXPECT_EQ(function.to_string(), str);
            }

            // Check an empty boolean function
            EXPECT_EQ(BooleanFunction().to_string(), "<empty>");

        TEST_END
    }

    /**
     * Testing the integer values for enum BooleanFunction::Value.
     */
    TEST_F(BooleanFunctionTest, check_enum_values) {
        TEST_START
            EXPECT_EQ(static_cast<BooleanFunction::Value>(0), BooleanFunction::Value::ZERO);
            EXPECT_EQ(static_cast<BooleanFunction::Value>(1), BooleanFunction::Value::ONE);
        TEST_END
    }

    /**
     * Testing the functions is_constant_one and is_constant_zero, by passing some sample inputs
     *
     * Functions: is_constant_one, is_constant_zero
     */
    TEST_F(BooleanFunctionTest, check_is_constant) {
        TEST_START
           const auto  a = BooleanFunction::Var("A"),
                      _0 = BooleanFunction::Const(BooleanFunction::Value::ZERO),
                      _1 = BooleanFunction::Const(BooleanFunction::Value::ONE);
            {
                // Some samples that are constant zero
                EXPECT_TRUE((_0).is_constant_zero());
                EXPECT_TRUE((~_1).is_constant_zero());
                EXPECT_TRUE((a ^ a).optimize().is_constant_zero());
                EXPECT_TRUE((a & (~a)).optimize().is_constant_zero());
                EXPECT_TRUE((_0 | _0).optimize().is_constant_zero());
            }
            {
                // Some samples that are constant one
                EXPECT_TRUE((_1).is_constant_one());
                EXPECT_TRUE((~_0).is_constant_one());
                EXPECT_TRUE((a ^ (~a)).optimize().is_constant_one());
                EXPECT_TRUE((a | (~a)).optimize().is_constant_one());
                EXPECT_TRUE((_1 & _1).optimize().is_constant_one());
                EXPECT_TRUE((_0 | _1).optimize().is_constant_one());
            }
            {
                // Some samples that are NOT constant zero
                EXPECT_FALSE((_1).is_constant_zero());
                EXPECT_FALSE((_0 | _1).optimize().is_constant_zero());
                EXPECT_FALSE((a).is_constant_zero());
                EXPECT_FALSE((_0 | _1).optimize().is_constant_zero());
            }
            {
                // Some samples that are NOT constant one
                EXPECT_FALSE((_0).is_constant_one());
                EXPECT_FALSE((a).is_constant_one());
                EXPECT_FALSE((_0 & _1).optimize().is_constant_one());
            }

        TEST_END
    }

    /**
     * Testing the is_empty function
     *
     * Functions: is_empty
     */
    TEST_F(BooleanFunctionTest, check_is_empty) {
        TEST_START
            {
                // The boolean function is not empty
                auto not_empty = BooleanFunction::Var("A");
                EXPECT_FALSE(not_empty.is_empty());
            }
            {
                // The boolean function is empty
                BooleanFunction empty;
                EXPECT_TRUE(empty.is_empty());
            }
        TEST_END
    }

    /**
     * Testing the get_variables function
     *
     * Functions: get_variables
     */
    TEST_F(BooleanFunctionTest, check_get_variables) {
        TEST_START
            {
                // Get variables
                const auto  a = BooleanFunction::Var("A"),
                            b = BooleanFunction::Var("B"),
                            c = BooleanFunction::Var("C"),
                           a2 = BooleanFunction::Var("A");
                EXPECT_EQ((a | b | c | a2).get_variables(), std::vector<std::string>({"A", "B", "C"}));
            }
        TEST_END
    }

    /**
     * Testing comparation operator
     *
     * Functions: operator==, operator!=
     */
    TEST_F(BooleanFunctionTest, check_compare_operator) {
        TEST_START
            const auto  a = BooleanFunction::Var("A"),
                        b = BooleanFunction::Var("B"),
                       _1 = BooleanFunction::Const(BooleanFunction::Value::ONE);

            // Tests for ==
            {
                // Compare the same object
                EXPECT_TRUE((a == a));
            }
            {
                // The boolean functions are equivalent in syntax
                EXPECT_TRUE(((a | b) == (a | b)));
            }
            {
                // Compare two empty expressions
                EXPECT_TRUE(BooleanFunction() == BooleanFunction());
            }
            // Tests for !=
            {
                // The boolean function are equivalent in semantic, but do not share the same variable
                EXPECT_TRUE((a != b));
            }
            {
                // Compare boolean functions of different types (constant, variable, expression)
                EXPECT_TRUE((a != (a | (b & _1)))); // variable - expression
                EXPECT_TRUE((a != _1)); // variable - constant
                EXPECT_TRUE(((a | (b & _1)) != _1)); // expression - constant
            }
            {
                // Compare semantically different expressions
                EXPECT_TRUE(((a & b) != (a | b)));
                EXPECT_TRUE(((a ^ b) != (a & b)));
                EXPECT_TRUE(((a ^ b) != ((~a) & b)));
            }
        TEST_END
    }

    /**
     * Testing the integrity of the optimize function
     *
     * Functions: optimize
     */
    TEST_F(BooleanFunctionTest, check_optimize_correctness)
    {
        TEST_START
           const auto  a = BooleanFunction::Var("A"),
                       b = BooleanFunction::Var("B"),
                       c = BooleanFunction::Var("C"),
                      _0 = BooleanFunction::Const(BooleanFunction::Value::ZERO),
                      _1 = BooleanFunction::Const(BooleanFunction::Value::ONE);
            {
                // Optimize some boolean functions and compare their truth_table
                BooleanFunction bf = (~(a ^ b & c) | (b | c & _1)) ^((a & b) | (a | b | c));
                EXPECT_EQ(bf.get_truth_table(std::vector<std::string>({"C", "B", "A"})),
                          bf.optimize().get_truth_table(std::vector<std::string>({"C", "B", "A"})));
            }
            {
                // Optimize some boolean functions and compare their truth_table
                BooleanFunction bf = (a | b | c);
                EXPECT_EQ(bf.get_truth_table(std::vector<std::string>({"C", "B", "A"})),
                          bf.optimize().get_truth_table(std::vector<std::string>({"C", "B", "A"})));
            }
            {
                // Optimize a boolean function that is constant one
                BooleanFunction bf = (a & b) | (~a & b) | (a & ~b) | (~a & ~b);
                EXPECT_EQ(bf.get_truth_table(std::vector<std::string>({"A", "B"})),
                          bf.optimize().get_truth_table(std::vector<std::string>({"A", "B"})));
            }
            {
                // Optimize a boolean function that is constant zero
                BooleanFunction bf = (a & ~a) | (b & ~b);
                EXPECT_EQ(bf.get_truth_table(std::vector<std::string>({"A", "B"})),
                          bf.optimize().get_truth_table(std::vector<std::string>({"A", "B"})));
            }
        TEST_END
    }

    /**
     * Testing that the optimize function actually returns a minimized result
     *
     * Functions: optimize
     */
    TEST_F(BooleanFunctionTest, check_optimize_minimality)
    {
        TEST_START
            const auto  a = BooleanFunction::Var("A"),
                       b = BooleanFunction::Var("B"),
                       c = BooleanFunction::Var("C"),
                      _0 = BooleanFunction::Const(BooleanFunction::Value::ZERO),
                      _1 = BooleanFunction::Const(BooleanFunction::Value::ONE);
            {
                // Optimize MUX function
                BooleanFunction bf = (a & c) | (b & ~c) | (a & b);
                EXPECT_EQ(bf.optimize().to_string(), "((A & C) | (B & (! C)))");
            }
        TEST_END
    }


    /**
     * Testing the integrity of the from_string function
     *
     * Functions: from_string
     */
    TEST_F(BooleanFunctionTest, check_from_string) {
        TEST_START
            std::string f_str = "A B C D(1)";
            {
                // Check default case
                auto bf = BooleanFunction::from_string(f_str);
                EXPECT_EQ(bf.get_variables(), std::vector<std::string>({"A", "B", "C", "D"}));
            }
            {
                // Declare existing variable
                auto bf = BooleanFunction::from_string(f_str, {"A"});
                EXPECT_EQ(bf.get_variables(), std::vector<std::string>({"A", "B", "C", "D"}));
            }
            {
                // Declare custom variable
                auto bf = BooleanFunction::from_string(f_str, {"A B"});
                EXPECT_EQ(bf.get_variables(), std::vector<std::string>({"A B", "C", "D"}));
            }
            {
                // Declare custom variable
                auto bf = BooleanFunction::from_string(f_str, {"A B C D"});
                EXPECT_EQ(bf.get_variables(), std::vector<std::string>({"A B C D"}));
            }
            {
                // Declare custom variable
                auto bf = BooleanFunction::from_string(f_str, {"D(1)"});
                EXPECT_EQ(bf.get_variables(), std::vector<std::string>({"A", "B", "C", "D(1)"}));
            }
            {
                // Declare non-existing custom variable
                auto bf = BooleanFunction::from_string(f_str, {"X"});
                EXPECT_EQ(bf.get_variables(), std::vector<std::string>({"A", "B", "C", "D"}));
            }
            {
                auto bf = BooleanFunction::from_string("!(A1 | A2)", {"A", "B"});
                EXPECT_EQ(bf.get_variables(), std::vector<std::string>({"2", "A"}));
            }

        TEST_END
    }


    /**
     * Test string parsing, dnf, and optimization for a collection of functions
     *
     * Functions: from_string, to_dnf, optimize
     */
    TEST_F(BooleanFunctionTest, check_test_vectors) {
        TEST_START
            {
                /* clang-format off */
                std::vector<std::string> test_cases = {
                    "0",
                    "1",
                    "a",
                    "a ^ a",
                    "a | a",
                    "a & a",
                    "a ^ b",
                    "a | b",
                    "a & b",
                    "!(a ^ a) ^ !(!(b ^ b))",
                    "(!I0 & I1 & I2) | (I0 & I1 & I2)",
                    "!(((!8 & !(!(!19 | !20) & 26)) | (8 & !((!26 & !(!19 | !20)) | (26 & !(!19 | 20))))) & !(((((((!(!20 | 19) & 28 & !((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | (!(!20 | 19) & !28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | ((!20 | 19) & 28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | (!(!20 | 19) & 28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19))))) & 27 & !((9 & (!20 | 19)) | (!9 & !(!20 | 19)))) | (((!(!20 | 19) & 28 & !((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | (!(!20 | 19) & !28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | ((!20 | 19) & 28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | (!(!20 | 19) & 28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19))))) & !27 & ((9 & (!20 | 19)) | (!9 & !(!20 | 19)))) | (!((!(!20 | 19) & 28 & !((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | (!(!20 | 19) & !28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | ((!20 | 19) & 28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | (!(!20 | 19) & 28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19))))) & 27 & ((9 & (!20 | 19)) | (!9 & !(!20 | 19)))) | (((!(!20 | 19) & 28 & !((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | (!(!20 | 19) & !28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | ((!20 | 19) & 28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | (!(!20 | 19) & 28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19))))) & 27 & ((9 & (!20 | 19)) | (!9 & !(!20 | 19))))) & !26 & !((8 & (!20 | 19)) | (!8 & !(!20 | 19)))) | (!((((!(!20 | 19) & 28 & !((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | (!(!20 | 19) & !28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | ((!20 | 19) & 28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | (!(!20 | 19) & 28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19))))) & 27 & !((9 & (!20 | 19)) | (!9 & !(!20 | 19)))) | (((!(!20 | 19) & 28 & !((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | (!(!20 | 19) & !28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | ((!20 | 19) & 28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | (!(!20 | 19) & 28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19))))) & !27 & ((9 & (!20 | 19)) | (!9 & !(!20 | 19)))) | (!((!(!20 | 19) & 28 & !((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | (!(!20 | 19) & !28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | ((!20 | 19) & 28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | (!(!20 | 19) & 28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19))))) & 27 & ((9 & (!20 | 19)) | (!9 & !(!20 | 19)))) | (((!(!20 | 19) & 28 & !((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | (!(!20 | 19) & !28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | ((!20 | 19) & 28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | (!(!20 | 19) & 28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19))))) & 27 & ((9 & (!20 | 19)) | (!9 & !(!20 | 19))))) & 26 & !((8 & (!20 | 19)) | (!8 & !(!20 | 19)))) | (!((((!(!20 | 19) & 28 & !((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | (!(!20 | 19) & !28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | ((!20 | 19) & 28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | (!(!20 | 19) & 28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19))))) & 27 & !((9 & (!20 | 19)) | (!9 & !(!20 | 19)))) | (((!(!20 | 19) & 28 & !((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | (!(!20 | 19) & !28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | ((!20 | 19) & 28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | (!(!20 | 19) & 28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19))))) & !27 & ((9 & (!20 | 19)) | (!9 & !(!20 | 19)))) | (!((!(!20 | 19) & 28 & !((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | (!(!20 | 19) & !28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | ((!20 | 19) & 28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | (!(!20 | 19) & 28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19))))) & 27 & ((9 & (!20 | 19)) | (!9 & !(!20 | 19)))) | (((!(!20 | 19) & 28 & !((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | (!(!20 | 19) & !28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | ((!20 | 19) & 28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | (!(!20 | 19) & 28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19))))) & 27 & ((9 & (!20 | 19)) | (!9 & !(!20 | 19))))) & !26 & ((8 & (!20 | 19)) | (!8 & !(!20 | 19)))) | (((((!(!20 | 19) & 28 & !((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | (!(!20 | 19) & !28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | ((!20 | 19) & 28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | (!(!20 | 19) & 28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19))))) & 27 & !((9 & (!20 | 19)) | (!9 & !(!20 | 19)))) | (((!(!20 | 19) & 28 & !((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | (!(!20 | 19) & !28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | ((!20 | 19) & 28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | (!(!20 | 19) & 28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19))))) & !27 & ((9 & (!20 | 19)) | (!9 & !(!20 | 19)))) | (!((!(!20 | 19) & 28 & !((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | (!(!20 | 19) & !28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | ((!20 | 19) & 28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | (!(!20 | 19) & 28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19))))) & 27 & ((9 & (!20 | 19)) | (!9 & !(!20 | 19)))) | (((!(!20 | 19) & 28 & !((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | (!(!20 | 19) & !28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | ((!20 | 19) & 28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19)))) | (!(!20 | 19) & 28 & ((10 & (!20 | 19)) | (!10 & !(!20 | 19))))) & 27 & ((9 & (!20 | 19)) | (!9 & !(!20 | 19))))) & 26 & ((8 & (!20 | 19)) | (!8 & !(!20 | 19))))) & !19))",
                };
                /* clang-format on */

                for (const auto &f_str : test_cases) {
                    auto f = BooleanFunction::from_string(f_str);
                    auto tmp_vars = f.get_variables();
                    std::vector<std::string> ordered_variables(tmp_vars.begin(), tmp_vars.end());
                    auto original_truth_table = f.get_truth_table(ordered_variables);
                    auto dnf = f.to_dnf();
                    auto dnf_truth_table = dnf.get_truth_table(ordered_variables);
                    auto optimized = f.optimize();
                    auto optimized_truth_table = optimized.get_truth_table(ordered_variables);

                    if (original_truth_table != dnf_truth_table) {
                        EXPECT_TRUE(false) << "ERROR: DNF function does not match original function" << std::endl
                                           << "  original function:  " << f << std::endl
                                           << "  DNF of function:    " << dnf << std::endl
                                           << "  --------------------" << std::endl
                                           << "  TT original function:  " << utils::join("", original_truth_table, [](auto x){return x == BooleanFunction::X?"-":std::to_string(x);}) << std::endl
                                           << "  TT DNF of function:    " << utils::join("", dnf_truth_table, [](auto x){return x == BooleanFunction::X?"-":std::to_string(x);}) << std::endl;
                    }

                    if (original_truth_table != optimized_truth_table) {
                        EXPECT_TRUE(false) << "ERROR: optimized function does not match original function" << std::endl
                                           << "  original function:  " << f << std::endl
                                           << "  optimized function: " << optimized << std::endl
                                           << "  --------------------" << std::endl
                                           << "  TT original function:  " << utils::join("", original_truth_table, [](auto x){return x == BooleanFunction::X?"-":std::to_string(x);}) << std::endl
                                           << "  TT optimized function: " << utils::join("", optimized_truth_table, [](auto x){return x == BooleanFunction::X?"-":std::to_string(x);}) << std::endl;
                    }

                    EXPECT_TRUE(optimized.to_string() == optimized.optimize().to_string()) << "ERROR: re-optimizing changed the function" << std::endl
                                           << "  optimized function: " << optimized << std::endl
                                           << "  optimized again:    " << optimized.optimize() << std::endl;

                    // std::cout << f << std::endl << optimized << std::endl << std::endl;
                }
            }

        TEST_END
    }

    /**
     * Testing the substitution a variable within a boolean function with another boolean function
     *
     * Functions: substitute
     */
    TEST_F(BooleanFunctionTest, check_substitute) {
        TEST_START
            const auto  a = BooleanFunction::Var("A"),
                        b = BooleanFunction::Var("B"),
                        c = BooleanFunction::Var("C"),
                        d = BooleanFunction::Var("D");

            {
                // Substitute a variable with another one
                BooleanFunction bf = a & b & c;
                BooleanFunction sub_bf = bf.substitute("C", "D");

                EXPECT_EQ(sub_bf, a & b & d);
            }
            {
                // Substitute a variable with a boolean function (negated variable)
                BooleanFunction bf = a & b;
                BooleanFunction sub_bf = bf.substitute("B", ~c);

                EXPECT_EQ(sub_bf, a & ~c);
            }
            {
                // Substitute a variable with a boolean function (term)
                BooleanFunction bf = a & b;
                BooleanFunction sub_bf = bf.substitute("B", b | c | d);

                EXPECT_EQ(sub_bf, a & (b | c | d));
            }
            // NEAGATIVE
            /*{
                // Pass an empty boolean function (NOTE: requirement?)
                BooleanFunction bf = a & b;
                BooleanFunction sub_bf = bf.substitute("B", BooleanFunction());

                EXPECT_EQ(sub_bf, a);
            }*/

        TEST_END
    }

    /**
     * Testing the get_dnf_clauses function that accesses the clauses of the DNF in a 2D Vector.
     *
     * Functions: get_dnf_clauses
     */
    TEST_F(BooleanFunctionTest, check_get_dnf_clauses) {
        TEST_START
            const auto  a = BooleanFunction::Var("A"),
                        b = BooleanFunction::Var("B"),
                        c = BooleanFunction::Var("C"),
                        d = BooleanFunction::Var("D"),
                      _0 = BooleanFunction::Const(BooleanFunction::Value::ZERO),
                      _1 = BooleanFunction::Const(BooleanFunction::Value::ONE);
            {
                // Get the dnf clauses of a boolean function that is already in dnf
                BooleanFunction bf = (a & b & ~c) | (a & ~b) | d;
                auto dnf_clauses = bf.get_dnf_clauses();

                std::vector<std::vector<std::pair<std::string, bool>>> exp_clauses;
                exp_clauses.push_back(std::vector<std::pair<std::string, bool>>(
                    {std::make_pair("A", true), std::make_pair("B", true), std::make_pair("C", false)}));
                exp_clauses.push_back(std::vector<std::pair<std::string, bool>>(
                    {std::make_pair("A", true), std::make_pair("B", false)}));
                exp_clauses.push_back(std::vector<std::pair<std::string, bool>>(
                    {std::make_pair("D", true)}));

                EXPECT_EQ(dnf_clauses, exp_clauses);
            }
            {
                // Get the dnf clauses of a variable
                BooleanFunction bf = a;
                auto dnf_clauses = bf.get_dnf_clauses();

                std::vector<std::vector<std::pair<std::string, bool>>> exp_clauses;
                exp_clauses.push_back(std::vector<std::pair<std::string, bool>>(
                    {std::make_pair("A", true)}));

                EXPECT_EQ(dnf_clauses, exp_clauses);
            }
            {
                // Get the dnf clauses of a constant
                BooleanFunction bf = _1;
                auto dnf_clauses = bf.get_dnf_clauses();

                std::vector<std::vector<std::pair<std::string, bool>>> exp_clauses;
                exp_clauses.push_back(std::vector<std::pair<std::string, bool>>(
                    {std::make_pair("1", true)}));

                EXPECT_EQ(dnf_clauses, exp_clauses);
            }
            {
                // Get the dnf clauses of a constant
                BooleanFunction bf = a & ~b & c;
                auto dnf_clauses = bf.get_dnf_clauses();

                std::vector<std::vector<std::pair<std::string, bool>>> exp_clauses;
                exp_clauses.push_back(std::vector<std::pair<std::string, bool>>(
                    {std::make_pair("A", true), std::make_pair("B", false), std::make_pair("C", true)}));

                EXPECT_EQ(dnf_clauses, exp_clauses);
            }
            // NEGATIVE
            {
                // Get the dnf clauses of an empty boolean function
                BooleanFunction bf = BooleanFunction();

                std::vector<std::vector<std::pair<std::string, bool>>> exp_clauses;

                EXPECT_EQ(bf.get_dnf_clauses(), exp_clauses);
            }
        TEST_END
    }

    TEST_F(BooleanFunctionTest, QueryConfig) {
        {
            const auto config = SMT::QueryConfig()
                .with_solver(SMT::SolverType::Z3)
                .with_local_solver()
                .with_model_generation()
                .with_timeout(42);

            EXPECT_EQ(config.solver, SMT::SolverType::Z3);
            EXPECT_EQ(config.local, true);
            EXPECT_EQ(config.generate_model, true);
            EXPECT_EQ(config.timeout_in_seconds, 42);
        }
        {
        const auto config = SMT::QueryConfig()
            .with_solver(SMT::SolverType::Boolector)
            .with_remote_solver()
            .without_model_generation();

        EXPECT_EQ(config.solver, SMT::SolverType::Boolector);
        EXPECT_EQ(config.local, false);
        EXPECT_EQ(config.generate_model, false);
        }
    }

    TEST_F(BooleanFunctionTest, SatisfiableConstraint) {
        const auto  a = BooleanFunction::Var("A"),
                    b = BooleanFunction::Var("B"),
                   _0 = BooleanFunction::Const(BooleanFunction::Value::ZERO),
                   _1 = BooleanFunction::Const(BooleanFunction::Value::ONE);

        auto formulas = std::vector<std::vector<SMT::Constraint>>({
            {
                SMT::Constraint(a & b,_1)
            },
            {
                SMT::Constraint(a | b, _1),
                SMT::Constraint(a, _1),
                SMT::Constraint(b, _0),
            },
            {
                SMT::Constraint(a & b, _1),
                SMT::Constraint(a, _1),
                SMT::Constraint(b, _1),
            },
            {
                SMT::Constraint((a & ~b) | (~a & b), _1),
                SMT::Constraint(a, _1),
            }
        });

        for (auto&& constraints : formulas) {
            const auto solver = SMT::Solver(std::move(constraints));

            for (auto&& solver_type : {SMT::SolverType::Z3}) {
                if (!SMT::Solver::has_local_solver_for(solver_type)) {
                    continue;
                }

                auto [status, result] = solver.query(
                    SMT::QueryConfig()
                        .with_solver(solver_type)
                        .with_local_solver()
                        .with_model_generation()
                        .with_timeout(1000)
                );

                EXPECT_TRUE(status);
                EXPECT_EQ(result.type, SMT::ResultType::Sat);
                EXPECT_TRUE(result.model.has_value());
            }
        }
    }

    TEST_F(BooleanFunctionTest, UnSatisfiableConstraint) {
        const auto  a = BooleanFunction::Var("A"),
                    b = BooleanFunction::Var("B"),
                   _0 = BooleanFunction::Const(BooleanFunction::Value::ZERO),
                   _1 = BooleanFunction::Const(BooleanFunction::Value::ONE);

        auto formulas = std::vector<std::vector<SMT::Constraint>>({
            {
                SMT::Constraint(a, b),
                SMT::Constraint(a, _1),
                SMT::Constraint(b, _0),
            },
            {
                SMT::Constraint(a | b, _1),
                SMT::Constraint(a, _0),
                SMT::Constraint(b, _0),
            },
            {
                SMT::Constraint(a & b, _1),
                SMT::Constraint(a, _0),
                SMT::Constraint(b, _1),
            },
            {
                SMT::Constraint(a & b, _1),
                SMT::Constraint(a, _1),
                SMT::Constraint(b, _0),
            },
            {
                SMT::Constraint((a & ~b) | (~a & b), _1),
                SMT::Constraint(a, _1),
                SMT::Constraint(b, _1),
            }
        });

        for (auto&& constraints : formulas) {
            const auto solver = SMT::Solver(std::move(constraints));
            for (auto&& solver_type : {SMT::SolverType::Z3}) {
                if (!SMT::Solver::has_local_solver_for(solver_type)) {
                    continue;
                }

                auto [status, result] = solver.query(
                    SMT::QueryConfig()
                        .with_solver(solver_type)
                        .with_local_solver()
                        .with_model_generation()
                        .with_timeout(1000)
                );

                EXPECT_TRUE(status);
                EXPECT_EQ(result.type, SMT::ResultType::UnSat);
                EXPECT_FALSE(result.model.has_value());
            }
        }
    }

    TEST_F(BooleanFunctionTest, Model) {
        const auto  a = BooleanFunction::Var("A"),
                    b = BooleanFunction::Var("B"),
                   _0 = BooleanFunction::Const(BooleanFunction::Value::ZERO),
                   _1 = BooleanFunction::Const(BooleanFunction::Value::ONE);

        auto formulas = std::vector<std::tuple<std::vector<SMT::Constraint>, SMT::Model>>({
            {
                {
                    SMT::Constraint(a & b, _1)
                },
                SMT::Model({{"A", {1, 1}}, {"B", {1, 1}}})
            },
            {
                {
                    SMT::Constraint(a | b, _1),
                    SMT::Constraint(b, _0),
                },
                SMT::Model({{"A", {1, 1}}, {"B", {0, 1}}})
            },
            {
                {
                    SMT::Constraint(a & b, _1),
                    SMT::Constraint(a, _1),
                },
                SMT::Model({{"A", {1, 1}}, {"B", {1, 1}}})
            },
            {
                {
                    SMT::Constraint((~a & b) | (a & ~b), _1),
                    SMT::Constraint(a, _1),
                },
                SMT::Model({{"A", {1, 1}}, {"B", {0, 1}}})
            }
        });

        for (auto&& [constraints, model] : formulas) {
            const auto solver = SMT::Solver(std::move(constraints));

            for (auto&& solver_type : {SMT::SolverType::Z3}) {
                if (!SMT::Solver::has_local_solver_for(solver_type)) {
                    continue;
                }

                auto [status, result] = solver.query(
                    SMT::QueryConfig()
                        .with_solver(solver_type)
                        .with_local_solver()
                        .with_model_generation()
                        .with_timeout(1000)
                );

                EXPECT_EQ(status, true);
                EXPECT_EQ(result.type, SMT::ResultType::Sat);
                EXPECT_EQ(*result.model, model);
            }
        }
    }
} //namespace hal
