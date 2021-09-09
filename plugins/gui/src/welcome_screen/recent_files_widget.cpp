#include "gui/welcome_screen/recent_files_widget.h"
#include "hal_core/netlist/project_manager.h"

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
        connect(FileManager::get_instance(), &FileManager::projectOpened, this, &RecentFilesWidget::handleProjectUsed);
        connect(FileManager::get_instance(), &FileManager::projectSaved,  this, &RecentFilesWidget::handleProjectUsed);

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

    void RecentFilesWidget::handleProjectUsed(const QString& projDir, const QString& fileName)
    {
        Q_UNUSED(fileName);
        prependPath(projDir);
    }

    void RecentFilesWidget::prependPath(const QString &path)
    {
        auto it = mItems.begin();
        int count = 0;
        while (it != mItems.end())
        {
            // order of conditions is important here, don't count path entry which gets erased
            if ((*it)->file() == path || count++ >= sMaxItems - 1)
            {
                RecentFileItem* item = *it;
                mLayout->removeWidget(item);
                it = mItems.erase(it);
                item->deleteLater();
            }
            else
                ++it;
        }

        RecentFileItem* item = new RecentFileItem(path, this);
        mLayout->insertWidget(0, item);
        mItems.prepend(item);
        item->repolish();

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
        for (int i = 0; i < sMaxItems; ++i)
        {
            gGuiState->setArrayIndex(i);
            QString file = gGuiState->value("file").toString();

            if (file.isEmpty())
                continue;

            RecentFileItem* item = new RecentFileItem(gGuiState->value("file").toString(), this);
            connect(item, &RecentFileItem::removeRequested, this, &RecentFilesWidget::handleRemoveRequested);

            FileManager::DirectoryStatus stat = FileManager::directoryStatus(file);

            if (!item->missing())
                item->setEnabled(stat==FileManager::ProjectDirectory || stat==FileManager::IsFile);

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
            if(index >= sMaxItems)
                break;
        }
        gGuiState->endArray();

    }
}
