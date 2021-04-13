#include "gui/keybind_edit/keybind_edit.h"
#include "gui/settings/assigned_keybind_map.h"
#include "gui/settings/settings_items/settings_item_keybind.h"

#include <QEvent>
#include <QKeyEvent>
#include <QStyle>

namespace hal
{
    KeybindEdit::KeybindEdit(QWidget* parent)
        : QKeySequenceEdit(parent), mItem(nullptr),
          mSkipValidate(false), mValidated(true), mGrab(false)
    {;}

    void KeybindEdit::setHasGrab(bool isgrab)
    {
        if (isgrab==mGrab) return;
        mGrab = isgrab;

        setStyleSheet(QString("background: rgb(0,0,%1);").arg(mGrab ? 40 : 0));

        QStyle* s = style();
        s->unpolish(this);
        s->polish(this);

    }

    void KeybindEdit::setValidated(bool valid)
    {
       mValidated = valid;
    }

    bool KeybindEdit::hasGrab()
    {
        return mGrab;
    }

    bool KeybindEdit::validated()
    {
        return mValidated;
    }

    bool KeybindEdit::doValidate()
    {
        if (mSkipValidate) return true;
        QKeySequence current = keySequence();
        SettingsItemKeybind* item = AssignedKeybindMap::instance()->currentAssignment(current);
        bool ok = (item == nullptr || item == mItem);

        setValidated(ok);
        if (ok)
        {
            AssignedKeybindMap::instance()->assign(current,mItem,mOldSequence);
            mOldSequence = current;
            Q_EMIT(editAccepted());
        }
        else
        {
            // revert
            Q_EMIT(editRejected(
                        QString("<%1> is already assigned:\n<%2>")
                        .arg(current.toString())
                        .arg(item->label())));
            restoreOldSequence();
        }
        return ok;
    }

    void KeybindEdit::load(const QKeySequence& seq, SettingsItemKeybind* item)
    {
        QKeySequence previousSequence = keySequence();
        mOldSequence = seq;
        mItem = item;
        QKeySequenceEdit::setKeySequence(seq);
        AssignedKeybindMap::instance()->assign(seq, item, previousSequence);
    }

    void KeybindEdit::restoreOldSequence()
    {
        mSkipValidate = true;
        setKeySequence(mOldSequence);
        mSkipValidate = false;
    }

    bool KeybindEdit::event(QEvent* e)
    {
        bool recognized = true;
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
            setHasGrab(true);
            break;
        case QEvent::FocusOut:
            // FIXME this messes with the API. Better define a Validator that
            // can actually handle QVariants and thus QKeySequences.
            doValidate();
            setHasGrab(false);
            releaseKeyboard();
            break;
        case QEvent::KeyRelease:
            // FIXME this messes with the API. Better define a Validator that
            // can actually handle QVariants and thus QKeySequences.
            doValidate();
            break;
        default:
            recognized = false;
            break;
        }
        recognized |= QKeySequenceEdit::event(e);
        return recognized;
    }
}
