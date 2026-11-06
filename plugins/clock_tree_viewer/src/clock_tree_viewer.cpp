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

#include "clock_tree_viewer/clock_tree_viewer.h"

#include "clock_tree_extractor/clock_tree.h"
#include "clock_tree_viewer/clock_tree_graphics_view.h"
#include "clock_tree_viewer/clock_tree_scene.h"
#include "gui/file_manager/file_manager.h"
#include "gui/graph_widget/graphics_qss_adapter.h"
#include "gui/grouping/grouping_manager_widget.h"
#include "gui/grouping/grouping_table_model.h"
#include "gui/gui_def.h"
#include "gui/gui_globals.h"
#include "gui/gui_utils/graphics.h"
#include "gui/logger/logger_qss_adapter.h"
#include "gui/main_window/main_window.h"
#include "gui/settings/settings_items/settings_item_dropdown.h"
#include "gui/toolbar/toolbar.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/module.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/utilities/log.h"

#include <QAction>
#include <QColor>
#include <QFileDialog>
#include <QFileInfo>
#include <QGuiApplication>
#include <QLabel>
#include <QLocale>
#include <QMessageBox>
#include <QMetaObject>
#include <QPalette>
#include <QProgressDialog>
#include <QRegularExpression>
#include <QStackedWidget>
#include <QStatusBar>
#include <QStringList>
#include <QStyle>
#include <QThread>
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>
#include <exception>
#include <memory>

namespace hal
{
    namespace
    {
        // How many of the gates that cannot be drawn are named in the log.
        constexpr std::size_t MAX_LISTED_GATES = 20;

        // The color of a selected item in the graph view. It is GraphicsItem::sSelectionColor, which is protected, so this
        // is the one place it is repeated: everything in the viewer that shows a highlight takes it from here.
        constexpr QColor HAL_SELECTION_COLOR( 240, 173, 0 );

        // For a theme that does not define the colors of warnings and errors in the log.
        constexpr QColor FALLBACK_WARNING_COLOR( 217, 164, 65 );
        constexpr QColor FALLBACK_ERROR_COLOR( 217, 83, 79 );

        QString text_color_style( const QColor &color, const QColor &fallback )
        {
            return QString( "color: %1;" ).arg( ( color.isValid() ? color : fallback ).name() );
        }

        /**
         * Lay out a clock tree, but only one of the netlist that is open: the viewer colors and highlights the gates by
         * their IDs in that netlist.
         */
        Result<ctv::Layout> layout_of( const cte::ClockTree &tree,
                                       const Netlist *open_netlist,
                                       const std::optional<std::string> &x_key,
                                       const std::optional<std::string> &y_key )
        {
            if( open_netlist != nullptr && tree.get_netlist() != open_netlist )
            {
                return ERR( "the clock tree was extracted from another netlist than the one that is open, so its gates "
                            "cannot be matched to the gates in HAL" );
            }

            return ctv::build_layout( tree, x_key, y_key );
        }

        /**
         * Extract the clock tree of a netlist and lay it out. Runs in a thread of its own, so it must not touch a widget.
         */
        Result<ctv::Layout> extract_layout( const Netlist *netlist,
                                            const std::optional<std::string> &x_key,
                                            const std::optional<std::string> &y_key )
        {
            // An exception escaping the thread would end the program.
            try
            {
                const Result<std::unique_ptr<cte::ClockTree>> tree = cte::ClockTree::from_netlist( netlist );
                if( tree.is_error() )
                {
                    return ERR( tree.get_error().get() );
                }

                return layout_of( *tree.get(), netlist, x_key, y_key );
            } catch( const std::exception &e )
            {
                return ERR( std::string( "the extraction failed: " ) + e.what() );
            }
        }

        constexpr auto IDLE_TEXT = "No clock tree to show.\n\n"
                                   "Extract the clock tree of the loaded netlist with the button in the toolbar,\n"
                                   "or pass a clock tree to clock_tree_viewer.show_clock_tree() in Python.";

