#include "gui/export/export_registered_format.h"
#include <QFileDialog>
#include <QDir>
#include <QApplication>
#include <QFile>
#include <QColor>
#include <QFileInfo>
#include <QMessageBox>
#include "gui/gui_globals.h"
#include "hal_core/netlist/netlist_writer/netlist_writer_manager.h"

namespace hal {
    ExportRegisteredFormat::ExportRegisteredFormat(const QStringList &exts, const QString& filename)
        : mExtensions(exts), mFilename(filename)
    {;}

    bool ExportRegisteredFormat::queryFilename()
    {
        int wordcount = 0;
        QString filter;
        for (const QString& s : mExtensions)
        {
            switch(wordcount++)
            {
            case 0: break;
            case 1: filter += " (*"; break;
            default: filter += " *";
            }
            filter += s;
        }
        filter += ")";
        QWidget* parent = qApp->activeWindow();

        for (;;) // loop if no existing file override
        {
            mFilename = QFileDialog::getSaveFileName(
                        parent,
                        "Export netlist",
                        QDir::currentPath(),
                        filter,
                        nullptr);
            if (mFilename.isEmpty()) return false;

            bool goodExtension = false;
            for (int i=1; i<mExtensions.size(); i++)
                if (mFilename.endsWith(mExtensions.at(i)))
                {
                    goodExtension = true;
                    break;
                }

            if (!goodExtension)
            {
                QFileInfo fi(mFilename);
                mFilename = fi.path() + "/" + fi.completeBaseName() + mExtensions.last();

                if (QFileInfo(mFilename).exists())
                {
                    if (QMessageBox::question(parent, "Export netlist",
                                              mFilename + " already exists.\nDo you want to replace it?",
                                              QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok)
                            != QMessageBox::Ok) continue;
                }
            }

            return true;
        }
    }

    void ExportRegisteredFormat::exportNetlist() const
    {
        netlist_writer_manager::write(gNetlist, mFilename.toStdString());
    }
}
