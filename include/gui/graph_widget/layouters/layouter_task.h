#pragma once

#include "gui/thread_pool/task.h"
namespace hal{
class graph_layouter;

class layouter_task : public task
{
public:
    layouter_task(graph_layouter* const layouter);

    void execute() override;

private:
    graph_layouter* m_layouter;
};
}
