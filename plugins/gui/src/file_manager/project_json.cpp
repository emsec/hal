#include "gui/file_manager/project_json.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>

namespace hal {
    ProjectJson::ProjectJson(const QString& filename)
        : mStatus(NotExisting)
    {
        mPathname = QFileInfo(filename).path();

        QFile ff(filename);
        if (!ff.exists()) return;

        mStatus = CannotOpen;
        if (!ff.open(QIODevice::ReadOnly)) return;

        mStatus = ParseError;
        mDocument = QJsonDocument::fromJson(ff.readAll(), &mParseError);
        if (mParseError.error != QJsonParseError::NoError) return;
        if (!mDocument.isObject()) return;

        mStatus = NetlistError;
        if (!mDocument.object().contains("netlist")) return;
        mNetlist = mDocument.object()["netlist"].toString();
        if (mNetlist.isEmpty()) return;
        if (QFileInfo(mNetlist).isAbsolute())
        {
            if (!QFileInfo(mNetlist).exists()) return;
        }
        else
        {
            if (!QFileInfo(QDir(mPathname).absoluteFilePath(mNetlist)).exists()) return;
        }

        mStatus = GatelibError;
        if (!mDocument.object().contains("gate_library")) return;
        mGatelib = mDocument.object()["gate_library"].toString();
        if (mGatelib.isEmpty()) return;
        if (QFileInfo(mGatelib).isAbsolute())
        {
            if (!QFileInfo(mGatelib).exists()) return;
        }
        else
        {
            if (!QFileInfo(QDir(mPathname).absoluteFilePath(mGatelib)).exists()) return;
        }

        mStatus = Ok;
    }

    void ProjectJson::setGateLibraryFilename(const QString& gl)
    {
        if (mGatelib == gl) return;
        mGatelib = gl;

        QJsonObject obj = mDocument.object();
        obj.remove("gate_library");
        obj.insert("gate_library", mGatelib);
        mDocument = QJsonDocument(obj);

        QString fname = QDir(mPathname).absoluteFilePath(".project.json");
        QFile::remove(fname);
        QFile of(fname);
        if (of.open(QIODevice::WriteOnly))
            of.write(mDocument.toJson(QJsonDocument::Compact));
    }
}
