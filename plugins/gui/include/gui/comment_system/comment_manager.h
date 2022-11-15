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

#pragma once

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

        /**
         * Get list of entries for node sorted by creation date.
         * Developer might want to sort criteria parameter eventually.
         * @param nd The node key for search.
         * @return List of pointer to entries.
         */
        QList<CommentEntry*> getEntriesForNode(const Node& nd) const;

        /**
         * Return whether or not comment entries for node exist.
         * @param nd The node key for search.
         * @return True if at least a single entry exists, false otherwise
         */
        bool contains(const Node& nd) const;

        /**
         * Removes and deletes the given entry.
         * Before deleting the comment, the entryAboutToBeDeleted is emmitted.
         * (Perhaps return true on success instead of just void?)
         * @param nd
         * @param entry
         */
        void deleteComment(CommentEntry* entry);

        /**
         * Adds a new comment Entry.
         *
         * @param entry - The new entry.
         */
        void addComment(CommentEntry* entry);

        /**
         * This function must be called whenever an instance modifies an entry.
         * Similiar as to the selectionModel.
         *
         * @param entry - The modified entry.
         * @param sender - The sender.
         */
        void relayEntryModified(CommentEntry* entry);

    Q_SIGNALS:
        /**
         * This signal is emmitted just before the entry is deleted (from the
         * deleteComment function).
         *
         * @param entry - The entry that will be deleted.
         */
        void entryAboutToBeDeleted(CommentEntry* entry);

        /**
         * This signal is emmitted when a new entry is added to the CommentManager
         * (from the addComment function).
         *
         * @param entry - The entry that was added.
         */
        void entryAdded(CommentEntry* entry);

        /**
         * This signal is emmited whenever an entry is modified. To emit this signal,
         * the function relayEntryModifed must be called from the instance that modified
         * the entry. Similiar as to the selectionModel.
         *
         * @param entry - The modified entry.
         */
        void entryModified(CommentEntry* entry);

    private:
        QMultiHash<Node,CommentEntry*> mEntries;
        bool restoreInternal(const std::filesystem::path& loaddir, const std::string& relFilename);
    };
}
