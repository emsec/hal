#include "gui/selection_details_widget/gate_details_tab_widget.h"

#include "gui/code_editor/syntax_highlighter/python_qss_adapter.h"
#include "gui/gui_globals.h"
#include "gui/input_dialog/input_dialog.h"
#include "gui/python/py_code_provider.h"
#include "gui/selection_details_widget/details_frame_widget.h"
#include "gui/selection_details_widget/gate_details_widget/gate_info_table.h"
#include "gui/selection_details_widget/gate_details_widget/gate_pin_tree.h"
#include "gui/user_action/action_add_boolean_function.h"
#include "gui/user_action/action_set_object_data.h"
#include "gui/validator/hexadecimal_validator.h"
#include "hal_core/netlist/gate.h"
#include "hal_core/netlist/gate_library/gate_type_component/ff_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/gate_type_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/init_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/latch_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/lut_component.h"
#include "hal_core/netlist/gate_library/gate_type_component/state_component.h"

#include <QApplication>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QClipboard>
#include <QDebug>
#include "gui/comment_system/widgets/comment_widget.h"
// temporal debug includes
#include "gui/comment_system/widgets/comment_item.h"
#include "gui/comment_system/comment_entry.h"
#include <QMenu>

namespace hal
{
    GateDetailsTabWidget::GateDetailsTabWidget(QWidget* parent) : DetailsTabWidget(parent)
    {
        setIcon(SelectionDetailsIconProvider::GateIcon);

        //general tab
        mGateInfoTable        = new GateInfoTable(this);
        mGateInformationFrame = new DetailsFrameWidget(mGateInfoTable, "Gate Information", this);
        addTab("General", mGateInformationFrame, true);

        //groupings tab
        mGroupingsOfItemTable = new GroupingsOfItemWidget;
        mGroupingsFrame       = new DetailsFrameWidget(mGroupingsOfItemTable, "Groupings", this);    //replace QWidget
        connect(mGroupingsOfItemTable, &GroupingsOfItemWidget::updateText, mGroupingsFrame, &DetailsFrameWidget::setText);
        addTab("Groupings", mGroupingsFrame, false);

        //pins tab
        mPinsTree  = new GatePinTree(this);
        mPinsFrame = new DetailsFrameWidget(mPinsTree, "Pins", this);
        connect(mPinsTree, &GatePinTree::updateText, mPinsFrame, &DetailsFrameWidget::setText);

        addTab("Pins", mPinsFrame, false);

        //(ff / latch / lut) tab - would love to use seperate tabs, but it's a hassle to hide multiple individual tabs witouth setTabVisible() from qt 5.15
        mFfFunctionTable   = new BooleanFunctionTable(this);
        mLatchFunctionTable = new BooleanFunctionTable(this);
        mLutPinButtonGroup = new QButtonGroup(this);
        mLutPinButtonGroup->setExclusive(true);
        mLutRadioWidget    = new QWidget(this);
        mLutRadioLayout    = new QVBoxLayout(mLutRadioWidget);
        mLutRadioLayout->setContentsMargins(4, 4, 4, 4);
        mLutRadioLayout->setSpacing(4);
        mLutTable          = new LUTTableWidget(this);
        mLutConfigLabel = new QLabel("default", this);
        mLutConfigLabel->setContextMenuPolicy(Qt::CustomContextMenu);
        mLutConfigLabel->setWordWrap(true);
        mLutConfigLabel->setStyleSheet(QString("QLabel{color: %1;}").arg(PythonQssAdapter::instance()->numberColor().name()));    //tmp.
        connect(mLutConfigLabel, &QWidget::customContextMenuRequested, this, &GateDetailsTabWidget::handleLutConfigContextMenuRequested);

        mFfFrame               = new DetailsFrameWidget(mFfFunctionTable, "FF Information", this);
        mLatchFrame            = new DetailsFrameWidget(mLatchFunctionTable, "Latch Information", this);
        mLutFrame              = new DetailsFrameWidget(mLutRadioWidget, "Boolean Functions", this);
        mLutConfigurationFrame = new DetailsFrameWidget(mLutConfigLabel, "Configuration String", this);
        mTruthTableFrame       = new DetailsFrameWidget(mLutTable, "Truth Table", this);
        // DetailsFrameWidget sets Expanding vertical policy on all content widgets, which causes the
        // container to distribute vertical space equally. Override for the two compact LUT frames so
        // they stay at their natural content height and the truth table gets any remaining space.
        mLutFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        mLutConfigurationFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        QList<DetailsFrameWidget*> framesFfLatchLutTab({mFfFrame, mLatchFrame, mLutFrame, mLutConfigurationFrame, mTruthTableFrame});
        mMultiTabIndex   = addTab("(FF / Latch / LUT)", framesFfLatchLutTab);    //save index of multi tab -> needed for show / hide
        mMultiTabContent = widget(mMultiTabIndex);                               // save content of multi tab -> needed for show / hide

        //boolean functions tab
        mFullFunctionTable = new BooleanFunctionTable(this);
        mFullFunctionTable->setContextMenuPlainDescr(true);
        mFullFunctionTable->setContextMenuPythonPlainDescr(true);
        mBooleanFunctionsFrame = new DetailsFrameWidget(mFullFunctionTable, "Boolean Functions", this);

        addTab("Boolean Functions", mBooleanFunctionsFrame, false);
        connect(gNetlistRelay, &NetlistRelay::gateBooleanFunctionChanged, this, &GateDetailsTabWidget::handleGateBooleanFunctionChanged);

        //data tab
        mDataTable = new DataTableWidget(this);
        mDataFrame = new DetailsFrameWidget(mDataTable, "Data", this);

        addTab("Data", mDataFrame, false);

        //comments tab, no frame is used here
        mCommentWidget = new CommentWidget(this);
        QTabWidget::addTab(mCommentWidget, "Comments");
    }

