#include "gui/welcome_screen/recent_files_widget.h"
#include "hal_core/utilities/project_manager.h"

#include "gui/file_manager/file_manager.h"
#include "gui/gui_globals.h"
#include "gui/welcome_screen/recent_file_item.h"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QList>
#include <QSettings>
#include <QStyle>
#include <QVBoxLayout>
#include <QDebug>

namespace hal
{
    RecentFilesWidget::RecentFilesWidget(QWidget* parent) : QFrame(parent), mLayout(new QVBoxLayout())
    {
        connect(FileManager::get_instance(), &FileManager::fileOpened, this, &RecentFilesWidget::handleFileOpened);
        connect(FileManager::get_instance(), &FileManager::projectOpened, this, &RecentFilesWidget::handleProjectOpened);

        mLayout->setContentsMargins(0, 0, 0, 0);
        mLayout->setSpacing(0);

        setLayout(mLayout);
        mLayout->setAlignment(Qt::AlignTop);

        readSettings();
    }

    void RecentFilesWidget::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        for (QObject* object : mLayout->children())
        {
            RecentFileItem* item = qobject_cast<RecentFileItem*>(object);

            if (item)
                item->repolish();
        }
    }

    void RecentFilesWidget::handleProjectOpened(const QString& projDir, const QString& fileName)
    {
        Q_UNUSED(fileName);
        handleFileOpened(projDir);
    }

    void RecentFilesWidget::handleFileOpened(const QString& fileName)
    {
        for (const RecentFileItem* item : mItems)
        {
            if (item->file() == fileName)
                return; // DEBUG
        }

        RecentFileItem* item = new RecentFileItem(fileName);
        mItems.prepend(item);
        mLayout->insertWidget(0, item);

        if (mItems.size() > 14)
        {
            // HACKY, FIX
            RecentFileItem* last_item = mItems.last();
            mItems.removeLast();
            mLayout->removeWidget(last_item);
        }

        updateSettings();
    }

    void RecentFilesWidget::handleRemoveRequested(RecentFileItem* item)
    {
        mLayout->removeWidget(item);
        mItems.removeOne(item);

        delete item;

        updateSettings();
    }

    void RecentFilesWidget::readSettings()
    {
        gGuiState->beginReadArray("recent_files");
        for (int i = 0; i < 14; ++i)
        {
            gGuiState->setArrayIndex(i);
            QString file = gGuiState->value("file").toString();

            if (file.isEmpty())
                continue;

            RecentFileItem* item = new RecentFileItem(gGuiState->value("file").toString(), this);
            connect(item, &RecentFileItem::removeRequested, this, &RecentFilesWidget::handleRemoveRequested);

            QFileInfo info(file);

            bool disabl = false;
            if (info.exists())
            {
                if (info.isDir())
                {
                    disabl = !QFileInfo(QDir(info.absoluteFilePath()).absoluteFilePath(QString::fromStdString(ProjectManager::s_project_file))).exists();
                }
                else
                    disabl = !info.isFile();
            }
            else
                disabl = true;

            if (disabl)
                item->setDisabled(disabl);

            mItems.append(item);
            mLayout->addWidget(item);
            item->repolish();
        }
        gGuiState->endArray();
    }

    void RecentFilesWidget::updateSettings()
    {
        gGuiState->beginGroup("recent_files");
        gGuiState->remove("");
        gGuiState->endGroup();

        gGuiState->beginWriteArray("recent_files");
        int index = 0;
        for(RecentFileItem* item : mItems)
        {
            gGuiState->setArrayIndex(index);
            gGuiState->setValue("file", item->file());
            index++;
            if(index == 14)
                break;
        }
        gGuiState->endArray();

    }
}
