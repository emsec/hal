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

        void beginChange();
        void endChange();

        void add(const QSet<u32>& modules, const QSet<u32>& gates, PlacementHint placement = PlacementHint());
        void remove(const QSet<u32>& modules, const QSet<u32>& gates);
        void clear();

        void foldModuleOfGate(const u32 id);
        void unfoldModule(const u32 id);

        bool empty() const;
        bool isShowingModule(const u32 id) const;
        bool isShowingModule(const u32 id, const QSet<u32>& minus_modules, const QSet<u32>& minus_gates, const QSet<u32>& plus_modules, const QSet<u32>& plus_gates) const;

        bool isShowingNetSource(const u32 mNetId) const;
        bool isShowingNetDestination(const u32 mNetId) const;

        const QSet<u32>& modules() const;
        const QSet<u32>& gates() const;
        const QSet<u32>& nets() const;

        GraphicsScene* scene();

        QString name() const;

        void setLayouter(GraphLayouter* layouter);
        void setShader(GraphShader* shader);

        bool sceneUpdateInProgress() const;

        void scheduleSceneUpdate();

        Node nodeForGate(const u32 id) const;

        GraphLayouter* debugGetLayouter() const;

        QDateTime getTimestamp() const;

    private Q_SLOTS:
        void handleLayouterUpdate(const int percent);
        void handleLayouterUpdate(const QString& message);
        void handleLayouterFinished();

    private:
        void evaluateChanges();
        void update();
        void applyChanges();
        void startSceneUpdate();

        QList<GraphContextSubscriber*> mSubscribers;

        QString mName;

        GraphLayouter* mLayouter;
        GraphShader* mShader;

        QSet<u32> mModules;
        QSet<u32> mGates;
        QSet<u32> mNets;

        QSet<u32> mAddedModules;
        QSet<u32> mAddedGates;

        QMultiMap<PlacementHint, u32> mModuleHints;
        QMultiMap<PlacementHint, u32> mGateHints;

        QSet<u32> mRemovedModules;
        QSet<u32> mRemovedGates;

        u32 mUserUpdateCount;

        bool mUnappliedChanges;
        bool mSceneUpdateRequired;
        bool mSceneUpdateInProgress;

        QDateTime mTimestamp;
    };
}
