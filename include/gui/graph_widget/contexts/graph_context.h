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
    Q_OBJECT

public:
    enum class context_type
    {
        module,
        dynamic
    };

    explicit graph_context(context_type type, graph_layouter* layouter, graph_shader* shader, QObject* parent = nullptr);
    ~graph_context();

    virtual void handle_navigation_left(graph_context_subscriber* const subscriber) = 0;
    virtual void handle_navigation_right(graph_context_subscriber* const subscriber) = 0;

    void subscribe(graph_context_subscriber* const subscriber);
    void unsubscribe(graph_context_subscriber* const subscriber);

    void add(const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& nets);
    void remove(const QSet<u32>& modules, const QSet<u32>& gates, const QSet<u32>& nets);

    const QSet<u32>& modules() const;
    const QSet<u32>& gates() const;
    const QSet<u32>& nets() const;

    graphics_scene* scene();

    context_type get_type();

    // PROBABLY OBSOLETE
    //graph_layouter* layouter();
    //graph_shader* shader();

    bool available() const;
    bool update_in_progress() const;

    void update();

    void request_update();

    bool node_for_gate(hal::node& node, const u32 id) const;

private Q_SLOTS:
    void handle_layouter_update(const int percent);
    void handle_layouter_update(const QString& message);
    void handle_layouter_finished();

protected:
    QSet<u32> m_modules;
    QSet<u32> m_gates;
    QSet<u32> m_nets;

    graph_layouter* m_layouter;
    graph_shader* m_shader; // MOVE SHADER TO VIEW ? USE BASE SHADER AND ADDITIONAL SHADERS ? LAYER SHADERS ?

    bool m_unhandled_changes;
    bool m_scene_update_required;

    bool m_update_requested;

private:
    void evaluate_changes();
    void apply_changes();
    void update_scene();

    const context_type m_type;

    QSet<u32> m_added_modules;
    QSet<u32> m_added_gates;
    QSet<u32> m_added_nets;

    QSet<u32> m_removed_modules;
    QSet<u32> m_removed_gates;
    QSet<u32> m_removed_nets;

    QList<graph_context_subscriber*> m_subscribers;

    bool m_scene_available;
    bool m_update_in_progress;
};

#endif // GRAPH_CONTEXT_H
