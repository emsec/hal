#ifndef MODULE_CONTEXT_H
#define MODULE_CONTEXT_H

#include "netlist/module.h"

#include "gui/graph_widget/contexts/graph_context.h"

class module_context : public graph_context
{
public:
    module_context(const std::shared_ptr<const module> m);

    virtual void handle_navigation_left(graph_context_subscriber* const subscriber) Q_DECL_OVERRIDE;
    virtual void handle_navigation_right(graph_context_subscriber* const subscriber) Q_DECL_OVERRIDE;

    u32 get_id() const;

private:
    const u32 m_id;
};

#endif // MODULE_CONTEXT_H
