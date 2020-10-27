#include <cstdio>
#include <algorithm>
#include <cctype>
#include "gtest/gtest.h"
#include "netlist_test_utils.h"

// Processes custom command line arguments
void handle_command_line_arguments(int argc, char **argv)
{
    for (int i = 1; i < argc; i++){
        std::string arg = argv[i];
        std::transform(arg.begin(), arg.end(), arg.begin(), [](unsigned char c){return std::tolower(c); });
        if(arg == "--run_known_issue_tests" || arg == "--run-known-issue-tests")
        {
            hal::test_utils::activate_known_issue_tests();
        }
    }
}

GTEST_API_ int main(int argc, char **argv) {
    printf("Running main() from %s\n", __FILE__);
    handle_command_line_arguments(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

