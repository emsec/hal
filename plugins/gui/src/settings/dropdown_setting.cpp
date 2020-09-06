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
    DropdownSetting::DropdownSetting(const QString& key, const QString& title, const QMap<QString, QVariant>& options, const QString& description, QWidget *parent) : SettingsWidget(key, parent), m_options(options)
    {
        m_labels.append(QPair<QLabel*, QString>(m_name, title));

        QHBoxLayout* layout = new QHBoxLayout();
        m_container->addLayout(layout);

        m_combo_box = new QComboBox(this);
        m_combo_box->addItems(options.keys());
        m_combo_box->setStyleSheet("QComboBox{width: 150px;}");
        connect(m_combo_box, QOverload<const QString &>::of(&QComboBox::currentIndexChanged), this, &DropdownSetting::on_index_changed);


        layout->addWidget(m_combo_box);

        QLabel* label = new QLabel();
        layout->addWidget(label);

        m_labels.append(QPair<QLabel*, QString>(label, description));
    }

    void DropdownSetting::load(const QVariant& value)
    {
        m_combo_box->setCurrentText(m_options.key(value.toString()));
    }

    QVariant DropdownSetting::value()
    {
        return m_options.value(m_combo_box->currentText());
    }

    // void DropdownSetting::rollback()
    // {

    // }

    void DropdownSetting::on_index_changed(QString text)
    {
        Q_UNUSED(text);
        this->trigger_setting_updated();
    }
}
