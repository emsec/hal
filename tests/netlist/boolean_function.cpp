#include "netlist_test_utils.h"
#include "gtest/gtest.h"
#include "hal_core/netlist/boolean_function.h"
#include <iostream>
#include <type_traits>

namespace hal {

    class BooleanFunctionTest : public ::testing::Test {
    protected:

        const BooleanFunction::Value X = BooleanFunction::X;
        const BooleanFunction::Value ZERO = BooleanFunction::ZERO;
        const BooleanFunction::Value ONE = BooleanFunction::ONE;

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
                    case BooleanFunction::ONE:std::cout << "1";
                        break;
                    case BooleanFunction::ZERO:std::cout << "0";
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

        /**
         * Create a string to value map, that can be used by the evaluate function. Each variable MUST be one character long.
         * I.e: ("ABC","10X") creates the map: ("A" -> ONE, "B" -> ZERO, "C" -> X).
         *
         * If the inputs are invalid, an empty map s returned.
         *
         * @param variables - the names of the variables next to each others
         * @param values - the values the variables should be set to
         * @returns a variables to values map, that can be interpreted by the boolean funcitons evaluate function.
         */
        std::unordered_map<std::string, BooleanFunction::Value> create_input_map(std::string variables, std::string values) {
            std::unordered_map<std::string, BooleanFunction::Value> res;
            // Booth strings must be equal in length
            if (variables.size() != values.size()) {
                return res;
            }
            for (int c = 0; c < variables.size(); c++) {
                std::string var = std::string(1, variables.at(c));
                std::string val = std::string(1, values.at(c));
                // Can't set the same variable twice
                if (res.find(var) != res.end()) {
                    return {};
                }
                if (val == "0") {
                    res.insert(std::pair<std::string, BooleanFunction::Value>(var, BooleanFunction::ZERO));
                } else if (val == "1") {
                    res.insert(std::pair<std::string, BooleanFunction::Value>(var, BooleanFunction::ONE));
                } else if (val == "x" || val == "X") {
                    res.insert(std::pair<std::string, BooleanFunction::Value>(var, BooleanFunction::X));
                }
                    // If the values string contains an illegal character, exit
                else {
                    return {};
                }
            }
            return res;
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
            {
                // Constuctor with variables
                BooleanFunction a("A");
                BooleanFunction b("B");
                BooleanFunction c("C");
                // Constructor with constant
                BooleanFunction _1(ONE);

                // Combining them
                BooleanFunction r = ((a & b) | c) ^_1;

                EXPECT_EQ(r(create_input_map("ABC", "000")), ONE);
                EXPECT_EQ(r(create_input_map("ABC", "001")), ZERO);
                EXPECT_EQ(r(create_input_map("ABC", "010")), ONE);
                EXPECT_EQ(r(create_input_map("ABC", "011")), ZERO);

                EXPECT_EQ(r(create_input_map("ABC", "100")), ONE);
                EXPECT_EQ(r(create_input_map("ABC", "101")), ZERO);
                EXPECT_EQ(r(create_input_map("ABC", "110")), ZERO);
                EXPECT_EQ(r(create_input_map("ABC", "111")), ZERO);

                std::vector<BooleanFunction::Value>
                    truth_table = r.get_truth_table(std::vector<std::string>({"C", "B", "A"}));

                EXPECT_EQ(truth_table,
                          std::vector<BooleanFunction::Value>({ONE, ZERO, ONE, ZERO, ONE, ZERO, ZERO, ZERO}));
            }

        TEST_END
    }

    /**
     * Testing the to_string function
     *
     * Functions: to_string
     */
    TEST_F(BooleanFunctionTest, check_to_string) {
        TEST_START
            BooleanFunction a("A");
            BooleanFunction b("B");
            BooleanFunction c("C");
            BooleanFunction _0(ZERO);
            BooleanFunction _1(ONE);

            // Check some bf strings
            std::vector<std::pair<BooleanFunction, std::string>> test_cases =
                {
                    {(a & b), "A & B"},
                    {(a & (b | c)), "A & (B | C)"},
                    {((a & b) ^ (b & c)), "(A & B) ^ (B & C)"},
                    {(a ^ _1), "A ^ 1"},
                    {(a ^ _0), "A ^ 0"},
                    {(~a), "!A"}
                };

            for (auto tc : test_cases) {
                EXPECT_EQ(no_space(tc.first.to_string()), no_space(tc.second));
            }

            // Check an empty boolean function
            EXPECT_TRUE(test_utils::string_contains_substring(BooleanFunction().to_string(), "empty"));

        TEST_END
    }

