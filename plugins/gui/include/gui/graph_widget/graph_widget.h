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

#include "hal_core/defines.h"

#include "gui/gui_globals.h"
#include "gui/gui_def.h"
#include "gui/content_widget/content_widget.h"
#include "gui/graph_widget/contexts/graph_context_subscriber.h"

#include <deque>

namespace hal
{
    class WidgetOverlay;
    class GraphContext;
    class GraphGraphicsView;
    class GraphLayoutSpinnerWidget;
    class GraphNavigationWidget;
    class GraphNavigationWidgetV2;

    class GraphWidget : public ContentWidget, public GraphContextSubscriber
    {
        Q_OBJECT

    public:
        explicit GraphWidget(GraphContext* context, QWidget* parent = nullptr);

        GraphContext* getContext() const;

        void handleSceneAvailable() override;
        void handleSceneUnavailable() override;
        void handleContextAboutToBeDeleted() override;

        void handleStatusUpdate(const int percent) override;
        void handleStatusUpdate(const QString& message) override;

        GraphGraphicsView* view();

        void ensureSelectionVisible();

    protected:
        void keyPressEvent(QKeyEvent* event) override;

    private Q_SLOTS:
        void handleNavigationJumpRequested(const Node& origin, const u32 via_net, const QSet<u32>& to_gates, const QSet<u32>& to_modules);
        void handleModuleDoubleClicked(const u32 id);
        void resetFocus();

    private:
        void handleNavigationLeftRequest();
        void handleNavigationRightRequest();
        void handleNavigationUpRequest();
        void handleNavigationDownRequest();

        void substituteByVisibleModules(const QSet<u32>& gates, const QSet<u32>& modules, QSet<u32>& insert_gates, QSet<u32>& insert_modules,
                                           QSet<u32>& remove_gates, QSet<u32>& remove_modules) const;
        void setModifiedIfModule();

        void handleEnterModuleRequested(const u32 id);

        void ensureItemsVisible(const QSet<u32>& gates, const QSet<u32>& modules);

        GraphGraphicsView* mView;
        GraphContext* mContext;

        WidgetOverlay* mOverlay;
        GraphNavigationWidgetV2* mNavigationWidgetV2;
        GraphLayoutSpinnerWidget* mSpinnerWidget;

        u32 mCurrentExpansion;
    };
}
