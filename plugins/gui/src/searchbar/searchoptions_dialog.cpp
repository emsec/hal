#include "gui/searchbar/searchoptions_dialog.h"

namespace hal
{
    SearchOptionsDialog::SearchOptionsDialog(QWidget* parent): QDialog(parent)
    {
        setWindowTitle("Search");
        mLayout = new QGridLayout(this);
    }
}
