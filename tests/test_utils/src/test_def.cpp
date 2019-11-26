#include "test_def.h"

void test_def::capture_stdout()
{
    if (!output_captured)
    {
        output_captured = true;
        ::testing::internal::CaptureStdout();
    }
}

std::string test_def::get_captured_stdout()
{
    if (output_captured)
    {
        output_captured = false;
        return ::testing::internal::GetCapturedStdout();
    }
    return "";
}


// ====== no_cout_test_block definition ======

test_def::no_cout_test_block::no_cout_test_block(testing::TestInfo *test_info)
{
    capture_stdout();
    active++;
    m_test_info         = test_info;
    m_test_failed_start = m_test_info->result()->total_part_count();
}

test_def::no_cout_test_block::~no_cout_test_block()
{
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
}

void test_def::no_cout_test_block::print_captured_output(std::string capOut)
{
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
}

void test_def::no_cout_test_block::print_failure_message(testing::TestPartResult pr)
{
    std::cerr << "\n" << pr.file_name() << ":" << pr.line_number() << ": Failure" << pr.message() << std::endl;
}


// === no_cout_block definition ===

test_def::no_cout_block::no_cout_block()
{
    capture_stdout();
    active++;
}

test_def::no_cout_block::~no_cout_block()
{
    active--;
    if (active == 0)
    {
        get_captured_stdout();
    }
}
