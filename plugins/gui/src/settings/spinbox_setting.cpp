#include "gui/settings/spinbox_setting.h"

#include "gui/label_button/label_button.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPair>

#include <QSpinBox>
#include <QStringList>
#include "gui/gui_globals.h"

namespace hal
{
    SpinboxSetting::SpinboxSetting(const QString& key, const QString& title, const int min, const int max, const QString& mDescription, QWidget* parent) : SettingsWidget(key, parent)
    {
        mLabels.append(QPair<QLabel*, QString>(mName, title));

        QHBoxLayout* layout = new QHBoxLayout();
        mContainer->addLayout(layout);

        mSpinbox = new QSpinBox(this);
        mSpinbox->setMinimum(min);
        mSpinbox->setMaximum(max);
        connect(mSpinbox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &SpinboxSetting::onSpinboxValueChanged);

        layout->addWidget(mSpinbox);

        QLabel* label = new QLabel();
        layout->addWidget(label);

        mLabels.append(QPair<QLabel*, QString>(label, mDescription));
    }

    void SpinboxSetting::load(const QVariant& value)
    {
        mSpinbox->setValue(value.toInt());
    }

    QVariant SpinboxSetting::value()
    {
        return QVariant(mSpinbox->value());
    }

    void SpinboxSetting::onSpinboxValueChanged(int value)
    {
        Q_UNUSED(value);
        triggerSettingUpdated();
    }
}
