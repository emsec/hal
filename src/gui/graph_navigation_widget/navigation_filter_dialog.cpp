#include "graph_navigation_widget/navigation_filter_dialog.h"
#include <QRegExp>

navigation_filter_dialog::navigation_filter_dialog(QRegExp*& regExp, QWidget* parent) : QDialog(parent), m_button_box(QDialogButtonBox::Ok | QDialogButtonBox::Cancel), m_regExp(regExp)
{
    setWindowTitle("New Filter");
    setSizeGripEnabled(true);
    setMinimumWidth(600);
    setLayout(&m_content_layout);
    m_content_layout.addLayout(&m_form_layout);
    m_name.setPlaceholderText("Filter Name");
    m_regex.setPlaceholderText("Regular Expression");

    m_form_layout.addRow(&m_name);
    m_form_layout.addRow(&m_regex);
    m_form_layout.addRow("<font color=\"DeepPink\">Filter Attribut</font>", &m_type_combo_box);

    m_status_message.setStyleSheet("QLabel { background-color: rgba(64, 64, 64, 1);color: rgba(255, 0, 0, 1);border: 1px solid rgba(255, 0, 0, 1)}");
    m_status_message.setText("No Name specified");
    m_status_message.setAlignment(Qt::AlignCenter);
    m_status_message.setMinimumHeight(90);
    m_status_message.hide();
    m_content_layout.addWidget(&m_status_message);
    m_content_layout.addWidget(&m_button_box, Qt::AlignBottom);

    connect(&m_button_box, SIGNAL(accepted()), this, SLOT(verify()));
    connect(&m_button_box, SIGNAL(rejected()), this, SLOT(reject()));

    //connect(this, SIGNAL(finished(int)), this, SLOT(reset(int)));
}

void navigation_filter_dialog::verify()
{
    if (m_name.text().isEmpty())
    {
        m_status_message.setText("No Name specified");
        m_status_message.show();
        return;
    }
    if (m_regex.text().isEmpty())
    {
        m_status_message.setText("No Regular Expression specified");
        m_status_message.show();
        return;
    }
    m_regExp = new QRegExp(m_regex.text());
    if (!m_regExp->isValid())
    {
        m_status_message.setText("Specified Regular Expression is invalid");
        m_status_message.show();
        delete m_regExp;
        m_regExp = nullptr;
        return;
    }
    m_status_message.hide();
    accept();
}

//void hal_navigation_filter_dialog::reset()
//{
//    m_name.clear();
//    m_regex.clear();
//}

//hal_filter_dialog::filter_combo_box::filter_combo_box(QWidget *parent) : QComboBox(parent)
//{
//    addItem("Process", QVariant(0));
//    addItem("Show All", QVariant(1));
//    addItem("Hide All", QVariant(2));
//}
