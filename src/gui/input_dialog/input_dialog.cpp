#include "input_dialog/input_dialog.h"


#include<QDialogButtonBox>

#include <QDebug>

input_dialog::input_dialog(QWidget* parent, Qt::WindowFlags f) : QDialog(parent, f)
{
    init();
}

input_dialog::input_dialog(const QString& title, const QString& info_text, QStringList* forbidden_strings, QStringList* unique_strings, std::function<bool()>* condition_function, Qt::WindowFlags f, QWidget* parent) : QDialog(parent, f)
{
    
    init();

    setWindowTitle(title);
    m_label_info_text->setText(info_text);
    m_forbidden_strings = forbidden_strings;
    m_unique_strings = unique_strings;
    m_accept_condition_function = condition_function;
}

void input_dialog::init()
{
    m_label_info_text = new QLabel();
    m_label_warning_text = new QLabel();
    m_input_text_edit = new QLineEdit();
 
    QDialogButtonBox* button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    m_ok_button = button_box->button(QDialogButtonBox::Ok);

    m_layout = new QVBoxLayout(this);
    m_layout->addWidget(m_label_info_text);
    m_layout->addWidget(m_input_text_edit);
    m_layout->addWidget(m_label_warning_text);
    m_layout->addWidget(button_box);


    m_label_info_text->setStyleSheet("font-weight: bold");
    m_label_info_text->setContentsMargins(0, 0, 0, 15);

    m_ok_button->setDisabled(true);

    m_label_warning_text->setStyleSheet("color: red");
    m_label_warning_text->setAlignment(Qt::AlignHCenter);
    m_label_warning_text->setContentsMargins(0, 0, 0, 3);

    m_layout->setContentsMargins(15, 15, 15, 10);

    connect(button_box, &QDialogButtonBox::accepted, this, &input_dialog::handle_ok_clicked);
    connect(button_box, &QDialogButtonBox::rejected, this, &input_dialog::handle_cancel_clicked);
    connect(m_input_text_edit, &QLineEdit::textChanged, this, &input_dialog::handle_text_changed);
}

void input_dialog::set_window_title(const QString& title)
{
    setWindowTitle(title);
}

void input_dialog::set_info_text(const QString& info_text)
{
    m_label_info_text->setText(info_text);
}

void input_dialog::set_custom_warning_text(const QString& warning_text)
{
    m_warning_text_custom = warning_text;
}

void input_dialog::show_warning_text()
{
    m_label_warning_text->setText(m_warning_text);
}

void input_dialog::hide_warning_text()
{
    m_label_warning_text->setText("");
}

void input_dialog::enable_progression()
{
    m_ok_button->setDisabled(false);
    hide_warning_text();
}

void input_dialog::disable_progression()
{
    m_ok_button->setDisabled(true);
    show_warning_text();
}

void input_dialog::set_conditional_function(std::function<bool()>* accept_condition_function, const QString& warning_text)
{
   m_warning_text_custom = warning_text; 
   m_accept_condition_function = accept_condition_function;   
}

void input_dialog::remove_conditional_function()
{
    m_accept_condition_function = nullptr;
}

void input_dialog::set_forbidden_strings(QStringList* strings)
{
    m_forbidden_strings = strings;
}

void input_dialog::remove_forbidden_strings()
{
    m_forbidden_strings = nullptr;
}

void input_dialog::set_unique_strings(QStringList* strings)
{
    m_unique_strings = strings;
}

void input_dialog::remove_unique_strings()
{
    m_unique_strings = nullptr;
}

QString input_dialog::text_value() const
{
    return m_input_text_edit->text();
}


void input_dialog::handle_text_changed(const QString& text)
{
    bool allow_progress = true;

    //check for forbidden strings
    if(m_forbidden_strings != nullptr)
    {
        if(m_forbidden_strings->contains(text))
        {
            allow_progress = false;
            m_warning_text = m_warning_text_forbidden;
        }
    }

    //check if strings is unique
    if(allow_progress && m_unique_strings != nullptr)
    {
        if(m_unique_strings->contains(text))
        {
            allow_progress = false;
            m_warning_text = m_warning_text_unique;
        }
    }

    //check conditional function
    if(allow_progress && m_accept_condition_function != nullptr)
    {
        auto f = *(m_accept_condition_function);

        if(!f())
        {
            allow_progress = false;
            m_warning_text = m_warning_text_custom;
        }
    }

    //check if input is empty
    if(allow_progress && text.isEmpty())
    {   
        allow_progress = false;
        m_warning_text = m_warning_text_empty;
    }
        
    //show or hide corresponding warning + enable or disable ok button depending on result
    if(!allow_progress)
        disable_progression();
    else
        enable_progression();

}

void input_dialog::handle_ok_clicked()
{
    done(QDialog::Accepted);
}

void input_dialog::handle_cancel_clicked()
{
    done(QDialog::Rejected);
}

/*
QString input_dialog::get_text(bool* ok)
{
    input_dialog* ipd = new input_dialog();
    ipd->set_window_title("Rename view");
    ipd->set_info_text("Please enter a new and uniqe name for the selected view.");
    int x = ipd->exec();

    if(x == QDialog::Accepted)
    {
        *ok = true;
        return ipd->text_value();
    }
    else
    {
        *ok = false;
        return nullptr; 
    }
} */
