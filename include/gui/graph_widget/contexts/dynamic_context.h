#ifndef DYNAMIC_CONTEXT_H
#define DYNAMIC_CONTEXT_H

#include "gui/graph_widget/contexts/graph_context.h"

class dynamic_context : public graph_context
{
public:
    dynamic_context(const QString& name);

    QString name() const;

private:
    QString m_name;

    bool m_extend_gnd;
    bool m_extend_vcc;
    bool m_extend_global;
};

#endif // DYNAMIC_CONTEXT_H