        QString list_gate_ids( const std::vector<u32> &ids )
        {
            QStringList listed;
            for( std::size_t idx = 0; idx < std::min( ids.size(), MAX_LISTED_GATES ); idx++ )
            {
                listed.push_back( QString::number( ids[idx] ) );
            }

            QString result = listed.join( ", " );
            if( ids.size() > MAX_LISTED_GATES )
            {
                result += QString( " and %1 more" ).arg( ids.size() - MAX_LISTED_GATES );
            }
            return result;
        }
    }  // namespace

    ClockTreeViewer::ClockTreeViewer( QWidget *parent )
        : ExternalContentWidget( sPluginName, "ClockTreeViewer", parent )
    {
        mExtractAction = new QAction( this );
        mExtractAction->setToolTip( "Extract the clock tree of the netlist and plot it" );
        mExtractAction->setIcon( gui_utility::getStyledSvgIcon( "all->#20FF80", ":/icons/run" ) );
        connect( mExtractAction, &QAction::triggered, this, &ClockTreeViewer::handleExtractClicked );

        mFitAction = new QAction( this );
        mFitAction->setToolTip( "Zoom to the whole clock tree" );
        mFitAction->setIcon( QIcon( ":/icons/zoom_waveform" ) );
        connect( mFitAction, &QAction::triggered, this, &ClockTreeViewer::handleFitClicked );

        mZoomToHighlightAction = new QAction( this );
        mZoomToHighlightAction->setToolTip( "Zoom to the highlighted gates" );
        mZoomToHighlightAction->setIcon( gui_utility::getStyledSvgIcon(
            QString( "all->%1" ).arg( QColor( HAL_SELECTION_COLOR ).name() ), ":/icons/to_selection" ) );
        connect( mZoomToHighlightAction, &QAction::triggered, this, &ClockTreeViewer::handleZoomToHighlightClicked );

        mSavePictureAction = new QAction( this );
        mSavePictureAction->setToolTip( "Save a picture of the clock tree" );
        mSavePictureAction->setIcon( gui_utility::getStyledSvgIcon( "all->#3192C5", ":/icons/save" ) );
        connect( mSavePictureAction, &QAction::triggered, this, &ClockTreeViewer::handleSavePictureClicked );

        mScene = new ClockTreeScene( this );
        mView = new ClockTreeGraphicsView( mScene, this );
        connect( mView, &ClockTreeGraphicsView::coordinatesChanged, this, &ClockTreeViewer::handleCoordinatesChanged );

        mMessageLabel = new QLabel( IDLE_TEXT, this );
        mMessageLabel->setAlignment( Qt::AlignCenter );
        mMessageLabel->setWordWrap( true );
        mMessageLabel->setMargin( 20 );

        // The message takes the place of the drawing, so an error cannot be overlooked behind an old plot.
        mStack = new QStackedWidget( this );
        mStack->addWidget( mMessageLabel );
        mStack->addWidget( mView );
        mContentLayout->addWidget( mStack, 1 );

        mCoordinateLabel = new QLabel( "  x: 0, y: 0", this );
        mWarningLabel = new QLabel( this );
        mWarningLabel->hide();

        mHighlightLabel = new QLabel( this );
        mHighlightLabel->hide();

        applyThemeColors();

        mStatusBar = new QStatusBar( this );
        mStatusBar->addWidget( mCoordinateLabel );
        mStatusBar->addWidget( mWarningLabel, 1 );
        mStatusBar->addPermanentWidget( mHighlightLabel );
        mContentLayout->addWidget( mStatusBar );

        // Many things change the color of a gate at once, e.g., a selection of hundreds of gates, but the plot is only
        // painted once for all of them.
        mRecolorTimer = new QTimer( this );
        mRecolorTimer->setSingleShot( true );
        mRecolorTimer->setInterval( 0 );
        connect( mRecolorTimer, &QTimer::timeout, this, &ClockTreeViewer::recolor );

        // Everything that changes the colors of the plot in HAL's graph view.
        if( MainWindow::sSettingStyle != nullptr )
        {
            connect( MainWindow::sSettingStyle,
                     &SettingsItemDropdown::intChanged,
                     this,
                     &ClockTreeViewer::handleStyleChanged );
        }
        if( gNetlistRelay != nullptr )
        {
            connect( gNetlistRelay->getModuleColorManager(),
                     &ModuleColorManager::moduleColorChanged,
                     this,
                     &ClockTreeViewer::scheduleRecolor );
            connect( gNetlistRelay, &NetlistRelay::moduleGateAssigned, this, &ClockTreeViewer::scheduleRecolor );
            connect( gNetlistRelay, &NetlistRelay::moduleGateRemoved, this, &ClockTreeViewer::scheduleRecolor );
            connect( gNetlistRelay, &NetlistRelay::groupingColorChanged, this, &ClockTreeViewer::scheduleRecolor );
            connect( gNetlistRelay, &NetlistRelay::groupingGateAssigned, this, &ClockTreeViewer::scheduleRecolor );
            connect( gNetlistRelay, &NetlistRelay::groupingGateRemoved, this, &ClockTreeViewer::scheduleRecolor );
            connect( gNetlistRelay, &NetlistRelay::groupingRemoved, this, &ClockTreeViewer::scheduleRecolor );
        }
        if( gSelectionRelay != nullptr )
        {
            connect(
                gSelectionRelay, &SelectionRelay::selectionChanged, this, &ClockTreeViewer::handleSelectionChanged );
        }

        // Emitted while the netlist still exists, so a running extraction can be waited for before it is deleted.
        connect( FileManager::get_instance(),
                 &FileManager::fileAboutToClose,
                 this,
                 &ClockTreeViewer::handleFileAboutToClose );

        updateActions();
    }

