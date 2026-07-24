#include "gui/gatelibrary_management/gatelibrary_pages/lut_wizardpage.h"
#include "gui/gatelibrary_management/gatelibrary_wizard.h"
#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"
#include "gui/pin_model/pin_item.h"

#include <QHeaderView>
#include <QSpinBox>

namespace hal
{
    LUTWizardPage::LUTWizardPage(QWidget* parent) : QWizardPage(parent)
    {
        setTitle("LUT Init");
        setSubTitle("Enter parameters for LUT component");
        mLayout = new QGridLayout(this);

        mLabAscending = new QLabel("Bit order ascending: ");
        mAscending    = new QCheckBox(this);
        mLayout->addWidget(mLabAscending, 0, 0);
        mLayout->addWidget(mAscending,    0, 1);

        mPinConfigTable = new QTableWidget(0, 4, this);
        mPinConfigTable->setHorizontalHeaderLabels({"Output Pin", "Init Identifier", "Bit Offset", "Bit Count"});
        mPinConfigTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        mPinConfigTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        mPinConfigTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        mPinConfigTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
        mPinConfigTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        mPinConfigTable->setSelectionMode(QAbstractItemView::SingleSelection);
        mLayout->addWidget(mPinConfigTable, 1, 0, 1, 2);

        QWidget*     btnWidget = new QWidget(this);
        QHBoxLayout* btnLayout = new QHBoxLayout(btnWidget);
        btnLayout->setContentsMargins(0, 0, 0, 0);
        btnLayout->addStretch();
        mLayout->addWidget(btnWidget, 2, 0, 1, 2);

        setLayout(mLayout);

        connect(mAscending, &QCheckBox::stateChanged, this, &LUTWizardPage::completeChanged);
        connect(mPinConfigTable, &QTableWidget::itemChanged, this, &LUTWizardPage::completeChanged);
    }

    void LUTWizardPage::setData(GateType* gate)
    {
        mSavedConfigs.clear();

        if (gate == nullptr || !gate->has_component_of_type(GateTypeComponent::ComponentType::lut))
            return;

        auto* lutc = gate->get_component_as<LUTComponent>([](const GateTypeComponent* c) { return LUTComponent::is_class_of(c); });
        if (lutc == nullptr)
            return;

        const auto& configs = lutc->get_output_pin_configs();
        bool first_ascending = true;
        if (!configs.empty())
            first_ascending = configs.begin()->second.is_ascending;
        mAscending->setChecked(first_ascending);

        for (const auto& [pin, cfg] : configs)
            mSavedConfigs.push_back({pin, cfg.init_identifier, cfg.bit_offset, cfg.bit_count});
    }

    void LUTWizardPage::initializePage()
    {
        const QStringList pins = getLutPinsFromWizard();

        auto* wiz = static_cast<GateLibraryWizard*>(wizard());
        int n = wiz ? wiz->mPinModel->getInputPins().size() : 0;
        if (n > 30) n = 30;
        const u32 maxBitCount = 1u << n;

        mPinConfigTable->setRowCount(0);
        for (const QString& pin : pins)
        {
            const std::string pinName = pin.toStdString();

            // prefill from an existing configuration if this pin already has one
            const SavedConfig* saved = nullptr;
            for (const auto& cfg : mSavedConfigs)
            {
                if (cfg.pinName == pinName)
                {
                    saved = &cfg;
                    break;
                }
            }

            if (saved != nullptr)
                addTableRow(pin, QString::fromStdString(saved->initIdentifier), saved->bitOffset, saved->bitCount, maxBitCount);
            else
                addTableRow(pin, "INIT", 0, 1, maxBitCount);
        }
    }

    QStringList LUTWizardPage::getLutPinsFromWizard() const
    {
        auto* wiz = static_cast<GateLibraryWizard*>(wizard());
        if (!wiz) return {};

        QStringList result;
        for (PinItem* group : wiz->getPingroups())
        {
            for (auto* child : group->getChildren())
            {
                auto* pin = static_cast<PinItem*>(child);
                if (pin->getItemType() == PinItem::TreeItemType::Pin &&
                    pin->getPinType()  == PinType::lut)
                    result << pin->getName();
            }
        }
        return result;
    }

    void LUTWizardPage::addTableRow(const QString& pinName, const QString& initId, u32 bitOffset, u32 bitCount, u32 maxBitCount)
    {
        int row = mPinConfigTable->rowCount();
        mPinConfigTable->insertRow(row);

        auto* pinItem = new QTableWidgetItem(pinName);
        pinItem->setFlags(pinItem->flags() & ~Qt::ItemIsEditable);
        mPinConfigTable->setItem(row, 0, pinItem);

        mPinConfigTable->setItem(row, 1, new QTableWidgetItem(initId));

        auto* offsetSpin = new QSpinBox(mPinConfigTable);
        offsetSpin->setRange(0, 1 << 20);
        offsetSpin->setValue(static_cast<int>(bitOffset));
        mPinConfigTable->setCellWidget(row, 2, offsetSpin);

        auto* countSpin = new QSpinBox(mPinConfigTable);
        countSpin->setRange(1, static_cast<int>(maxBitCount));
        countSpin->setValue(static_cast<int>(bitCount > 0 ? bitCount : 1));
        mPinConfigTable->setCellWidget(row, 3, countSpin);
        connect(countSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &LUTWizardPage::completeChanged);
    }

    QVector<LUTWizardPage::OutputPinEntry> LUTWizardPage::getOutputPinConfigs() const
    {
        QVector<OutputPinEntry> result;
        for (int r = 0; r < mPinConfigTable->rowCount(); ++r)
        {
            auto* pinItem    = mPinConfigTable->item(r, 0);
            auto* offsetSpin = qobject_cast<QSpinBox*>(mPinConfigTable->cellWidget(r, 2));
            auto* countSpin  = qobject_cast<QSpinBox*>(mPinConfigTable->cellWidget(r, 3));

            const QString pin = pinItem ? pinItem->text() : QString();
            if (pin.isEmpty()) continue;

            auto* idItem     = mPinConfigTable->item(r, 1);
            const QString id = idItem ? idItem->text() : QString();

            result.push_back({pin, id,
                               static_cast<u32>(offsetSpin ? offsetSpin->value() : 0),
                               static_cast<u32>(countSpin  ? countSpin->value()  : 0)});
        }
        return result;
    }

    bool LUTWizardPage::isComplete() const
    {
        for (int row = 0; row < mPinConfigTable->rowCount(); ++row)
        {
            auto* pinInitIdentifier = mPinConfigTable->item(row, 1);
            if (pinInitIdentifier == nullptr || pinInitIdentifier->text().trimmed().isEmpty()) return false;

            auto* countSpin  = qobject_cast<QSpinBox*>(mPinConfigTable->cellWidget(row, 3));
            const int count = countSpin ? countSpin->value() : 1;
            if (count == 0 || count & (count - 1)) return false; // ensure bit count is non-zero power of two
        }

        return true;
    }
}
