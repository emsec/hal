#include "settings/gui_theme_settings.h"

#include "label_button/label_button.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPair>

#include <QComboBox>
#include <QStringList>
#include "gui_globals.h"

gui_theme_settings::gui_theme_settings(QWidget *parent) : settings_widget(parent)
{
    m_labels.append(QPair<QLabel*, QString>(m_name, "Main Style Theme"));

    QHBoxLayout* layout = new QHBoxLayout();
    m_layout->addLayout(layout);

    m_style_box = new QComboBox(this);
    m_style_box->addItems(QStringList() << "Darcula" << "Sunny");
    m_style_box->setStyleSheet("QComboBox{width: 150px;}");

    if(g_settings.value("main_style/theme", "") == "sunny")//default is darcula
        m_style_box->setCurrentIndex(1);

    layout->addWidget(m_style_box);

    QLabel* label = new QLabel();
    layout->addWidget(label);

    m_labels.append(QPair<QLabel*, QString>(label, " will be set as your theme after restarting"));

    connect(m_style_box, QOverload<const QString &>::of(&QComboBox::currentIndexChanged), this, &gui_theme_settings::on_index_changed);
}

void gui_theme_settings::load_settings()
{

}

void gui_theme_settings::save_settings()
{

}

void gui_theme_settings::restore_default_settings()
{

}

void gui_theme_settings::on_index_changed(QString text)
{
    if(text == "Darcula")
        g_settings.setValue("main_style/theme", "darcula");
    else if(text == "Sunny")
        g_settings.setValue("main_style/theme", "sunny");

    g_settings.sync();
}