    ClockTreeViewer::~ClockTreeViewer()
    {
        if( mExtraction != nullptr )
        {
            // The thread deletes itself once it has finished, see handleExtractClicked.
            mExtraction->wait();
            QGuiApplication::restoreOverrideCursor();
        }
    }

    ClockTreeViewer *ClockTreeViewer::instance()
    {
        return dynamic_cast<ClockTreeViewer *>(
            ExternalContent::instance()->openWidgets.value( sPluginName, nullptr ) );
    }

    void ClockTreeViewer::setupToolbar( Toolbar *toolbar )
    {
        toolbar->addAction( mExtractAction );
        toolbar->addAction( mFitAction );
        toolbar->addAction( mZoomToHighlightAction );
        toolbar->addAction( mSavePictureAction );
    }

    bool ClockTreeViewer::setClockTree( const cte::ClockTree &tree,
                                        const ctv::StyleOverrides &overrides,
                                        const std::optional<std::string> &x_key,
                                        const std::optional<std::string> &y_key )
    {
        // Laying out only reads the clock tree and its gates and touches no widget, so it happens right here. Only
        // showing the result has to happen in the GUI thread.
        const Result<ctv::Layout> layout = layout_of( tree, gNetlist, x_key, y_key );

        if( QThread::currentThread() == thread() )
        {
            return display( layout, overrides, x_key, y_key );
        }

        bool shown = false;
        QMetaObject::invokeMethod(
            this, [&]() { shown = display( layout, overrides, x_key, y_key ); }, Qt::BlockingQueuedConnection );
        return shown;
    }

