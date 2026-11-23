#include "gui/graph_widget/contexts/graph_context.h"

#include "gtest/gtest.h"
#include "netlist_test_utils.h"

#include <QLoggingCategory>

namespace hal
{
    /**
     * A graph context keeps the modules and gates it shows apart from the changes that are still pending. Pending
     * changes are held back while a change bracket is open, e.g., for as long as a script started from the Python
     * editor runs. The tests below never close the bracket, so nothing is applied and no layouter, shader or netlist
     * is needed.
     */
    class GraphContextTest : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            NO_COUT_BLOCK;
            test_utils::init_log_channels();

            // a context connects to the main window and the netlist relay on construction, neither of which exists here
            QLoggingCategory::setFilterRules("qt.core.qobject.connect=false");
        }

        virtual void TearDown()
        {
        }
    };

    /**
     * Test that whether a context shows the folded top module is decided by what it shows once its pending changes
     * are applied, also while none of them has been applied yet.
     *
     * Functions: GraphContext::isShowingFoldedTopModule
     */
    TEST_F(GraphContextTest, check_is_showing_folded_top_module_with_pending_changes)
    {
        TEST_START
        {
            // the top module has ID 1
            GraphContext ctx(1, "pending top module");
            ctx.beginChange();
            EXPECT_FALSE(ctx.isShowingFoldedTopModule());

            ctx.add({1}, {});
            EXPECT_TRUE(ctx.isShowingFoldedTopModule());
        }
        {
            // a single module other than the top module, as left behind when all gates of an unfolded module are moved
            // into a new submodule of it
            GraphContext ctx(2, "pending submodule");
            ctx.beginChange();
            ctx.add({5}, {});
            EXPECT_FALSE(ctx.isShowingFoldedTopModule());
        }
        {
            GraphContext ctx(3, "top module and a gate");
            ctx.beginChange();
            ctx.add({1}, {7});
            EXPECT_FALSE(ctx.isShowingFoldedTopModule());
        }
        {
            GraphContext ctx(4, "top module and a submodule");
            ctx.beginChange();
            ctx.add({1, 5}, {});
            EXPECT_FALSE(ctx.isShowingFoldedTopModule());
        }
        {
            // removing the top module again cancels its pending addition
            GraphContext ctx(5, "top module removed again");
            ctx.beginChange();
            ctx.add({1}, {});
            ctx.remove({1}, {});
            EXPECT_FALSE(ctx.isShowingFoldedTopModule());
        }
        TEST_END
    }
}    // namespace hal
