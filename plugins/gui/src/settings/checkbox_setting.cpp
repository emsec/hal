#include "gui/settings/checkbox_setting.h"

#include "gui/label_button/label_button.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPair>

#include <QCheckBox>
#include <QStringList>
#include "gui/gui_globals.h"

namespace hal
{
    CheckboxSetting::CheckboxSetting(const QString& key, const QString& title, const QString& text, const QString& mDescription, QWidget *parent) : SettingsWidget(key, parent)
    {
        mLabels.append(QPair<QLabel*, QString>(mName, title));

        QHBoxLayout* layout = new QHBoxLayout();
        mContainer->addLayout(layout);

        mCheckBox = new QCheckBox(text, this);
        // mCheckBox->setStyleSheet("QComboBox{width: 150px;}");
        connect(mCheckBox, &QCheckBox::clicked, this, &CheckboxSetting::onStateChanged);


        layout->addWidget(mCheckBox);

        QLabel* label = new QLabel();
        layout->addWidget(label);

        mLabels.append(QPair<QLabel*, QString>(label, mDescription));
    }

    void CheckboxSetting::load(const QVariant& value)
    {
        mCheckBox->setChecked(value.toBool());
    }

    QVariant CheckboxSetting::value()
    {
        return QVariant(mCheckBox->isChecked());
    }

    // void DropdownSetting::rollback()
    // {

    // }

    void CheckboxSetting::onStateChanged(bool mChecked)
    {
        Q_UNUSED(mChecked);
        this->triggerSettingUpdated();
    }
}
