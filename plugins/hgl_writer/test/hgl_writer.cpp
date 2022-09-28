#include "gate_library_test_utils.h"
#include "hgl_parser/hgl_parser.h"
#include "hgl_writer/hgl_writer.h"
#include "netlist_test_utils.h"

namespace hal
{
    class HGLWriterTest : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            test_utils::init_log_channels();
            test_utils::create_sandbox_directory();
        }

        virtual void TearDown()
        {
            test_utils::remove_sandbox_directory();
        }
    };

    /**
     * Testing writing an HGL file.
     *
     * Functions: write
     */
    TEST_F(HGLWriterTest, check_library)
    {
        TEST_START
        {
            std::string path_lib     = test_utils::create_sandbox_path("test.hgl");
            const GateLibrary* gl_original = test_utils::get_gate_library();
            ASSERT_NE(gl_original, nullptr);

            HGLWriter writer;
            ASSERT_TRUE(writer.write(gl_original, path_lib));
            HGLParser parser;
            auto gl_res = parser.parse(path_lib);
            ASSERT_TRUE(gl_res.is_ok());
            std::unique_ptr<GateLibrary> gl_written = gl_res.get();
            ASSERT_NE(gl_written, nullptr);

            EXPECT_TRUE(test_utils::gate_libraries_are_equal(gl_original, gl_written.get()));
        }
        TEST_END
    }
}    //namespace hal
