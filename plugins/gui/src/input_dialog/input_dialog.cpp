#include "gui/input_dialog/input_dialog.h"

#include "gui/validator/stacked_validator.h"

#include<QDialogButtonBox>

namespace hal
{
    InputDialog::InputDialog(QWidget* parent, Qt::WindowFlags f) : QDialog(parent, f)
    {
        init();
    }

    InputDialog::InputDialog(const QString& window_title, const QString& info_text, const QString& input_text, QWidget* parent, Qt::WindowFlags f) : QDialog(parent, f)
    {
        init();

        setWindowTitle(window_title);
        set_info_text(info_text);
        set_input_text(input_text);
    }

    void InputDialog::init()
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


        m_validator = StackedValidator();

        connect(button_box, &QDialogButtonBox::accepted, this, &InputDialog::handle_ok_clicked);
        connect(button_box, &QDialogButtonBox::rejected, this, &InputDialog::handle_cancel_clicked);
        connect(m_input_text_edit, &QLineEdit::textChanged, this, &InputDialog::handle_text_changed);
    }

    void InputDialog::set_window_title(const QString& title)
    {
        setWindowTitle(title);
    }

    void InputDialog::set_info_text(const QString& text)
    {
        m_label_info_text->setText(text);
    }

    void InputDialog::set_input_text(const QString& text)
    {
        m_input_text_edit->setText(text.trimmed());
        m_input_text_edit->selectAll();
    }

    void InputDialog::set_warning_text(const QString& text)
    {
        m_warning_text = text;
    }

    void InputDialog::show_warning_text()
    {
        m_label_warning_text->setText(m_warning_text);
    }

    void InputDialog::hide_warning_text()
    {
        m_label_warning_text->setText("");
    }

    void InputDialog::enable_progression()
    {
        m_ok_button->setDisabled(false);
        hide_warning_text();
    }

    void InputDialog::disable_progression()
    {
        set_warning_text(m_validator.fail_text());
        m_ok_button->setDisabled(true);
        show_warning_text();
    }

    void InputDialog::add_validator(Validator* Validator)
    {
        m_validator.add_validator(Validator);
    }

    void InputDialog::remove_validator(Validator* Validator)
    {
        m_validator.remove_validator(Validator);
    }

    void InputDialog::clear_validators()
    {
        m_validator.clear_validators();
    }

    QString InputDialog::text_value() const
    {
        return m_input_text_edit->text().trimmed();
    }

    void InputDialog::handle_text_changed(const QString& text)
    {
        if(!m_validator.validate(text.trimmed()))
            disable_progression();
        else
            enable_progression();
    }

    void InputDialog::handle_ok_clicked()
    {
        done(QDialog::Accepted);
    }

    void InputDialog::handle_cancel_clicked()
    {
        done(QDialog::Rejected);
    }
}
