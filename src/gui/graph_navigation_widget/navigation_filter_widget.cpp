#include "graph_navigation_widget/navigation_filter_widget.h"
#include <QApplication>
#include <QPalette>

navigation_filter_widget::navigation_filter_widget(QWidget *parent) : QLineEdit(parent)
{
    connect(this, &QLineEdit::textEdited, this, &navigation_filter_widget::set_active);
}

void navigation_filter_widget::set_valid()
{
    QPalette pal = QApplication::palette();
    pal.setColor(QPalette::Base, QColor(114, 140, 0));
    pal.setColor(QPalette::Text, QColor(32, 36, 37));

    setPalette(pal);
}

void navigation_filter_widget::set_invalid()
{
    QPalette pal = QApplication::palette();
    pal.setColor(QPalette::Base, QColor(255, 0, 0));
    pal.setColor(QPalette::Text, QColor(32, 36, 37));

    setPalette(pal);
}

void navigation_filter_widget::set_active(const QString &text)
{
    Q_UNUSED(text)

    QPalette pal = QApplication::palette();
    pal.setColor(QPalette::Base, QColor(32, 36, 37));
    pal.setColor(QPalette::Text, QColor(228, 228, 228));

    setPalette(pal);
}
