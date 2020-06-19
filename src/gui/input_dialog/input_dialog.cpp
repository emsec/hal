#include "input_dialog/input_dialog.h"

#include "validator/stacked_validator.h"

#include<QDialogButtonBox>

namespace hal
{
    input_dialog::input_dialog(QWidget* parent, Qt::WindowFlags f) : QDialog(parent, f)
    {
        init();
    }

    input_dialog::input_dialog(const QString& window_title, const QString& info_text, const QString& input_text, QWidget* parent, Qt::WindowFlags f) : QDialog(parent, f)
    {
        init();

        setWindowTitle(window_title);
        set_info_text(info_text);
        set_input_text(input_text);
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


        m_validator = stacked_validator();

        connect(button_box, &QDialogButtonBox::accepted, this, &input_dialog::handle_ok_clicked);
        connect(button_box, &QDialogButtonBox::rejected, this, &input_dialog::handle_cancel_clicked);
        connect(m_input_text_edit, &QLineEdit::textChanged, this, &input_dialog::handle_text_changed);
    }

    void input_dialog::set_window_title(const QString& title)
    {
        setWindowTitle(title);
    }

    void input_dialog::set_info_text(const QString& text)
    {
        m_label_info_text->setText(text);
    }

    void input_dialog::set_input_text(const QString& text)
    {
        m_input_text_edit->setText(text);
        m_input_text_edit->selectAll();
    }

    void input_dialog::set_warning_text(const QString& text)
    {
        m_warning_text = text;
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
        set_warning_text(m_validator.fail_text());
        m_ok_button->setDisabled(true);
        show_warning_text();
    }

    void input_dialog::add_validator(validator* validator)
    {
        m_validator.add_validator(validator);
    }

    void input_dialog::remove_validator(validator* validator)
    {
        m_validator.remove_validator(validator);
    }

    void input_dialog::clear_validators()
    {
        m_validator.clear_validators();
    }

    QString input_dialog::text_value() const
    {
        return m_input_text_edit->text().trimmed();
    }

    void input_dialog::handle_text_changed(const QString& text)
    {
        if(!m_validator.validate(text))
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
}
