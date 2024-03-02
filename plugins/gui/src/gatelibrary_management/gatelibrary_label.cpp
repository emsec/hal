#include "gui/gatelibrary_management/gatelibrary_label.h"

namespace hal {
    GateLibraryLabel::GateLibraryLabel(bool isVal, const QString& txt, QWidget *parent)
        : QLabel(txt,parent)
    {
        mValue = isVal;
    }
}
