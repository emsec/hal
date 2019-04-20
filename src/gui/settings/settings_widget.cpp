#include "settings/settings_widget.h"

#include <QLabel>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QVBoxLayout>

settings_widget::settings_widget(QWidget* parent) : QFrame(parent), m_layout(new QVBoxLayout()), m_name(new QLabel()), m_unsaved_changes(false), m_highlight_color(52, 56, 57)
{
    setFrameStyle(QFrame::NoFrame);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    m_name->setObjectName("name-label");

    setLayout(m_layout);
    m_layout->addWidget(m_name);
    hide();
}

QColor settings_widget::highlight_color()
{
    return m_highlight_color;
}

bool settings_widget::unsaved_changes()
{
    return m_unsaved_changes;
}

void settings_widget::set_highlight_color(const QColor& color)
{
    m_highlight_color = color;
}

void settings_widget::reset_labels()
{
    for (QPair<QLabel*, QString>& pair : m_labels)
    {
        pair.first->setText(pair.second);
    }
}

bool settings_widget::match_labels(const QString& string)
{
    bool match_found = false;

    if (!string.isEmpty())
    {
        QString color        = m_highlight_color.name();
        QString opening_html = "<span style=\"background-color:" + color + "\">";
        QString closing_html = "</span>";
        int string_length    = string.length();
        int html_lenght      = opening_html.length() + string_length + closing_html.length();

        for (QPair<QLabel*, QString>& pair : m_labels)
        {
            int index = pair.second.indexOf(string, 0, Qt::CaseInsensitive);
            if (index != -1)
            {
                match_found         = true;
                QString highlighted = pair.second;
                while (index != -1)
                {
                    QString substring;
                    substring   = highlighted.mid(index, string_length);
                    highlighted = highlighted.remove(index, string_length);
                    highlighted.insert(index, opening_html + substring + closing_html);
                    index = highlighted.indexOf(string, index + html_lenght, Qt::CaseInsensitive);
                }
                pair.first->setText(highlighted);
            }
            else
                pair.first->setText(pair.second);
        }
    }
    return match_found;
}
