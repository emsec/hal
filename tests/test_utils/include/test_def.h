#ifndef HAL_TEST_DEF_H_
#define HAL_TEST_DEF_H_

#include "hal_core/defines.h"
#include "gtest/gtest.h"
#include <sstream>

/**
 * Macros for catching error throws within the test
 */
#define TEST_START \
    try            \
    {
#define TEST_END                                                            \
    }                                                                       \
    catch (const std::runtime_error& re)                                    \
    {                                                                       \
        FAIL() << "Runtime error: " << re.what() << "\n";                   \
    }                                                                       \
    catch (const std::exception& ex)                                        \
    {                                                                       \
        FAIL() << "Error occurred: " << ex.what() << " -> Test failed!\n";  \
    }                                                                       \
    catch (...)                                                             \
    {                                                                       \
        FAIL() << "Unknown failure occurred. Possible memory corruption..." \
               << " -> Test failed!\n";                                     \
    }

/**
 * Marcro which prevents output for single function call
 */
#define NO_COUT(FUNC)                    \
    do                                   \
    {                                    \
        test_def::capture_stdout();      \
        FUNC;                            \
        test_def::get_captured_stdout(); \
    } while (0)

/**
 * Marcro which prevents output for the rest of the block
 */
#define NO_COUT_BLOCK test_def::no_cout_block noCoutBlock

/**
 * This macro is used in tests to print out the correct failure message, if a test fails, otherwise
 * all output is captured. It can only be used in tests. Can only be used inside tests.
 */
#define NO_COUT_TEST_BLOCK test_def::no_cout_test_block noCoutTestBlock(this->test_info_)

namespace test_def
{
    // Marks if output is currently captured by capture_stdout
    static bool output_captured = false;

    /**
     * Captures the std output until the call of get_captured_stdout
     */
    void capture_stdout();

    /*{
        if (!output_captured)
        {
            output_captured = true;
            ::testing::internal::CaptureStdout();
        }
    }*/

    /**
     * Get the captured output, if any is captured. Else return an empty string.
     *
     * @return  The captured output as a string
     */
    std::string get_captured_stdout();
    /*{
        if (output_captured)
        {
            output_captured = false;
            return ::testing::internal::GetCapturedStdout();
        }
        return "";
    }*/

    // Used by the no_cout_test_block as well as the no_cout_block class
    static int active = 0;
    /**
     * Captures the std output while at least one object exists. Uses the constructor and destructor mechanic to
     * capture output until the end of a block.
     */
    class no_cout_test_block
    {
    public:
        /**
         * Constructor. Prevents output until the destructor call.
         *
         * @param test_info - The information of the current test block
         */
        no_cout_test_block(testing::TestInfo* test_info) ;
        /*{
            capture_stdout();
            active++;
            m_test_info         = test_info;
            m_test_failed_start = m_test_info->result()->total_part_count();
        }*/

        /**
         * Destructor. Prints the captured output, if any test expectation fails. Otherwise the captured output is written.
         * (Only if it is the last no_cout_test_block object)
         * Since the failure messages of Google Test are captured as well, we need to print them separately.
         */
        ~no_cout_test_block() ;
        /*{
            active--;
            if (active == 0)
            {
                if (testing::Test::HasFailure())
                {
                    if (m_test_info->result()->total_part_count() > m_test_failed_start)
                    {
                        for (int i = m_test_failed_start; i < m_test_info->result()->total_part_count(); i++)
                        {
                            print_failure_message(m_test_info->result()->GetTestPartResult(i));
                        }
                        //print_captured_output(get_captured_stdout());
                    }
                }
                else
                {
                    get_captured_stdout();
                }
                //std::cout << first_test_index << std::endl;
            }
        }*/

    private:

        testing::TestInfo* m_test_info;
        int m_test_failed_start;

        /**
         * Helping function for printing the captured output.
         *
         * @param capOut - the captured output
         */
        void print_captured_output(std::string capOut);
        /*{
            if (capOut != "")
            {
                std::cout << "Test failed! Captured output(some errors might be expected):" << std::endl;
                std::istringstream ssCapOut;
                ssCapOut.str(capOut);
                for (std::string line; std::getline(ssCapOut, line);)
                {
                    std::cout << "[ " << line << " ]" << std::endl;
                }
            }
        }*/

        /**
         * Prints a single failure message form a TestPartResult object (from Google Test)
         *
         * @param pr
         */
        void print_failure_message(testing::TestPartResult pr);
        /*{
            std::cerr << "\n" << pr.file_name() << ":" << pr.line_number() << ": Failure" << pr.message() << std::endl;
        }*/
    };

    /**
     * Captures the std output while at least one object exists. Uses the constructor and destructor mechanic to
     * capture output until the end of a block. Should be used outside tests (e.g. helper functions). It is not
     * intended to access the output, captured by this class.
     */
    class no_cout_block
    {
    public:
        no_cout_block();
        /*{
            capture_stdout();
            active++;
        }*/
        ~no_cout_block();
        /*{
            active--;
            if (active == 0)
            {
                get_captured_stdout();
            }
        }*/
    };

}    // namespace test_def

#endif //HAL_TEST_DEF_H_
