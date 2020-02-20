#ifndef GRAPH_CONTEXT_MANAGER_H
#define GRAPH_CONTEXT_MANAGER_H

#include "def.h"

#include <QObject>
#include <QStringList>
#include <QVector>

class gate;
class module;
class net;

class graph_context;
class graph_layouter;
class graph_shader;
class graph_context;

class graph_context_manager : public QObject
{
    Q_OBJECT

public:
    graph_context_manager();

    graph_context* create_new_context(const QString& name);
    void rename_graph_context(graph_context* ctx, const QString& new_name);
    void delete_graph_context(graph_context* ctx);
    QVector<graph_context*> get_contexts() const;
    bool context_with_name_exists(const QString& name) const;

    //void handle_module_created(const std::shared_ptr<module> m) const;
    void handle_module_removed(const std::shared_ptr<module> m);
    void handle_module_name_changed(const std::shared_ptr<module> m) const;
    void handle_module_color_changed(const std::shared_ptr<module> m) const;
    //void handle_module_parent_changed(const std::shared_ptr<module> m) const;
    void handle_module_submodule_added(const std::shared_ptr<module> m, const u32 added_module) const;
    void handle_module_submodule_removed(const std::shared_ptr<module> m, const u32 removed_module);
    void handle_module_gate_assigned(const std::shared_ptr<module> m, const u32 inserted_gate) const;
    void handle_module_gate_removed(const std::shared_ptr<module> m, const u32 removed_gate);

    //void handle_gate_created(const std::shared_ptr<gate> g) const;
    //void handle_gate_removed(const std::shared_ptr<gate> g) const;
    void handle_gate_name_changed(const std::shared_ptr<gate> g) const;

    void handle_net_created(const std::shared_ptr<net> n) const;
    void handle_net_removed(const std::shared_ptr<net> n) const;
    void handle_net_name_changed(const std::shared_ptr<net> n) const;
    void handle_net_source_changed(const std::shared_ptr<net> n) const;
    void handle_net_destination_added(const std::shared_ptr<net> n, const u32 dst_gate_id) const;
    void handle_net_destination_removed(const std::shared_ptr<net> n, const u32 dst_gate_id) const;
    void handle_marked_global_input(u32 net_id);
    void handle_marked_global_output(u32 net_id);
    void handle_unmarked_global_input(u32 net_id);
    void handle_unmarked_global_output(u32 net_id);

    graph_layouter* get_default_layouter(graph_context* const context) const;
    graph_shader* get_default_shader(graph_context* const context) const;

Q_SIGNALS:
    void context_created(graph_context* context);
    void context_renamed(graph_context* context);
    void deleting_context(graph_context* context);

private:
    QVector<graph_context*> m_graph_contexts;
};

#endif // GRAPH_CONTEXT_MANAGER_H
