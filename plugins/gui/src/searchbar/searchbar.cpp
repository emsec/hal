#include "gui/searchbar/searchbar.h"

#include "gui/gui_utils/graphics.h"
#include "gui/label_button/label_button.h"
#include "gui/searchbar/searchoptions_dialog.h"

#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QStyle>
#include <QToolButton>
#include <QDebug>
#include <gui/searchbar/searchoptions.h>

namespace hal
{
    Searchbar::Searchbar(QWidget* parent)
        : QFrame(parent), mLayout(new QHBoxLayout()), mSearchIconLabel(new QLabel()), mLineEdit(new QLineEdit()), mClearIconLabel(new QLabel()), mDownButton(new QToolButton()),
          mUpButton(new QToolButton()), mSearchOptionsButton(new QToolButton()), mClearButton(new QToolButton())
    {
        setLayout(mLayout);

        mLayout->setContentsMargins(0, 0, 0, 0);
        mLayout->setSpacing(0);

        ensurePolished();

        mCurrentOptions = new SearchOptions(8);//exact match and search in all columns is on
        mIncrementalSearch = true;
        mSearchIconLabel->setPixmap(gui_utility::getStyledSvgIcon(mSearchIconStyle, mSearchIcon).pixmap(QSize(16, 16)));
        mSearchIconLabel->installEventFilter(this);
        mLineEdit->setPlaceholderText("Search");

        mClearIconLabel->setPixmap(gui_utility::getStyledSvgIcon(mClearIconStyle, mClearIcon).pixmap(QSize(10, 10)));

        //Placeholder icons get better ones
        mDownButton->setIcon(QIcon(":/icons/arrow-down"));
        mUpButton->setIcon(QIcon(":/icons/arrow-up"));
        mSearchOptionsButton->setIcon(gui_utility::getStyledSvgIcon(mOptionDialogIconStyle,mOptionDialogIcon));

        mSearchIconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
        mLineEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        mLayout->addWidget(mSearchIconLabel);
        mLayout->addWidget(mLineEdit);

        mClearButton->setIcon(gui_utility::getStyledSvgIcon(mClearIconStyle, mClearIcon));
        mClearButton->setIconSize(QSize(10, 10));
        mClearButton->setFixedWidth(32);
        mClearButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
        mClearButton->setToolTip("Clear");
        mLayout->addWidget(mClearButton);

        mSearchOptionsButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
        mSearchOptionsButton->setCheckable(true);
        mSearchOptionsButton->setToolTip("Search Options");
        mLayout->addWidget(mSearchOptionsButton);

        setFrameStyle(QFrame::NoFrame);

        connect(mLineEdit, &QLineEdit::textEdited, this, &Searchbar::handleTextEdited);
        connect(mLineEdit, &QLineEdit::returnPressed, this, &Searchbar::handleReturnPressed);
        connect(mClearButton, &QToolButton::clicked, this, &Searchbar::handleClearClicked);

        connect(mSearchOptionsButton, &QToolButton::clicked, this, &Searchbar::handleSearchOptionsDialog);

        setFocusProxy(mLineEdit);
    }

