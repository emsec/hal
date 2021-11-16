#pragma once

#include <QDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QList>
#include <vector>

namespace hal {

    class Net;

    class ClockSetDialog : public QDialog
    {
        Q_OBJECT
    public:
        explicit ClockSetDialog(const QList<const Net*>& inputs, QWidget *parent = nullptr);

        int netIndex() const { return mComboNet->currentIndex(); }
        int period() const { return mSpinPeriod->value(); }
        int startValue() const { return mSpinStartValue->value(); }
        int duration() const { return mSpinDuration->value(); }
        bool dontUseClock() const { return mDontUseClock->isChecked(); }
    private Q_SLOTS:
        void dontUseClockChanged(bool state);
    private:
        QComboBox* mComboNet;
        QSpinBox* mSpinPeriod;
        QSpinBox* mSpinStartValue;
        QSpinBox* mSpinDuration;
        QCheckBox* mDontUseClock;

        QDialogButtonBox* mButtonBox;
    };
}
