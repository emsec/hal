#include "hal_action/hal_action.h"

namespace hal
{
    HalAction::HalAction(QObject* parent) : QAction(parent), m_tooltip_modified(false)
    {

    }

    HalAction::HalAction(const QString& text, QObject* parent) : QAction(parent), m_tooltip_modified(false)
    {
        setText(text);
    }

    HalAction::HalAction(const QIcon& icon, const QString& text, QObject* parent) : QAction(parent), m_tooltip_modified(false)
    {
        setText(text);
        setIcon(icon);
    }

    void HalAction::setText(const QString& text)
    {
        QAction::setText(text);
        update_tooltip(shortcut());
    }

    void HalAction::setShortcut(const QKeySequence& seq)
    {
        QAction::setShortcut(seq);
        update_tooltip(seq);
    }

    void HalAction::setToolTip(const QString& tooltip)
    {
        // calling this method will disable automatic tooltips
        m_tooltip_modified = true;
        QAction::setToolTip(tooltip);
    }

    void HalAction::update_tooltip(const QKeySequence& seq)
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
