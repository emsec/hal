#include "gui/file_status_manager/file_status_manager.h"
#include "gui/file_manager/file_manager.h"

#include "gui/gui_globals.h"

namespace hal
{
    FileStatusManager::FileStatusManager(QObject* parent) : QObject(parent),
        mModifiedFilesUuid(QSet<QUuid>()), mModifiedFilesDescriptors(QMap<QUuid, QString>()),
        mNetlistState(NotOpen), mGatelibState(NotOpen)
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

        return !mModifiedFilesUuid.empty() || mNetlistState == Dirty;
    }

    void FileStatusManager::netlistOpened()
    {
        mNetlistState = Clean;
        Q_EMIT open_changed(false, true);
    }

    void FileStatusManager::netlistChanged()
    {
        bool before = modifiedFilesExisting();
        mNetlistState = Dirty;
        if (before != modifiedFilesExisting())
            Q_EMIT status_changed(false, true);
    }

    void FileStatusManager::netlistSaved()
    {
        bool before = modifiedFilesExisting();
        mNetlistState = Clean;
        if (before != modifiedFilesExisting())
            Q_EMIT status_changed(false, false);
    }

    void FileStatusManager::netlistClosed()
    {
        mNetlistState = NotOpen;
        mModifiedFilesUuid.clear();
        mModifiedFilesDescriptors.clear();
        Q_EMIT status_changed(false, false);
        Q_EMIT open_changed(false, false);
    }

    void FileStatusManager::gatelibOpened()
    {
        mGatelibState = Clean;
        Q_EMIT open_changed(true, true);
    }

    void FileStatusManager::gatelibChanged()
    {
        mGatelibState = Dirty;
        Q_EMIT status_changed(true, true);
    }

    void FileStatusManager::gatelibSaved()
    {
        mGatelibState = Clean;
        Q_EMIT status_changed(true, false);
    }

    bool FileStatusManager::isGatelibModified() const
    {
        return mGatelibState == Dirty;
    }

    void FileStatusManager::gatelibClosed()
    {
        mGatelibState = NotOpen;
        Q_EMIT open_changed(true, false);
    }

    QList<QString> FileStatusManager::getUnsavedChangeDescriptors() const
    {
        QList<QString> unsaved_changes_descriptors;

        if(mNetlistState==Dirty)
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
