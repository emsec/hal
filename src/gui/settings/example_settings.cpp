#include "settings/example_settings.h"

#include "label_button/label_button.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPair>

example_settings::example_settings(QWidget* parent) : settings_widget(parent)
{
    m_labels.append(QPair<QLabel*, QString>(m_name, "Example"));

    QHBoxLayout* layout = new QHBoxLayout();
    m_layout->addLayout(layout);

    QLabel* label = new QLabel();
    layout->addWidget(label);

    m_labels.append(QPair<QLabel*, QString>(label, "Click this button if you enjoy clicking buttons"));

    label_button* button = new label_button();
    layout->addWidget(button);

    m_labels.append(QPair<QLabel*, QString>(button, "Button"));
}

void example_settings::load_settings()
{
}

void example_settings::save_settings()
{
}

void example_settings::restore_default_settings()
{
}
