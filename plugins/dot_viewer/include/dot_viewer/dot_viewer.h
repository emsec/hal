// MIT License
//
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS").
// All Rights reserved.
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
#include "gui/content_manager/content_manager.h"

#include <mutex>
#include <string>
#include <vector>
#include <QObject>

class QGVScene;

namespace hal
{
    class Netlist;
    class Toolbar;
}

namespace hal
{
    class DotGraphicsView;

    /**
     * The content widget that displays a DOT graph within the GUI.
     */
    class NETLIST_API DotViewer : public ExternalContentWidget
    {
        Q_OBJECT
    public:
        /**
         * Construct a new DOT viewer widget.
         *
         * @param[in] pluginName - The name of the plugin that the widget belongs to.
         * @param[in] parent - The parent object. Defaults to a `nullptr`.
         */
        DotViewer(const QString &pluginName, QObject* parent = nullptr);

        ~DotViewer();

        /**
         * Add the actions of the DOT viewer to the given toolbar.
         *
         * @param[in] toolbar - The toolbar to add the actions to.
         */
        void setupToolbar(Toolbar* toolbar);

        /**
         * Detach all interactions from the displayed graph, e.g., when the netlist is about to be closed.
         */
        void disableInteractions();

        /**
         * Get the DOT viewer instance that is currently open.
         *
         * @returns The DOT viewer instance, a `nullptr` if none is open.
         */
        static DotViewer* getDotviewerInstance();

        /**
         * Get the path to the `.dot` file that is currently displayed.
         *
         * @returns The path to the file.
         */
        QString filename() const { return mFilename; }

        /**
         * Get the name of the plugin that created the currently displayed graph.
         *
         * @returns The name of the plugin.
         */
        QString creatorPlugin() const { return mCreatorPlugin; }

        /**
         * Load a `.dot` file and display the graph that it describes.
         *
         * @param[in] fileName - The path to the `.dot` file.
         * @param[in] creator - The name of the plugin that created the file. Defaults to an empty string.
         * @returns `true` on success, `false` otherwise.
         */
        bool loadDotFile(const QString& fileName, const QString& creator = QString());

    public Q_SLOTS:
        /**
         * Load and display the given `.dot` file.
         * In contrast to `loadDotFile`, this slot may also be invoked from another thread.
         *
         * @param[in] fileName - The path to the `.dot` file.
         * @param[in] creator - The name of the plugin that created the file. Defaults to an empty string.
         */
        void handleOpenInputFileByName(const QString& fileName, const QString& creator = QString());

    private Q_SLOTS:
        /**
         * Ask the user for a `.dot` file and display it.
         */
        void handleOpenInputFileDialog();

        /**
         * Show or hide the background grid of the graph.
         */
        void handleToggleGrid();

        /**
         * Let the user pick the colors that the graph is drawn with.
         */
        void handleColorSelect();

        /**
         * Redraw the graph using the selected layout style.
         *
         * @param[in] istyle - The index of the selected style.
         */
        void handleStyleChanged(int istyle);

    private:
        /** The scene that holds the rendered graph. */
        QGVScene* mDotScene;

        /** The view that displays the scene. */
        DotGraphicsView* mDotGraphicsView;

        /** The toolbar action that opens a `.dot` file. */
        QAction* mOpenInputfileAction;

        /** The toolbar action that toggles the background grid. */
        QAction* mToggleGridAction;

        /** The toolbar action that opens the color selection. */
        QAction* mColorSelectAction;

        /** The path to the `.dot` file that is currently displayed. */
        QString mFilename;

        /** The name of the plugin that created the currently displayed graph. */
        QString mCreatorPlugin;
    };

    /**
     * Lets a non-GUI thread ask the DOT viewer to open a file by forwarding the request to the GUI thread.
     */
    class DotViewerCallFromThread : public QObject
    {
        Q_OBJECT
    public:
        /**
         * Construct a new forwarder.
         *
         * @param[in] parent - The parent object. Defaults to a `nullptr`.
         */
        DotViewerCallFromThread(QObject* parent = nullptr) : QObject(parent) {;}

        /**
         * Ask the given DOT viewer to open a file, no matter which thread this is called from.
         *
         * @param[in] callee - The DOT viewer that shall open the file.
         * @param[in] filename - The path to the `.dot` file.
         * @param[in] plugin - The name of the plugin that created the file.
         * @returns `true` on success, `false` otherwise.
         */
        bool openInputFileByName(DotViewer* callee, QString filename, QString plugin);

    Q_SIGNALS:
        /**
         * Q_SIGNAL that carries the request to open a file over to the GUI thread.
         *
         * @param[in] filename - The path to the `.dot` file.
         * @param[in] plugin - The name of the plugin that created the file.
         */
        void callOpenInputFileByName(QString filename, QString plugin);
    };
}  // namespace hal
