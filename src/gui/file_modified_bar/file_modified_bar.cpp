#include "file_modified_bar/file_modified_bar.h"

#include <QFileInfo>

namespace hal
{
    file_modified_bar::file_modified_bar(QWidget* parent)
    {
        Q_UNUSED(parent);
        m_layout = new QHBoxLayout();
        setLayout(m_layout);

        m_message_label = new QLabel("");
        m_layout->addWidget(m_message_label);

        m_reload_button = new QPushButton("Reload");
        m_reload_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_layout->addWidget(m_reload_button);

        m_ignore_button = new QPushButton("Ignore");
        m_ignore_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_layout->addWidget(m_ignore_button);

        m_ok_button = new QPushButton("Ok");
        m_ok_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_layout->addWidget(m_ok_button);

        connect(m_reload_button, &QPushButton::clicked, this, &file_modified_bar::handle_reload_clicked);
        connect(m_ignore_button, &QPushButton::clicked, this, &file_modified_bar::handle_ignore_clicked);
        connect(m_ok_button, &QPushButton::clicked, this, &file_modified_bar::handle_ok_clicked);
    }

    void file_modified_bar::handle_file_changed(QString path)
    {
        QFile file(path);

        if(file.exists())
            handle_file_content_modified(path);
        else
            handle_file_mov_or_del(path);
    }

    void file_modified_bar::handle_file_content_modified(QString path)
    {
        m_ok_button->setHidden(true);
        m_reload_button->setHidden(false);
        m_ignore_button->setHidden(false);

        m_message_label->setText(path + " has been modified on disk.");
    }

    void file_modified_bar::handle_file_mov_or_del(QString path)
    {
        m_ok_button->setHidden(false);
        m_reload_button->setHidden(true);
        m_ignore_button->setHidden(true);

        m_message_label->setText(path + " has been moved on disk.");
    }

    void file_modified_bar::handle_reload_clicked()
    {
        Q_EMIT reload_clicked();
    }

    void file_modified_bar::handle_ignore_clicked()
    {
        Q_EMIT ignore_clicked();
    }

    void file_modified_bar::handle_ok_clicked()
    {
        Q_EMIT ok_clicked();
    }
}
