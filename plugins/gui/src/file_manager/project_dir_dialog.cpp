#include "gui/file_manager/project_dir_dialog.h"
#include "gui/file_manager/file_manager.h"
#include <QDialogButtonBox>
#include <QEvent>
#include <QMessageBox>
#include <QGridLayout>
#include <QLabel>
#include <QIcon>
#include <QStyle>
#include <QHBoxLayout>

namespace hal {
    // credits to icon designer https://www.flaticon.com/authors/ian-june
    QPixmap* ProjectDirDialogStatus::sCheckMark = nullptr;

    // credits to icon designer https://www.flaticon.com/authors/amonrat-rungreangfangsai
    QPixmap* ProjectDirDialogStatus::sAttention = nullptr;

    ProjectDirDialogStatus::ProjectDirDialogStatus(QWidget* parent)
        : QFrame(parent)
    {
        setFrameStyle(QFrame::Panel | QFrame::Sunken);
        setLineWidth(2);
        QHBoxLayout* lay = new QHBoxLayout(this);
        lay->addWidget(mIcon = new QLabel(this));
        lay->addWidget(mText = new QLabel(this));
        lay->addStretch(0);
    }

    QPixmap ProjectDirDialogStatus::getPixmap(bool ok)
    {
        if (ok)
        {
            if (!sCheckMark) sCheckMark = new QPixmap(":/icons/check-mark", "PNG");
            return *sCheckMark;
        }
        if (!sAttention) sAttention = new QPixmap(":/icons/attention", "PNG");
        return *sAttention;
    }

    void ProjectDirDialogStatus::setMessage(const QString &path, FileManager::DirectoryStatus stat)
    {
        mIcon->setPixmap(getPixmap(stat == FileManager::ProjectDirectory));
        mText->setText(path + "\n" + FileManager::directoryStatusText(stat));
    }

    ProjectDirDialog::ProjectDirDialog(const QString &title, const QString &defaultDir, QWidget* parent)
        : QFileDialog(parent), mChooseButton(nullptr), mStatus(nullptr),
          mSelectedDirectoryStatus(FileManager::OtherDirectory)
    {
        setWindowTitle(title);
        setOption(QFileDialog::DontUseNativeDialog, true);
        setOption(QFileDialog::ShowDirsOnly,true);
        setFileMode(QFileDialog::Directory);
        setFilter(QDir::Dirs | QDir::NoDot  | QDir::NoDotDot  );
        setDirectory(defaultDir);
        connect(this, &QFileDialog::currentChanged, this, &ProjectDirDialog::handleCurrentChanged);
        connect(this, &QFileDialog::directoryEntered, this, &ProjectDirDialog::handleCurrentChanged);

        QGridLayout* glay = findChild<QGridLayout*>("gridLayout");
        if (glay)
        {
            mStatus = new ProjectDirDialogStatus(this);
            glay->addWidget(mStatus,4,0,1,3);
        }

        QDialogButtonBox* bbox = findChild<QDialogButtonBox*>("buttonBox");
        if (bbox)
        {
//            dumpObjectTree();
            for (QAbstractButton* but : bbox->buttons())
            {
                if (but->text() == "&Choose" || but->text() == "&Open")
                {
                    mChooseButton = but;
                    break;
                }
            }
        }
        if (mChooseButton)
            mChooseButton->installEventFilter(this);
        handleCurrentChanged(defaultDir);
    }

    bool ProjectDirDialog::isSelectable() const
    {
        return mSelectedDirectoryStatus == FileManager::ProjectDirectory;
    }

    bool ProjectDirDialog::eventFilter(QObject* obj, QEvent* event)
    {
        if (obj == mChooseButton)
        {
           if (event->type() == QEvent::EnabledChange)
            {
                if (isSelectable())
                {
                    if (mChooseButton->isEnabled())
                    {
                        return false;
                    }
                    else
                    {
                        mChooseButton->setEnabled(true);
                        mChooseButton->update();
                        return true;
                    }
                }
                else
                {
                    if (mChooseButton->isEnabled())
                    {
                        mChooseButton->setEnabled(false);
                        mChooseButton->update();
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
            }
        }
        return QFileDialog::eventFilter(obj, event);
    }

    void ProjectDirDialog::accept()
    {
        if (isSelectable())
        {
            QDialog::accept();
        }
        else
        {
            QMessageBox::warning(this,"Warning","Selected item is not a HAL project:\n"
                                 + FileManager::directoryStatusText(mSelectedDirectoryStatus));
            QDialog::reject();
        }
    }

    void ProjectDirDialog::handleCurrentChanged(const QString& path)
    {
        QString testPath = QFileInfo(path).isAbsolute()
                ? path
                : QDir(directory()).absoluteFilePath(path);

        mSelectedDirectoryStatus = FileManager::directoryStatus(testPath);

        if (mChooseButton)
            mChooseButton->setEnabled(isSelectable());
        if (mStatus)
            mStatus->setMessage(testPath, mSelectedDirectoryStatus);
    }

}
