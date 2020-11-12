#include "gui/svg_icon_engine/svg_icon_engine.h"

#include <QPainter>
#include <QtSvg/QSvgRenderer>

namespace hal
{
    SvgIconEngine::SvgIconEngine(const std::string& svg_data)
    {
        mData = QByteArray::fromStdString(svg_data);
    }

    void SvgIconEngine::paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state)
    {
        Q_UNUSED(mode)
        Q_UNUSED(state)

        QSvgRenderer renderer(mData);
        renderer.render(painter, rect);
    }

    QIconEngine* SvgIconEngine::clone() const
    {
        return new SvgIconEngine(*this);
    }

    QPixmap SvgIconEngine::pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state)
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
