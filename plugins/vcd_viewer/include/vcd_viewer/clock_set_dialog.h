#pragma once

#include <QDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QSpinBox>
#include <vector>

namespace hal {

    class Net;

    class ClockSetDialog : public QDialog
    {
        Q_OBJECT
    public:
        explicit ClockSetDialog(const std::vector<Net*>& inputs, QWidget *parent = nullptr);

        int netIndex() const { return mComboNet->currentIndex(); }
        int period() const { return mSpinPeriod->value(); }
        int startValue() const { return mSpinStartValue->value(); }
        int duration() const { return mSpinDuration->value(); }
    private:
        QComboBox* mComboNet;
        QSpinBox* mSpinPeriod;
        QSpinBox* mSpinStartValue;
        QSpinBox* mSpinDuration;
        QDialogButtonBox* mButtonBox;
    };
}
