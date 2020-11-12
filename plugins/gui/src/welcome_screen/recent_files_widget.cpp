#include "gui/welcome_screen/recent_files_widget.h"

#include "gui/file_manager/file_manager.h"
#include "gui/gui_globals.h"
#include "gui/welcome_screen/recent_file_item.h"

#include <QList>
#include <QSettings>
#include <QStyle>
#include <QVBoxLayout>

#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>

namespace hal
{
    RecentFilesWidget::RecentFilesWidget(QWidget* parent) : QFrame(parent), mLayout(new QVBoxLayout())
    {
        connect(FileManager::get_instance(), &FileManager::fileOpened, this, &RecentFilesWidget::handleFileOpened);

        mLayout->setContentsMargins(0, 0, 0, 0);
        mLayout->setSpacing(0);

        setLayout(mLayout);
        mLayout->setAlignment(Qt::AlignTop);

        //write_settings();
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

    void RecentFilesWidget::handleFileOpened(const QString& fileName)
    {
        Q_UNUSED(fileName)
        for (const auto item : mItems)
            item->deleteLater();

        mItems.clear();

        // FIX !!!!!!!!!!!!!!!!

        //    for (const QString& file : recent_files)
        //    {
        //        RecentFileItem* item = new RecentFileItem(file, this);
        //        mItems.append(item);
        //        mLayout->addWidget(item);
        //    }
    }

    void RecentFilesWidget::handleRemoveRequested(RecentFileItem *item)
    {
        mLayout->removeWidget(item);
        mItems.removeOne(item);
        //need to delete item, otherwise the item is buggy and will drawn halfway in the widget
        //delete item;
        item->deleteLater();

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
            if(!(info.exists() && info.isFile()))
                item->setDisabled(true);

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
