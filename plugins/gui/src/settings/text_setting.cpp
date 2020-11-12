#include "gui/settings/text_setting.h"

#include "gui/label_button/label_button.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPair>

#include <QComboBox>
#include <QStringList>
#include "gui/gui_globals.h"

namespace hal
{
    TextSetting::TextSetting(const QString& key, const QString& title, const QString& mDescription, const QString& placeholder, QWidget *parent) : SettingsWidget(key, parent)
    {
        mLabels.append(QPair<QLabel*, QString>(mName, title));

        QHBoxLayout* layout = new QHBoxLayout();
        mContainer->addLayout(layout);

        mTextfield = new QLineEdit(this);
        mTextfield->setPlaceholderText(placeholder);
        mTextfield->setStyleSheet("QLineEdit{width: 200px;}");
        connect(mTextfield, &QLineEdit::textChanged, this, &TextSetting::onTextChanged);


        layout->addWidget(mTextfield);

        QLabel* label = new QLabel();
        layout->addWidget(label);

        mLabels.append(QPair<QLabel*, QString>(label, mDescription));
    }

    void TextSetting::load(const QVariant& value)
    {
        mTextfield->setText(value.toString());
    }

    QVariant TextSetting::value()
    {
        return QVariant(mTextfield->text());
    }

    // void DropdownSetting::rollback()
    // {

    // }

    void TextSetting::onTextChanged()
    {
        this->triggerSettingUpdated();
    }
}