    bool ClockTreeViewer::display( const Result<ctv::Layout> &layout,
                                   const ctv::StyleOverrides &overrides,
                                   const std::optional<std::string> &x_key,
                                   const std::optional<std::string> &y_key )
    {
        if( layout.is_error() )
        {
            return showError( QString::fromStdString( layout.get_error().get() ) );
        }

        const Result<ctv::Style> style = ctv::resolve_style( layout.get(), overrides );
        if( style.is_error() )
        {
            return showError( QString::fromStdString( style.get_error().get() ) );
        }

        mOverrides = overrides;
        mXKey = x_key;
        mYKey = y_key;
        mLayout = layout.get();

        mNodeOfGate.clear();
        for( std::size_t idx = 0; idx < mLayout.nodes.size(); idx++ )
        {
            mNodeOfGate.insert( mLayout.nodes[idx].gate_id, static_cast<int>( idx ) );
        }

        // Whatever is selected in HAL when the plot appears is highlighted right away. The highlight is part of the colors,
        // so they are only worked out after it.
        highlightFromSelection();

        mScene->setLayout( mLayout, style.get(), currentColors() );
        mScene->setHighlight( highlightedNodes() );
        mShowsError = false;
        mStack->setCurrentWidget( mView );
        mView->fitToScene();
        updateActions();

        log_info(
            "clock_tree_viewer", "Showing {} gates and {} connections.", mLayout.nodes.size(), mLayout.edges.size() );

        if( mLayout.gates_without_coordinates.empty() )
        {
            showWarning( QString() );
        }
        else
        {
            const std::size_t total = mLayout.nodes.size() + mLayout.gates_without_coordinates.size();

            log_warning(
                "clock_tree_viewer",
                "{} of {} gates have no coordinates and are not shown, {} connections are left out. Gate IDs: {}",
                mLayout.gates_without_coordinates.size(),
                total,
                mLayout.omitted_edges,
                list_gate_ids( mLayout.gates_without_coordinates ).toStdString() );

            showWarning( QString( "%1 of %2 gates have no coordinates and are not shown (%3 connections left out)" )
                             .arg( mLayout.gates_without_coordinates.size() )
                             .arg( total )
                             .arg( mLayout.omitted_edges ) );
        }

        return true;
    }

    bool ClockTreeViewer::showError( const QString &message )
    {
        log_error( "clock_tree_viewer", "{}", message.toStdString() );

        // The errors of a Result start with the source location they come from, which says nothing to the user.
        static const QRegularExpression source_location( R"(\[[^\]\n]*:\d+\]\s*)" );

        QString text = message;
        text.remove( source_location );

        mScene->clearLayout();
        mLayout = ctv::Layout();
        mNodeOfGate.clear();
        mHighlightedGates.clear();
        mShowsError = true;
        applyThemeColors();
        mMessageLabel->setText( "The clock tree cannot be shown:\n\n" + text.trimmed() );
        mStack->setCurrentWidget( mMessageLabel );
        showWarning( QString() );
        updateActions();

        return false;
    }

    void ClockTreeViewer::showWarning( const QString &message ) const
    {
        mWarningLabel->setText( message );
        mWarningLabel->setVisible( !message.isEmpty() );
    }

    void ClockTreeViewer::applyThemeColors() const
    {
        const LoggerQssAdapter *theme = LoggerQssAdapter::instance();

        mWarningLabel->setStyleSheet( text_color_style( theme->warningColor(), FALLBACK_WARNING_COLOR ) );
        mHighlightLabel->setStyleSheet( text_color_style( HAL_SELECTION_COLOR, HAL_SELECTION_COLOR ) );
        mMessageLabel->setStyleSheet( mShowsError ? text_color_style( theme->errorColor(), FALLBACK_ERROR_COLOR )
                                                  : QString() );
    }

    QVector<int> ClockTreeViewer::highlightedNodes() const
    {
        QVector<int> nodes;
        nodes.reserve( mHighlightedGates.size() );
        for( const u32 gateId : mHighlightedGates )
        {
            nodes.push_back( mNodeOfGate.value( gateId, -1 ) );
        }
        return nodes;
    }

    void ClockTreeViewer::updateActions() const
    {
        mFitAction->setEnabled( mScene->hasLayout() );
        mSavePictureAction->setEnabled( mScene->hasLayout() );
        mZoomToHighlightAction->setEnabled( !mScene->highlightedNodes().isEmpty() );

        const int highlighted = mScene->highlightedNodes().size();
        mHighlightLabel->setText( QString( "%1 highlighted  " ).arg( highlighted ) );
        mHighlightLabel->setVisible( highlighted > 0 );
    }

    QColor ClockTreeViewer::gateColor( u32 gateId, const QColor &fallback ) const
    {
        // The same order of precedence as the items of the graph view have, see GraphicsItem::penColor.
        if( mHighlightedGates.contains( gateId ) )
        {
            return HAL_SELECTION_COLOR;
        }

        if( gNetlist == nullptr )
        {
            return fallback;
        }

        const Gate *gate = gNetlist->get_gate_by_id( gateId );
        if( gate == nullptr )
        {
            return fallback;
        }

        if( gContentManager != nullptr && gContentManager->getGroupingManagerWidget() != nullptr )
        {
            const QColor grouping =
                gContentManager->getGroupingManagerWidget()->getModel()->colorForItem( ItemType::Gate, gateId );
            if( grouping.isValid() )
            {
                return grouping;
            }
        }

        if( const Module *module = gate->get_module(); module != nullptr && gNetlistRelay != nullptr )
        {
            return gNetlistRelay->getModuleColor( module->get_id() );
        }

        return fallback;
    }

