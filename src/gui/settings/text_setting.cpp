#include "settings/text_setting.h"

#include "label_button/label_button.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPair>

#include <QComboBox>
#include <QStringList>
#include "gui_globals.h"

text_setting::text_setting(const QString& key, const QString& title, const QString& description, const QString& placeholder, QWidget *parent) : settings_widget(key, parent)
{
    m_labels.append(QPair<QLabel*, QString>(m_name, title));

    QHBoxLayout* layout = new QHBoxLayout();
    m_layout->addLayout(layout);

    m_textfield = new QLineEdit(this);
    m_textfield->setPlaceholderText(placeholder);
    m_textfield->setStyleSheet("QLineEdit{width: 200px;}");
    connect(m_textfield, &QLineEdit::textChanged, this, &text_setting::on_text_changed);


    layout->addWidget(m_textfield);

    QLabel* label = new QLabel();
    layout->addWidget(label);

    m_labels.append(QPair<QLabel*, QString>(label, description));
}

void text_setting::load(const QVariant& value)
{
    m_textfield->setText(value.toString());
}

QVariant text_setting::value()
{
    return QVariant(m_textfield->text());
}

// void dropdown_setting::rollback()
// {

// }

void text_setting::on_text_changed()
{
    this->trigger_setting_updated();
}
