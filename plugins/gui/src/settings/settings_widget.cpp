#include "gui/settings/settings_widget.h"

#include <QBoxLayout>
#include <QLabel>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QStyle>

// enable this to apply all settings as they are modified
//#define SETTINGS_UPDATE_IMMEDIATELY
namespace hal
{
    SettingsWidget::SettingsWidget(const QString& key, QWidget* parent)
        : QFrame(parent), mLayout(new QVBoxLayout()), mContainer(new QBoxLayout(QBoxLayout::TopToBottom)), mTopBar(new QHBoxLayout()), mName(new QLabel()), mRevert(new QToolButton()),
          mDefault(new QToolButton()), mHighlightColor(52, 56, 57), mKey(key)
    {
        setFrameStyle(QFrame::NoFrame);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        mLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        mLayout->setContentsMargins(0, 0, 0, 0);
        mLayout->setSpacing(0);

        mName->setObjectName("name-label");

        setLayout(mLayout);

        mRevert->setText("Undo");
        mRevert->setToolTip("Revert your last change");
        //mRevert->setMaximumWidth(20);
        mRevert->setVisible(false);
        mRevert->setEnabled(false);
        QSizePolicy sp_revert = mRevert->sizePolicy();
        sp_revert.setRetainSizeWhenHidden(true);
        mRevert->setSizePolicy(sp_revert);
        connect(mRevert, &QToolButton::clicked, this, &SettingsWidget::handleRollback);
        mDefault->setText("Default");
        mDefault->setToolTip("Load the default value");
        //mDefault->setMaximumWidth(20);
        mDefault->setVisible(false);
        mDefault->setEnabled(false);
        QSizePolicy sp_default = mDefault->sizePolicy();
        sp_default.setRetainSizeWhenHidden(true);
        mDefault->setSizePolicy(sp_default);
        connect(mDefault, &QToolButton::clicked, this, &SettingsWidget::handleReset);
        mTopBar->addWidget(mName);
        mTopBar->addStretch();
        mTopBar->addWidget(mRevert);
        mTopBar->addWidget(mDefault);
        mLayout->addLayout(mTopBar);
        mLayout->addLayout(mContainer);

        hide();
    }

    QColor SettingsWidget::highlightColor()
    {
        return mHighlightColor;
    }

    QString SettingsWidget::key()
    {
        return mKey;
    }

    void SettingsWidget::setHighlightColor(const QColor& color)
    {
        mHighlightColor = color;
    }

    void SettingsWidget::resetLabels()
    {
        for (QPair<QLabel*, QString>& pair : mLabels)
        {
            pair.first->setText(pair.second);
            if (pair.second.isEmpty())
            {
                pair.first->hide();
            }
            else
            {
                pair.first->show();
            }
        }
    }

    bool SettingsWidget::matchLabels(const QString& string)
    {
        bool match_found = false;

        if (!string.isEmpty())
        {
            QString color        = mHighlightColor.name();
            QString opening_html = "<span style=\"background-color:" + color + "\">";
            QString closing_html = "</span>";
            int string_length    = string.length();
            int html_lenght      = opening_html.length() + string_length + closing_html.length();

            for (QPair<QLabel*, QString>& pair : mLabels)
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

    void SettingsWidget::triggerSettingUpdated()
    {
        QVariant val = value();
        if (m_preview)
        {
            m_preview->update(val);
        }
        if (mSignalsEnabled)
        {
            Q_EMIT settingUpdated(this, key(), val);
        }
        #ifndef SETTINGS_UPDATE_IMMEDIATELY
        setDirty(mLoadedValue != val);
        #endif
        mDefault->setEnabled(mDefaultValue != val);
    }

    void SettingsWidget::handleReset()
    {
        if (mPrepared)
        {
            load(mDefaultValue);
            triggerSettingUpdated();
        }
    }

    void SettingsWidget::handleRollback()
    {
        if (mPrepared)
        {
            load(mLoadedValue);
            triggerSettingUpdated();
        }
    }

    void SettingsWidget::setDirty(bool dirty)
    {
        mDirty   = dirty;

        mRevert->setEnabled(dirty);

        QStyle* s = style();
        s->unpolish(this);
        s->polish(this);
    }

    bool SettingsWidget::dirty() const
    {
        return mDirty;
    }

    void SettingsWidget::prepare(const QVariant& value, const QVariant& default_value)
    {
        mSignalsEnabled = false;
        load(value);
        mLoadedValue    = value;
        mDefaultValue   = default_value;
        mSignalsEnabled = true;
        mPrepared        = true;
        setDirty(false);
        mDefault->setEnabled(mDefaultValue != mLoadedValue);
    }

    void SettingsWidget::markSaved()
    {
        setDirty(false);
        mLoadedValue = value();
    }

    void SettingsWidget::setConflicts(bool conflicts)
    {
        mConflicts = conflicts;
        QStyle* s   = style();
        s->unpolish(this);
        s->polish(this);
    }

    bool SettingsWidget::conflicts() const
    {
        return mConflicts;
    }

    void SettingsWidget::setPreviewWidget(PreviewWidget* widget)
    {
        if (m_preview)
        {
            mContainer->removeWidget(m_preview);
        }
        m_preview = widget;
        mContainer->addWidget(m_preview);
        if (mPrepared)
        {
            m_preview->update(value());
        }
    }

    void SettingsWidget::setPreviewPosition(preview_position position)
    {
        QBoxLayout::Direction direction;
        switch (position)
        {
            case preview_position::bottom:
                direction = QBoxLayout::TopToBottom;
                break;
            case preview_position::right:
                direction = QBoxLayout::LeftToRight;
                break;
            default:
                return;
        }
        mContainer->setDirection(direction);
    }

    void SettingsWidget::enterEvent(QEvent* event)
    {
        Q_UNUSED(event);
        mRevert->setVisible(true);
        mDefault->setVisible(true);
    }

    void SettingsWidget::leaveEvent(QEvent* event)
    {
        Q_UNUSED(event);
        mRevert->setVisible(false);
        mDefault->setVisible(false);
    }
}
