#include "gui/gatelibrary_management/gatelibrary_label.h"
#include <QStyle>

namespace hal {
    GateLibraryLabel::GateLibraryLabel(bool isVal, const QString& txt, QWidget *parent)
        : QLabel(txt,parent)
    {
        mValue = isVal;
        setWordWrap(true);
    }

    void GateLibraryLabel::setValue(bool isVal)
    {
        if (isVal == mValue) return;
        mValue = isVal;

        QStyle* s = style();
        s->unpolish(this);
        s->polish(this);
    }

}
