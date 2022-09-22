//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
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

#include <QObject>
#include <QList>
#include <QMultiHash>

#include "gui/comment_system/comment_entry.h"
#include "gui/gui_def.h"
#include "hal_core/netlist/project_serializer.h"

namespace hal
{
    class CommentManager : public QObject, public ProjectSerializer
    {
        Q_OBJECT

    public:
        CommentManager(QObject* parent = nullptr);

        ~CommentManager();

        /**
         * Call to restore
         * @return true if successful, false otherwise
         */
        bool restore();

        /**
         * ProjectSerializer callback gets called upon save and autosave
         * @param netlist The current netlist (unused)
         * @param savedir The directory to which the project gets persisted
         * @param isAutosave Whether save request was issued from autosave (unused)
         * @return relative filename where the comments were saved
         */
        std::string serialize(Netlist* netlist, const std::filesystem::path& savedir, bool isAutosave) override;

        /**
         * ProjectSerializer callback when project gets loaded
         * @param netlist The current netlist (unused)
         * @param loaddir The directory from which the file gets loaded
         */
        void deserialize(Netlist* netlist, const std::filesystem::path& loaddir) override;

        /**
         * Cleanup and clear when netlist gets closed
         */
        void clear();
    private:
        QMultiHash<Node,CommentEntry*> mEntries;
        bool restoreInternal(const std::filesystem::path& loaddir, const std::string& relFilename);
    };
}