    /**
     * Testing the functions is_constant_one and is_constant_zero, by passing some sample inputs
     *
     * Functions: is_constant_one, is_constant_zero
     */
    TEST_F(BooleanFunctionTest, check_is_constant) {
        TEST_START
            BooleanFunction a("A");
            BooleanFunction b("B");
            BooleanFunction c("C");
            BooleanFunction _0(ZERO);
            BooleanFunction _1(ONE);
            {
                // Some samples that are constant zero
                EXPECT_TRUE((_0).is_constant_zero());
                EXPECT_TRUE((~_1).is_constant_zero());
                EXPECT_TRUE((a ^ a).is_constant_zero());
                EXPECT_TRUE((a & (~a)).is_constant_zero());
                EXPECT_TRUE((_0 | _0).is_constant_zero());
            }
            {
                // Some samples that are constant one
                EXPECT_TRUE((_1).is_constant_one());
                EXPECT_TRUE((~_0).is_constant_one());
                EXPECT_TRUE((a ^ (~a)).is_constant_one());
                EXPECT_TRUE((a | (~a)).is_constant_one());
                EXPECT_TRUE((_1 & _1).is_constant_one());
            }
            {
                // Some samples that are NOT constant zero
                EXPECT_FALSE((_1).is_constant_zero());
                EXPECT_FALSE((a).is_constant_zero());
                EXPECT_FALSE((a ^ a ^ b).is_constant_zero());
                EXPECT_FALSE((a & b).is_constant_zero());
                EXPECT_FALSE((_0 | _1).is_constant_zero());
            }
            {
                // Some samples that are NOT constant one
                EXPECT_FALSE((_0).is_constant_one());
                EXPECT_FALSE((a).is_constant_one());
                EXPECT_FALSE((a ^ b ^ c).is_constant_one());
                EXPECT_FALSE((a & b).is_constant_one());
                EXPECT_FALSE((_0 & _1).is_constant_one());
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
                BooleanFunction not_empty("A");
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
                BooleanFunction a("A");
                BooleanFunction b("B");
                BooleanFunction c("C");
                BooleanFunction a_2("A");
                EXPECT_EQ((a | b | c | a_2).get_variables(), std::vector<std::string>({"A", "B", "C"}));
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
            // Tests for ==
            {
                // Compare the same object
                BooleanFunction a("A");
                EXPECT_TRUE((a == a));
            }
            {
                // The boolean functions are equivalent in syntax
                BooleanFunction a("A");
                BooleanFunction b("B");
                EXPECT_TRUE(((a | b) == (a | b)));
            }
            {
                // The boolean functions are equivalent in semantic (but not in syntax)
                BooleanFunction a("A");
                BooleanFunction b("B");
                // EXPECT_TRUE(((a|b|b) == (a|b)));
            }
            {
                // Compare two empty expressions
                BooleanFunction a = BooleanFunction();
                EXPECT_TRUE(a == BooleanFunction());
            }
            // Tests for !=
            {
                // The boolean function are equivalent in semantic, but do not share the same variable
                BooleanFunction a("A");
                BooleanFunction b("B");
                EXPECT_TRUE((a != b));
            }
            {
                // Compare boolean functions of different types (constant, variable, expression)
                BooleanFunction a("A");
                BooleanFunction b("B");
                BooleanFunction _1(ONE);
                EXPECT_TRUE((a != (a | (b & _1)))); // variable - expression
                EXPECT_TRUE((a != _1)); // variable - constant
                EXPECT_TRUE(((a | (b & _1)) != _1)); // expression - constant
            }
            {
                // Compare semantically different expressions
                BooleanFunction a("A");
                BooleanFunction b("B");
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
    TEST_F(BooleanFunctionTest, check_optimize) {
        TEST_START
            BooleanFunction a("A");
            BooleanFunction b("B");
            BooleanFunction c("C");
            BooleanFunction _0(ZERO);
            BooleanFunction _1(ONE);
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
                                           << "  optimized function: " << optimized << std::endl;
                    }

                    if (original_truth_table != optimized_truth_table) {
                        EXPECT_TRUE(false) << "ERROR: optimized function does not match original function" << std::endl
                                           << "  original function:  " << f << std::endl
                                           << "  DNF of function:    " << dnf << std::endl
                                           << "  optimized function: " << optimized << std::endl;
                    }
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
            BooleanFunction a("A"), b("B"), c("C"), d("D");
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
            BooleanFunction a("A"), b("B"), c("C"), d("D"), _0(ZERO), _1(ONE);
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
} //namespace hal
