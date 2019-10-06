#include "settings/checkbox_setting.h"

#include "label_button/label_button.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPair>

#include <QCheckBox>
#include <QStringList>
#include "gui_globals.h"

checkbox_setting::checkbox_setting(const QString& key, const QString& title, const QString& text, const QString& description, QWidget *parent) : settings_widget(key, parent)
{
    m_labels.append(QPair<QLabel*, QString>(m_name, title));

    QHBoxLayout* layout = new QHBoxLayout();
    m_container->addLayout(layout);

    m_check_box = new QCheckBox(text, this);
    // m_check_box->setStyleSheet("QComboBox{width: 150px;}");
    connect(m_check_box, &QCheckBox::clicked, this, &checkbox_setting::on_state_changed);


    layout->addWidget(m_check_box);

    QLabel* label = new QLabel();
    layout->addWidget(label);

    m_labels.append(QPair<QLabel*, QString>(label, description));
}

void checkbox_setting::load(const QVariant& value)
{
    m_check_box->setChecked(value.toBool());
}

QVariant checkbox_setting::value()
{
    return QVariant(m_check_box->isChecked());
}

// void dropdown_setting::rollback()
// {

// }

void checkbox_setting::on_state_changed(bool checked)
{
    Q_UNUSED(checked);
    this->trigger_setting_updated();
}
