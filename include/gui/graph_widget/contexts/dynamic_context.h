#ifndef DYNAMIC_CONTEXT_H
#define DYNAMIC_CONTEXT_H

#include "gui/graph_widget/contexts/graph_context.h"

class dynamic_context : public graph_context
{
public:
    dynamic_context(const QString& name, const u32 scope = 0);

    QString name() const;
    u32 scope() const;

private:
    QString m_name;
    u32 m_scope;

    bool m_extend_gnd;
    bool m_extend_vcc;
    bool m_extend_global;
};

#endif // DYNAMIC_CONTEXT_H
