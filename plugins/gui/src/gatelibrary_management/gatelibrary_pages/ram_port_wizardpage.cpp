#include "gui/gatelibrary_management/gatelibrary_pages/ram_port_wizardpage.h"
#include "gui/gatelibrary_management/gatelibrary_wizard.h"
#include "gui/gatelibrary_management/gatelibrary_label.h"
#include "hal_core/netlist/gate_library/gate_type_component/ram_port_component.h"

namespace hal
{
    RAMPortWizardPage::RAMPortWizardPage(QWidget* parent) : QWizardPage(parent)
    {
        setTitle("RAM Port");
        setSubTitle("Enter parameters for RAM Port component");
        mLayout = new QGridLayout(this);


        /*mDataGroup = new QLineEdit(this);
        mAddressGroup = new QLineEdit(this);
        mClockFunction = new QLineEdit(this);
        mEnableFunciton = new QLineEdit(this);
        mIsWritePort = new QLineEdit(this);

        mLabDataGroup = new QLabel("Name of the data pingroup: ");
        mLabAddressGroup = new QLabel("Name of the address pingroup: ");
        mLabClockFunction = new QLabel("Clock boolean function: ");
        mLabEnableFunciton = new QLabel("Enable boolean function: ");
        mLabIsWritePort = new QLabel("Is a write port: ");

        mLayout->addWidget(mLabDataGroup, 0, 0);
        mLayout->addWidget(mDataGroup, 0, 1);
        mLayout->addWidget(mLabAddressGroup, 1, 0);
        mLayout->addWidget(mAddressGroup, 1, 1);
        mLayout->addWidget(mLabClockFunction, 2, 0);
        mLayout->addWidget(mClockFunction, 2, 1);
        mLayout->addWidget(mLabEnableFunciton, 3, 0);
        mLayout->addWidget(mEnableFunciton, 3, 1);
        mLayout->addWidget(mLabIsWritePort, 4, 0);
        mLayout->addWidget(mIsWritePort, 4, 1);*/

        setLayout(mLayout);
    }

void RAMPortWizardPage::initializePage(){
    mWizard = static_cast<GateLibraryWizard*>(wizard());
    int ramPortCnt = 0;
    QList<PinItem*> pinGroups = mWizard->getPingroups();
    std::vector<GateTypeComponent*> ram_ports;
    if(mGate != nullptr && mGate->has_component_of_type(GateTypeComponent::ComponentType::ram_port))
    {
        ram_ports = mGate->get_components([](const GateTypeComponent* c) {return RAMPortComponent::is_class_of(c);});
    }

    //create empty lines for ram_port for each data/address pair
    //assumption at this point: #data fields = #address fields
    for (int i=0; i<pinGroups.length()-1; i++) { //-1 for dummy entries
        QString type = pinGroups[i]->getType();
        if(type == "data")
        {
            ramPortCnt++;
            mLayout->addWidget(new GateLibraryLabel(false, QString("RAM Port %1").arg(ramPortCnt), this), 6*(ramPortCnt-1), 0);

            mDataGroup = new QLineEdit(this);
            mAddressGroup = new QLineEdit(this);
            mClockFunction = new QLineEdit(this);
            mEnableFunciton = new QLineEdit(this);
            mIsWritePort = new QLineEdit(this);

            mLabDataGroup = new QLabel("Name of the data pingroup: ");
            mLabAddressGroup = new QLabel("Name of the address pingroup: ");
            mLabClockFunction = new QLabel("Clock boolean function: ");
            mLabEnableFunciton = new QLabel("Enable boolean function: ");
            mLabIsWritePort = new QLabel("Is a write port: ");

            mLayout->addWidget(mLabDataGroup, 6*(ramPortCnt-1)+1, 0);
            mLayout->addWidget(mDataGroup, 6*(ramPortCnt-1)+1, 1);
            mLayout->addWidget(mLabAddressGroup, 6*(ramPortCnt-1)+2, 0);
            mLayout->addWidget(mAddressGroup, 6*(ramPortCnt-1)+2, 1);
            mLayout->addWidget(mLabClockFunction, 6*(ramPortCnt-1)+3, 0);
            mLayout->addWidget(mClockFunction, 6*(ramPortCnt-1)+3, 1);
            mLayout->addWidget(mLabEnableFunciton, 6*(ramPortCnt-1)+4, 0);
            mLayout->addWidget(mEnableFunciton, 6*(ramPortCnt-1)+4, 1);
            mLayout->addWidget(mLabIsWritePort, 6*(ramPortCnt-1)+5, 0);
            mLayout->addWidget(mIsWritePort, 6*(ramPortCnt-1)+5, 1);

            if(!ram_ports.empty()) {
                auto ram_port = ram_ports[ramPortCnt-1]->convert_to<RAMPortComponent>();
                mDataGroup->setText(QString::fromStdString(ram_port->get_data_group()));
                mAddressGroup->setText(QString::fromStdString(ram_port->get_address_group()));
                mClockFunction->setText(QString::fromStdString(ram_port->get_clock_function().to_string()));
                mEnableFunciton->setText(QString::fromStdString(ram_port->get_clock_function().to_string()));
                mIsWritePort->setText(ram_port->is_write_port() ? "True":"False");
            }
        }
    }

    setLayout(mLayout);
}

    void RAMPortWizardPage::setData(GateType *gate){
        mGate = gate;
    }
}
