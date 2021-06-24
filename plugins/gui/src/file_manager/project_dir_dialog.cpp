#include "gui/file_manager/project_dir_dialog.h"
#include "hal_core/utilities/project_manager.h"
#include <QDebug>
#include <QDialogButtonBox>
#include <QEvent>
#include <QMessageBox>

namespace hal {


    ProjectDirDialog::ProjectDirDialog(QWidget* parent)
        : QFileDialog(parent), mChooseButton(nullptr), mSelectable(false)
    {
        setFilter(QDir::Dirs | QDir::NoDot  | QDir::NoDotDot  );
        setDirectory("/home/langhein/netlist");
        setFileMode(QFileDialog::DirectoryOnly);
        connect(this, &QFileDialog::currentChanged, this, &ProjectDirDialog::handleCurrentChanged);

        QDialogButtonBox* bbox = findChild<QDialogButtonBox*>("buttonBox");
        if (bbox)
        {
            for (QAbstractButton* but : bbox->buttons())
                if (but->text() == "&Choose")
                {
                    mChooseButton = but;
                    break;
                }
        }
        if (mChooseButton)
            mChooseButton->installEventFilter(this);
    }

    bool ProjectDirDialog::isSelectable(const QString &path) const
    {
        QDir currentDir(path.isEmpty() ? directory() : path);
        return QFileInfo(currentDir.absoluteFilePath(QString::fromStdString(ProjectManager::s_project_file))).exists();
    }

    bool ProjectDirDialog::eventFilter(QObject* obj, QEvent* event)
    {
        if (obj == mChooseButton)
        {
           if (event->type() == QEvent::EnabledChange)
            {
                if (mSelectable)
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
        if (mSelectable)
        {
            QDialog::accept();
        }
        else
        {
            QMessageBox::warning(this,"Warning","Selected directory is not a hal project");
        }
    }

    void ProjectDirDialog::handleCurrentChanged(const QString& path)
    {
        mSelectable = isSelectable(path);
        if (mChooseButton)
            mChooseButton->setEnabled(mSelectable);
    }

}
