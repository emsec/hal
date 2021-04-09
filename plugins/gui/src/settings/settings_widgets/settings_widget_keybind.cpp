#include "gui/settings/settings_widgets/settings_widget_keybind.h"

#include "gui/label_button/label_button.h"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPair>

#include <QCheckBox>
#include <QKeySequenceEdit>
#include <QStringList>
#include <QTimer>
#include "gui/gui_globals.h"

#include "gui/settings/settings_items/settings_item_keybind.h"

namespace hal
{
    SettingsWidgetKeybind::SettingsWidgetKeybind(SettingsItemKeybind* item, QWidget* parent)
        : SettingsWidget(item, parent)
    {

        QVBoxLayout* layout = new QVBoxLayout();
        mContainer->addLayout(layout);

        QLabel* label = new QLabel(this);
        layout->addWidget(label);

        mKeybindEdit = new KeybindEdit(this);
        connect(mKeybindEdit, &QKeySequenceEdit::editingFinished, this, &SettingsWidgetKeybind::clearErrorMessage);
        connect(mKeybindEdit, &KeybindEdit::editAccepted, this, &SettingsWidgetKeybind::onKeybindEditAccepted);
        connect(mKeybindEdit, &KeybindEdit::editRejected, this, &SettingsWidgetKeybind::onKeybindEditRejected);
        layout->addWidget(mKeybindEdit);

        mErrorMessage = new QLabel(this);
        mErrorMessage->setStyleSheet("color: red;");
        layout->addWidget(mErrorMessage);
        load(item->value());
    }

    void SettingsWidgetKeybind::load(const QVariant& value)
    {
        SettingsItemKeybind* sik = static_cast<SettingsItemKeybind*>(mSettingsItem);
        QKeySequence currentSeq(value.toString());
        mKeybindEdit->load(currentSeq, sik);
        mErrorMessage->setText(QString());
    }

    QVariant SettingsWidgetKeybind::value()
    {
        QKeySequence seq = mKeybindEdit->keySequence();
        if (seq.isEmpty())
            return QVariant(QVariant::Invalid);
        return QVariant(seq); // auto-cast
    }

    void SettingsWidgetKeybind::onKeybindEditAccepted()
    {
        trigger_setting_updated();
    }

    void SettingsWidgetKeybind::clearErrorMessage()
    {
        mErrorMessage->setText(QString());
    }

    void SettingsWidgetKeybind::onKeybindEditRejected(const QString& errMsg)
    {
        mErrorMessage->setText(errMsg);

        QTimer::singleShot(2500, this, &SettingsWidgetKeybind::clearErrorMessage);
    }
}
