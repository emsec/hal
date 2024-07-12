#include "gui/gatelibrary_management/gatelibrary_pages/ram_port_wizardpage.h"
#include "gui/gatelibrary_management/gatelibrary_wizard.h"
#include "gui/gatelibrary_management/gatelibrary_label.h"

namespace hal
{
    RAMPortWizardPage::RAMPortWizardPage(QWidget* parent) : QWizardPage(parent)
    {
        setTitle("RAM Port");
        setSubTitle("Enter parameters for RAM Port component");
        mLayout = new QGridLayout(this);

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
    for (int i=0; i<pinGroups.length(); i++) { //-1 for dummy entries
        if(pinGroups[i]->getItemType() == PinItem::TreeItemType::GroupCreator) continue;
        PinType type = pinGroups[i]->getPinType();
        if(type == PinType::data)
        {
            ramPortCnt++;
            mLayout->addWidget(new GateLibraryLabel(false, QString("RAM Port %1").arg(ramPortCnt), this), 6*(ramPortCnt-1), 0);

            RAMPort rp;
            rp.dataGroup = new QLineEdit(this);
            rp.addressGroup = new QLineEdit(this);
            rp.clockFunction = new QLineEdit(this);
            rp.enableFunciton = new QLineEdit(this);
            rp.isWritePort = new QComboBox(this);
            rp.isWritePort->addItems({"True", "False"});

            mLabDataGroup = new QLabel("Name of the data pingroup: ");
            mLabAddressGroup = new QLabel("Name of the address pingroup: ");
            mLabClockFunction = new QLabel("Clock boolean function: ");
            mLabEnableFunciton = new QLabel("Enable boolean function: ");
            mLabIsWritePort = new QLabel("Is a write port: ");

            mLayout->addWidget(mLabDataGroup, 6*(ramPortCnt-1)+1, 0);
            mLayout->addWidget(rp.dataGroup, 6*(ramPortCnt-1)+1, 1);
            mLayout->addWidget(mLabAddressGroup, 6*(ramPortCnt-1)+2, 0);
            mLayout->addWidget(rp.addressGroup, 6*(ramPortCnt-1)+2, 1);
            mLayout->addWidget(mLabClockFunction, 6*(ramPortCnt-1)+3, 0);
            mLayout->addWidget(rp.clockFunction, 6*(ramPortCnt-1)+3, 1);
            mLayout->addWidget(mLabEnableFunciton, 6*(ramPortCnt-1)+4, 0);
            mLayout->addWidget(rp.enableFunciton, 6*(ramPortCnt-1)+4, 1);
            mLayout->addWidget(mLabIsWritePort, 6*(ramPortCnt-1)+5, 0);
            mLayout->addWidget(rp.isWritePort, 6*(ramPortCnt-1)+5, 1);

            if(!ram_ports.empty()) {
                auto ram_port = ram_ports[ramPortCnt-1]->convert_to<RAMPortComponent>();
                rp.dataGroup->setText(QString::fromStdString(ram_port->get_data_group()));
                rp.addressGroup->setText(QString::fromStdString(ram_port->get_address_group()));
                rp.clockFunction->setText(QString::fromStdString(ram_port->get_clock_function().to_string()));
                rp.enableFunciton->setText(QString::fromStdString(ram_port->get_clock_function().to_string()));
                rp.isWritePort->setCurrentText(ram_port->is_write_port() ? "True":"False");
            }
            mRamPortEdits.append(rp);
        }
    }

    setLayout(mLayout);
}

    void RAMPortWizardPage::setData(GateType *gate){
        mGate = gate;
    }

    QList<RAMPortWizardPage::RAMPort> RAMPortWizardPage::getRamPorts(){
        return mRamPortEdits;
    }
}
