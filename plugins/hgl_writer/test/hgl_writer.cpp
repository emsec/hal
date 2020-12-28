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
        }

        virtual void TearDown()
        {
        }
    };

    // TODO add testing
}    //namespace hal
