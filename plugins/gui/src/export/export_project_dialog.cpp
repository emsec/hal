#include "gui/export/export_project_dialog.h"
#include "gui/file_manager/project_json.h"
#include "gui/python/python_editor.h"
#include "hal_core/netlist/project_manager.h"
#include "hal_core/utilities/log.h"

#include <QFileInfo>
#include <QTemporaryDir>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <JlCompress.h> // deps/quazip-1.5
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

namespace hal
{
    ExportProjectDialog::ExportProjectDialog(QWidget *parent)
        : QFileDialog(parent), mStatus(NoExport)
    {
        QFileInfo pinfo(QString::fromStdString(ProjectManager::instance()->get_project_directory().get_default_filename()));
        mProjectName = pinfo.baseName().simplified();
        mProjectName.replace(" ","");
        setDefaultSuffix(".zip");
        setAcceptMode(QFileDialog::AcceptSave);
        setLabelText(QFileDialog::FileType, QString("Compressed archive (*.zip"));
        setLabelText(QFileDialog::FileName, "Export project " + mProjectName + " to …");
        selectFile("hal_project_" + mProjectName + ".zip");
        setWindowTitle("Export project");
    }

    void ExportProjectDialog::exportProject()
    {
        if (selectedFiles().isEmpty())
            return;
        QString archiveFilename = selectedFiles().at(0);

        QString projTargetDir;
        QTemporaryDir tempDir(QDir::temp().absoluteFilePath("hal_project_export_XXXXXX"));
        if (!tempDir.isValid() || !QDir().mkpath(projTargetDir = QDir(tempDir.path()).absoluteFilePath(mProjectName))) {
            log_warning("gui", "Failed to create temporary project export directory '{}'.", projTargetDir.toStdString());
            mStatus = ErrorCreateTempDir;
            return;
        }

        QString projSourceDir = QString::fromStdString(ProjectManager::instance()->get_project_directory().string());
        QStringList filesToCopy;
        QStringList dirsToCreate;

        QDirIterator it(projSourceDir, QDir::NoDotAndDotDot |
                        QDir::Readable |
                        QDir::NoSymLinks |
                        QDir::Hidden |
                        QDir::Dirs |
                        QDir::Files,
                    QDirIterator::Subdirectories);
        while (it.hasNext())
        {
            QString relPath = it.next();
            if (relPath.startsWith(projSourceDir))
                relPath.remove(0,projSourceDir.size()+1);
            else
                log_warning("gui", "Project export found file '{}' not in project folder '{}'.",
                            relPath.toStdString(), projSourceDir.toStdString());

            if (it.fileInfo().isDir())
                dirsToCreate.append(relPath);
            else if (it.fileInfo().isFile())
            {
                if (relPath.startsWith("autosave/")) continue;
                filesToCopy.append(relPath);
            }
            else
                log_warning("gui", "Project export ignored directory entry '{}' with unknown type.", relPath.toStdString());
        }

        // Create project subdirectories in export temp folder
        for (QString dir : dirsToCreate)
        {
            QString fullpath = QDir(projTargetDir).absoluteFilePath(dir);
            if (!QDir().mkpath(fullpath))
                log_warning("gui", "Failed to create directory '{}' in project export folder.", dir.toStdString());
        }

        // Copy files to export temp folder
        for (const QString& s : filesToCopy)
        {
            QString src = QDir(projSourceDir).absoluteFilePath(s);
            QString dst = QDir(projTargetDir).absoluteFilePath(s);
            if (!QFile::copy(src,dst))
            {
                log_warning("gui", "Failed to copy file '{}' into project export folder.", s.toStdString());
                if (s == ".project.json" || s.endsWith(".hal") || s.endsWith(".hgl"))
                {
                    mStatus = ErrorCopy;
                    return;
                }
            }
        }

        mStatus = ErrorPrepare;
        // Copy relevant files into export temp folder
        ProjectJson projFile(QDir(projTargetDir).absoluteFilePath(".project.json"));
        if (projFile.status() != ProjectJson::Ok) return;
        QFileInfo glFinfo(projFile.gateLibraryFilename());
        if (glFinfo.isAbsolute())
        {
            if (!QFile::copy(glFinfo.absoluteFilePath(),QDir(projTargetDir).absoluteFilePath(glFinfo.fileName())))
                log_warning("gui", "Failed to copy file '{}' into project export folder.", glFinfo.absoluteFilePath().toStdString());
            else
                projFile.setGateLibraryFilename(glFinfo.fileName());
        }
        copyExternalPython(projTargetDir);

        if (JlCompress::compressDir( archiveFilename, tempDir.path(), true, QDir::Hidden ))
            mStatus = Ok;
        else
        {
            log_warning("gui", "Failed to compress project to archive '{}'.", archiveFilename.toStdString());
            mStatus = ErrorCompress;
        }
    }

    void ExportProjectDialog::copyExternalPython(const QString &projTempDir)
    {
        QString pyControlFileName = QDir(projTempDir).absoluteFilePath(QString::fromStdString(PythonSerializer::sControlFileName));
        QFile ff(pyControlFileName);
        if (!ff.open(QIODevice::ReadOnly)) return;

        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(ff.readAll(), &err);
        if (err.error != QJsonParseError::NoError) return;
        ff.close();

        QJsonObject obj = doc.object();
        if (!obj.contains("tabs") || !obj["tabs"].isArray()) return;

        int replaceCount = 0;
        QString pyTempDir(QDir(projTempDir).absoluteFilePath(PythonSerializer::sPythonRelDir));

        QJsonArray oldTabs = obj["tabs"].toArray();
        QJsonArray newTabs;
        int ntabs           = oldTabs.size();
        for (int itab = 0; itab < ntabs; itab++)
        {
            QJsonObject tab = oldTabs.at(itab).toObject();
            if (!tab.contains("filename")) continue;
            QString absFilename = tab["filename"].toString();
            if (QFileInfo(absFilename).isAbsolute())
            {
                QString relFilename = QFileInfo(absFilename).fileName();
                QString tmpFilename = QDir(pyTempDir).absoluteFilePath(relFilename);
                if (QFileInfo(tmpFilename).exists())
                    QFile::remove(tmpFilename);
                if (QFile::copy(absFilename,tmpFilename))
                {
                    ++replaceCount;
                    tab.remove("filename");
                    tab.insert("filename", relFilename);
                }
            }
            newTabs.append(tab);
        }

        if (replaceCount)
        {
            obj.remove("tabs");
            obj.insert("tabs", newTabs);
            QFile::remove(pyControlFileName);
            QFile fo(pyControlFileName);
            if (!fo.open(QIODevice::WriteOnly)) return;
            fo.write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
        }
    }
}
