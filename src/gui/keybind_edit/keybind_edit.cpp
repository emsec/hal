#include "gui/keybind_edit/keybind_edit.h"

#include <QEvent>
#include <QKeyEvent>
#include <QDebug>

keybind_edit::keybind_edit(QWidget* parent): QKeySequenceEdit(parent), m_validator(stacked_validator()), m_old_sequence(QKeySequence())
{
    // do nothing
}

void keybind_edit::add_validator(validator* v)
{
    m_validator.add_validator(v);
}

void keybind_edit::set_validated(bool validated)
{
    qDebug() << validated;
    m_validated = validated;
}

bool keybind_edit::validated()
{
    return m_validated;
}

void keybind_edit::revalidate()
{
    set_validated(m_validator.validate(keySequence().toString()));
}

bool keybind_edit::event(QEvent* e)
{
    bool recognized = false;
    switch(e->type())
    {
        /*
         * Make sure we have the keyboard exclusively.
         * This suppresses defined keyboard shortcuts
         * and lets us react properly if the user
         * attempts to configure stupid keybinds like
         * Alt-F4.
         */
        case QEvent::FocusIn:
            grabKeyboard();
            recognized = true;
            break;
        case QEvent::FocusOut:
            // FIXME this messes with the API. Better define a validator that
            // can actually handle QVariants and thus QKeySequences.
            if (m_validator.validate(keySequence().toString()))
            {
                // revert
                setKeySequence(m_old_sequence);
                Q_EMIT(edit_rejected());
            }
            else
            {
                m_old_sequence = keySequence();
            }
            releaseKeyboard();
            recognized = true;
            break;
        case QEvent::KeyRelease:
            // FIXME this messes with the API. Better define a validator that
            // can actually handle QVariants and thus QKeySequences.
            revalidate();
            recognized = true;
            break;
        default: break; // suppress -Wswitch compiler warning
    }
    recognized |= QKeySequenceEdit::event(e);
    return recognized;
}