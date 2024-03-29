#include "gui/gatelibrary_management/gatelibrary_frames/gatelibrary_frame_ram_port.h"
#include "gui/gatelibrary_management/gatelibrary_label.h"
#include "hal_core/netlist/gate_library/gate_type_component/ram_port_component.h"

#include <QDebug>

namespace hal
{
    GateLibraryFrameRAMPort::GateLibraryFrameRAMPort(QWidget* parent)
        : GatelibraryComponentFrame("RAM Ports", parent)
    {
        mLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    }

    void GateLibraryFrameRAMPort::update(GateType* gt)
    {
        QLayoutItem* item;
        if (!mLayout->isEmpty())
        {
            while ((item = mLayout->takeAt(0)) != nullptr)  {
                delete item->widget();
                delete item;
            }
        }

        mLayout->insertRow(0, new GateLibraryLabel(false, "RAM Ports", this));


        if(gt->has_component_of_type(GateTypeComponent::ComponentType::ram_port))
        {
            auto ram_ports = gt->get_components([](const GateTypeComponent* c) {return RAMPortComponent::is_class_of(c);});
            int cnt = 1;

            if(!ram_ports.empty())
            {
                for (auto comp : ram_ports) {
                    mLayout->addRow(new GateLibraryLabel(false, QString("RAM Port %1").arg(cnt), this));
                    auto ram_port = comp->convert_to<RAMPortComponent>();

                    GateLibraryLabel* dataGroup = new GateLibraryLabel(true,QString::fromStdString(ram_port->get_data_group()), this);
                    GateLibraryLabel* addressGroup = new GateLibraryLabel(true, QString::fromStdString(ram_port->get_address_group()), this);
                    GateLibraryLabel* clkFunc = new GateLibraryLabel(true, QString::fromStdString(ram_port->get_clock_function().to_string()), this);
                    GateLibraryLabel* enableFunc = new GateLibraryLabel(true, QString::fromStdString(ram_port->get_enable_function().to_string()), this);
                    GateLibraryLabel* writePort = new GateLibraryLabel(true, ram_port->is_write_port() ? "True" : "False", this);

                    mLayout->addRow(new GateLibraryLabel(false, "Name of the data pingroup:", this), dataGroup);
                    mLayout->addRow(new GateLibraryLabel(false, "Name of the address pingroup:", this), addressGroup);
                    mLayout->addRow(new GateLibraryLabel(false, "Clock boolean function:", this), clkFunc);
                    mLayout->addRow(new GateLibraryLabel(false, "Enable boolean function:", this), enableFunc);
                    mLayout->addRow(new GateLibraryLabel(false, "Is a write port:", this), writePort);

                    cnt++;
                }

                show();
            }
            else
                hide();
        }
        else
            hide();
    }
}

