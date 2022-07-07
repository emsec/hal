#include "gui/file_manager/project_dir_dialog.h"
#include "gui/file_manager/file_manager.h"
#include "hal_core/netlist/project_manager.h"
#include <QDebug>
#include <QDialogButtonBox>
#include <QEvent>
#include <QMessageBox>

namespace hal {


    ProjectDirDialog::ProjectDirDialog(const QString &title, QWidget* parent)
        : QFileDialog(parent), mChooseButton(nullptr), mSelectable(false)
    {
        setWindowTitle(title);
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
        return (FileManager::directoryStatus(path) == FileManager::ProjectDirectory);
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
            QDialog::reject();
        }
    }

    void ProjectDirDialog::handleCurrentChanged(const QString& path)
    {
        mSelectable = isSelectable(path);
        if (mChooseButton)
            mChooseButton->setEnabled(mSelectable);
    }

}
