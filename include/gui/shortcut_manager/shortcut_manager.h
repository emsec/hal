#pragma once

#include <QObject>
#include <QShortcut>
namespace hal{
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
}
