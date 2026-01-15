#include "gui/expanding_list/expanding_list_button.h"

#include "gui/gui_utils/graphics.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QStyle>

namespace hal
{
    QHash<QString,QString> ExpandingListButton::sIconMap = ExpandingListButton::defaultIconMap();

    ExpandingListButton::ExpandingListButton(int levl, QWidget* parent)
        : QFrame(parent),
          mLayout(new QHBoxLayout(this)),
          mLeftBorder(new QFrame(this)),
          mIconLabel(new QLabel(this)),
          mTextLabel(new QLabel(this)),
          mRightBorder(new QFrame(this)),
          mLevel(levl), mHover(false), mSelected(false)
    {
        mLayout->setContentsMargins(0, 0, 0, 0);
        mLayout->setSpacing(0);

        mLeftBorder->setObjectName("left-border");
        mLeftBorder->setFrameStyle(QFrame::NoFrame);
        mLayout->addWidget(mLeftBorder);

        mIconLabel->setObjectName("icon-label");
        mIconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
        mLayout->addWidget(mIconLabel);

        mTextLabel->setObjectName("text-label");
        mLayout->addWidget(mTextLabel);

        mRightBorder->setObjectName("right-border");
        mRightBorder->setFrameStyle(QFrame::NoFrame);
        mLayout->addWidget(mRightBorder);
    }

    QHash<QString,QString> ExpandingListButton::defaultIconMap()
    {
        const char* iconList[] = {"style", "eye",
                                  "graph", "graph",
                                  "navig", "graph",
                                  "pytho", "python",
                                  "selec", "gen-window",
                                  "appea", "preferences",
                                  "exper", "preferences",
                                  "keybi", "keyboard",
                                  "autos", "save",
                                  "debug", "bug",
                                  nullptr, nullptr};
        QHash<QString,QString> retval;
        int i = 0;
        const char* key;
        while ( (key=iconList[i++]) )
            retval[key] = QString(":/icons/%1").arg(iconList[i++]);
        return retval;
    }

    void ExpandingListButton::enterEvent(QEnterEvent *event)
    {
        Q_UNUSED(event)

        mHover = true;
        repolish();
    }

    void ExpandingListButton::leaveEvent(QEvent* event)
    {
        Q_UNUSED(event)

        mHover = false;
        repolish();
    }

    void ExpandingListButton::mousePressEvent(QMouseEvent* event)
    {
        Q_UNUSED(event)

        Q_EMIT clicked();
        event->accept();
    }

    QString ExpandingListButton::text() const
    {
        return mTextLabel->text();
    }

    int ExpandingListButton::level() const
    {
        return mLevel;
    }

    bool ExpandingListButton::hover() const
    {
        return mHover;
    }

    bool ExpandingListButton::selected() const
    {
        return mSelected;
    }

    /*
    QString ExpandingListButton::type()
    {
        return mType;
    }
*/

    QString ExpandingListButton::iconStyle() const
    {
        return mIconStyle;
    }

    void ExpandingListButton::setSelected(bool selected)
    {
        if (mSelected == selected)
            return;

        mSelected = selected;
        repolish();
    }

    /*
    void ExpandingListButton::set_type(const QString& type)
    {
        if (mType == type)
            return;

        mType = type;
        repolish();
    }
*/

    void ExpandingListButton::setIconStyle(const QString& style)
    {
        if (mIconStyle == style)
            return;

        mIconStyle = style;
        repolish();
    }

    void ExpandingListButton::setIconPath(const QString& path)
    {
        if (mIconPath == path)
            return;

        mIconPath = path;
        repolish();
    }

    void ExpandingListButton::setDefaultIcon(const QString& text)
    {
        QString iconPath = sIconMap.value(text.left(5).toLower());
        if (iconPath.isEmpty()) iconPath = ":/icons/python";
        setIconPath(iconPath);
    }

    void ExpandingListButton::setText(const QString& text)
    {
        mTextLabel->setText(text);
    }

    void ExpandingListButton::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        s->unpolish(mLeftBorder);
        s->polish(mLeftBorder);

        s->unpolish(mIconLabel);
        s->polish(mIconLabel);

        s->unpolish(mTextLabel);
        s->polish(mTextLabel);

        s->unpolish(mRightBorder);
        s->polish(mRightBorder);

        if (!mIconPath.isEmpty())
            mIconLabel->setPixmap(gui_utility::getStyledSvgIcon(mIconStyle, mIconPath).pixmap(QSize(20, 20)));
    }
}
