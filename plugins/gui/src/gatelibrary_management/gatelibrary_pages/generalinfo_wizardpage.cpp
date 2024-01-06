#include "gui/gatelibrary_management/gatelibrary_pages/generalinfo_wizardpage.h"

namespace hal
{
    GeneralInfoWizardPage::GeneralInfoWizardPage(const GateLibrary* gt, QWidget* parent) : QWizardPage(parent)
    {
        setTitle("Step 1: General Information");
        setSubTitle("Add general information about the gate, such as name and properties");
        mLayout = new QGridLayout(this);
        mName = new QLineEdit(this);
        mProperties = new QListWidget(this);
        mAddProperty = new QComboBox(this);
        mGateLibrary = gt;

        mLabelName = new QLabel("Name *", this);
        mLabelProperties = new QLabel("Properties *", this);
        mLabelAddProperty = new QLabel("Add property", this);

        mAddBtn = new QPushButton("Add", this);
        mDelBtn = new QPushButton("Delete", this);

        mLayout->addWidget(mLabelName, 0, 0);
        mLayout->addWidget(mName, 0, 1, 1, 2);
        mLayout->addWidget(mLabelProperties, 1, 0);
        mLayout->addWidget(mProperties, 2, 0, 1, 2);
        mLayout->addWidget(mLabelAddProperty, 3, 0);
        mLayout->addWidget(mAddProperty, 3, 1, 1, 2);
        mLayout->addWidget(mAddBtn, 4, 0);
        mLayout->addWidget(mDelBtn, 4, 1);

        setLayout(mLayout);

        registerField("name*", mName);
        registerField("properties", mProperties);

        //TODO: fetch data from enum GateTypeProperty
        mAddProperty->addItems(QStringList{"combinational", "sequential"});
        connect(mAddBtn, &QPushButton::clicked, this, &GeneralInfoWizardPage::addProperty);
        connect(mDelBtn, &QPushButton::clicked, this, &GeneralInfoWizardPage::deleteProperty);

    }

    void GeneralInfoWizardPage::setData(QString name, QStringList properties)
    {
        mName->setText(name);
        mProperties->addItems(properties);
    }

    void GeneralInfoWizardPage::addProperty()
    {
        QList<QListWidgetItem*> items = mProperties->findItems(mAddProperty->currentText(), Qt::MatchExactly);
        if(items.size() == 0) mProperties->addItem(mAddProperty->currentText());
    }

    void GeneralInfoWizardPage::deleteProperty()
    {
        QListWidgetItem *it = mProperties->takeItem(mProperties->currentRow());
        delete it;
    }

    bool GeneralInfoWizardPage::validatePage()
    {
        for (auto it : mGateLibrary->get_gate_types()) {
            if(QString::fromStdString(it.first) == mName->text() || mProperties->count() == 0) return false;
        }
        return true;
    }
}
