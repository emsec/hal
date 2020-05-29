#ifndef GRAPH_CONTEXT_H
#define GRAPH_CONTEXT_H

#include "gui/graph_widget/layouters/graph_layouter.h"
#include "gui/graph_widget/shaders/graph_shader.h"
#include "gui/gui_def.h"

#include <QObject>
#include <QSet>
#include <QDateTime>

class graph_context_subscriber;

class graph_context : public QObject
{
    friend class graph_context_manager;
    Q_OBJECT

public:
    explicit graph_context(const QString& name, QObject* parent = nullptr);
    ~graph_context();

    void subscribe(graph_context_subscriber* const subscriber);
    void unsubscribe(graph_context_subscriber* const subscriber);

    void begin_change();
    void end_change();

    void add(const QSet<u32>& modules, const QSet<u32>& gates, hal::placement_hint placement = hal::placement_hint{hal::placement_mode::standard, hal::node()});
    void remove(const QSet<u32>& modules, const QSet<u32>& gates);
    void clear();

    void fold_module_of_gate(const u32 id);
    void unfold_module(const u32 id);

    bool empty() const;
    bool is_showing_module(const u32 id) const;
    bool is_showing_module(const u32 id, const QSet<u32>& minus_modules, const QSet<u32>& minus_gates, const QSet<u32>& plus_modules, const QSet<u32>& plus_gates) const;

    bool is_showing_net_source(const u32 net_id) const;
    bool is_showing_net_destination(const u32 net_id) const;

    const QSet<u32>& modules() const;
    const QSet<u32>& gates() const;
    const QSet<u32>& nets() const;

    graphics_scene* scene();

    QString name() const;

    void set_layouter(graph_layouter* layouter);
    void set_shader(graph_shader* shader);

    bool scene_update_in_progress() const;

    void schedule_scene_update();

    bool node_for_gate(hal::node& node, const u32 id) const;

    graph_layouter* debug_get_layouter() const;

    QDateTime get_timestamp() const;

private Q_SLOTS:
    void handle_layouter_update(const int percent);
    void handle_layouter_update(const QString& message);
    void handle_layouter_finished();

private:
    void evaluate_changes();
    void update();
    void apply_changes();
    void start_scene_update();

    QList<graph_context_subscriber*> m_subscribers;

    QString m_name;

    graph_layouter* m_layouter;
    graph_shader* m_shader;

    QSet<u32> m_modules;
    QSet<u32> m_gates;
    QSet<u32> m_nets;

    QSet<u32> m_added_modules;
    QSet<u32> m_added_gates;

    QMultiMap<hal::placement_hint, u32> m_module_hints;
    QMultiMap<hal::placement_hint, u32> m_gate_hints;

    QSet<u32> m_removed_modules;
    QSet<u32> m_removed_gates;

    u32 m_user_update_count;

    bool m_unapplied_changes;
    bool m_scene_update_required;
    bool m_scene_update_in_progress;

    QDateTime m_timestamp;
};

#endif // GRAPH_CONTEXT_H
