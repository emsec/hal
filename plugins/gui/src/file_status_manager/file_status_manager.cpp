#include "gui/file_status_manager/file_status_manager.h"
#include "gui/file_manager/file_manager.h"

#include "gui/gui_globals.h"

namespace hal
{
    FileStatusManager::FileStatusManager(QObject* parent) : QObject(parent), mModifiedFilesUuid(QSet<QUuid>()), mModifiedFilesDescriptors(QMap<QUuid, QString>())
    {
        mNetlistModified = false;
    }

    FileStatusManager::~FileStatusManager()
    {
    }

    void FileStatusManager::fileChanged(const QUuid uuid, const QString& descriptor)
    {
        mModifiedFilesUuid.insert(uuid);
        mModifiedFilesDescriptors.insert(uuid, descriptor);
    }

    void FileStatusManager::fileSaved(const QUuid uuid)
    {
        mModifiedFilesUuid.remove(uuid);
        mModifiedFilesDescriptors.remove(uuid);
    }

    bool FileStatusManager::modifiedFilesExisting() const
    {
        if (!FileManager::get_instance()->fileOpen())
            return false;

        return !mModifiedFilesUuid.empty() || mNetlistModified;
    }

    void FileStatusManager::flushUnsavedChanges()
    {
        mModifiedFilesUuid.clear();
        mModifiedFilesDescriptors.clear();
        netlistSaved();
    }

    void FileStatusManager::netlistChanged()
    {
        mNetlistModified = true;
    }

    void FileStatusManager::netlistSaved()
    {
        mNetlistModified = false;
    }

    QList<QString> FileStatusManager::getUnsavedChangeDescriptors() const
    {
        QList<QString> unsaved_changes_descriptors;

        if(mNetlistModified)
        {
            unsaved_changes_descriptors.append("Netlist modifications");
        }

        for(QUuid uuid : mModifiedFilesUuid)
        {
            unsaved_changes_descriptors.append(mModifiedFilesDescriptors.value(uuid));
        }

        return unsaved_changes_descriptors;
    }
}
