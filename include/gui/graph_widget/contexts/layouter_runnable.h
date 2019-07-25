#ifndef LAYOUTER_RUNNABLE_H
#define LAYOUTER_RUNNABLE_H

#include <QObject>
#include <QRunnable>

class graph_layouter;

class layouter_runnable : public QObject, public QRunnable
{
    Q_OBJECT

public:
    layouter_runnable(graph_layouter* const layouter);

    void run() Q_DECL_OVERRIDE;

private:
    graph_layouter* m_layouter;
};

#endif // LAYOUTER_RUNNABLE_H
