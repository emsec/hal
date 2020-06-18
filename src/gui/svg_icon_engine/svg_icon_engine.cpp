#include "svg_icon_engine/svg_icon_engine.h"

#include <QPainter>
#include <QtSvg/QSvgRenderer>
namespace hal{
svg_icon_engine::svg_icon_engine(const std::string& svg_data)
{
    m_data = QByteArray::fromStdString(svg_data);
}

void svg_icon_engine::paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state)
{
    Q_UNUSED(mode)
    Q_UNUSED(state)

    QSvgRenderer renderer(m_data);
    renderer.render(painter, rect);
}

QIconEngine* svg_icon_engine::clone() const
{
    return new svg_icon_engine(*this);
}

QPixmap svg_icon_engine::pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state)
{
    QImage image(size, QImage::Format_ARGB32);
    image.fill(qRgba(0, 0, 0, 0));
    QPixmap pixmap = QPixmap::fromImage(image, Qt::NoFormatConversion);
    QPainter painter(&pixmap);
    QRect rect(QPoint(0.0, 0.0), size);
    this->paint(&painter, rect, mode, state);
    return pixmap;
}
}
