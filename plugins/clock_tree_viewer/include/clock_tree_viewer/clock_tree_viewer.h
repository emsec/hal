// MIT License
//
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
// Copyright (c) 2025-2026 Sascha Tommasone. All rights reserved.
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

#include "clock_tree_viewer/clock_tree_layout.h"
#include "gui/content_manager/content_manager.h"
#include "hal_core/utilities/result.h"

#include <QHash>
#include <QSet>
#include <QString>
#include <cstddef>
#include <vector>

class QAction;
class QColor;
class QLabel;
class QStackedWidget;
class QStatusBar;
class QTimer;

namespace hal
{
    namespace cte
    {
        class ClockTree;
    }

    class ClockTreeGraphicsView;
    class ClockTreeScene;
    struct ClockTreeColors;
    class Toolbar;

    /**
     * The content widget that plots a clock tree at the coordinates of its gates.
     *
     * The plot is in the colors of HAL, as in the graph view. The background and the lines follow the theme. A gate has
     * the color of the module it belongs to, the color of its grouping if it is in one, or the color of a selected item
     * if it is highlighted, in that order of precedence. The plot follows all of them when they change.
     *
     * Gates are highlighted by the color of a selected item and by a ring around them, which has the same size on the
     * screen at every zoom. The gates that are selected in HAL are highlighted, as far as they are in the plot, and the
     * highlight follows the selection. This goes one way only, from the netlist to the viewer: nothing done in the viewer
     * changes the selection in HAL. Scripts can highlight gates, too, see `highlightGates`.
     *
     * The viewer does not keep the clock tree it is given: it copies what it draws, so it stays valid when the clock
     * tree or its netlist is gone.
     */
    class ClockTreeViewer : public ExternalContentWidget
    {
        Q_OBJECT

      public:
        /** The name of the plugin the viewer belongs to. */
        static constexpr auto sPluginName = "clock_tree_viewer";

        /**
         * Construct a new clock tree viewer, which shows nothing yet.
         *
         * @param[in] parent - The parent widget. Defaults to a `nullptr`.
         */
        explicit ClockTreeViewer( QWidget *parent = nullptr );

        /**
         * Get the clock tree viewer that is currently open.
         *
         * @returns The viewer, a `nullptr` if none is open.
         */
        static ClockTreeViewer *instance();

        /**
         * Add the actions of the viewer to the given toolbar.
         *
         * @param[in] toolbar - The toolbar to add the actions to.
         */
        void setupToolbar( Toolbar *toolbar ) override;

        /**
         * Plot a clock tree.
         *
         * The gates are drawn at their coordinates, the sizes of the drawing follow from them unless provided. If no gate
         * has coordinates, nothing is drawn and the error is shown instead. If only some have none, those are left out
         * and a warning is shown.
         *
         * @param[in] tree - The clock tree.
         * @param[in] overrides - The sizes of the drawing. Whatever is not provided is derived from the coordinates.
         * @returns `true` if the clock tree is drawn, `false` otherwise.
         */
        bool setClockTree( const cte::ClockTree &tree, const ctv::StyleOverrides &overrides = ctv::StyleOverrides() );

        /**
         * Save a picture of the whole clock tree that is displayed, independent of the zoom and the size of the window.
         * Must be called from the GUI thread.
         *
         * @param[in] filename - The file to write. The suffix decides the format, e.g. `.png`.
         * @returns `true` on success, `false` otherwise.
         */
        bool savePicture( const QString &filename );

        /**
         * Highlight gates in the plot, instead of the ones that were highlighted before. Gates that are not in the plot are
         * ignored, as are those that are in the clock tree but have no coordinates, so cannot be drawn. The highlight is
         * replaced as soon as the selection in HAL changes.
         *
         * @param[in] gateIds - The IDs of the gates. Pass none to remove the highlight.
         * @returns The number of gates that are highlighted now.
         */
        std::size_t highlightGates( const std::vector<u32> &gateIds );

