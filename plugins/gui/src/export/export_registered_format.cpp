#include "gui/export/export_registered_format.h"
#include <QFileDialog>
#include <QDir>
#include <QApplication>
#include <QFile>
#include <QColor>
#include <QFileInfo>
#include "gui/gui_globals.h"
#include "hal_core/netlist/netlist_writer/netlist_writer_manager.h"

namespace hal {
    ExportRegisteredFormat::ExportRegisteredFormat(const QString &extension, const QString& filename)
        : mExtension(extension), mFilename(filename)
    {;}

    bool ExportRegisteredFormat::queryFilename()
    {
        QWidget* parent = qApp->activeWindow();
        mFilename = QFileDialog::getSaveFileName(
                    parent,
                    "Export netlist",
                    QDir::currentPath(),
                    QString("%1 file (%2)").arg(mExtension.mid(1).toUpper()).arg(mExtension),
                    nullptr);
        if (mFilename.isEmpty()) return false;

        if (!mFilename.endsWith(mExtension))
        {
            QFileInfo fi(mFilename);
            mFilename = fi.path() + "/" + fi.completeBaseName() + mExtension;
        }
        return true;
    }

    void ExportRegisteredFormat::exportNetlist() const
    {
        netlist_writer_manager::write(gNetlist, mFilename.toStdString());
    }
}