    void GateDetailsTabWidget::setGate(Gate* gate)
    {
        if (gate) setIcon(SelectionDetailsIconProvider::GateIcon, gate->get_id());

        //pass gate or other stuff to widgets
        mCurrentGate = gate;
        mGateInfoTable->setGate(gate);
        mGroupingsOfItemTable->setGate(gate);
        mPinsTree->setGate(gate);
        mDataTable->setGate(gate);
        mCommentWidget->nodeChanged(Node(gate->get_id(), Node::NodeType::Gate));

        // Logic for LUT/FF/LATCH
        GateDetailsTabWidget::GateTypeCategory gateTypeCategory = getGateTypeCategory(gate);
        hideOrShorMultiTab(gateTypeCategory);
        setupBooleanFunctionTables(gate, gateTypeCategory);
    }

    void GateDetailsTabWidget::handleGateBooleanFunctionChanged(Gate* g)
    {
        if (g == mCurrentGate && g != nullptr)
        {
            GateDetailsTabWidget::GateTypeCategory gateTypeCategory = getGateTypeCategory(g);
            setupBooleanFunctionTables(g, gateTypeCategory);
            mDataTable->setGate(g);
        }
    }

    void GateDetailsTabWidget::hideOrShorMultiTab(GateDetailsTabWidget::GateTypeCategory gateTypeCategory)
    {
        if (gateTypeCategory != GateDetailsTabWidget::GateTypeCategory::none)
        {
            showMultiTab(gateTypeCategory);
        }
        else
        {
            hideMultiTab();
        }
    }

    void GateDetailsTabWidget::hideMultiTab()
    {
        if (mMultiTabVisible)
        {
            removeTab(mMultiTabIndex);
            mMultiTabVisible = false;
        }
    }

