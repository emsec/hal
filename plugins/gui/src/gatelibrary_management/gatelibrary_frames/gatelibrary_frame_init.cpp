#include "gui/gatelibrary_management/gatelibrary_frames/gatelibrary_frame_init.h"
#include "gui/gatelibrary_management/gatelibrary_label.h"
#include "hal_core/netlist/gate_library/gate_type_component/init_component.h"

namespace hal
{
    GatelibraryFrameInit::GatelibraryFrameInit(QWidget* parent)
        : GatelibraryComponentFrame("Init", parent)
    {
        mCategoryLabel                = new GateLibraryLabel(true, " - ", this);
        mIdentifiersLabel                = new GateLibraryLabel(true, " - ", this);

        mLayout->addRow(new GateLibraryLabel(false, "Category:",                  parent), mCategoryLabel);
        mLayout->addRow(new GateLibraryLabel(false, "Identifiers:",                  parent), mIdentifiersLabel);

    }

    void GatelibraryFrameInit::update(GateType* gt)
    {
        if(gt->has_component_of_type(GateTypeComponent::ComponentType::init))
        {
            auto init = gt->get_component_as<InitComponent>([](const GateTypeComponent* c) { return InitComponent::is_class_of(c); });

            if(init != nullptr)
            {
                mCategoryLabel->setText(QString::fromStdString(init->get_init_category()));
                QString ids = "";

                bool first = true;
                for (std::string i : init->get_init_identifiers()) {
                    if(first)
                        first = false;
                    else
                        ids.append("\n");

                    ids.append(QString::fromStdString(i));
                }
                mIdentifiersLabel->setText(ids);
                show();
            }
            else hide();
        }
        else hide();
    }
}

