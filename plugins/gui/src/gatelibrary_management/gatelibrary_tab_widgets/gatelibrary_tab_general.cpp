
#include "gui/gatelibrary_management/gatelibrary_tab_widgets/gatelibrary_tab_general.h"

#include "gui/gui_globals.h"
#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"
#include <QFrame>

namespace hal
{
    GateLibraryLabel::GateLibraryLabel(bool isVal, const QString& txt, QWidget *parent)
        : QLabel(txt,parent)
    {
        mValue = isVal;
    }

    GateLibraryTabGeneral::GateLibraryTabGeneral(QWidget* parent) : GateLibraryTabInterface(parent)
    {
        QVBoxLayout* layout = new QVBoxLayout(this);
        mGeneralFrame = new QFrame(this);
        mGeneralFrame->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
        mGeneralFrame->setFrameStyle(QFrame::Sunken | QFrame::Panel);
        mGeneralFrame->setLineWidth(2);
        QFormLayout* generalLayout = new QFormLayout(mGeneralFrame);

        mNamePropertyLabel      = new GateLibraryLabel(true, " - ", mGeneralFrame);
        mIdPropertyLabel        = new GateLibraryLabel(true, " - ", mGeneralFrame);
        mComponentPropertyLabel = new GateLibraryLabel(true, " - ", mGeneralFrame);

        generalLayout->addRow(new GateLibraryLabel(false, "General", mGeneralFrame));
        generalLayout->addRow(new GateLibraryLabel(false, "Name:", mGeneralFrame), mNamePropertyLabel);
        generalLayout->addRow(new GateLibraryLabel(false, "ID:", mGeneralFrame), mIdPropertyLabel);
        generalLayout->addRow(new GateLibraryLabel(false, "Component:", mGeneralFrame), mComponentPropertyLabel);
        layout->addWidget(mGeneralFrame);

        mFlipflopFrame = new QFrame(this);
        mFlipflopFrame->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
        mFlipflopFrame->setFrameStyle(QFrame::Sunken | QFrame::Panel);
        mFlipflopFrame->setLineWidth(2);
        QFormLayout* flipflopLayout = new QFormLayout(mFlipflopFrame);

        mClockPropertyLabel                = new GateLibraryLabel(true, " - ", mFlipflopFrame);
        mNextStatePropertyLabel            = new GateLibraryLabel(true, " - ", mFlipflopFrame);
        mAsynchronousResetPropertyLabel    = new GateLibraryLabel(true, " - ", mFlipflopFrame);
        mInternalStatePropertyLabel        = new GateLibraryLabel(true, " - ", mFlipflopFrame);
        mNegatedInternalStatePropertyLabel = new GateLibraryLabel(true, " - ", mFlipflopFrame);

        flipflopLayout->addRow(new GateLibraryLabel(false, "Flip Flops", mFlipflopFrame));
        flipflopLayout->addRow(new GateLibraryLabel(false, "Clock:", parent), mClockPropertyLabel);
        flipflopLayout->addRow(new GateLibraryLabel(false, "Next state:", parent), mNextStatePropertyLabel);
        flipflopLayout->addRow(new GateLibraryLabel(false, "Asynchronous reset:", parent), mAsynchronousResetPropertyLabel);
        flipflopLayout->addRow(new GateLibraryLabel(false, "Internal state:", parent), mInternalStatePropertyLabel);
        flipflopLayout->addRow(new GateLibraryLabel(false, "Negated internal state:", parent), mNegatedInternalStatePropertyLabel);
        layout->addWidget(mFlipflopFrame);
        mFlipflopFrame->hide();

        mBooleanFrame = new QFrame(this);
        mBooleanFrame->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
        mBooleanFrame->setFrameStyle(QFrame::Sunken | QFrame::Panel);
        mBooleanFrame->setLineWidth(2);
        QFormLayout* booleanLayout = new QFormLayout(mBooleanFrame);
        mBooleanFunctionPropertyLabel = new GateLibraryLabel(true, " - ", mBooleanFrame);
        booleanLayout->addRow(new GateLibraryLabel(false, "Boolean Functions", mBooleanFrame));
        booleanLayout->addRow(new GateLibraryLabel(false, "O:", mBooleanFrame), mBooleanFunctionPropertyLabel);
        layout->addWidget(mBooleanFrame);
        mBooleanFrame->hide();
    }

    void GateLibraryTabGeneral::update(GateType* gate)
    {

        if(!gate){
            //TODO make default look
            mNamePropertyLabel->setText("-");
            mIdPropertyLabel->setText("-");
            mComponentPropertyLabel->setText("-");
            mFlipflopFrame->hide();
            mBooleanFrame->hide();

            return;
        }

        mNamePropertyLabel->setText(QString::fromStdString(gate->get_name()));
        mIdPropertyLabel->setText(QString::number(gate->get_id()));
        //TODO add component
        mComponentPropertyLabel->setText("TODO");


        mBooleanFunctionPropertyLabel->setText(QString::fromStdString(gate->get_boolean_function().to_string()));
        if (mBooleanFunctionPropertyLabel->text().isEmpty())
            mBooleanFrame->hide();
        else
            mBooleanFrame->show();

        if (gate->has_component_of_type(GateTypeComponent::ComponentType::ff))
        {
            auto ff = gate->get_component_as<FFComponent>([](const GateTypeComponent* c) { return FFComponent::is_class_of(c); });

            if (ff != nullptr)
            {
                mClockPropertyLabel->setText(QString::fromStdString(ff->get_clock_function().to_string()));
                mNextStatePropertyLabel->setText(QString::fromStdString(ff->get_next_state_function().to_string()));
                mAsynchronousResetPropertyLabel->setText(QString::fromStdString(ff->get_async_reset_function().to_string()));

                mInternalStatePropertyLabel->setText(QString::fromStdString(gate->get_boolean_function().to_string()));

                Result<BooleanFunction> result = BooleanFunction::Not(gate->get_boolean_function(), gate->get_boolean_function().size());
                if(result.is_ok())
                    mNegatedInternalStatePropertyLabel->setText(QString::fromStdString(result.get().to_string()));
                else{
                    mNegatedInternalStatePropertyLabel->setText("ERROR");
                }
                mFlipflopFrame->show();
            }
            else
                mFlipflopFrame->hide();
        }
        else
            mFlipflopFrame->hide();

        QString in = "  ";
        QString out = "  ";

        for(auto const& pin : gate->get_input_pin_names()){
            in += QString::fromStdString(pin) + "\n  ";
        }

        for(auto const& pin : gate->get_output_pin_names()){
            out += QString::fromStdString(pin) + "\n  ";
        }

    }


}