    void GateDetailsTabWidget::showMultiTab(GateDetailsTabWidget::GateTypeCategory gateTypeCategory)
    {
        QString label = "";

        switch (gateTypeCategory)
        {
            case GateDetailsTabWidget::GateTypeCategory::lut: {
                mLutFrame->setVisible(true);
                mLutConfigurationFrame->setVisible(true);
                mTruthTableFrame->setVisible(true);
                mFfFrame->setVisible(false);
                mLatchFrame->setVisible(false);
                label = "LUT";
                break;
            }
            case GateDetailsTabWidget::GateTypeCategory::ff: {
                mLutFrame->setVisible(false);
                mLutConfigurationFrame->setVisible(false);
                mTruthTableFrame->setVisible(false);
                mFfFrame->setVisible(true);
                mLatchFrame->setVisible(false);
                label = "Flip-Flop";
                break;
            }
            case GateDetailsTabWidget::GateTypeCategory::latch: {
                mLutFrame->setVisible(false);
                mLutConfigurationFrame->setVisible(false);
                mTruthTableFrame->setVisible(false);
                mFfFrame->setVisible(false);
                mLatchFrame->setVisible(true);
                label = "Latch";
                break;
            }
            default:
                break;
        }

        if (!mMultiTabVisible)
        {
            insertTab(mMultiTabIndex, mMultiTabContent, label);
            mMultiTabVisible = true;
        }
        else
        {
            setTabText(mMultiTabIndex, label);
        }
    }

    GateDetailsTabWidget::GateTypeCategory GateDetailsTabWidget::getGateTypeCategory(Gate* gate) const
    {
        if (gate == nullptr)
        {
            return GateDetailsTabWidget::GateTypeCategory::none;
        }

        GateType* type = gate->get_type();

        std::set<hal::GateTypeProperty> gateTypeProperties = type->get_properties();
        std::set<hal::GateTypeProperty> relevantProperties{GateTypeProperty::c_lut, GateTypeProperty::latch, GateTypeProperty::ff};

        auto relevantFind = find_first_of(begin(gateTypeProperties), end(gateTypeProperties), begin(relevantProperties), end(relevantProperties));

        if (relevantFind == end(gateTypeProperties))
        {
            return GateDetailsTabWidget::GateTypeCategory::none;
        }

        switch (*relevantFind)
        {
            case GateTypeProperty::c_lut: {
                return GateDetailsTabWidget::GateTypeCategory::lut;
                break;
            }
            case GateTypeProperty::ff: {
                return GateDetailsTabWidget::GateTypeCategory::ff;
                break;
            }
            case GateTypeProperty::latch: {
                return GateDetailsTabWidget::GateTypeCategory::latch;
                break;
            }
            default: {
                return GateDetailsTabWidget::GateTypeCategory::none;
                break;
            }
        }
    }

    void GateDetailsTabWidget::handleLutPinSelectionChanged(int id)
    {
        if (mCurrentGate == nullptr || id < 0 || id >= static_cast<int>(mCurrentLutPins.size()))
            return;

        const std::string& pinName = mCurrentLutPins[id];
        mSelectedLutPin            = pinName;

        BooleanFunction bf = mCurrentGate->get_boolean_function(pinName);
        mLutTable->setBooleanFunction(bf, QString::fromStdString(pinName));

        GateType*      gt             = mCurrentGate->get_type();
        InitComponent* init_component = gt->get_component_as<InitComponent>([](const GateTypeComponent* c) { return InitComponent::is_class_of(c); });
        if (init_component == nullptr)
        {
            mLutConfigLabel->setText(" Could not load init string.");
            return;
        }
        std::string initKey    = init_component->get_init_identifiers()[0];
        u32         bit_offset = 0;
        u32         bit_count  = 0;
        if (LUTComponent* lc = gt->get_component_as<LUTComponent>([](const GateTypeComponent* c) { return LUTComponent::is_class_of(c); }); lc != nullptr)
        {
            if (const LUTComponent::LUTOutputConfig* cfg = lc->get_output_pin_config(pinName); cfg != nullptr)
            {
                initKey    = cfg->init_identifier;
                bit_offset = cfg->bit_offset;
                bit_count  = cfg->bit_count;
            }
        }
        const std::string raw_str    = std::get<1>(mCurrentGate->get_data(init_component->get_init_category(), initKey));
        const auto        slice_res  = LUTComponent::extract_init_slice(raw_str, bit_offset, bit_count);
        const std::string displayHex = slice_res.is_ok() ? slice_res.get() : raw_str;
        mLutConfigLabel->setText(" 0x" + QString::fromStdString(displayHex));
    }

