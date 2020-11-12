#include "gui/searchbar/searchbar.h"

#include "gui/gui_utils/graphics.h"
#include "gui/label_button/label_button.h"

#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QStyle>
#include <QToolButton>

namespace hal
{
    Searchbar::Searchbar(QWidget* parent)
        : QFrame(parent), mLayout(new QHBoxLayout()), mSearchIconLabel(new QLabel()), mLineEdit(new QLineEdit()), mClearIconLabel(new QLabel()), mModeButton(new QPushButton()),
          mDownButton(new QToolButton()), mUpButton(new QToolButton())
    {
        setLayout(mLayout);

        mLayout->setContentsMargins(0, 0, 0, 0);
        mLayout->setSpacing(0);

        ensurePolished();

        mSearchIconLabel->setPixmap(gui_utility::getStyledSvgIcon(mSearchIconStyle, mSearchIcon).pixmap(QSize(16, 16)));
        mLineEdit->setPlaceholderText("Search");
        mClearIconLabel->setPixmap(gui_utility::getStyledSvgIcon(mClearIconStyle, mClearIcon).pixmap(QSize(10, 10)));
        mModeButton->setText("Mode");

        //Placeholder icons get better ones
        mDownButton->setIcon(QIcon(":/icons/arrow-down"));
        mUpButton->setIcon(QIcon(":/icons/arrow-up"));

        mSearchIconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
        mLineEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        mModeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);

        mLayout->addWidget(mSearchIconLabel);
        mLayout->addWidget(mLineEdit);
        //mLayout->addWidget(mClearIconLabel);
        //mLayout.addWidget(mButton);

        //Hack solution, because QPushButton not working as expected, fix
        LabelButton* hack = new LabelButton("Mode", this);
        hack->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
        mLayout->addWidget(hack);

        //mLayout.addWidget(&mDownButton);
        //mLayout.addWidget(&mUpButton);

        setFrameStyle(QFrame::NoFrame);

        connect(mLineEdit, &QLineEdit::textEdited, this, &Searchbar::handleTextEdited);
        connect(mLineEdit, &QLineEdit::returnPressed, this, &Searchbar::handleReturnPressed);
        connect(hack, &LabelButton::clicked, this, &Searchbar::handleModeClicked);

        setFocusProxy(mLineEdit);
    }

    QString Searchbar::searchIcon() const
    {
        return mSearchIcon;
    }

    QString Searchbar::searchIconStyle() const
    {
        return mSearchIconStyle;
    }

    QString Searchbar::clearIcon() const
    {
        return mClearIcon;
    }

    QString Searchbar::clearIconStyle() const
    {
        return mClearIconStyle;
    }

    void Searchbar::setSearchIcon(const QString& icon)
    {
        mSearchIcon = icon;
    }

    void Searchbar::setSearchIconStyle(const QString& style)
    {
        mSearchIconStyle = style;
    }

    void Searchbar::setClearIcon(const QString& icon)
    {
        mClearIcon = icon;
    }

    void Searchbar::setClearIconStyle(const QString& style)
    {
        mClearIconStyle = style;
    }

    void Searchbar::setPlaceholderText(const QString& text)
    {
        mLineEdit->setPlaceholderText(text);
    }

    void Searchbar::setModeButtonText(const QString& text)
    {
        mModeButton->setText(text);
    }

    void Searchbar::clear()
    {
        mLineEdit->clear();
        repolish();

        Q_EMIT textEdited(mLineEdit->text());
    }

    QString Searchbar::getCurrentText()
    {
        return mLineEdit->text();
    }

    void Searchbar::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        s->unpolish(mSearchIconLabel);
        s->polish(mSearchIconLabel);

        s->unpolish(mLineEdit);
        s->polish(mLineEdit);

        s->unpolish(mModeButton);
        s->polish(mModeButton);
    }

    void Searchbar::handleTextEdited(const QString& text)
    {
        repolish();
        Q_EMIT textEdited(text);
    }

    void Searchbar::handleReturnPressed()
    {
        Q_EMIT returnPressed();
    }

    void Searchbar::handleModeClicked()
    {
        Q_EMIT modeClicked();
    }
}
