#pragma once

#include <QString>
#include <QXmlStreamWriter>

namespace hal {
    class Gate;
    class Net;

    class ExportRegisteredFormat
    {
        QString mExtension;
        QString mFilename;


    public:
        ExportRegisteredFormat(const QString& extension, const QString& filename = QString());
        bool queryFilename();
        void exportNetlist() const;
        QString filename() const { return mFilename; }
    };
}
