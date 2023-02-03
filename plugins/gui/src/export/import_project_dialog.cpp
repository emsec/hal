#include "gui/export/import_project_dialog.h"
#include "gui/file_manager/file_manager.h"
#include "gui/gui_utils/graphics.h"
#include "hal_core/utilities/log.h"
#include <QGridLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QLabel>
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

        mButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,this);
        connect(mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        layout->addWidget(mButtonBox);
        handleSelectionStatusChanged();
    }

    void ImportProjectDialog::handleSelectionStatusChanged()
    {
        mButtonBox->button(QDialogButtonBox::Ok)->setEnabled(mZippedFile->valid() && mTargetDirectory->valid());
    }

    bool ImportProjectDialog::importProject()
    {
        mStatus = NoFileSelected;
        if (mZippedFile->selection().isEmpty()) return false;

        mStatus = ErrorDecompress;
        QStringList extracted = JlCompress::extractDir(mZippedFile->selection(),mTargetDirectory->selection());
        if (extracted.isEmpty()) return false;

        mExtractedProjectDir = extracted.at(0);
        while (mExtractedProjectDir.back().unicode() == '/' || mExtractedProjectDir.back().unicode() == '\\')
            mExtractedProjectDir.chop(1);

        switch (FileManager::directoryStatus(mExtractedProjectDir))
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
            log_warning("gui", "Failed to decompress archiv '{}', result is neither a file nor a directory.", mZippedFile->selection().toStdString());
            mStatus = NotAHalProject;
            break;
        }

        if (mStatus != Ok)
        {
            // Not a hal project, delete extracted
            auto it = extracted.begin();
            while (it != extracted.end())
                if (QFileInfo(*it).isDir())
                    ++it;
                else
                {
                    QFile::remove(*it);
                    it = extracted.erase(it);
                }

            // Delete directories in revers order
            if (!extracted.isEmpty())
            {
                for (;;)
                {
                    --it;
                    QFile::remove(*it);
                    if (it == extracted.begin())
                        break;
                }
            }
            return false;
        }

        return true;
    }
}
