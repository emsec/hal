#include "gui/gatelibrary_management/gatelibrary_pages/generalinfo_wizardpage.h"

namespace hal
{
    GeneralInfoWizardPage::GeneralInfoWizardPage(QWidget* parent) : QWizardPage(parent)
    {
        setTitle("Step 1: General Information");
        setSubTitle("Add general information about the gate, such as name and properties");
        mLayout = new QGridLayout(this);
        mName = new QLineEdit(this);
        mProperties = new QListWidget(this);
        mAddProperty = new QComboBox(this);

        mLabelName = new QLabel("Name", this);
        mLabelProperties = new QLabel("Properties", this);
        mLabelAddProperty = new QLabel("Add property", this);

        mLayout->addWidget(mLabelName, 0, 0);
        mLayout->addWidget(mName, 0, 1, 1, 2);
        mLayout->addWidget(mLabelProperties, 1, 0);
        mLayout->addWidget(mProperties, 2, 0, 1, 2);
        mLayout->addWidget(mLabelAddProperty, 3, 0);
        mLayout->addWidget(mAddProperty, 3, 1, 1, 2);

        setLayout(mLayout);

        //TODO: fetch data from enum GateTypeProperty
        mAddProperty->addItems(QStringList{"combinational", "sequential"});
    }

    void GeneralInfoWizardPage::setData(QString name, QStringList properties)
    {
        mName->setText(name);
        mProperties->addItems(properties);
    }
}
