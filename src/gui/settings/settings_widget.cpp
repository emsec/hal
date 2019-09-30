#include "settings/settings_widget.h"

#include <QLabel>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QStyle>
#include <QVBoxLayout>

// enable this to apply all settings as they are modified
//#define SETTINGS_UPDATE_IMMEDIATELY

settings_widget::settings_widget(const QString& key, QWidget* parent) : QFrame(parent), m_layout(new QVBoxLayout()), m_top_bar(new QHBoxLayout()), m_name(new QLabel()), m_revert(new QToolButton()), m_default(new QToolButton()), m_unsaved_changes(false), m_highlight_color(52, 56, 57), m_key(key)
{
    setFrameStyle(QFrame::NoFrame);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    m_name->setObjectName("name-label");

    setLayout(m_layout);

    m_revert->setText("R");
    m_revert->setToolTip("Revert your last change");
    m_revert->setMaximumWidth(20);
    connect(m_revert, &QToolButton::clicked, this, &settings_widget::handle_rollback);
    m_default->setText("D");
    m_default->setToolTip("Load the default value");
    m_default->setMaximumWidth(20);
    connect(m_default, &QToolButton::clicked, this, &settings_widget::handle_reset);
    m_top_bar->addWidget(m_name);
    m_top_bar->addStretch();
    m_top_bar->addWidget(m_revert);
    m_top_bar->addWidget(m_default);
    m_layout->addLayout(m_top_bar);
    
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

QString settings_widget::key()
{
    return m_key;
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

void settings_widget::trigger_setting_updated()
{
    QVariant val = value();
    if (m_preview)
    {
        m_preview->update(val);
    }
    if (m_signals_enabled)
    {
        #ifdef SETTINGS_UPDATE_IMMEDIATELY
        Q_EMIT setting_updated(this, key(), val);
        #else
        set_dirty(m_loaded_value != val);
        #endif
    }
}

void settings_widget::handle_reset()
{
    if (m_prepared)
    {
        load(m_default_value);
        #ifndef SETTINGS_UPDATE_IMMEDIATELY
        set_dirty(m_loaded_value != m_default_value);
        #endif
    }
}

void settings_widget::handle_rollback()
{
    if (m_prepared)
    {
        load(m_loaded_value);
        #ifndef SETTINGS_UPDATE_IMMEDIATELY
        set_dirty(false);
        #endif
    }
}

void settings_widget::set_dirty(bool dirty)
{
    m_dirty = dirty;
    QStyle* s = style();
    s->unpolish(this);
    s->polish(this);
}

bool settings_widget::dirty()
{
    return m_dirty;
}

void settings_widget::prepare(const QVariant& value, const QVariant& default_value)
{
    m_signals_enabled = false;
    load(value);
    m_loaded_value = value;
    m_default_value = default_value;
    m_signals_enabled = true;
    m_prepared = true;
}

void settings_widget::mark_saved()
{
    set_dirty(false);
    m_loaded_value = value();
}

void settings_widget::set_preview_widget(preview_widget* widget)
{
    if (m_preview)
    {
        m_layout->removeWidget(m_preview);
    }
    m_preview = widget;
    m_layout->addWidget(m_preview);
    if (m_prepared)
    {
        m_preview->update(value());
    }
}