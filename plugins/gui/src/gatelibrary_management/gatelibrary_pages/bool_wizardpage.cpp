#include "gui/gatelibrary_management/gatelibrary_pages/bool_wizardpage.h"

namespace hal
{
    BoolWizardPage::BoolWizardPage(QWidget* parent) : QWizardPage(parent)
    {
        setTitle("Step 4: Boolean functions");
        setSubTitle("TODO: subtitle");
        mLayout = new QGridLayout(this);
    }
}
