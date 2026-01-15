#include "gui/searchbar/searchable_label.h"
#include "gui/searchbar/searchoptions.h"
#include <QPainter>
#include <QFontMetrics>
#include <QDebug>
#include <QRegularExpression>

namespace hal {

    SearchableLabel::SearchableLabel(const QString &txt, QWidget *parent)
        : QLabel(txt, parent)
    {;}

    SearchableLabel::SearchableLabel(QWidget *parent)
        : QLabel(parent)
    {;}

    void SearchableLabel::paintEvent(QPaintEvent* ev)
    {
        if (mMatchPositions.empty())
        {
            QLabel::paintEvent(ev);
            return;
        }
        QPainter painter(this);
        mMetrics = new QFontMetrics(font());
        int fw = mMetrics->horizontalAdvance(text());
        int fh = mMetrics->height();
        int rw = rect().width();
        int rh = rect().height();
        if (alignment() & Qt::AlignHCenter)
            xText = (rw-fw) / 2;
        else if (alignment() & Qt::AlignRight)
            xText = rw - fw;
        else
            xText = 0;
        if (alignment() & Qt::AlignVCenter)
            yText = (rh-fh) / 2;
        else if (alignment() & Qt::AlignBottom)
            yText = rh - fh;
        else
            yText = 0;
        if (autoFillBackground())
            painter.fillRect(rect(),palette().color(QPalette::Window));
        int p0 = 0;
        for (QPair<int,int> p1 : mMatchPositions)
        {
            drawSubstring(painter, p0, p1.first-p0, false);
            drawSubstring(painter, p1.first, p1.second, true);
            p0 = p1.first + p1.second;
        }
        if (p0 < text().size())
            drawSubstring(painter, p0, text().size()-p0, false);
        delete mMetrics;
    }

    void SearchableLabel::drawSubstring(QPainter& painter, int pos, int len, bool hilite)
    {
        if (len <= 0) return;

        QColor storeColor = painter.pen().color();
        QString sub = text().mid(pos,len);
        int fw = mMetrics->horizontalAdvance(sub);
        int fh = mMetrics->height();
        if (hilite)
        {
            painter.setPen(QColor(255, 252, 240));
            painter.fillRect(xText,yText-sHiliteOffset,fw,fh+2*sHiliteOffset,QBrush(QColor(33, 66, 133)));
        }
        painter.drawText(xText,yText,fw,fh,0,sub);
        xText += fw;
        if (hilite)
        {
            painter.setPen(storeColor);
        }
    }

    bool SearchableLabel::exactWordMatch(int pos, int len) const
    {
        if (pos && !text().at(pos-1).isSpace()) return false;
        if (pos + len < text().size() && !text().at(pos+len).isSpace()) return false;
        return true;
    }

    bool SearchableLabel::hasMatch() const
    {
        return !mMatchPositions.isEmpty();
    }

    void SearchableLabel::handleSearchChanged(const QString& string, int option)
    {
        mMatchPositions.clear();
        SearchOptions opts(option);
        Qt::CaseSensitivity cs = opts.isCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive;
        if (!string.isEmpty())
        {
            int pos = 0;
            if (opts.isRegularExpression())
            {
                // Regular expression search
                QRegularExpression regexp(opts.isExactMatch() ? "\\b" + string + "\\b" : string,
                                          opts.isCaseSensitive() ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);
                if (regexp.isValid())
                {
                    QRegularExpressionMatch rmatch;
                    while (pos >= 0)
                    {
                        pos = text().indexOf(regexp,pos,&rmatch);
                        if (pos >= 0)
                        {
                            mMatchPositions.append(qMakePair(pos,rmatch.captured().size()));
                            ++pos;
                        }
                    }
                }
            }
            else
            {
                // String search
                while (pos >= 0)
                {
                    pos = text().indexOf(string,pos,cs);
                    if (pos >= 0)
                    {
                        if (!opts.isExactMatch() || exactWordMatch(pos,string.size()))
                            mMatchPositions.append(qMakePair(pos,string.size()));
                        ++pos;
                    }
                }
            }
        }
        update();
    }
}
