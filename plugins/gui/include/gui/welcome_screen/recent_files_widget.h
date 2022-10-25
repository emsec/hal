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

#include <QFrame>
#include <QFileSystemWatcher>

class QVBoxLayout;

namespace hal
{
    class RecentFileItem;

    /**
     * @ingroup gui
     * @brief The welcome screen's RecentFiles section.
     *
     * The widget in the welcome screen that shows the recently used files.
     */
    class RecentFilesWidget : public QFrame
    {
        Q_OBJECT

    public:
        /**
         * Constructor. <br>
         * The list of recently used files are drawn from the settings in the constructor.
         *
         * @param parent - The parent widget
         */
        RecentFilesWidget(QWidget* parent = nullptr);

        /**
         * Initializes the appearance of the widget.
         */
        void repolish();

    public Q_SLOTS:

        /**
         * Q_SLOT to handle that a project has been opend or saved.
         * Used to append the project to the list of recently used files.
         * The updated list is persisted by updating the settings.
         *
         * @param projDir - The path of the opened project folder
         * @param fileName - The path of the opened netlist file
         */
        void handleProjectUsed(const QString& projDir, const QString& fileName);

        /**
         * Handles that a recentFileItem should be removed (by clicking the 'x'). It removed the item from the widget.
         *
         * @param item - The item to remove
         */
        void handleRemoveRequested(RecentFileItem* item);

        /**
         * Handles the case an item disappeared (e.g. deleted, moved to different location) while selection widget is open
         * @param path - Path to item which which must be checked
         */
        void handleItemDisappeared(const QString& path);

    private:
        void readSettings();
        void updateSettings();

        QVBoxLayout* mLayout;

        QList<RecentFileItem*> mItems;
        QFileSystemWatcher* mFileSystemWatcher;

        void prependPath(const QString& path);
        void addWatcher(const QString& path);
        static const int sMaxItems = 14;
    };
}
