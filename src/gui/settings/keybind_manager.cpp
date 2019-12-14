#include "settings/keybind_manager.h"

#include "gui/gui_globals.h"

#include <QDebug>

keybind_manager::keybind_manager(QObject* parent) : QObject(parent)
{
    connect(&g_settings_relay, &settings_relay::setting_changed, this, &keybind_manager::handle_global_setting_changed);
}

void keybind_manager::bind(hal_action* action, const QString& key)
{
    Q_ASSERT(!m_bound_actions.contains(action));
    // keep track of this action
    m_bound_actions.insert(action);
    m_bindings_actions.insert(key, action);
    // set initial key bind
    QKeySequence seq = g_settings_manager.get(key).toString();
    action->setShortcut(seq);
}

void keybind_manager::bind(QShortcut* shortcut, const QString& key)
{
    Q_ASSERT(!m_bound_shortcuts.contains(shortcut));
    // keep track of this shortcut
    m_bound_shortcuts.insert(shortcut);
    m_bindings_shortcuts.insert(key, shortcut);
    // set initial key bind
    QKeySequence seq = g_settings_manager.get(key).toString();
    shortcut->setKey(seq);
}

void keybind_manager::release(hal_action* action)
{
    Q_ASSERT(m_bound_actions.remove(action));
    // remove all matching values from map
    delete_all_values(m_bindings_actions, action);
}

void keybind_manager::release(QShortcut* shortcut)
{
    Q_ASSERT(m_bound_shortcuts.remove(shortcut));
    // remove all matching values from map
    delete_all_values(m_bindings_shortcuts, shortcut);
}

QShortcut* keybind_manager::make_shortcut(QWidget* parent, const QString& key)
{
    // initialize shortcut with whatever keybind is configured under that key
    QKeySequence seq = g_settings_manager.get(key).toString();
    QShortcut* shortcut = new QShortcut(seq, parent);
    // keep track of this shortcut
    m_bound_shortcuts.insert(shortcut);
    m_bindings_shortcuts.insert(key, shortcut);
    return shortcut;
}

void keybind_manager::handle_global_setting_changed(void* sender, const QString& key, const QVariant& value)
{
    Q_UNUSED(sender);
    QKeySequence seq = value.toString();
    QList<hal_action*> affected_actions = m_bindings_actions.values(key);
    for (hal_action* a : affected_actions)
    {
        a->setShortcut(seq);
        qDebug() << a->text();
    }
    QList<QShortcut*> affected_shortcuts = m_bindings_shortcuts.values(key);
    for (QShortcut* s : affected_shortcuts)
    {
        s->setKey(seq);
        qDebug() << s->objectName();
    }
}
