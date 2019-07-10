#include "file_status_manager/file_status_manager.h"
#include "file_manager/file_manager.h"

#include "gui_globals.h"

file_status_manager::file_status_manager(QObject* parent) : QObject(parent), m_modified_files_uuid(QSet<QUuid>()), m_modified_files_descriptors(QMap<QUuid, QString>())
{

}

file_status_manager::~file_status_manager()
{

}

void file_status_manager::file_changed(const QUuid uuid, const QString& descriptor)
{
    m_modified_files_uuid.insert(uuid);
    m_modified_files_descriptors.insert(uuid, descriptor);
}

void file_status_manager::file_saved(const QUuid uuid, const QString& descriptor)
{
    m_modified_files_uuid.remove(uuid);
    m_modified_files_descriptors.remove(uuid);
}

bool file_status_manager::modified_files_existing() const
{
    if (!file_manager::get_instance()->file_open())
        return false;

    return !m_modified_files_uuid.empty() || m_netlist_modified;
}

void file_status_manager::flush_unsaved_changes()
{
    m_modified_files_uuid.clear();
    m_modified_files_descriptors.clear();
    netlist_saved();
}

void file_status_manager::netlist_changed()
{
    m_netlist_modified = true;
}

void file_status_manager::netlist_saved()
{
    m_netlist_modified = false;
}

QList<QString> file_status_manager::get_unsaved_change_descriptors() const
{
    QList<QString> unsaved_changes_descriptors;

    if(m_netlist_modified)
        unsaved_changes_descriptors.append("Netlist modifications");

    for(QUuid uuid : m_modified_files_uuid)
    {
        unsaved_changes_descriptors.append(m_modified_files_descriptors.value(uuid));
    }

    return unsaved_changes_descriptors;
}
