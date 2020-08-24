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
    RecentFilesWidget::RecentFilesWidget(QWidget* parent) : QFrame(parent), m_layout(new QVBoxLayout())
    {
        connect(FileManager::get_instance(), &FileManager::file_opened, this, &RecentFilesWidget::handle_file_opened);

        m_layout->setContentsMargins(0, 0, 0, 0);
        m_layout->setSpacing(0);

        setLayout(m_layout);
        m_layout->setAlignment(Qt::AlignTop);

        //write_settings();
        read_settings();
    }

    void RecentFilesWidget::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        for (QObject* object : m_layout->children())
        {
            RecentFileItem* item = qobject_cast<RecentFileItem*>(object);

            if (item)
                item->repolish();
        }
    }

    void RecentFilesWidget::handle_file_opened(const QString& file_name)
    {
        Q_UNUSED(file_name)
        for (const auto item : m_items)
            item->deleteLater();

        m_items.clear();

        // FIX !!!!!!!!!!!!!!!!

        //    for (const QString& file : recent_files)
        //    {
        //        RecentFileItem* item = new RecentFileItem(file, this);
        //        m_items.append(item);
        //        m_layout->addWidget(item);
        //    }
    }

    void RecentFilesWidget::handle_remove_requested(RecentFileItem *item)
    {
        m_layout->removeWidget(item);
        m_items.removeOne(item);
        //need to delete item, otherwise the item is buggy and will drawn halfway in the widget
        //delete item;
        item->deleteLater();

        update_settings();
    }

    void RecentFilesWidget::read_settings()
    {
        g_gui_state.beginReadArray("recent_files");
        for (int i = 0; i < 14; ++i)
        {
            g_gui_state.setArrayIndex(i);
            QString file = g_gui_state.value("file").toString();

            if (file.isEmpty())
                continue;

            RecentFileItem* item = new RecentFileItem(g_gui_state.value("file").toString(), this);
            connect(item, &RecentFileItem::remove_requested, this, &RecentFilesWidget::handle_remove_requested);

            QFileInfo info(file);
            if(!(info.exists() && info.isFile()))
                item->set_disabled(true);

            m_items.append(item);
            m_layout->addWidget(item);
            item->repolish();
        }
        g_gui_state.endArray();
    }

    void RecentFilesWidget::update_settings()
    {
        g_gui_state.beginGroup("recent_files");
        g_gui_state.remove("");
        g_gui_state.endGroup();

        g_gui_state.beginWriteArray("recent_files");
        int index = 0;
        for(RecentFileItem* item : m_items)
        {
            g_gui_state.setArrayIndex(index);
            g_gui_state.setValue("file", item->file());
            index++;
            if(index == 14)
                break;
        }
        g_gui_state.endArray();

    }
}
