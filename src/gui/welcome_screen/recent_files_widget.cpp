#include "welcome_screen/recent_files_widget.h"

#include "file_manager/file_manager.h"
#include "gui_globals.h"
#include "welcome_screen/recent_file_item.h"

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
    recent_files_widget::recent_files_widget(QWidget* parent) : QFrame(parent), m_layout(new QVBoxLayout())
    {
        connect(FileManager::get_instance(), &FileManager::file_opened, this, &recent_files_widget::handle_file_opened);

        m_layout->setContentsMargins(0, 0, 0, 0);
        m_layout->setSpacing(0);

        setLayout(m_layout);
        m_layout->setAlignment(Qt::AlignTop);

        //write_settings();
        read_settings();
    }

    void recent_files_widget::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        for (QObject* object : m_layout->children())
        {
            recent_file_item* item = qobject_cast<recent_file_item*>(object);

            if (item)
                item->repolish();
        }
    }

    void recent_files_widget::handle_file_opened(const QString& file_name)
    {
        Q_UNUSED(file_name)
        for (const auto item : m_items)
            item->deleteLater();

        m_items.clear();

        // FIX !!!!!!!!!!!!!!!!

        //    for (const QString& file : recent_files)
        //    {
        //        recent_file_item* item = new recent_file_item(file, this);
        //        m_items.append(item);
        //        m_layout->addWidget(item);
        //    }
    }

    void recent_files_widget::handle_remove_requested(recent_file_item *item)
    {
        m_layout->removeWidget(item);
        m_items.removeOne(item);
        //need to delete item, otherwise the item is buggy and will drawn halfway in the widget
        //delete item;
        item->deleteLater();

        update_settings();
    }

    void recent_files_widget::read_settings()
    {
        g_gui_state.beginReadArray("recent_files");
        for (int i = 0; i < 14; ++i)
        {
            g_gui_state.setArrayIndex(i);
            QString file = g_gui_state.value("file").toString();

            if (file.isEmpty())
                continue;

            recent_file_item* item = new recent_file_item(g_gui_state.value("file").toString(), this);
            connect(item, &recent_file_item::remove_requested, this, &recent_files_widget::handle_remove_requested);

            QFileInfo info(file);
            if(!(info.exists() && info.isFile()))
                item->set_disabled(true);

            m_items.append(item);
            m_layout->addWidget(item);
            item->repolish();
        }
        g_gui_state.endArray();
    }

    void recent_files_widget::update_settings()
    {
        g_gui_state.beginGroup("recent_files");
        g_gui_state.remove("");
        g_gui_state.endGroup();

        g_gui_state.beginWriteArray("recent_files");
        int index = 0;
        for(recent_file_item* item : m_items)
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
