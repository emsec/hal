#ifndef GRAPH_CONTEXT_H
#define GRAPH_CONTEXT_H

#include "gui/graph_widget/layouters/graph_layouter.h"
#include "gui/graph_widget/shaders/graph_shader.h"
#include "gui/gui_def.h"

#include <QObject>
#include <QSet>

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

    void add(const QSet<u32>& modules, const QSet<u32>& gates);
    void remove(const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& nets);
    void clear();

    void fold_module_of_gate(u32 id);
    void unfold_module(u32 id);

    const QSet<u32>& modules() const;
    const QSet<u32>& gates() const;
    const QSet<u32>& nets() const;

    graphics_scene* scene();

    QString name() const;

    void set_layouter(graph_layouter* layouter);
    void set_shader(graph_shader* shader);

    bool available() const;
    bool update_in_progress() const;

    void update();

    void request_update();

    bool node_for_gate(hal::node& node, const u32 id) const;

private Q_SLOTS:
    void handle_layouter_update(const int percent);
    void handle_layouter_update(const QString& message);
    void handle_layouter_finished();

    void handle_net_removed(std::shared_ptr<net>);

private:
    virtual void evaluate_changes();
    virtual void apply_changes();

    void update_scene();

    QList<graph_context_subscriber*> m_subscribers;

    QString m_name;

    graph_layouter* m_layouter;
    graph_shader* m_shader;

    QSet<u32> m_modules;
    QSet<u32> m_gates;
    QSet<u32> m_nets;

    QSet<u32> m_added_modules;
    QSet<u32> m_added_gates;
    QSet<u32> m_added_nets;

    QSet<u32> m_removed_modules;
    QSet<u32> m_removed_gates;
    QSet<u32> m_removed_nets;

    bool m_unhandled_changes;
    bool m_scene_update_required;
    u32 m_user_update_count;
    bool m_update_requested;
    bool m_scene_available;
    bool m_update_in_progress;
};

#endif // GRAPH_CONTEXT_H
