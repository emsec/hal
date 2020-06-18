#include "hal_action/hal_action.h"
namespace hal{
hal_action::hal_action(QObject* parent) : QAction(parent), m_tooltip_modified(false)
{

}

hal_action::hal_action(const QString& text, QObject* parent) : QAction(parent), m_tooltip_modified(false)
{
    setText(text);
}

hal_action::hal_action(const QIcon& icon, const QString& text, QObject* parent) : QAction(parent), m_tooltip_modified(false)
{
    setText(text);
    setIcon(icon);
}

void hal_action::setText(const QString& text)
{
    QAction::setText(text);
    update_tooltip(shortcut());
}

void hal_action::setShortcut(const QKeySequence& seq)
{
    QAction::setShortcut(seq);
    update_tooltip(seq);
}

void hal_action::setToolTip(const QString& tooltip)
{
    // calling this method will disable automatic tooltips
    m_tooltip_modified = true;
    QAction::setToolTip(tooltip);
}

void hal_action::update_tooltip(const QKeySequence& seq)
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
