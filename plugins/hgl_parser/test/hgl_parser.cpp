#include "gate_library_test_utils.h"
#include "hgl_parser/hgl_parser.h"
#include "netlist_test_utils.h"

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
            const GateLibrary* gl_original = test_utils::get_gate_library();
            ASSERT_NE(gl_parsed.get(), nullptr);
            ASSERT_NE(gl_original, nullptr);

            EXPECT_TRUE(test_utils::gate_libraries_are_equal(gl_parsed.get(), gl_original));
        }
        TEST_END
    }
}    //namespace hal
