#include "gui/action/action.h"

namespace hal
{
    Action::Action(QObject* parent) : QAction(parent), m_tooltip_modified(false)
    {

    }

    Action::Action(const QString& text, QObject* parent) : QAction(parent), m_tooltip_modified(false)
    {
        setText(text);
    }

    Action::Action(const QIcon& icon, const QString& text, QObject* parent) : QAction(parent), m_tooltip_modified(false)
    {
        setText(text);
        setIcon(icon);
    }

    void Action::setText(const QString& text)
    {
        QAction::setText(text);
        update_tooltip(shortcut());
    }

    void Action::setShortcut(const QKeySequence& seq)
    {
        QAction::setShortcut(seq);
        update_tooltip(seq);
    }

    void Action::setToolTip(const QString& tooltip)
    {
        // calling this method will disable automatic tooltips
        m_tooltip_modified = true;
        QAction::setToolTip(tooltip);
    }

    void Action::update_tooltip(const QKeySequence& seq)
    {
        if (m_tooltip_modified)
            return;

        // automatically add the keyboard shortcut to the tooltip
        if (!seq.isEmpty())
        {
            QAction::setToolTip(text() + " '" + seq.toString(QKeySequence::NativeText) + "'");
        }
        else
        {
            QAction::setToolTip(text());
        }
    }
}
