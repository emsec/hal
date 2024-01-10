#include "gui/gatelibrary_management/gatelibrary_pages/bool_wizardpage.h"

#include <gui/gatelibrary_management/gatelibrary_wizard.h>

namespace hal
{
    BoolWizardPage::BoolWizardPage(QWidget* parent) : QWizardPage(parent)
    {
        setTitle("Step 4: Boolean functions");
        setSubTitle("TODO: subtitle");
        mLayout = new QGridLayout(this);
    }
    int BoolWizardPage::nextId() const
    {
        auto parentWizard = wizard();
        if(!parentWizard)
            return -1;
        return static_cast<GateLibraryWizard*>(parentWizard)->getNextPageId(GateLibraryWizard::BooleanFunction);
    }
}
