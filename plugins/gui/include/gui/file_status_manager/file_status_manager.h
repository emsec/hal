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
#include <QSet>
#include <QMap>
#include <QString>
#include <QList>
#include <QUuid>

namespace hal
{
    /**
     * @ingroup file_management
     * @brief Holds information about the state (e.g. saved/unsaved) of all laded files (i.e. .hal/.python files).
     *
     * The FileStatusManager class is the central point to store the status of any file
     * (or element) that can be modified, such as the modifications to the current netlist
     * or (yet to be created /saved) python tabs / files.
     */
    class FileStatusManager : public QObject
    {
        Q_OBJECT
    public:
        /**
         * The constructor.
         *
         * @param parent - The manager's parent.
         */
        FileStatusManager(QObject* parent = nullptr);

        /**
         * The destructor.
         */
        ~FileStatusManager();

        /**
         * Checks if either the netlist is modified (in the case a netlist is opened) or
         * any files are registered to this manager.
         *
         * @return True if registered files are present. False otherwise.
         */
        bool modifiedFilesExisting() const;

        /**
         * Registers a tuple consisting of an unique id (e.g. the id of a PythonCodeEditor
         * that basically represents a file) and its description (e.g. the name of the file or tab).
         *
         * @param uuid - The unique id.
         * @param descriptor - The description of the file / object.
         */
        void fileChanged(const QUuid uuid, const QString& descriptor);

        /**
         * Removes a registered element.
         *
         * @param uuid - The id of the element that is to be removed.
         */
        void fileSaved(const QUuid uuid);

        /**
         * Sets the netlist-modified flag that affects the modifiedFilesExisting() method.
         */
        void netlistChanged();

        /**
         * Unsets the netlist-modified flag that affects the modifiedFilesExisting() method.
         */
        void netlistSaved();

        /**
         * Removes all registered files and unsets every modified-flag, effectively resetting
         * the state of the manager.
         */
        void flushUnsavedChanges();

        /**
         * Get a list of the descriptions of all modified files that are registered to this manager.
         * Appends "Netlist modifications"  to the list if the netlist-modified flag is set.
         *
         * @return The list of the descriptions.
         */
        QList<QString> getUnsavedChangeDescriptors() const;

    private:
        QSet<QUuid> mModifiedFilesUuid;
        QMap<QUuid, QString> mModifiedFilesDescriptors;

        bool mNetlistModified;
    };
}
