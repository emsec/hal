#include "gui/gatelibrary_management/gatelibrary_pages/generalinfo_wizardpage.h"

#include <QDebug>

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
        mAddProperty->addItems(QStringList{
                                   "combinational",
                                   "sequential",
                                   "tristate",
                                   "power",
                                   "ground",
                                   "ff",
                                   "latch",
                                   "ram",
                                   "io",
                                   "dsp",
                                   "pll",
                                   "oscillator",
                                   "scan",
                                   "c_buffer",
                                   "c_inverter",
                                   "c_and",
                                   "c_nand",
                                   "c_or",
                                   "c_nor",
                                   "c_xor",
                                   "c_xnor",
                                   "c_aoi",
                                   "c_oai",
                                   "c_mux",
                                   "c_carry",
                                   "c_half_adder",
                                   "c_full_adder",
                                   "c_lut"
                               });
        connect(mAddBtn, &QPushButton::clicked, this, &GeneralInfoWizardPage::addProperty);
        connect(mDelBtn, &QPushButton::clicked, this, &GeneralInfoWizardPage::deleteProperty);

    }

    void GeneralInfoWizardPage::setMode(bool edit)
    {
        mIsEdit = edit;
    }

    void GeneralInfoWizardPage::setData(QString name, QStringList properties)
    {
        mName->setText(name);
        mProperties->addItems(properties);
        if(gateInit == "") gateInit = name;
    }

    QString GeneralInfoWizardPage::getName()
    {
        return mName->text();
    }

    QStringList GeneralInfoWizardPage::getProperties()
    {
        QStringList res;
        for (int i = 0; i < mProperties->count(); i++) {
            qInfo()<<mProperties->item(i)->text();
            res.append(mProperties->item(i)->text());
        }
        return res;
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
            if(!mIsEdit & (QString::fromStdString(it.first) == mName->text() || mProperties->count() == 0)) return false;
            else if (mIsEdit & mName->text() != gateInit & (QString::fromStdString(it.first) == mName->text() || mProperties->count() == 0)) return false;
        }
        return true;
    }
}
