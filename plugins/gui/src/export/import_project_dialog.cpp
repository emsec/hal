#include "gui/export/import_project_dialog.h"
#include "gui/file_manager/file_manager.h"
#include "gui/gui_utils/graphics.h"
#include "hal_core/utilities/log.h"
#include <QRegularExpression>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QLabel>
#include <QUuid>
#include <QMessageBox>
#include <JlCompress.h>

namespace hal {
    FileSelectWidget::FileSelectWidget(const QString &defaultEntry, bool existingDir, QWidget* parent)
        : QFrame(parent), mExistingDir(existingDir), mValid(false)
    {
        setMaximumHeight(48);
        setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
        setLineWidth(3);
        setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);

        QGridLayout* layout = new QGridLayout(this);
        mEditor = new QLineEdit(this);
        mEditor->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
        mEditor->setMinimumWidth(320);
        mEditor->setText(defaultEntry);
        connect(mEditor,&QLineEdit::textChanged,this,&FileSelectWidget::handleTextChanged);
        layout->addWidget(mEditor,0,0);

        mButton = new QPushButton(gui_utility::getStyledSvgIcon("all->#3192C5",":/icons/folder"),"",this);
        mButton->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Preferred);
        connect(mButton,&QPushButton::clicked,this,&FileSelectWidget::handleActivateFileDialog);
        layout->addWidget(mButton,0,1);
        handleTextChanged(defaultEntry);
    }

    void FileSelectWidget::handleActivateFileDialog()
    {
        QString entry = mExistingDir
                ? QFileDialog::getExistingDirectory(this, "Select directory:", mEditor->text())
                : QFileDialog::getOpenFileName(this, "Select file:", mEditor->text(), "Zipped files (*.zip)");
        mEditor->setText(entry);
    }

    void FileSelectWidget::handleTextChanged(const QString &txt)
    {
        bool val = !txt.isEmpty();
        if (val) val = QFileInfo(txt).exists();
        if (val == mValid) return;
        mValid = val;
        Q_EMIT selectionStatusChanged();
    }

    ImportProjectDialog::ImportProjectDialog(QWidget *parent)
        : QDialog(parent), mStatus(NoImport)
    {
        setWindowTitle("Import Project");
        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->addWidget(new QLabel("Zipped HAL project file:",this));
        mZippedFile = new FileSelectWidget("", false, this);
        connect(mZippedFile,&FileSelectWidget::selectionStatusChanged, this, &ImportProjectDialog::handleSelectionStatusChanged);
        layout->addWidget(mZippedFile);

        layout->addStretch();

        layout->addWidget(new QLabel("Decompress in directory:",this));
        mTargetDirectory = new FileSelectWidget(QDir::currentPath(), true, this);
        connect(mTargetDirectory,&FileSelectWidget::selectionStatusChanged, this, &ImportProjectDialog::handleSelectionStatusChanged);
        layout->addWidget(mTargetDirectory);

        layout->addStretch();

        layout->addWidget(new QLabel("Decompressed HAL project name:"));
        mExtractProjectEdit = new QLineEdit(this);
        layout->addWidget(mExtractProjectEdit);

        layout->addStretch();

        mButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,this);
        connect(mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        layout->addWidget(mButtonBox);
        handleSelectionStatusChanged();
    }

    void ImportProjectDialog::handleSelectionStatusChanged()
    {
        bool valid = mTargetDirectory->valid() && mZippedFile->valid();
        if (valid)
        {
            QRegularExpression reProj("(.*)/\\.project.json$");
            QStringList zipFlist = JlCompress::getFileList(mZippedFile->selection());
            int inx = zipFlist.indexOf(reProj);
            if (inx < 0)
            {
                QMessageBox::warning(this,"Invalid ZIP-file", "The zipped file " + mZippedFile->selection() + " does not seem to be a hal project.\n");
                valid = false;
            }
            else
            {
                QRegularExpressionMatch match = reProj.match(zipFlist.at(inx));
                mTargetProjectName = match.captured(1);
                QString suggestedDir = suggestedProjectDir(QDir(mTargetDirectory->selection()).absoluteFilePath(mTargetProjectName));
                mExtractProjectEdit->setText(QFileInfo(suggestedDir).fileName());
            }
        }
        mButtonBox->button(QDialogButtonBox::Ok)->setEnabled(valid);
    }

    bool ImportProjectDialog::importProject()
    {
        mStatus = NoFileSelected;
        if (mZippedFile->selection().isEmpty()) return false;

        mExtractedProjectAbsolutePath = QDir(mTargetDirectory->selection()).absoluteFilePath(mExtractProjectEdit->text());
        if (QFileInfo(mExtractedProjectAbsolutePath).exists())
        {
            QMessageBox::warning(this, "Cannot Create Project", "Direcotry " + mExtractedProjectAbsolutePath + " already exists");
            return false;
        }
        QStringList extracted;
        QString tempdir;
        if (mExtractProjectEdit->text() != mTargetProjectName)
        {
            tempdir = QDir(mTargetDirectory->selection()).absoluteFilePath("hal_temp_" + QUuid::createUuid().toString(QUuid::WithoutBraces));
            QDir().mkpath(tempdir);
            extracted = JlCompress::extractDir(mZippedFile->selection(),tempdir);
        }
        else
        {
            extracted = JlCompress::extractDir(mZippedFile->selection(),mTargetDirectory->selection());
        }
        mStatus = ErrorDecompress;
        if (extracted.isEmpty()) return false;

        QString extractedDir = extracted.at(0);
        while (extractedDir.back().unicode() == '/' || extractedDir.back().unicode() == '\\')
            extractedDir.chop(1);

        switch (FileManager::directoryStatus(extractedDir))
        {
        case FileManager::ProjectDirectory:
            mStatus = Ok;
            break;
        case FileManager::OtherDirectory:
            log_warning("gui", "No hal project file found in '{}' archive.", mZippedFile->selection().toStdString());
            mStatus = NotAHalProject;
            break;
        case FileManager::IsFile:
            log_warning("gui", "Archive '{}' contains single file, not a hal project directory.", mZippedFile->selection().toStdString());
            mStatus = NotAHalProject;
            break;
        case FileManager::NotExisting:
            log_warning("gui", "Failed to decompress archive '{}', extracted HAL directory not found.", mZippedFile->selection().toStdString());
            mStatus = NotAHalProject;
            break;
        case FileManager::InvalidExtension:
            log_warning("gui", "HAL directory extracted from archive '{}' must not have an extension.", mZippedFile->selection().toStdString());
            mStatus = NotAHalProject;
            break;
        case FileManager::ParseError:
            log_warning("gui", "Failed to parse HAL project file from archive '{}'.", mZippedFile->selection().toStdString());
            mStatus = NotAHalProject;
            break;
        case FileManager::NetlistError:
            log_warning("gui", "Failed to find netlist in archiv '{}'.", mZippedFile->selection().toStdString());
            mStatus = NotAHalProject;
            break;
        case FileManager::GatelibError:
            log_warning("gui", "Failed to find gate library in archiv '{}'.", mZippedFile->selection().toStdString());
            mStatus = NotAHalProject;
            break;
        case FileManager::UnknownDirectoryEntry:
            log_warning("gui", "Failed to decompress archive '{}', result is neither a file nor a directory.", mZippedFile->selection().toStdString());
            mStatus = NotAHalProject;
            break;
        }

        if (!tempdir.isEmpty())
        {
            if (!QFile::rename(extractedDir,mExtractedProjectAbsolutePath))
            {
                log_warning("gui", "Failed to move decompressed archive to new location '{}'.", mExtractedProjectAbsolutePath.toStdString());
                mStatus = ErrorDecompress;
            }
            else
            {
                // delete everything left in tempdir after moving project to project dir
                deleteFilesRecursion(tempdir);
            }
        }

        if (mStatus != Ok)
        {
            // Not a hal project, delete all extracted
            deleteFilesList(extracted);
            return false;
        }

        return true;
    }

    void ImportProjectDialog::deleteFilesRecursion(QString dir)
    {
        for (QFileInfo finfo : QDir(dir).entryInfoList(QDir::AllEntries | QDir::Hidden |  QDir::NoDotAndDotDot))
        {
            if (finfo.isDir())
                deleteFilesRecursion(finfo.absoluteFilePath());
            else
                QFile::remove(finfo.absoluteFilePath());
        }
        QDir().rmdir(dir);
    }

    void ImportProjectDialog::deleteFilesList(QStringList files)
    {
        // Delete files, keep directories (might be not empty)
        auto it = files.begin();
        while (it != files.end())
            if (QFileInfo(*it).isDir())
                ++it;
            else
            {
                QFile::remove(*it);
                it = files.erase(it);
            }

        // Delete directories in revers order
        if (!files.isEmpty())
        {
            for (;;)
            {
                --it;
                QFile::remove(*it);
                if (it == files.begin())
                    break;
            }
        }
    }

    QString ImportProjectDialog::suggestedProjectDir(const QString& filename)
    {
        QString retval = filename;
        retval.remove(QRegularExpression("\\.\\w*$"));

        QString basedir = retval;
        int count       = 2;

        for (;;)    // loop until non existing directory found
        {
            if (!QFileInfo(retval).exists())
                return retval;
            retval = QString("%1_%2").arg(basedir).arg(count++);
        }
        return retval;
    }
}
