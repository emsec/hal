#ifndef DYNAMIC_CONTEXT_H
#define DYNAMIC_CONTEXT_H

#include "gui/graph_widget/contexts/graph_context.h"

class dynamic_context : public graph_context
{
public:
    dynamic_context(const QString& name);

    virtual void handle_navigation_left(graph_context_subscriber* const subscriber) Q_DECL_OVERRIDE;
    virtual void handle_navigation_right(graph_context_subscriber* const subscriber) Q_DECL_OVERRIDE;

    QString name() const;

private:
    QString m_name;

    bool m_extend_gnd;
    bool m_extend_vcc;
    bool m_extend_global;
};

#endif // DYNAMIC_CONTEXT_H
