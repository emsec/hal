//  MIT License
//
//  Copyright (c) 2019 Ruhr-University Bochum, Germany, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once

#include "gui/graph_widget/layouters/graph_layouter.h"
#include "gui/graph_widget/shaders/graph_shader.h"
#include "gui/gui_def.h"

#include <QObject>
#include <QSet>
#include <QDateTime>

namespace hal
{
    class GraphContextSubscriber;

    class GraphContext : public QObject
    {
        friend class GraphContextManager;
        Q_OBJECT

    public:
        explicit GraphContext(const QString& name, QObject* parent = nullptr);
        ~GraphContext();

        void subscribe(GraphContextSubscriber* const subscriber);
        void unsubscribe(GraphContextSubscriber* const subscriber);

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

        GraphicsScene* scene();

        QString name() const;

        void set_layouter(GraphLayouter* layouter);
        void set_shader(GraphShader* shader);

        bool scene_update_in_progress() const;

        void schedule_scene_update();

        bool node_for_gate(hal::node& node, const u32 id) const;

        GraphLayouter* debug_get_layouter() const;

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

        QList<GraphContextSubscriber*> m_subscribers;

        QString m_name;

        GraphLayouter* m_layouter;
        GraphShader* m_shader;

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
}
