#ifndef CONE_CONTEXT_H
#define CONE_CONTEXT_H

#include "gui/graph_widget/contexts/graph_context.h"

class cone_context : public graph_context
{
public:
    cone_context();

//    const QSet<u32>& initial_modules() const;
//    const QSet<u32>& initial_gates() const;
    const QSet<u32>& initial_nets() const;
    const QSet<u32>& global_nets() const;
    const QSet<u32>& separated_nets() const;

private:
//    QSet<u32> m_initial_modules;
//    QSet<u32> m_initial_gates;
    QSet<u32> m_initial_nets;
    QSet<u32> m_global_nets;
    QSet<u32> m_separated_nets;
};

#endif // CONE_CONTEXT_H
