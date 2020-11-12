#include "gui/action/action.h"

namespace hal
{
    Action::Action(QObject* parent) : QAction(parent), mTooltipModified(false)
    {

    }

    Action::Action(const QString& text, QObject* parent) : QAction(parent), mTooltipModified(false)
    {
        setText(text);
    }

    Action::Action(const QIcon& icon, const QString& text, QObject* parent) : QAction(parent), mTooltipModified(false)
    {
        setText(text);
        setIcon(icon);
    }

    void Action::setText(const QString& text)
    {
        QAction::setText(text);
        updateTooltip(shortcut());
    }

    void Action::setShortcut(const QKeySequence& seq)
    {
        QAction::setShortcut(seq);
        updateTooltip(seq);
    }

    void Action::setToolTip(const QString& tooltip)
    {
        // calling this method will disable automatic tooltips
        mTooltipModified = true;
        QAction::setToolTip(tooltip);
    }

    void Action::updateTooltip(const QKeySequence& seq)
    {
        if (mTooltipModified)
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
