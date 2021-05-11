#pragma once

#include <QStringList>
#include <QXmlStreamWriter>

namespace hal {
    class Gate;
    class Net;

    class ExportRegisteredFormat
    {
        QStringList mExtensions;
        QString mFilename;


    public:
        ExportRegisteredFormat(const QStringList& exts, const QString& filename = QString());
        bool queryFilename();
        void exportNetlist() const;
        QString filename() const { return mFilename; }
    };
}
