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

#include "hal_core/netlist/project_serializer.h"
#include "gui/content_manager/content_manager.h"

namespace hal {

    /**
     * Creates DotViewer widgets on demand and restores the DOT files that were open when the project was last saved.
     */
    class NETLIST_API DotViewerFactory : public ContentFactory, ProjectSerializer
    {
        /** The path to the `.dot` file that was open when the project was saved. */
        QString mFilename;

        /** The name of the plugin that created that file. */
        QString mCreator;

    public:
        /**
         * Construct a new factory.
         *
         * @param[in] name - The name that the created content widget is registered under.
         */
        DotViewerFactory(const QString& name);

        /**
         * Restore the DOT viewer from the state that was stored in the given project file.
         *
         * @param[in] loaddir - The project directory to load from.
         * @param[in] jsonfile - The name of the file within the project directory that holds the stored state.
         */
        void restoreDotViewer(const std::filesystem::path& loaddir, const std::string& jsonfile);

        /**
         * Create a new DOT viewer widget.
         *
         * @returns The created widget.
         */
        ExternalContentWidget* contentFactory() const;

        /**
         * Store the `.dot` file that is currently displayed so that it can be restored with the project.
         *
         * @param[in] netlist - The netlist of the project.
         * @param[in] savedir - The project directory to save to.
         * @param[in] isAutosave - `true` if the call was issued by the autosave, `false` otherwise.
         * @returns The name of the written file, an empty string if nothing was written.
         */
        std::string serialize(Netlist* netlist, const std::filesystem::path& savedir, bool isAutosave) override;

        /**
         * Read back the `.dot` file that was displayed when the project was saved.
         *
         * @param[in] netlist - The netlist of the project.
         * @param[in] loaddir - The project directory to load from.
         */
        void deserialize(Netlist* netlist, const std::filesystem::path& loaddir) override;

        /**
         * Reopen the previously stored `.dot` file in the DOT viewer.
         */
        void restore();
    };
}