    void GateDetailsTabWidget::handleLutConfigContextMenuRequested(QPoint pos)
    {
        auto resolveInitKey = [this](InitComponent* ic) -> std::string {
            std::string key = ic->get_init_identifiers()[0];
            if (LUTComponent* lc = mCurrentGate->get_type()->get_component_as<LUTComponent>([](const GateTypeComponent* c) { return LUTComponent::is_class_of(c); }); lc != nullptr)
                if (const LUTComponent::LUTOutputConfig* cfg = lc->get_output_pin_config(mSelectedLutPin); cfg != nullptr)
                    key = cfg->init_identifier;
            return key;
        };

        // Resolve the bit slice for the currently selected pin (bit_count==0 means full string)
        u32 bit_offset = 0, bit_count = 0;
        if (LUTComponent* lc = mCurrentGate->get_type()->get_component_as<LUTComponent>([](const GateTypeComponent* c) { return LUTComponent::is_class_of(c); }); lc != nullptr)
            if (const LUTComponent::LUTOutputConfig* cfg = lc->get_output_pin_config(mSelectedLutPin); cfg != nullptr)
            { bit_offset = cfg->bit_offset; bit_count = cfg->bit_count; }

        QMenu menu;

        menu.addAction("Configuration string to clipboard", [this]() { QApplication::clipboard()->setText(mLutConfigLabel->text().remove(" 0x")); });
        menu.addAction("Change configuration string", [this, resolveInitKey, bit_offset, bit_count]() {
            // Strip leading " 0x" from the label to get the current hex digits
            QString currentHex = mLutConfigLabel->text().trimmed();
            if (currentHex.startsWith("0x", Qt::CaseInsensitive)) currentHex = currentHex.mid(2);

            InputDialog ipd("Change configuration string", "New configuration string", currentHex);
            HexadecimalValidator hexValidator;
            ipd.addValidator(&hexValidator);
            if (ipd.exec() != QDialog::Accepted || ipd.textValue().isEmpty()) return;

            InitComponent* ic = mCurrentGate->get_type()->get_component_as<InitComponent>([](const GateTypeComponent* c) { return InitComponent::is_class_of(c); });
            if (!ic) { log_error("gui", "Could not load InitComponent from gate with id {}.", mCurrentGate->get_id()); return; }

            const std::string cat    = ic->get_init_category();
            const std::string key    = resolveInitKey(ic);
            QString           result = ipd.textValue().toUpper();

            if (bit_count > 0)
            {
                const std::string raw_str  = std::get<1>(mCurrentGate->get_data(cat, key));
                auto              splice_r = LUTComponent::splice_init_slice(raw_str, ipd.textValue().toStdString(), bit_offset, bit_count);
                if (splice_r.is_ok())
                    result = QString::fromStdString(splice_r.get());
            }

            ActionSetObjectData* act = new ActionSetObjectData(QString::fromStdString(cat), QString::fromStdString(key), "bit_vector", result);
            act->setObject(UserActionObject(mCurrentGate->get_id(), UserActionObjectType::Gate));
            act->exec();
            setGate(mCurrentGate);    // must update config string and data table, no signal for that
        });
        menu.addAction(QIcon(":/icons/python"), "Get configuration string", [this, resolveInitKey]() {
            if (InitComponent* ic = mCurrentGate->get_type()->get_component_as<InitComponent>([](const GateTypeComponent* c) { return InitComponent::is_class_of(c); }); ic != nullptr)
            {
                std::string cat = ic->get_init_category();
                std::string key = resolveInitKey(ic);
                QApplication::clipboard()->setText(PyCodeProvider::pyCodeGateData(mCurrentGate->get_id(), QString::fromStdString(cat), QString::fromStdString(key)));
            }
            else
                log_error("gui", "Could not load InitComponent from gate with id {}.", mCurrentGate->get_id());
        });

        menu.exec(mLutConfigLabel->mapToGlobal(pos));
    }

