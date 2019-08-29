#ifndef INPUT_DIALOG_H
#define INPUT_DIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class input_dialog : public QDialog
{
    public:
        input_dialog(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
        input_dialog(const QString& title, const QString& info_text, QStringList* forbidden_strings = nullptr, QStringList* unique_strings = nullptr, std::function<bool()>* condition_function = nullptr, Qt::WindowFlags f = Qt::WindowFlags(), QWidget* parent = nullptr);

        //static QString get_text(bool* ok);

        void set_window_title(const QString& title);
        void set_info_text(const QString& info_text);

        void set_custom_warning_text(const QString& warning_text);

        void set_unique_strings(QStringList* strings);
        void remove_unique_strings();

        void set_forbidden_strings(QStringList* strings);
        void remove_forbidden_strings();

        void set_conditional_function(std::function<bool()>* accept_condition_function, const QString& warning_text = "");
        void remove_conditional_function();

        QString text_value() const;

    private:
        QVBoxLayout* m_layout;
        QLabel* m_label_info_text;
        QLabel* m_label_warning_text;
        QLineEdit* m_input_text_edit;
        QPushButton* m_ok_button;

        QString m_warning_text = "";
        QString m_warning_text_custom = "";
        QString m_warning_text_empty = "Choosen input can't be empty.";
        QString m_warning_text_unique = "Coosen input already assigned.";
        QString m_warning_text_forbidden = "Choosen input is reserved.";

        std::function<bool()>* m_accept_condition_function = nullptr;
        QStringList* m_unique_strings = nullptr;
        QStringList* m_forbidden_strings = nullptr;


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

#endif      // INPUT_DIALOG_H
