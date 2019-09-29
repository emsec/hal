#include "settings/dropdown_setting.h"

#include "label_button/label_button.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPair>

#include <QComboBox>
#include <QStringList>
#include "gui_globals.h"

dropdown_setting::dropdown_setting(const QString& key, const QString& title, const QStringList& options, const QString& description, QWidget *parent) : settings_widget(key, parent)
{
    m_labels.append(QPair<QLabel*, QString>(m_name, title));

    QHBoxLayout* layout = new QHBoxLayout();
    m_layout->addLayout(layout);

    m_combo_box = new QComboBox(this);
    m_combo_box->addItems(options);
    m_combo_box->setStyleSheet("QComboBox{width: 150px;}");
    connect(m_combo_box, QOverload<const QString &>::of(&QComboBox::currentIndexChanged), this, &dropdown_setting::on_index_changed);


    layout->addWidget(m_combo_box);

    QLabel* label = new QLabel();
    layout->addWidget(label);

    m_labels.append(QPair<QLabel*, QString>(label, description));
}

void dropdown_setting::load(const QVariant& value)
{
    if(value.toString() == "sunny")
        m_combo_box->setCurrentIndex(1);
    else
        m_combo_box->setCurrentIndex(0);
}

QVariant dropdown_setting::value()
{
    switch (m_combo_box->currentIndex())
    {
        case 0: return QVariant("darcula");
        case 1: return QVariant("sunny");
    }
}

// void dropdown_setting::rollback()
// {

// }

void dropdown_setting::on_index_changed(QString text)
{
    this->trigger_setting_updated();
}
