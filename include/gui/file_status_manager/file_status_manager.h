#ifndef FILE_STATUS_MANAGER_H
#define FILE_STATUS_MANAGER_H

#include <QObject>
#include <QSet>
#include <QMap>
#include <QString>
#include <QList>
#include <QUuid>

class file_status_manager : public QObject
{
    Q_OBJECT
public:
    file_status_manager(QObject* parent = 0);
    ~file_status_manager();

    bool modified_files_existing() const;

    void file_changed(const QUuid uuid, const QString& descriptor);
    void file_saved(const QUuid uuid, const QString& descriptor);

    QList<QString> get_unsaved_change_descriptors() const;

private:
    QSet<QUuid> m_modified_files_uuid;
    QMap<QUuid, QString> m_modified_files_descriptors;

};

#endif // FILE_STATUS_MANAGER_H
