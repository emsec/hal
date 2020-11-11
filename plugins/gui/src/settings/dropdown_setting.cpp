#include "gui/settings/dropdown_setting.h"

#include "gui/label_button/label_button.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPair>

#include <QMap>
#include <QComboBox>
#include <QStringList>
#include "gui/gui_globals.h"

namespace hal
{
    DropdownSetting::DropdownSetting(const QString& key, const QString& title, const QMap<QString, QVariant>& options, const QString& mDescription, QWidget *parent) : SettingsWidget(key, parent), mOptions(options)
    {
        mLabels.append(QPair<QLabel*, QString>(mName, title));

        QHBoxLayout* layout = new QHBoxLayout();
        mContainer->addLayout(layout);

        mComboBox = new QComboBox(this);
        mComboBox->addItems(options.keys());
        mComboBox->setStyleSheet("QComboBox{width: 150px;}");
        connect(mComboBox, QOverload<const QString &>::of(&QComboBox::currentIndexChanged), this, &DropdownSetting::onIndexChanged);


        layout->addWidget(mComboBox);

        QLabel* label = new QLabel();
        layout->addWidget(label);

        mLabels.append(QPair<QLabel*, QString>(label, mDescription));
    }

    void DropdownSetting::load(const QVariant& value)
    {
        mComboBox->setCurrentText(mOptions.key(value.toString()));
    }

    QVariant DropdownSetting::value()
    {
        return mOptions.value(mComboBox->currentText());
    }

    // void DropdownSetting::rollback()
    // {

    // }

    void DropdownSetting::onIndexChanged(QString text)
    {
        Q_UNUSED(text);
        this->triggerSettingUpdated();
    }
}
