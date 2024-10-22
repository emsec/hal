// MIT License
// 
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include "hal_core/defines.h"

#include "gui/gui_globals.h"
#include "gui/gui_def.h"
#include "gui/content_widget/content_widget.h"

#include <deque>

namespace hal
{
    class WidgetOverlay;
    class GraphContext;
    class GraphGraphicsView;
    class SpinnerWidget;
    class SettingsItemSpinbox;
    class GraphNavigationWidget;
    class AbstractBusyIndicator;
    class CommentWidget;

    /**
     * @ingroup graph
     * @brief Wraps a GraphContext and a GraphGraphicsView.
     *
     * The content widget that wraps a graph view of a certain GraphContext. <br>
     * It represents one tab of the GraphTabWidgets QTabWidget.
     */
    class GraphWidget : public ContentWidget
    {
        Q_OBJECT

    public:
        /**
         * Constructor.
         *
         * @param context - The GraphContext of the view it.
         * @param parent - The parent widget.
         */
        explicit GraphWidget(GraphContext* context, QWidget* parent = nullptr);

        /**
         * Destructor.
         */
        ~GraphWidget();

        /**
         * Gets the GraphContext this GraphWidget shows.
         *
         * @returns a pointer of the context.
         */
        GraphContext* getContext() const;

        /**
         * Should be called whenever the scene becomes available (after a new layout). <br>
         * Used to enable interactions and show the loaded scene.
         */
        void handleSceneAvailable();
        /**
         * Should be called whenever the scene becomes unavailable (at the beginning of a new layout or a scene update). <br>
         * Used to disable interactions.
         */
        void handleSceneUnavailable();

        /**
         * Notifies that the context this graph widget manages is about to be deleted.
         */
        void handleContextAboutToBeDeleted();

        /**
         * Subscriber should store the scene mapping to viewport so it can be restored upon handleSceneAvailable
         */
        void storeViewport();

        /**
         * Get the GraphGraphicsView this object manages.
         *
         * @returns the GraphGraphicsView.
         */
        GraphGraphicsView* view();

        /**
         * Used to move and scale the camera of the view so that the whole selection can be seen.
         */
        void ensureSelectionVisible();

        /**
         * Show progress in overlay
         * @param percent percent done
         */
        void showProgress(int percent, const QString& text=QString());

        /**
         * Open overlay and show nodes
         * @param nd The node for which comments should be shown
         */
        void showComments(const Node& nd);

        void showBusy(int percent, const QString& text);

        void focusGate(u32 gateId);
        void focusNet(u32 netId);
        void focusModule(u32 moduleId);
        static void pluginProgressIndicator(int percent, const std::string& msg);

    protected:
        /**
         * Handle key presses within the graph widget <br>
         * Used to handle the key presses for the navigation.
         *
         * @param event - The key event
         */
        void keyPressEvent(QKeyEvent* event) override;

    private Q_SLOTS:
        void handleNavigationJumpRequested(const Node& origin, const u32 via_net, const QSet<u32>& to_gates, const QSet<u32>& to_modules);
        void handleModuleDoubleClicked(const u32 id);
        void resetFocus();
        void hideOverlay();

    private:
        void handleNavigationLeftRequest();
        void handleNavigationRightRequest();
        void handleNavigationUpRequest();
        void handleNavigationDownRequest();

        void substituteByVisibleModules(const QSet<u32>& gates, const QSet<u32>& modules, QSet<u32>& insert_gates, QSet<u32>& insert_modules,
                                           QSet<u32>& remove_gates, QSet<u32>& remove_modules) const;

        void handleEnterModuleRequested(const u32 id);

        void ensureItemsVisible(const QSet<u32>& gates, const QSet<u32>& modules);

        void focusRect(QRectF targetRect, bool applyCenterFix);

        bool hasFocusedItem() const;

        GraphGraphicsView* mView;
        GraphContext* mContext;

        WidgetOverlay* mOverlay;
        GraphNavigationWidget* mNavigationWidgetV3;
        AbstractBusyIndicator* mProgressBar;

        SpinnerWidget* mSpinnerWidget;
        CommentWidget* mCommentWidget;

        u32 mCurrentExpansion;

        QRectF mRectAfterFocus;
        QRectF mLastTargetRect;

        class StoreViewport
        {
        public:
            bool mValid;
            QRectF mRect;
            QPair<QPoint,QPointF> mGrid;
            StoreViewport() : mValid(false) {;}
        };

        QRectF restoreViewport(bool reset = true);

        StoreViewport mStoreViewport;

        static SettingsItemSpinbox* sSettingAnimationDuration;

        static GraphWidget* sInstance;

    };
}
