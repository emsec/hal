#include "logic_evaluator/logic_evaluator_pingroup.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QtGlobal>
#include "gui/hal_qt_compat/hal_qt_compat.h"
#include "hal_core/netlist/net.h"

namespace hal {

    LogicEvaluatorHexSpinbox::LogicEvaluatorHexSpinbox(QWidget* parent)
        : QSpinBox(parent)
    {;}

    int LogicEvaluatorHexSpinbox::valueFromText(const QString &text) const
    {
        return text.toInt(nullptr,16);
    }

    QString LogicEvaluatorHexSpinbox::textFromValue(int val) const
    {
        return QString::number(val,16).toUpper();
    }

    QValidator::State LogicEvaluatorHexSpinbox::validate(QString &input, int &pos) const
    {
        Q_UNUSED(pos);
        if (input.isEmpty())
            return QValidator::Intermediate;
        bool ok;
        int val = input.toInt(&ok,16);
        if (ok && 0 <= val && val <= maximum())
            return QValidator::Acceptable;
        return QValidator::Invalid;
    }

//-------------------------------------------------------------------

    LogicEvaluatorValue::LogicEvaluatorValue(int nbits, QWidget* parent)
        : QWidget(parent), mLabel(nullptr), mSpinBox(nullptr)
    {
        QVBoxLayout* layout = new QVBoxLayout(this);
        QtCompat::setMarginWidth(layout,0);
        if (nbits > 1)
        {
            mSpinBox = new LogicEvaluatorHexSpinbox(this);
            mSpinBox->setMinimum(0);
            mSpinBox->setMaximum((1 << nbits)-1);
            mSpinBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
            connect(mSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &LogicEvaluatorValue::handleSpinBoxValueChanged);
            layout->addWidget(mSpinBox);
        }
        else
        {
            mLabel = new QLabel(this);
            layout->addWidget(mLabel);
        }
    }

    void LogicEvaluatorValue::handleSpinBoxValueChanged(int val)
    {
        Q_EMIT valueChanged(val);
    }

    int LogicEvaluatorValue::value() const
    {
        if (mSpinBox)
            return mSpinBox->value();
        return mLabel->text().toInt();
    }

    void LogicEvaluatorValue::setValue(int val)
    {
        if (mSpinBox)
            mSpinBox->setValue(val);
        else
            mLabel->setText(QString::number(val,16));
    }

    //-------------------------------------------------------------------

    LogicEvaluatorCheckBox::LogicEvaluatorCheckBox(const Net* n, QWidget* parent)
        : QCheckBox(QString::fromStdString(n->get_name()),parent), mNet(n)
    {;}

    QPair<const Net *, BooleanFunction::Value> LogicEvaluatorCheckBox::getValue() const
    {
        return QPair<const Net*,BooleanFunction::Value>(mNet, isChecked()?BooleanFunction::Value::ONE:BooleanFunction::Value::ZERO);
    }

    void LogicEvaluatorCheckBox::setValue(const Net* n, BooleanFunction::Value val)
    {
        if (n != mNet) return;
        setChecked(val==BooleanFunction::Value::ONE);
    }
    //-------------------------------------------------------------------

    LogicEvaluatorPingroup::LogicEvaluatorPingroup(const std::vector<const Net *> &nets, bool outp, const QString& grpName, QWidget* parent)
        : QFrame(parent), mOutput(outp)
    {
        constructor(nets,grpName);
    }

    LogicEvaluatorPingroup::LogicEvaluatorPingroup(const Net* net, bool outp, QWidget* parent)
        : QFrame(parent), mOutput(outp)
    {
        std::vector<const Net*> nets;
        nets.push_back(net);
        constructor(nets);
    }

    void LogicEvaluatorPingroup::constructor(const std::vector<const Net *> &nets, const QString &grpName)
    {
        int nbits = nets.size();
        QHBoxLayout* topLayout = new QHBoxLayout(this);
        topLayout->setAlignment(Qt::AlignTop);
        topLayout->setContentsMargins(mOutput?0:8,4,mOutput?8:0,4);
        QVBoxLayout* valLayout = new QVBoxLayout;
        valLayout->setSpacing(4);
        QtCompat::setMarginWidth(valLayout,0);

        if (!grpName.isEmpty())
        {
            QLabel* lab = new QLabel(grpName, this);
            valLayout->addWidget(lab,0,Qt::AlignBottom|Qt::AlignHCenter);
        }
        mGroupValue = new LogicEvaluatorValue(mOutput?0:nbits, this);
        connect(mGroupValue, &LogicEvaluatorValue::valueChanged, this, &LogicEvaluatorPingroup::handleGroupValueChanged);
        valLayout->addWidget(mGroupValue,0, Qt::AlignTop|Qt::AlignHCenter);

        QVBoxLayout* pinLayout = new QVBoxLayout;
        for (const Net* net : nets)
        {
            LogicEvaluatorCheckBox* lecb = new LogicEvaluatorCheckBox(net, this);
            connect(lecb, &QCheckBox::stateChanged, this, &LogicEvaluatorPingroup::handleCheckStateChanged);
            if (mOutput)
                lecb->setDisabled(true);
            else
            {
                lecb->setLayoutDirection(Qt::RightToLeft);
            }
            mPinList.prepend(lecb);
            pinLayout->addWidget(lecb);
        }

        if (mOutput)
        {
            topLayout->addLayout(pinLayout);
            topLayout->addSpacing(20);
            topLayout->addLayout(valLayout);
        }
        else
        {
            topLayout->addLayout(valLayout);
            topLayout->addSpacing(20);
            topLayout->addLayout(pinLayout);
        }
        mGroupValue->setValue(0);
    }

    void LogicEvaluatorPingroup::handleCheckStateChanged(int state)
    {
        Q_UNUSED(state);

        int mask = 1 << (mPinList.size()-1);
        int val = 0;
        for (LogicEvaluatorCheckBox* cb : mPinList)
        {
            if (cb->isChecked()) val |= mask;
            mask >>= 1;
        }
        mGroupValue->setValue(val);
        if (mGroupValue->isLabel())
           Q_EMIT triggerRecalc();
    }

    void LogicEvaluatorPingroup::handleGroupValueChanged(int val)
    {
        int mask = 1 << (mPinList.size()-1);
        for (LogicEvaluatorCheckBox* cb : mPinList)
        {
            cb->setChecked(val&mask);
            mask >>= 1;
        }
        Q_EMIT triggerRecalc();
    }

    QPair<const Net *, BooleanFunction::Value> LogicEvaluatorPingroup::getValue(int index) const
    {
        return mPinList.at(index)->getValue();
    }

    void LogicEvaluatorPingroup::setValue(const Net* n, BooleanFunction::Value val)
    {
        for (LogicEvaluatorCheckBox* cb : mPinList)
            cb->setValue(n,val);
    }
}
