#pragma once

#include <QString>
#include <QXmlStreamWriter>

namespace hal {
    class Gate;
    class Net;

    class ExportGexf
    {
        QString mFilename;
        int mEdgeId;

        void writeMeta(QXmlStreamWriter& xmlOut) const;
        void writeGraph(QXmlStreamWriter& xmlOut);
        void writeNode(QXmlStreamWriter& xmlOut, const Gate* g) const;
        void writeColor(QXmlStreamWriter& xmlOut, const Gate* g) const;
        void writeEdge(QXmlStreamWriter& xmlOut, const Net* n);
        void writeAttribute(QXmlStreamWriter& xmlOut, int id, const QString& title, const QString& type) const;
        void writeNodeAttribute(QXmlStreamWriter& xmlOut, const Gate* g, int inx) const;
        void writeEdgeAttribute(QXmlStreamWriter& xmlOut, const Net* n, int inx, const std::string pin = std::string()) const;

    public:
        ExportGexf(const QString& filename = QString());
        bool queryFilename();
        bool exportNetlist();
        QString filename() const { return mFilename; }
    };
}
