#include "gui/gatelibrary_management/gatelibrary_frames/gatelibrary_frame_lut.h"
#include "gui/gatelibrary_management/gatelibrary_label.h"
#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"

#include <algorithm>
#include <vector>

namespace hal
{
    GatelibraryFrameLut::GatelibraryFrameLut(QWidget* parent)
        : GatelibraryComponentFrame("LUT Init", parent)
    {
        mAscending = new GateLibraryLabel(true, " - ", this);
        mLayout->addRow(new GateLibraryLabel(false, "Bit Order:", parent), mAscending);
        mLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    }

    void GatelibraryFrameLut::update(GateType* gt)
    {
        if (!gt->has_component_of_type(GateTypeComponent::ComponentType::init))
        {
            hide();
            return;
        }

        auto lutc = gt->get_component_as<LUTComponent>([](const GateTypeComponent* c) { return LUTComponent::is_class_of(c); });
        if (lutc == nullptr)
        {
            hide();
            return;
        }

        mAscending->setText(lutc->is_init_ascending() ? "Ascending" : "Descending");

        // Remove any previously added output-pin rows.
        // Row 0 = title span (base class), row 1 = "Bit Order:" (fixed) — start removal at row 2.
        while (mLayout->rowCount() > 2)
            mLayout->removeRow(2);

        // Add one row per output pin config, sorted by pin name for deterministic order
        const auto& raw = lutc->get_output_pin_configs();
        std::vector<std::pair<std::string, LUTComponent::LUTOutputConfig>> configs(raw.begin(), raw.end());
        std::sort(configs.begin(), configs.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

        for (const auto& [pin_name, cfg] : configs)
        {
            QString value = QString::fromStdString(cfg.init_identifier);
            if (cfg.bit_count > 0)
                value += QString(" [%1..%2]").arg(cfg.bit_offset).arg(cfg.bit_offset + cfg.bit_count - 1);
            mLayout->addRow(new GateLibraryLabel(false, QString::fromStdString(pin_name) + ":", this),
                            new GateLibraryLabel(true, value, this));
        }

        show();
    }
}

