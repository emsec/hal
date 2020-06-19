#pragma once

#include "validator/stacked_validator.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
namespace hal{
class input_dialog : public QDialog
{
    public:
        input_dialog(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
        input_dialog(const QString& window_title, const QString& info_text, const QString& input_text, QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

        void set_window_title(const QString& title);

        void set_info_text(const QString& text);
        void set_input_text(const QString& text);

        void set_warning_text(const QString& text);
 
        void add_validator(validator* validator);
        void remove_validator(validator* validator);
        void clear_validators();

        QString text_value() const;

    private:
        QVBoxLayout* m_layout;
        QLabel* m_label_info_text;
        QLabel* m_label_warning_text;
        QLineEdit* m_input_text_edit;
        QPushButton* m_ok_button;

        QString m_warning_text = "";

        stacked_validator m_validator;

        void init();

        void show_warning_text();
        void hide_warning_text();

        void enable_ok_button();
        void disable_ok_button();

        void enable_progression();
        void disable_progression();

        void handle_ok_clicked();
        void handle_cancel_clicked();

        void handle_text_changed(const QString &text);
};
}