    void GateDetailsTabWidget::setupBooleanFunctionTables(Gate* gate, GateDetailsTabWidget::GateTypeCategory gateTypeCategory)
    {
        if (gate == nullptr)
        {
            return;
        }

        static QSet<QString> ffBfNames = {
            "clear",
            "preset",    // Both
            "clock",
            "clocked_on",
            "clocked_on_also",
            "next_state",
            "power_down_function"    // FF names
        };

        static QSet<QString> latchBfNames = {
            "clear",
            "preset",    // Both
            "enable",
            "data_in"    // Latch names
        };

        static QSet<QString> noSpecialBfNames = {};

        QSet<QString>* specialBfNames = &noSpecialBfNames;
        if (gateTypeCategory == GateDetailsTabWidget::GateTypeCategory::ff)
        {
            specialBfNames = &ffBfNames;
        }
        else if (gateTypeCategory == GateDetailsTabWidget::GateTypeCategory::latch)
        {
            specialBfNames = &latchBfNames;
        }

        std::unordered_map<std::string, BooleanFunction> allBfs = gate->get_boolean_functions(false);

        QMap<QString, BooleanFunction> specialFunctions;
        QMap<QString, BooleanFunction> otherFunctions;

        for (auto& it : allBfs)
        {
            QString bfName = QString::fromStdString(it.first);
            if (specialBfNames->contains(bfName))
            {
                // Function is a LUT/FF function
                specialFunctions.insert(bfName, it.second);
            }
            else
            {
                // Function is not a LUT/FF function
                otherFunctions.insert(bfName, it.second);
            }
        }

        QVector<QSharedPointer<BooleanFunctionTableEntry>> specialFunctionList;
        QVector<QSharedPointer<BooleanFunctionTableEntry>> otherFunctionList;
        QVector<QSharedPointer<BooleanFunctionTableEntry>> setPresetBehavior;

        QMap<QString, BooleanFunction>::iterator i;

        for (i = specialFunctions.begin(); i != specialFunctions.end(); i++)
            specialFunctionList.append(QSharedPointer<BooleanFunctionTableEntry>(new BooleanFunctionEntry(gate->get_id(), i.key(), i.value())));

        for (i = otherFunctions.begin(); i != otherFunctions.end(); i++)
            otherFunctionList.append(QSharedPointer<BooleanFunctionTableEntry>(new BooleanFunctionEntry(gate->get_id(), i.key(), i.value())));

        GateType* gt = gate->get_type();
        if (FFComponent* ff_component = gt->get_component_as<FFComponent>([](const GateTypeComponent* c) { return FFComponent::is_class_of(c); }); ff_component != nullptr)
        {
            if (ff_component->get_async_set_reset_behavior().first != AsyncSetResetBehavior::undef)
                setPresetBehavior.append(QSharedPointer<BooleanFunctionTableEntry>(new FFComponentEntry(gate->get_id(), ff_component->get_async_set_reset_behavior())));
            if (!ff_component->get_clock_function().is_empty())
                setPresetBehavior.append(QSharedPointer<BooleanFunctionTableEntry>(new FFComponentEntry(gate->get_id(), FFComponentEntry::FFCompFunc::Clock, ff_component->get_clock_function())));
            if (!ff_component->get_next_state_function().is_empty())
                setPresetBehavior.append(
                    QSharedPointer<BooleanFunctionTableEntry>(new FFComponentEntry(gate->get_id(), FFComponentEntry::FFCompFunc::NextState, ff_component->get_next_state_function())));
            if (!ff_component->get_async_set_function().is_empty())
                setPresetBehavior.append(
                    QSharedPointer<BooleanFunctionTableEntry>(new FFComponentEntry(gate->get_id(), FFComponentEntry::FFCompFunc::AsyncSet, ff_component->get_async_set_function())));
            if (!ff_component->get_async_reset_function().is_empty())
                setPresetBehavior.append(
                    QSharedPointer<BooleanFunctionTableEntry>(new FFComponentEntry(gate->get_id(), FFComponentEntry::FFCompFunc::AsyncReset, ff_component->get_async_reset_function())));
        }
        else if (LatchComponent* latch_component = gt->get_component_as<LatchComponent>([](const GateTypeComponent* c) { return LatchComponent::is_class_of(c); }); latch_component != nullptr)
        {
            if (latch_component->get_async_set_reset_behavior().first != AsyncSetResetBehavior::undef)
                setPresetBehavior.append(QSharedPointer<BooleanFunctionTableEntry>(new LatchComponentEntry(gate->get_id(), latch_component->get_async_set_reset_behavior())));
            if (!latch_component->get_enable_function().is_empty())
                setPresetBehavior.append(
                    QSharedPointer<BooleanFunctionTableEntry>(new LatchComponentEntry(gate->get_id(), LatchComponentEntry::LatchCompFunc::Enable, latch_component->get_enable_function())));
            if (!latch_component->get_data_in_function().is_empty())
                setPresetBehavior.append(
                    QSharedPointer<BooleanFunctionTableEntry>(new LatchComponentEntry(gate->get_id(), LatchComponentEntry::LatchCompFunc::DataInFunc, latch_component->get_data_in_function())));
            if (!latch_component->get_async_set_function().is_empty())
                setPresetBehavior.append(
                    QSharedPointer<BooleanFunctionTableEntry>(new LatchComponentEntry(gate->get_id(), LatchComponentEntry::LatchCompFunc::AsyncSet, latch_component->get_async_set_function())));
            if (!latch_component->get_async_reset_function().is_empty())
                setPresetBehavior.append(
                    QSharedPointer<BooleanFunctionTableEntry>(new LatchComponentEntry(gate->get_id(), LatchComponentEntry::LatchCompFunc::AsyncReset, latch_component->get_async_reset_function())));
        }

        if (StateComponent* state_component = gt->get_component_as<StateComponent>([](const GateTypeComponent* c) { return StateComponent::is_class_of(c); }); state_component != nullptr)
        {
            if (state_component->get_state_identifier() != "")
                setPresetBehavior.append(QSharedPointer<BooleanFunctionTableEntry>(
                    new StateComponentEntry(gate->get_id(), StateComponentEntry::StateCompType::PosState, QString::fromStdString(state_component->get_state_identifier()))));
            if (state_component->get_neg_state_identifier() != "")
                setPresetBehavior.append(QSharedPointer<BooleanFunctionTableEntry>(
                    new StateComponentEntry(gate->get_id(), StateComponentEntry::StateCompType::NegState, QString::fromStdString(state_component->get_neg_state_identifier()))));
        }

        // Fill the category (LUT/FF/LATCH) widgets
        switch (gateTypeCategory)
        {
            case GateDetailsTabWidget::GateTypeCategory::lut: {
                const std::vector<std::string> lutPins = gate->get_type()->get_pin_names([](const GatePin* p) { return p->get_type() == PinType::lut; });

                // Clear existing radio buttons and layout
                const auto existingButtons = mLutPinButtonGroup->buttons();
                for (QAbstractButton* btn : existingButtons)
                    mLutPinButtonGroup->removeButton(btn);
                while (QLayoutItem* item = mLutRadioLayout->takeAt(0))
                {
                    if (QWidget* w = item->widget())
                        w->deleteLater();
                    delete item;
                }
                mCurrentLutPins.clear();

                // Build one radio-button row per LUT output pin
                int idx = 0;
                for (const std::string& pin : lutPins)
                {
                    BooleanFunction bf  = gate->get_boolean_function(pin);
                    QString rowText     = QString::fromStdString(pin) + " = " + QString::fromStdString(bf.to_string());

                    QWidget*     row       = new QWidget(mLutRadioWidget);
                    QHBoxLayout* rowLayout = new QHBoxLayout(row);
                    rowLayout->setContentsMargins(0, 0, 0, 0);
                    rowLayout->setSpacing(4);

                    QRadioButton* rb = new QRadioButton(row);
                    mLutPinButtonGroup->addButton(rb, idx);

                    QLabel* fnLabel = new QLabel(rowText, row);
                    fnLabel->setWordWrap(true);
                    fnLabel->setContextMenuPolicy(Qt::CustomContextMenu);
                    connect(fnLabel, &QLabel::customContextMenuRequested, this, [this, pin, fnLabel](QPoint pos) {
                        if (!mCurrentGate) return;
                        const BooleanFunction bf  = mCurrentGate->get_boolean_function(pin);
                        const QString bfStr       = QString::fromStdString(bf.to_string());
                        const u32     gateId      = mCurrentGate->get_id();
                        const QString pinQStr     = QString::fromStdString(pin);
                        QMenu menu;
                        menu.addAction("Boolean function to clipboard", [bfStr]() {
                            QApplication::clipboard()->setText(bfStr);
                        });
                        menu.addAction("Change Boolean function", [this, pinQStr]() {
                            InputDialog ipd("Change Boolean function", "New function", "");
                            if (ipd.exec() == QDialog::Accepted && !ipd.textValue().isEmpty())
                            {
                                auto funcRes = BooleanFunction::from_string(ipd.textValue().toStdString());
                                if (funcRes.is_ok())
                                {
                                    ActionAddBooleanFunction* act = new ActionAddBooleanFunction(pinQStr, funcRes.get(), mCurrentGate->get_id());
                                    act->exec();
                                }
                            }
                        });
                        const QString pyCode = PyCodeProvider::pyCodeGateBooleanFunction(gateId, pinQStr);
                        if (!pyCode.isEmpty())
                            menu.addAction(QIcon(":/icons/python"), "Get boolean function", [pyCode]() {
                                QApplication::clipboard()->setText(pyCode);
                            });
                        menu.exec(fnLabel->mapToGlobal(pos));
                    });

                    rowLayout->addWidget(rb, 0, Qt::AlignTop);
                    rowLayout->addWidget(fnLabel, 1);
                    mLutRadioLayout->addWidget(row);
                    mCurrentLutPins.push_back(pin);

                    connect(rb, &QRadioButton::toggled, this, [this, idx](bool checked) {
                        if (checked) handleLutPinSelectionChanged(idx);
                    });
                    ++idx;
                }
                mLutRadioLayout->addStretch();

                // Restore the previously selected pin, falling back to the first pin.
                int pinToSelect = 0;
                if (!mSelectedLutPin.empty())
                {
                    auto it = std::find(mCurrentLutPins.begin(), mCurrentLutPins.end(), mSelectedLutPin);
                    if (it != mCurrentLutPins.end())
                        pinToSelect = static_cast<int>(it - mCurrentLutPins.begin());
                }
                if (!mCurrentLutPins.empty())
                    if (QAbstractButton* btn = mLutPinButtonGroup->button(pinToSelect))
                        btn->setChecked(true);
                break;
            }
            case GateDetailsTabWidget::GateTypeCategory::ff: {
                mFfFunctionTable->setEntries(specialFunctionList + setPresetBehavior);
                break;
            }
            case GateDetailsTabWidget::GateTypeCategory::latch: {
                mLatchFunctionTable->setEntries(specialFunctionList + setPresetBehavior);
                break;
            }
            default:
                break;
        }
        mFullFunctionTable->setEntries(specialFunctionList + otherFunctionList);
    }

}    // namespace hal
