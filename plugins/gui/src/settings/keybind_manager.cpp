#include "gui/settings/keybind_manager.h"

#include "gui/gui_globals.h"

#include <QDebug>

namespace hal
{
    KeybindManager::KeybindManager(QObject* parent) : QObject(parent)
    {
        connect(gSettingsRelay, &SettingsRelay::settingChanged, this, &KeybindManager::handleGlobalSettingChanged);
    }

    void KeybindManager::bind(Action* action, const QString& key)
    {
        Q_ASSERT(!mBoundActions.contains(action));
        // keep track of this action
        mBoundActions.insert(action);
        mBindingsActions.insert(key, action);
        // set initial key bind
        QKeySequence seq = gSettingsManager->get(key).toString();
        action->setShortcut(seq);
    }

    void KeybindManager::bind(QShortcut* shortcut, const QString& key)
    {
        Q_ASSERT(!mBoundShortcuts.contains(shortcut));
        // keep track of this shortcut
        mBoundShortcuts.insert(shortcut);
        mBindingsShortcuts.insert(key, shortcut);
        // set initial key bind
        QKeySequence seq = gSettingsManager->get(key).toString();
        shortcut->setKey(seq);
    }

    void KeybindManager::release(Action* action)
    {
        Q_ASSERT(mBoundActions.remove(action));
        // remove all matching values from map
        deleteAllValues(mBindingsActions, action);
    }

    void KeybindManager::release(QShortcut* shortcut)
    {
        Q_ASSERT(mBoundShortcuts.remove(shortcut));
        // remove all matching values from map
        deleteAllValues(mBindingsShortcuts, shortcut);
    }

    QShortcut* KeybindManager::makeShortcut(QWidget* parent, const QString& key)
    {
        // initialize shortcut with whatever keybind is configured under that key
        QKeySequence seq = gSettingsManager->get(key).toString();
        QShortcut* shortcut = new QShortcut(seq, parent);
        // keep track of this shortcut
        mBoundShortcuts.insert(shortcut);
        mBindingsShortcuts.insert(key, shortcut);
        return shortcut;
    }

    void KeybindManager::handleGlobalSettingChanged(void* sender, const QString& key, const QVariant& value)
    {
        Q_UNUSED(sender);
        QKeySequence seq = value.toString();
        QList<Action*> affected_actions = mBindingsActions.values(key);
        for (Action* a : affected_actions)
        {
            a->setShortcut(seq);
            qDebug() << a->text();
        }
        QList<QShortcut*> affected_shortcuts = mBindingsShortcuts.values(key);
        for (QShortcut* s : affected_shortcuts)
        {
            s->setKey(seq);
            qDebug() << s->objectName();
        }
    }
}