    SearchOptions Searchbar::getSearchOptions() const
    {
        if (!mCurrentOptions) return SearchOptions();
        return *mCurrentOptions;
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

    QString Searchbar::optionDialogIcon() const
    {
        return mOptionDialogIcon;
    }

    QString Searchbar::optionDialogIconStyle() const
    {
        return mOptionDialogIconStyle;
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

    void Searchbar::setColumnNames(QList<QString> list)
    {
        mColumnNames = list;
    }

    void Searchbar::setOptionDialogIcon(const QString& icon)
    {
        mOptionDialogIcon = icon;
    }

    void Searchbar::setOptionDialogIconStyle(const QString& style)
    {
        mOptionDialogIconStyle = style;
    }

    void Searchbar::setPlaceholderText(const QString& text)
    {
        mLineEdit->setPlaceholderText(text);
    }

    void Searchbar::clear()
    {
        mLineEdit->clear();
        handleTextEdited(); // TODO : check use cases . Must reset proxy filter when search string emptied.
    }

    void Searchbar::hideEvent(QHideEvent *)
    {
        Q_EMIT textEdited(QString());
    }

    void Searchbar::showEvent(QShowEvent *)
    {
        // ???
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

        textWithFlags.append(mCurrentOptions->isCaseSensitive() ? "(?-i)" : "(?i)");
        if (mCurrentOptions->isExactMatch())
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

    void Searchbar::handleTextEdited()
    {
        repolish();
        //if the line is empty then start a search with default searchOptions - no filtering
        if(mLineEdit->text().isEmpty()){
            Q_EMIT triggerNewSearch(mLineEdit->text(), SearchOptions().toInt());
        }
        else if(mIncrementalSearch && mLineEdit->text().length() >= mMinCharsToStartIncSearch)
        {
            Q_EMIT triggerNewSearch(mLineEdit->text(), mCurrentOptions->toInt());
            updateSearchHistory(mLineEdit->text());
        }
    }

    void Searchbar::handleReturnPressed()
    {
        //if the line is empty then start a search with default searchOptions - no filtering
        if(mLineEdit->text().isEmpty()){
            Q_EMIT triggerNewSearch(mLineEdit->text(), SearchOptions().toInt());
        }
        else{
            Q_EMIT triggerNewSearch(mLineEdit->text(), mCurrentOptions->toInt());
            updateSearchHistory(mLineEdit->text());
        }
    }

    void Searchbar::handleClearClicked()
    {
        clear();
    }

    void Searchbar::setEmitTextWithFlags(bool emitTextWithFlags)
    {
        mEmitTextWithFlags = emitTextWithFlags;
    }

    bool Searchbar::getEmitTextWithFlags()
    {
        return mEmitTextWithFlags;
    }

    bool Searchbar::eventFilter(QObject *object, QEvent *event)
    {
        if(object == mSearchIconLabel && event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent* ev = static_cast<QMouseEvent*>(event);
            if(ev->button() == Qt::MouseButton::LeftButton)
                Q_EMIT searchIconClicked();

            return true;
        }
        return QWidget::eventFilter(object, event);
    }

    bool Searchbar::filterApplied()
    {
        return !getCurrentText().isEmpty() || mCurrentOptions->isExactMatch() || mCurrentOptions->isCaseSensitive();
    }

    void Searchbar::handleSearchOptionsDialog()
    {
        //TODO discuss if previous options should be passed back to the dialog to build dialog from them.
        // otherwise the use has to enter the same options again
        SearchOptionsDialog sd;
        sd.setSearchHistory(mSearchHistory);
        sd.setOptions(mCurrentOptions, mLineEdit->text(),mColumnNames, mIncrementalSearch, mMinCharsToStartIncSearch);
        if (sd.exec() == QDialog::Accepted)
        {
            mCurrentOptions = sd.getOptions();

            QString txt = sd.getText();
            mIncrementalSearch = sd.getIncrementalSearch();
            mMinCharsToStartIncSearch = sd.getMinIncSearchValue();
            mLineEdit->setText(txt);

            updateSearchHistory(txt);


            qInfo() << "Searchbar starts search with: " << txt << " " << mCurrentOptions->toInt() << "  inc search: " << mIncrementalSearch << " " << mMinCharsToStartIncSearch;
            Q_EMIT triggerNewSearch(txt, mCurrentOptions->toInt());
            updateSearchHistory(txt);
        }
        if(mCurrentOptions->toInt() != SearchOptions().toInt())
            mSearchOptionsButton->setChecked(true);
        else
            mSearchOptionsButton->setChecked(false);
    }

    void Searchbar::updateSearchHistory(QString entry){
        if(entry.length() >= 3 && !mSearchHistory.contains(entry))
        {
            if(!mSearchHistory.empty() && entry.startsWith(mSearchHistory[0]))
                mSearchHistory[0] = entry;
            else
            {
                if (mSearchHistory.length() >= 10) mSearchHistory.removeLast();
                mSearchHistory.prepend(entry);
            }
        }
    }
}
