#include "settings/keybind_manager.h"

#include "gui/gui_globals.h"

#include <QDebug>

keybind_manager::keybind_manager(QObject* parent) : QObject(parent)
{
    connect(&g_settings_relay, &settings_relay::setting_changed, this, &keybind_manager::handle_global_setting_changed);
}

void keybind_manager::bind(hal_action* action, const QString& key)
{
    Q_ASSERT(!m_bound.contains(action));
    // keep track of this action
    m_bound.insert(action);
    m_bindings.insert(key, action);
    // set initial key bind
    QKeySequence seq = g_settings_manager.get(key).toString();
    action->setShortcut(seq);
}

void keybind_manager::release(hal_action* action)
{
    Q_ASSERT(m_bound.remove(action));
    // remove all matching values from map
    for (auto it = m_bindings.begin(); it != m_bindings.end();)
    {
        if (it.value() == action)
        {
            it = m_bindings.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void keybind_manager::handle_global_setting_changed(void* sender, const QString& key, const QVariant& value)
{
    QList<hal_action*> affected = m_bindings.values(key);
    QKeySequence seq = value.toString();
    for (hal_action* a : affected)
    {
        a->setShortcut(seq);
        qDebug() << a->text();
    }
}