#include "gui/gatelibrary_management/gatelibrary_pages/lut_wizardpage.h"
#include "gui/gatelibrary_management/gatelibrary_wizard.h"
#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"
#include "hal_core/netlist/gate_library/enums/pin_direction.h"
#include "gui/pin_model/pin_item.h"

#include <QComboBox>
#include <QHeaderView>
#include <QSpinBox>
#include <QHBoxLayout>

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
        mAddBtn    = new QPushButton("+", btnWidget);
        mRemoveBtn = new QPushButton("-", btnWidget);
        btnLayout->addWidget(mAddBtn);
        btnLayout->addWidget(mRemoveBtn);
        btnLayout->addStretch();
        mLayout->addWidget(btnWidget, 2, 0, 1, 2);

        setLayout(mLayout);

        connect(mAscending, &QCheckBox::stateChanged, this, &LUTWizardPage::completeChanged);
        connect(mAddBtn,    &QPushButton::clicked,    this, &LUTWizardPage::addRow);
        connect(mRemoveBtn, &QPushButton::clicked,    this, &LUTWizardPage::removeSelectedRow);
    }

    void LUTWizardPage::setData(GateType* gate)
    {
        mSavedConfigs.clear();
        mTableInitialized = false;

        if (gate == nullptr || !gate->has_component_of_type(GateTypeComponent::ComponentType::init))
            return;

        auto* lutc = gate->get_component_as<LUTComponent>([](const GateTypeComponent* c) { return LUTComponent::is_class_of(c); });
        if (lutc == nullptr)
            return;

        mAscending->setChecked(lutc->is_init_ascending());

        const auto& configs = lutc->get_output_pin_configs();
        for (const auto& [pin, cfg] : configs)
            mSavedConfigs.push_back({pin, cfg.init_identifier, cfg.bit_offset, cfg.bit_count});
    }

    void LUTWizardPage::initializePage()
    {
        const QStringList pins = getOutputPinsFromWizard();

        if (!mTableInitialized)
        {
            mTableInitialized = true;
            mPinConfigTable->setRowCount(0);
            for (const auto& cfg : mSavedConfigs)
                addTableRow(QString::fromStdString(cfg.pinName),
                            QString::fromStdString(cfg.initIdentifier),
                            cfg.bitOffset, cfg.bitCount, pins);
        }
        else
        {
            updateDropdowns(pins);
        }
    }

    QStringList LUTWizardPage::getOutputPinsFromWizard() const
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
                    pin->getDirection() == PinDirection::output)
                    result << pin->getName();
            }
        }
        return result;
    }

    void LUTWizardPage::addTableRow(const QString& pinName, const QString& initId,
                                     u32 bitOffset, u32 bitCount,
                                     const QStringList& availablePins)
    {
        int row = mPinConfigTable->rowCount();
        mPinConfigTable->insertRow(row);

        auto* combo = new QComboBox(mPinConfigTable);
        combo->addItems(availablePins);
        int idx = combo->findText(pinName);
        if (idx >= 0) combo->setCurrentIndex(idx);
        mPinConfigTable->setCellWidget(row, 0, combo);

        mPinConfigTable->setItem(row, 1, new QTableWidgetItem(initId.isEmpty() ? "INIT" : initId));

        auto* offsetSpin = new QSpinBox(mPinConfigTable);
        offsetSpin->setRange(0, 1 << 20);
        offsetSpin->setValue(static_cast<int>(bitOffset));
        mPinConfigTable->setCellWidget(row, 2, offsetSpin);

        auto* countSpin = new QSpinBox(mPinConfigTable);
        countSpin->setRange(0, 1 << 20);
        countSpin->setSpecialValueText("full");
        countSpin->setValue(static_cast<int>(bitCount));
        mPinConfigTable->setCellWidget(row, 3, countSpin);
    }

    void LUTWizardPage::updateDropdowns(const QStringList& pins)
    {
        for (int r = 0; r < mPinConfigTable->rowCount(); ++r)
        {
            auto* combo = qobject_cast<QComboBox*>(mPinConfigTable->cellWidget(r, 0));
            if (!combo) continue;
            const QString current = combo->currentText();
            combo->clear();
            combo->addItems(pins);
            int idx = combo->findText(current);
            if (idx >= 0) combo->setCurrentIndex(idx);
        }
    }

    void LUTWizardPage::addRow()
    {
        addTableRow("", "INIT", 0, 0, getOutputPinsFromWizard());
    }

    void LUTWizardPage::removeSelectedRow()
    {
        const int row = mPinConfigTable->currentRow();
        if (row >= 0)
            mPinConfigTable->removeRow(row);
    }

    QVector<LUTWizardPage::OutputPinEntry> LUTWizardPage::getOutputPinConfigs() const
    {
        QVector<OutputPinEntry> result;
        for (int r = 0; r < mPinConfigTable->rowCount(); ++r)
        {
            auto* combo      = qobject_cast<QComboBox*>(mPinConfigTable->cellWidget(r, 0));
            auto* offsetSpin = qobject_cast<QSpinBox*>(mPinConfigTable->cellWidget(r, 2));
            auto* countSpin  = qobject_cast<QSpinBox*>(mPinConfigTable->cellWidget(r, 3));

            const QString pin = combo ? combo->currentText() : QString();
            if (pin.isEmpty()) continue;

            const QString id = (mPinConfigTable->item(r, 1) && !mPinConfigTable->item(r, 1)->text().isEmpty())
                               ? mPinConfigTable->item(r, 1)->text()
                               : "INIT";

            result.push_back({pin, id,
                               static_cast<u32>(offsetSpin ? offsetSpin->value() : 0),
                               static_cast<u32>(countSpin  ? countSpin->value()  : 0)});
        }
        return result;
    }
}