        /**
         * Get the IDs of the gates that are highlighted. Must be called from the GUI thread.
         *
         * @returns The gates, which are all in the plot.
         */
        [[nodiscard]] const QSet<u32> &highlightedGates() const
        {
            return mHighlightedGates;
        }

      private Q_SLOTS:
        /**
         * Extract the clock tree of the loaded netlist and plot it.
         */
        void handleExtractClicked();

        /**
         * Zoom so that the whole tree is visible.
         */
        void handleFitClicked() const;

        /**
         * Ask for a file and save a picture of the tree to it.
         */
        void handleSavePictureClicked();

        /**
         * Zoom so that the highlighted gates are visible.
         */
        void handleZoomToHighlightClicked();

        /**
         * Highlight the gates that are selected in HAL, as far as they are in the plot.
         */
        void handleSelectionChanged();

        /**
         * Show the chip coordinates under the mouse in the status bar.
         *
         * @param[in] x - The x coordinate.
         * @param[in] y - The y coordinate.
         */
        void handleCoordinatesChanged( double x, double y );

        /**
         * Paint the plot in the colors of the theme that was chosen, which has just been applied.
         */
        void handleStyleChanged();

        /**
         * Paint the plot in the colors HAL has for its gates now.
         */
        void recolor();

        /**
         * Paint the plot anew soon. Many changes that happen at once, e.g., of a selection, cost a single repaint.
         */
        void scheduleRecolor();

      private:
        /**
         * Show a clock tree that was laid out already. Must be called from the GUI thread.
         */
        bool display( const Result<ctv::Layout> &layout, const ctv::StyleOverrides &overrides );

        /**
         * Show an error instead of a drawing and log it. Must be called from the GUI thread.
         *
         * @returns Always `false`, so callers can return it.
         */
        bool showError( const QString &message );

        /**
         * Show a warning below the drawing. An empty message hides the warning.
         */
        void showWarning( const QString &message );

        /**
         * Enable the actions that need a drawing only if there is one.
         */
        void updateActions();

        /**
         * Highlight the gates of the plot that are selected in HAL, without painting yet.
         */
        void highlightFromSelection();

        /**
         * Highlight gates in the plot and paint them. Must be called from the GUI thread.
         *
         * @returns The number of gates that are highlighted now.
         */
        std::size_t applyHighlight( const std::vector<u32> &gateIds );

        /**
         * Get the colors HAL gives the lines and the gates of the plot at this moment.
         */
        [[nodiscard]] ClockTreeColors currentColors() const;

        /**
         * Get the color HAL gives a gate.
         *
         * @param[in] gateId - The ID of the gate.
         * @param[in] fallback - The color for a gate that HAL does not know.
         */
        [[nodiscard]] QColor gateColor( u32 gateId, const QColor &fallback ) const;

        /**
         * Get the color behind the plot, which is up to the theme.
         */
        [[nodiscard]] QColor backgroundColor() const;

        ClockTreeScene *mScene;
        ClockTreeGraphicsView *mView;
        QStackedWidget *mStack;
        QLabel *mMessageLabel;
        QStatusBar *mStatusBar;
        QLabel *mCoordinateLabel;
        QLabel *mWarningLabel;
        QLabel *mHighlightLabel;

        QAction *mExtractAction;
        QAction *mFitAction;
        QAction *mSavePictureAction;
        QAction *mZoomToHighlightAction;

        QTimer *mRecolorTimer;

        /** The gates that are plotted, to find the color of each of them again. */
        ctv::Layout mLayout;

        /** The position of every gate that is plotted in `mLayout`, by its ID. */
        QHash<u32, int> mNodeOfGate;

        /** The gates that are highlighted, all of them are plotted. */
        QSet<u32> mHighlightedGates;

        ctv::StyleOverrides mOverrides;
    };
}  // namespace hal
