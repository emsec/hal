#include "gui/gatelibrary_management/gatelibrary_pages/flipflop_wizardpage.h"

#include <QDebug>

namespace hal
{
    FlipFlopWizardPage::FlipFlopWizardPage(QWidget* parent) : QWizardPage(parent)
    {
        setTitle("Step 3: Flip Flop");
        setSubTitle("TODO: subtitle");
        mLayout = new QGridLayout(this);
        mTabWidget = new QTabWidget(this);
        //mFlipFlopTab = new GateLibraryTabFlipFlop(this);
        //mTabWidget->addTab(mFlipFlopTab, "Flip Flop");

        //TODO:
        //mTabWidget->addTab(mStateTableTab, "State Table");

    }

    void FlipFlopWizardPage::initializePage()
    {
        qInfo() << field("name").toString();
        qInfo() << field("properties").toInt();
    }
}
