#ifndef LAYOUTER_TASK_H
#define LAYOUTER_TASK_H

#include "gui/thread_pool/task.h"

class graph_layouter;

class layouter_task : public task
{
public:
    layouter_task(graph_layouter* const layouter);

    void execute() override;

private:
    graph_layouter* m_layouter;
};

#endif // LAYOUTER_TASK_H
