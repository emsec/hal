#include "gui/export/export_project_dialog.h"
#include "hal_core/netlist/project_manager.h"

#include <QFileInfo>
#include <QDebug>
#include <QTemporaryDir>
#include <QDir>
#include <QDirIterator>
#include <QFile>

namespace hal
{
    ExportProjectDialog::ExportProjectDialog(QWidget *parent)
        : QFileDialog(parent), mStatus(NoExport)
    {
        QFileInfo pinfo(QString::fromStdString(ProjectManager::instance()->get_project_directory().get_default_filename()));
        mProjectName = pinfo.baseName().simplified();
        mProjectName.replace(" ","");
        setDefaultSuffix(".tar.gz");
        setAcceptMode(QFileDialog::AcceptSave);
        setLabelText(QFileDialog::FileType, QString("Compressed archive (*.tgz *.tar.gz *.zip"));
        setLabelText(QFileDialog::FileName, "Export project " + mProjectName + " to …");
        selectFile(mProjectName + ".tar.gz");
        setWindowTitle("Export project");
    }

    void ExportProjectDialog::accept()
    {
        if (selectedFiles().isEmpty())
            return;
        QString archiveFilename = selectedFiles().at(0);

        QTemporaryDir tempDir(QDir::temp().absoluteFilePath("hal_project_export_XXXXXX"));
        QString projTargetDir;
        if (!tempDir.isValid() || !QDir().mkpath(projTargetDir = QDir(tempDir.path()).absoluteFilePath(mProjectName))) {
            mStatus = ErrorCreateTempDir;
            return;
        }

        QString projSourceDir = QString::fromStdString(ProjectManager::instance()->get_project_directory().string());
        QStringList filesToCopy;
        QStringList dirsToCreate;

        QDirIterator it(projSourceDir, QDir::NoDotAndDotDot |
                        QDir::Readable |
                        QDir::NoSymLinks |
                        QDir::Dirs |
                        QDir::Files,
                    QDirIterator::Subdirectories);
        while (it.hasNext())
        {
            QString relPath = it.next();
            if (relPath.startsWith(projSourceDir))
                relPath.remove(0,projSourceDir.size()+1);
            else
                qDebug() << "not in path " << relPath << projSourceDir;

            if (it.fileInfo().isDir())
                dirsToCreate.append(relPath);
            else if (it.fileInfo().isFile())
            {
                if (relPath.startsWith("autosave/")) continue;
                filesToCopy.append(relPath);
            }
            else
                qDebug() << "unknown directory entry type" << relPath;
        }

        for (QString dir : dirsToCreate)
        {
            QString fullpath = QDir(projTargetDir).absoluteFilePath(dir);
            QDir().mkpath(fullpath);
        }

        for (const QString& s : filesToCopy)
        {
            QString src = QDir(projSourceDir).absoluteFilePath(s);
            QString dst = QDir(projTargetDir).absoluteFilePath(s);
            qDebug() << "copy" << src << dst << QFile::copy(src,dst);
        }
        qDebug() << "temp" << projTargetDir;
        mStatus = Ok;
        QFileDialog::accept();
    }
}
