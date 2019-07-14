#ifndef STYLEABLE_WIDGET_H
#define STYLEABLE_WIDGET_H

#include <QWidget>

class styleable_widget : public QWidget
{
    Q_OBJECT

public:
    explicit styleable_widget(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event);
};

#endif // STYLEABLE_WIDGET_H
