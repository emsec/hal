#include "gui/keybind_edit/keybind_edit.h"

#include <QEvent>
#include <QKeyEvent>
#include <QMessageBox>

namespace hal
{
    KeybindEdit::KeybindEdit(QWidget* parent): QKeySequenceEdit(parent)
    {;}

    void KeybindEdit::addValidator(Validator* v)
    {
        mValidator.addValidator(v);
    }

    void KeybindEdit::setValidated(bool validated)
    {
       mValidated = validated;
    }

    bool KeybindEdit::validated()
    {
        return mValidated;
    }

    void KeybindEdit::revalidate()
    {
        setValidated(mValidator.validate(keySequence().toString()));
    }

    bool KeybindEdit::event(QEvent* e)
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
                // FIXME this messes with the API. Better define a Validator that
                // can actually handle QVariants and thus QKeySequences.
                if (!mValidator.validate(keySequence().toString()))
                {
                    // revert
                    QKeySequence failed = keySequence();
                    setKeySequence(mOldSequence);
                    if (failed != mFailedValidate)
                    {
                        QMessageBox::warning(this, "Rejected!", mValidator.failText());
                        mFailedValidate = failed;
                    }
                    Q_EMIT(editRejected());
                }
                else
                {
                    mOldSequence = keySequence();
                }
                releaseKeyboard();
                recognized = true;
                break;
            case QEvent::KeyRelease:
                // FIXME this messes with the API. Better define a Validator that
                // can actually handle QVariants and thus QKeySequences.
                revalidate();
                recognized = true;
                break;
            default: break; // suppress -Wswitch compiler warning
        }
        recognized |= QKeySequenceEdit::event(e);
        return recognized;
    }
}
