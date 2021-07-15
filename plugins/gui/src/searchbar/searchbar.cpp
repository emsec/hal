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
        : QFrame(parent), mLayout(new QHBoxLayout()), mSearchIconLabel(new QLabel()), mLineEdit(new QLineEdit()), mClearIconLabel(new QLabel()), mDownButton(new QToolButton()),
          mUpButton(new QToolButton()), mCaseSensitiveButton(new QToolButton()), mExactMatchButton(new QToolButton()), mClearButton(new QToolButton())
    {
        setLayout(mLayout);

        mLayout->setContentsMargins(0, 0, 0, 0);
        mLayout->setSpacing(0);

        ensurePolished();

        mSearchIconLabel->setPixmap(gui_utility::getStyledSvgIcon(mSearchIconStyle, mSearchIcon).pixmap(QSize(16, 16)));
        mLineEdit->setPlaceholderText("Search");

        mClearIconLabel->setPixmap(gui_utility::getStyledSvgIcon(mClearIconStyle, mClearIcon).pixmap(QSize(10, 10)));

        //Placeholder icons get better ones
        mDownButton->setIcon(QIcon(":/icons/arrow-down"));
        mUpButton->setIcon(QIcon(":/icons/arrow-up"));

        mSearchIconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
        mLineEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        mLayout->addWidget(mSearchIconLabel);
        mLayout->addWidget(mLineEdit);

        mExactMatchButton->setText("==");
        mExactMatchButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
        mExactMatchButton->setCheckable(true);
        mExactMatchButton->setToolTip("Exact Match");
        mLayout->addWidget(mExactMatchButton);

        mCaseSensitiveButton->setText("Aa");
        mCaseSensitiveButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
        mCaseSensitiveButton->setCheckable(true);
        mCaseSensitiveButton->setToolTip("Case Sensitive");
        mLayout->addWidget(mCaseSensitiveButton);

        mClearButton->setIcon(gui_utility::getStyledSvgIcon(mClearIconStyle, mClearIcon));
        mClearButton->setIconSize(QSize(10, 10));
        mClearButton->setFixedWidth(32);
        mClearButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
        mClearButton->setToolTip("Clear");
        mLayout->addWidget(mClearButton);

        setFrameStyle(QFrame::NoFrame);

        connect(mLineEdit, &QLineEdit::textEdited, this, &Searchbar::emitTextEdited);
        connect(mLineEdit, &QLineEdit::returnPressed, this, &Searchbar::handleReturnPressed);
        connect(mCaseSensitiveButton, &QToolButton::clicked, this, &Searchbar::emitTextEdited);
        connect(mExactMatchButton, &QToolButton::clicked, this, &Searchbar::emitTextEdited);
        connect(mClearButton, &QToolButton::clicked, this, &Searchbar::handleClearClicked);

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

    QString Searchbar::caseSensitivityIcon() const
    {
        return mCaseSensitivityIcon;
    }

    QString Searchbar::caseSensitivityIconStyle() const
    {
        return mCaseSensitivityIconStyle;
    }

    QString Searchbar::exactMatchIcon() const
    {
        return mExactMatchIcon;
    }

    QString Searchbar::exactMatchIconStyle() const
    {
        return mExactMatchIconStyle;
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

    void Searchbar::setCaseSensitivityIcon(const QString& icon)
    {
        mCaseSensitivityIcon = icon;
    }

    void Searchbar::setCaseSensitivityIconStyle(const QString& style)
    {
        mCaseSensitivityIconStyle = style;
    }

    void Searchbar::setExactMatchIcon(const QString& icon)
    {
        mExactMatchIcon = icon;
    }

    void Searchbar::setExactMatchIconStyle(const QString& style)
    {
        mExactMatchIconStyle = style;
    }

    void Searchbar::setPlaceholderText(const QString& text)
    {
        mLineEdit->setPlaceholderText(text);
    }

    void Searchbar::clear()
    {
        mLineEdit->clear();
        emitTextEdited();
    }

    void Searchbar::hideEvent(QHideEvent *)
    {
        Q_EMIT textEdited(QString());
    }

    void Searchbar::showEvent(QShowEvent *)
    {
        emitTextEdited();
    }

    QString Searchbar::getCurrentText()
    {
        return mLineEdit->text();
    }

    QString Searchbar::getCurrentTextWithFlags()
    {
        return addFlags(mLineEdit->text());
    }

    QString Searchbar::addFlags(const QString& text)
    {
        QString textWithFlags;

        textWithFlags.append(mCaseSensitiveButton->isChecked() ? "(?-i)" : "(?i)");
        if (mExactMatchButton->isChecked())
        {
            textWithFlags.append("^");
            textWithFlags.append(text);
            textWithFlags.append("$");
        }
        else
            textWithFlags.append(text);

        return textWithFlags;
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
    }

    void Searchbar::emitTextEdited()
    {
        repolish();
        Q_EMIT textEdited(mEmitTextWithFlags ? getCurrentTextWithFlags() : getCurrentText());
    }

    void Searchbar::handleReturnPressed()
    {
        Q_EMIT returnPressed();
    }

    void Searchbar::handleClearClicked()
    {
        clear();
    }

    bool Searchbar::exactMatchChecked()
    {
        return mExactMatchButton->isChecked();
    }

    bool Searchbar::caseSensitiveChecked()
    {
        return mCaseSensitiveButton->isChecked();
    }

    void Searchbar::setEmitTextWithFlags(bool emitTextWithFlags)
    {
        mEmitTextWithFlags = emitTextWithFlags;
    }

    bool Searchbar::getEmitTextWithFlags()
    {
        return mEmitTextWithFlags;
    }

    bool Searchbar::filterApplied()
    {
        return !getCurrentText().isEmpty() || exactMatchChecked() || caseSensitiveChecked();
    }
}
