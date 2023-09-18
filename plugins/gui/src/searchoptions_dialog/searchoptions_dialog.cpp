#include "gui/src/searchoptions_dialog/searchoptions_dialog.h"

namespace hal
{
    SearchOptionsDialog::SearchOptionsDialog(QWidget* parent): QDialog(parent)
    {
        setWindowTitle("Search");
        mLayout = new QGridLayout(this);
    }
}
