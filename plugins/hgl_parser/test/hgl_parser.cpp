#include "hgl_parser/hgl_parser.h"

#include "gate_library_test_utils.h"
#include "netlist_test_utils.h"

#include <filesystem>

namespace hal
{
    class HGLParserTest : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            test_utils::init_log_channels();
        }

        virtual void TearDown()
        {
        }
    };

    /**
     * Testing parsing an HGL file.
     *
     * Functions: parse
     */
    TEST_F(HGLParserTest, check_library)
    {
        TEST_START
        {
            std::string path_lib = utils::get_base_directory().string() + "/bin/hal_plugins/test-files/test.hgl";

            HGLParser parser;
            auto gl_res = parser.parse(path_lib);
            ASSERT_TRUE(gl_res.is_ok());
            std::unique_ptr<GateLibrary> gl_parsed = gl_res.get();
            const GateLibrary* gl_original         = test_utils::get_gate_library();
            ASSERT_NE(gl_parsed.get(), nullptr);
            ASSERT_NE(gl_original, nullptr);

            EXPECT_TRUE(test_utils::gate_libraries_are_equal(gl_parsed.get(), gl_original));
        }
        TEST_END
    }

    /**
     * Parse every .hgl file found in the installed gate library directories and
     * verify that none of them produce a parser error.
     *
     * Functions: parse
     */
    TEST_F(HGLParserTest, check_all_gate_libraries)
    {
        TEST_START
        {
            const auto gl_dirs = utils::get_gate_library_directories();

            std::vector<std::filesystem::path> hgl_files;
            for (const auto& dir : gl_dirs)
            {
                if (!std::filesystem::exists(dir))
                    continue;
                for (const auto& entry : std::filesystem::recursive_directory_iterator(dir))
                {
                    if (entry.is_regular_file() && entry.path().extension() == ".hgl")
                        hgl_files.push_back(entry.path());
                }
            }

            ASSERT_FALSE(hgl_files.empty()) << "No .hgl files found in gate library directories — check build configuration.";

            HGLParser parser;
            for (const auto& path : hgl_files)
            {
                auto gl_res = parser.parse(path.string());
                EXPECT_TRUE(gl_res.is_ok()) << "Failed to parse gate library: " << path << "\n" << (gl_res.is_error() ? gl_res.get_error().get() : "");
                if (gl_res.is_ok())
                    EXPECT_NE(gl_res.get(), nullptr) << "Parser returned null for: " << path;
            }
        }
        TEST_END
    }
}    //namespace hal
