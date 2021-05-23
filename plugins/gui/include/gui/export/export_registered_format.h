#pragma once

#include <QStringList>
#include <QXmlStreamWriter>

namespace hal {
    class Gate;
    class Net;

    /**
     * @ingroup gui
     * @brief Utility class to export (save) different hal files.
     */
    class ExportRegisteredFormat
    {
        QStringList mExtensions;
        QString mFilename;

    public:
        /**
         * The constructor.
         *
         * @param exts - All valid file extensions.
         * @param filename - The filename to write to.
         */
        ExportRegisteredFormat(const QStringList& exts, const QString& filename = QString());

        /**
         * Opens a save file dialog with all valid extensions to get a filename to write
         * the project (netlist) to. It sets the filename internally.
         *
         * @return True on success, False otherwise.
         */
        bool queryFilename();

        /**
         * Exports (writes) the netlist.
         */
        void exportNetlist() const;

        /**
         * Get the filename to write to.
         *
         * @return The filename.
         */
        QString filename() const { return mFilename; }
    };
}
