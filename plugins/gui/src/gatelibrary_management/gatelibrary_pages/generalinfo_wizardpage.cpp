#include "gui/gatelibrary_management/gatelibrary_pages/generalinfo_wizardpage.h"

#include <QDebug>
#include "gui/gatelibrary_management/gatelibrary_wizard.h"
#include "gui/gui_utils/graphics.h"

namespace hal
{
    GeneralInfoWizardPage::GeneralInfoWizardPage(const GateLibrary* gt, QWidget* parent) : QWizardPage(parent)
    {
        setTitle("General Information");
        setSubTitle("Add general information about the gate, such as name and properties");
        mLayout = new QGridLayout(this);
        mName = new QLineEdit(this);
        mPropertiesSelected = new QListWidget(this);
        mPropertiesAvailable = new QListWidget(this);
        mAddProperty = new QComboBox(this);
        mGateLibrary = gt;

        mLabelName = new QLabel("Name *", this);
        QLabel* labPropertiesSelected = new QLabel("Selected properties *", this);
        QLabel* labPropertiesAvailable = new QLabel("Available properties", this);
        labPropertiesSelected->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        labPropertiesAvailable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        mLabelAddProperty = new QLabel("Add property", this);

        mAddBtn = new QPushButton(this);
        mDelBtn = new QPushButton(this);

        mAddBtn->setMaximumWidth(40);
        mDelBtn->setMaximumWidth(40);
        mLayout->addWidget(mLabelName, 0, 0);
        mLayout->addWidget(mName, 0, 1, 1, 2);
        mLayout->addWidget(labPropertiesSelected, 1, 0);
        mLayout->addWidget(labPropertiesAvailable, 1, 2);
        mLayout->addWidget(mPropertiesSelected, 2, 0, 3, 1);
        mLayout->addWidget(mAddBtn, 2, 1);
        mLayout->addWidget(mDelBtn, 3, 1);
        mLayout->addWidget(mPropertiesAvailable, 2, 2, 3, 1);
        mLayout->addWidget(mLabelAddProperty, 5, 0);
        mLayout->addWidget(mAddProperty, 5, 1, 1, 2);

        setLayout(mLayout);

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

    bool GeneralInfoWizardPage::isEdit()
    {
        return mIsEdit;
    }

    void GeneralInfoWizardPage::setData(QString name, QStringList properties)
    {
        mName->setText(name);
        mPropertiesSelected->addItems(properties);
        mPropertiesSelected->addItems(QStringList{
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
        if(gateInit == "") gateInit = name;

        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        mLeftArrowIcon = gui_utility::getStyledSvgIcon(mEnabledIconStyle, mLeftArrowIconPath);
        mRightArrowIcon = gui_utility::getStyledSvgIcon(mEnabledIconStyle, mRightArrowIconPath);
        mAddBtn->setIcon(mLeftArrowIcon);
        mDelBtn->setIcon(mRightArrowIcon);
    }

    QString GeneralInfoWizardPage::getName()
    {
        return mName->text();
    }

    QStringList GeneralInfoWizardPage::getProperties()
    {
        QStringList res;
        for (int i = 0; i < mPropertiesSelected->count(); i++) {
            res.append(mPropertiesSelected->item(i)->text());
        }
        return res;
    }

    void GeneralInfoWizardPage::addProperty()
    {
        QList<QListWidgetItem*> items = mPropertiesSelected->findItems(mAddProperty->currentText(), Qt::MatchExactly);
        if(items.size() == 0) mPropertiesSelected->addItem(mAddProperty->currentText());

    }

    void GeneralInfoWizardPage::deleteProperty()
    {
        QListWidgetItem *it = mPropertiesSelected->takeItem(mPropertiesSelected->currentRow());
        delete it;
    }

    bool GeneralInfoWizardPage::validatePage()
    {
        for (auto it : mGateLibrary->get_gate_types()) {
            if(!mIsEdit & (QString::fromStdString(it.first) == mName->text() || mPropertiesSelected->count() == 0)) return false;
            else if (mIsEdit & mName->text() != gateInit & (QString::fromStdString(it.first) == mName->text() || mPropertiesSelected->count() == 0)) return false;
        }
        return true;
    }
}
