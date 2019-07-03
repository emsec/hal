#ifndef SHORTCUT_MANAGER_H
#define SHORTCUT_MANAGER_H

#include <QObject>
#include <QShortcut>

class shortcut_manager : public QObject
{
    Q_OBJECT

public:
    explicit shortcut_manager(QObject* parent = nullptr);

private:
    QShortcut* m_open_file;
    QShortcut* m_close_file;
    QShortcut* m_save;
};

#endif // SHORTCUT_MANAGER_H