    ClockTreeColors ClockTreeViewer::currentColors() const
    {
        ClockTreeColors colors;

        // The color HAL draws its nets in, which depends on the theme.
        colors.edge = GraphicsQssAdapter::instance()->netBaseColor();
        colors.highlight = HAL_SELECTION_COLOR;

        colors.nodes.reserve( static_cast<int>( mLayout.nodes.size() ) );
        for( const ctv::Node &node : mLayout.nodes )
        {
            colors.nodes.push_back( gateColor( node.gate_id, colors.edge ) );
        }

        return colors;
    }

    QColor ClockTreeViewer::backgroundColor() const
    {
        // Nothing in the scene paints a background, the view shows the one the theme gives every widget.
        return mView->viewport()->palette().color( mView->viewport()->backgroundRole() );
    }

    void ClockTreeViewer::handleStyleChanged()
    {
        // The theme is a stylesheet that has been applied to the application by now, the adapters read their colors from
        // it once they are polished again.
        GraphicsQssAdapter::instance()->repolish();

        LoggerQssAdapter *logTheme = LoggerQssAdapter::instance();
        logTheme->style()->unpolish( logTheme );
        logTheme->style()->polish( logTheme );

        applyThemeColors();
        scheduleRecolor();
    }

    void ClockTreeViewer::scheduleRecolor()
    {
        if( mScene->hasLayout() )
        {
            mRecolorTimer->start();
        }
    }

    void ClockTreeViewer::recolor()
    {
        mScene->setColors( currentColors() );
        mScene->setHighlight( highlightedNodes() );

        updateActions();
    }

    void ClockTreeViewer::highlightFromSelection()
    {
        mHighlightedGates.clear();

        if( gSelectionRelay == nullptr )
        {
            return;
        }

        // Only gates, and only those in the plot: whatever else is selected is none of the viewer's business.
        for( const u32 gateId : gSelectionRelay->selectedGatesList() )
        {
            if( mNodeOfGate.contains( gateId ) )
            {
                mHighlightedGates.insert( gateId );
            }
        }
    }

    void ClockTreeViewer::handleSelectionChanged()
    {
        if( !mScene->hasLayout() )
        {
            return;
        }

        highlightFromSelection();
        scheduleRecolor();
    }

    std::size_t ClockTreeViewer::applyHighlight( const std::vector<u32> &gateIds )
    {
        mHighlightedGates.clear();

        for( const u32 gateId : gateIds )
        {
            if( mNodeOfGate.contains( gateId ) )
            {
                mHighlightedGates.insert( gateId );
            }
        }

        // A script waits for the call, so what it asked for is painted when the call returns.
        recolor();

        return static_cast<std::size_t>( mHighlightedGates.size() );
    }

    std::size_t ClockTreeViewer::highlightGates( const std::vector<u32> &gateIds )
    {
        std::size_t highlighted = 0;

        if( QThread::currentThread() == thread() )
        {
            highlighted = applyHighlight( gateIds );
        }
        else
        {
            QMetaObject::invokeMethod(
                this, [&]() { highlighted = applyHighlight( gateIds ); }, Qt::BlockingQueuedConnection );
        }

        if( !gateIds.empty() )
        {
            log_info( "clock_tree_viewer",
                      "Highlighted {} of {} gates, the others are not in the plot.",
                      highlighted,
                      gateIds.size() );
        }

        return highlighted;
    }

    void ClockTreeViewer::handleZoomToHighlightClicked()
    {
        if( mScene->highlightedNodes().isEmpty() )
        {
            return;
        }

        mView->showArea( mScene->highlightedRect() );
    }

