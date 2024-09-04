#include "gui/file_status_manager/file_status_manager.h"
#include "gui/file_manager/file_manager.h"

#include "gui/gui_globals.h"

namespace hal
{
    FileStatusManager::FileStatusManager(QObject* parent) : QObject(parent),
        mModifiedFilesUuid(QSet<QUuid>()), mModifiedFilesDescriptors(QMap<QUuid, QString>()),
        mNetlistModified(false), mGatelibModified(false)
    {;}

    FileStatusManager::~FileStatusManager()
    {;}

    void FileStatusManager::fileChanged(const QUuid uuid, const QString& descriptor)
    {
        bool before = modifiedFilesExisting();
        mModifiedFilesUuid.insert(uuid);
        mModifiedFilesDescriptors.insert(uuid, descriptor);
        if (before != modifiedFilesExisting())
            Q_EMIT status_changed(false, true);
    }

    void FileStatusManager::fileSaved(const QUuid uuid)
    {
        bool before = modifiedFilesExisting();
        mModifiedFilesUuid.remove(uuid);
        mModifiedFilesDescriptors.remove(uuid);
        if (before != modifiedFilesExisting())
            Q_EMIT status_changed(false, false);
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
        bool before = modifiedFilesExisting();
        mNetlistModified = true;
        if (before != modifiedFilesExisting())
            Q_EMIT status_changed(false, true);
    }

    void FileStatusManager::netlistSaved()
    {
        bool before = modifiedFilesExisting();
        mNetlistModified = false;
        if (before != modifiedFilesExisting())
            Q_EMIT status_changed(false, false);
    }

    void FileStatusManager::netlistClosed()
    {
        mNetlistModified = false;
        Q_EMIT status_changed(false, false);
    }

    void FileStatusManager::gatelibChanged()
    {
        mGatelibModified = true;
        Q_EMIT status_changed(true, true);
    }

    void FileStatusManager::gatelibSaved()
    {
        mGatelibModified = false;
        Q_EMIT status_changed(true, false);
    }

    bool FileStatusManager::isGatelibModified() const
    {
        return mGatelibModified;
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
