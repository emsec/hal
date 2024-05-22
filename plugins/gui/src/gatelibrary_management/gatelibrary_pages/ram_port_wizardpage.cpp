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
        mLabDataGroup = new QLabel("Name of the data pingroup: ");
        mLabAddressGroup = new QLabel("Name of the address pingroup: ");
        mLabClockFunction = new QLabel("Clock boolean function: ");
        mLabEnableFunciton = new QLabel("Enable boolean function: ");
        mLabIsWritePort = new QLabel("Is a write port: ");

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
    int ramPortCnt = 1;
    QList<PinItem*> pinGroups = mWizard->getPingroups();
    /*for (auto pinGroup : mWizard->getPingroups()) {
        //assumption at this point: #data fields = #address fields
        if(pinGroup->type == PinType::data) ramPortCnt++;
    }*/
    //check for equality?
    //if(dataCnt!=addrCnt)

    //create empty lines for ram_port for each data/address pair
    for (int i=0; i<pinGroups.length(); i++) {
        QString name = pinGroups[i]->getName();
        QString type = pinGroups[i]->getType();
        if(type == "data")
        {
            mLayout->addWidget(new GateLibraryLabel(false, QString("RAM Port %1").arg(ramPortCnt), this));

            mDataGroup = new QLineEdit(this);
            mAddressGroup = new QLineEdit(this);
            mClockFunction = new QLineEdit(this);
            mEnableFunciton = new QLineEdit(this);
            mIsWritePort = new QLineEdit(this);

            mLayout->addWidget(mLabDataGroup, 0+i*5, 0);
            mLayout->addWidget(mDataGroup, 0+i*5, 1);
            mLayout->addWidget(mLabAddressGroup, 1+i*5, 0);
            mLayout->addWidget(mAddressGroup, 1+i*5, 1);
            mLayout->addWidget(mLabClockFunction, 2+i*5, 0);
            mLayout->addWidget(mClockFunction, 2+i*5, 1);
            mLayout->addWidget(mLabEnableFunciton, 3+i*5, 0);
            mLayout->addWidget(mEnableFunciton, 3+i*5, 1);
            mLayout->addWidget(mLabIsWritePort, 4+i*5, 0);
            mLayout->addWidget(mIsWritePort, 4+i*5, 1);

            ramPortCnt++;
        }
    }

    setLayout(mLayout);

}

    void RAMPortWizardPage::setData(GateType *gate){
        if(gate != nullptr && gate->has_component_of_type(GateTypeComponent::ComponentType::ram_port))
        {
            auto ram_ports = gate->get_components([](const GateTypeComponent* c) {return RAMPortComponent::is_class_of(c);});
            int cnt = 1;

        if(!ram_ports.empty())
        {
            for (auto comp : ram_ports) {
                //mLayout->addWidget(new GateLibraryLabel(false, QString("RAM Port %1").arg(cnt), this));
                auto ram_port = comp->convert_to<RAMPortComponent>();

                GateLibraryLabel* dataGroup = new GateLibraryLabel(true,QString::fromStdString(ram_port->get_data_group()), this);
                GateLibraryLabel* addressGroup = new GateLibraryLabel(true, QString::fromStdString(ram_port->get_address_group()), this);
                GateLibraryLabel* clkFunc = new GateLibraryLabel(true, QString::fromStdString(ram_port->get_clock_function().to_string()), this);
                GateLibraryLabel* enableFunc = new GateLibraryLabel(true, QString::fromStdString(ram_port->get_enable_function().to_string()), this);
                GateLibraryLabel* writePort = new GateLibraryLabel(true, ram_port->is_write_port() ? "True" : "False", this);

                /*mLayout->addWidget(new GateLibraryLabel(false, "Name of the data pingroup:", this), dataGroup);
                mLayout->addWidget(new GateLibraryLabel(false, "Name of the address pingroup:", this), addressGroup);
                mLayout->addWidget(new GateLibraryLabel(false, "Clock boolean function:", this), clkFunc);
                mLayout->addWidget(new GateLibraryLabel(false, "Enable boolean function:", this), enableFunc);
                mLayout->addWidget(new GateLibraryLabel(false, "Is a write port:", this), writePort);*/

                cnt++;
            }
        }
    }
    }
}
