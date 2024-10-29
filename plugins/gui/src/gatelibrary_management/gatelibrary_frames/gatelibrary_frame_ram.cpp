#include "gui/gatelibrary_management/gatelibrary_frames/gatelibrary_frame_ram.h"
#include "gui/gatelibrary_management/gatelibrary_label.h"
#include "hal_core/netlist/gate_library/gate_type_component/ram_component.h"

namespace hal
{
    GateLibraryFrameRAM::GateLibraryFrameRAM(QWidget* parent)
        : GatelibraryComponentFrame("RAM", parent)
    {
        mBitSize = new GateLibraryLabel(true, " - ", this);

        mLayout->addRow(new GateLibraryLabel(false, "RAM bit size:", parent), mBitSize);

        mLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

    }

    void GateLibraryFrameRAM::update(GateType* gt)
    {
        if(gt->has_component_of_type(GateTypeComponent::ComponentType::ram))
        {
            auto ram = gt->get_component_as<RAMComponent>([](const GateTypeComponent* c) {return RAMComponent::is_class_of(c);});

            if(ram != nullptr)
            {
                mBitSize->setText(QString::fromStdString(std::to_string(ram->get_bit_size())));
                show();
            }
            else
                hide();
        }
        else
            hide();
    }
}

