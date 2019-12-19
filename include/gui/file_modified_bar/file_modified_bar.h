#ifndef FILE_MODIFIED_BAR_H
#define FILE_MODIFIED_BAR_H

#include "gui_def.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QString>

class file_modified_bar : public QFrame
{
    Q_OBJECT

public:
    file_modified_bar(QWidget* parent = 0);

    void set_message(QString message);

Q_SIGNALS:
    void reload_clicked();
    void ignore_clicked();
    void ok_clicked();

public Q_SLOTS:
    void handle_file_changed(QString path);

private:
    QHBoxLayout* m_layout;
    QLabel* m_message_label;
    QPushButton* m_reload_button;
    QPushButton* m_ignore_button;
    QPushButton* m_ok_button;

    void handle_reload_clicked();
    void handle_ignore_clicked();
    void handle_ok_clicked();

    void handle_file_content_modified(QString path);
    void handle_file_mov_or_del(QString path);
};

#endif    // FILE_MODIFIED_BAR_H
