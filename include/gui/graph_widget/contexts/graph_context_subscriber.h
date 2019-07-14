#ifndef GRAPH_CONTEXT_SUBSCRIBER_H
#define GRAPH_CONTEXT_SUBSCRIBER_H

class graph_context_subscriber
{
public:
    virtual void handle_scene_available() = 0;
    virtual void handle_scene_unavailable() = 0;
    virtual void handle_context_about_to_be_deleted() = 0;
};

#endif // GRAPH_CONTEXT_SUBSCRIBER_H