    bool ClockTreeViewer::savePicture( const QString &filename )
    {
        const Result<QImage> image = mScene->renderPicture( backgroundColor() );
        if( image.is_error() )
        {
            log_error( "clock_tree_viewer", "Cannot save a picture: {}", image.get_error().get() );
            return false;
        }

        if( !image.get().save( filename ) )
        {
            log_error( "clock_tree_viewer", "Could not save the picture to '{}'.", filename.toStdString() );
            return false;
        }

        log_info( "clock_tree_viewer", "Saved a picture of the clock tree to '{}'.", filename.toStdString() );
        return true;
    }

    void ClockTreeViewer::handleExtractClicked()
    {
        if( gNetlist == nullptr )
        {
            showError( "No netlist is loaded." );
            return;
        }

        if( mExtraction != nullptr )
        {
            return;
        }

        // Everything the thread needs is copied now, it must not read the members of the viewer.
        const Netlist *netlist = gNetlist;
        const ctv::StyleOverrides overrides = mOverrides;
        const std::optional<std::string> x_key = mXKey;
        const std::optional<std::string> y_key = mYKey;

        // Result has no empty state, so the thread fills an optional that both sides share.
        const auto result = std::make_shared<std::optional<Result<ctv::Layout>>>();

        mExtraction = QThread::create(
            [netlist, x_key, y_key, result]() { result->emplace( extract_layout( netlist, x_key, y_key ) ); } );

        // The extraction reads the netlist while it runs, so the dialog is modal and shown right away: a modal dialog only
        // blocks input once it is visible, and until the extraction is done nothing in the GUI may change the netlist.
        // The extraction cannot be interrupted, so there is no cancel button.
        auto *progress = new QProgressDialog( "Extracting the clock tree ...", QString(), 0, 0, this );
        progress->setWindowTitle( "Clock tree viewer" );
        progress->setWindowModality( Qt::ApplicationModal );
        progress->setCancelButton( nullptr );
        progress->show();

        // Connected first, so it runs before the thread deletes itself.
        connect( mExtraction, &QThread::finished, this, [this, result, overrides, x_key, y_key, progress]() {
            delete progress;
            QGuiApplication::restoreOverrideCursor();
            mExtraction = nullptr;
            mExtractAction->setEnabled( true );

            if( result->has_value() )
            {
                display( **result, overrides, x_key, y_key );
            }
            else
            {
                showError( "The extraction ended without a result." );
            }
        } );
        connect( mExtraction, &QThread::finished, mExtraction, &QObject::deleteLater );

        QGuiApplication::setOverrideCursor( Qt::BusyCursor );
        mExtractAction->setEnabled( false );
        mExtraction->start();
    }

    void ClockTreeViewer::handleFileAboutToClose()
    {
        if( mExtraction != nullptr )
        {
            mExtraction->wait();
        }
    }

    void ClockTreeViewer::handleFitClicked() const
    {
        mView->fitToScene();
    }

    void ClockTreeViewer::handleSavePictureClicked()
    {
        QString filename = QFileDialog::getSaveFileName(
            this, "Save a picture of the clock tree", "clock_tree.png", "Images (*.png *.jpg *.bmp)" );
        if( filename.isEmpty() )
        {
            return;
        }

        // The format is taken from the suffix, so a file without one would not be written.
        if( QFileInfo( filename ).suffix().isEmpty() )
        {
            filename += ".png";
        }

        if( !savePicture( filename ) )
        {
            QMessageBox::warning(
                this, "Clock tree picture", "The picture could not be saved, see the log for details." );
        }
    }

    void ClockTreeViewer::handleCoordinatesChanged( double x, double y )
    {
        // As many decimals as a pixel resolves at the current zoom: none while a pixel covers a unit or more, one more
        // with every tenfold zoom beyond that.
        const double pixelsPerUnit = mView->transform().m11();
        const int decimals =
            pixelsPerUnit > 1.0 ? std::min( 6, static_cast<int>( std::ceil( std::log10( pixelsPerUnit ) ) ) ) : 0;

        const QLocale english( QLocale::English );
        mCoordinateLabel->setText(
            QString( "  x: %1, y: %2" )
                .arg( english.toString( x, 'f', decimals ), english.toString( y, 'f', decimals ) ) );
    }
}  // namespace hal
