#include "gui/settings/settings_widgets/settings_widget_new.h"

#include <QBoxLayout>
#include <QLabel>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QStyle>
#include "gui/settings/settings_items/settings_item.h"

#include <QDebug>

namespace hal
{
    SettingsWidgetNew::SettingsWidgetNew(SettingsItem* item, QWidget* parent)
        : QFrame(parent), m_layout(new QVBoxLayout()),
          mContainer(new QBoxLayout(QBoxLayout::TopToBottom)), m_top_bar(new QHBoxLayout()),
          mNameLabel(new QLabel(this)), mRevertButton(new QToolButton()),
          mDefaultButton(new QToolButton()), mSettingsItem(item),
          mHighlightColor(52, 56, 57)
    {
        connect(mSettingsItem,&SettingsItem::destroyed,this,&SettingsWidgetNew::handleItemDestroyed);
        setFrameStyle(QFrame::NoFrame);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        m_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        m_layout->setContentsMargins(0, 0, 0, 0);
        m_layout->setSpacing(0);

        mNameLabel->setText(item->label());
        mNameLabel->setObjectName("name-label");

        setLayout(m_layout);

        mRevertButton->setText("Undo");
        mRevertButton->setToolTip("Revert your last change");
        mRevertButton->setVisible(false);
        mRevertButton->setEnabled(false);
        QSizePolicy sp_revert = mRevertButton->sizePolicy();
        sp_revert.setRetainSizeWhenHidden(true);
        mRevertButton->setSizePolicy(sp_revert);
        connect(mRevertButton, &QToolButton::clicked, this, &SettingsWidgetNew::handleRevertModification);
        mDefaultButton->setText("Default");
        mDefaultButton->setToolTip("Load the default value");
        mDefaultButton->setVisible(false);
        mDefaultButton->setEnabled(!mSettingsItem->isDefaultValue());
        QSizePolicy sp_default = mDefaultButton->sizePolicy();
        sp_default.setRetainSizeWhenHidden(true);
        mDefaultButton->setSizePolicy(sp_default);
        connect(mDefaultButton, &QToolButton::clicked, this, &SettingsWidgetNew::handleSetDefaultValue);
        m_top_bar->addWidget(mNameLabel);
        m_top_bar->addStretch();
        m_top_bar->addWidget(mRevertButton);
        m_top_bar->addWidget(mDefaultButton);
        m_layout->addLayout(m_top_bar);
        m_layout->addLayout(mContainer);

        hide();
    }

    QColor SettingsWidgetNew::highlightColor()
    {
        return mHighlightColor;
    }

    void SettingsWidgetNew::setHighlightColor(const QColor& color)
    {
        mHighlightColor = color;
    }

    void SettingsWidgetNew::reset_labels()
    {
        for (QPair<QLabel*, QString>& pair : m_labels)
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

    bool SettingsWidgetNew::matchLabel(const QString& needle)
    {
        if (!mSettingsItem) return false;
        return mSettingsItem->label().indexOf(needle,0,Qt::CaseInsensitive) >= 0;
    }

    void SettingsWidgetNew::trigger_setting_updated()
    {

        if (!mSettingsItem) return;
        QVariant val = value();
        setDirty(mSettingsItem->value() != val);
        mDefaultButton->setEnabled(mSettingsItem->defaultValue() != val);
    }

    void SettingsWidgetNew::handleSetDefaultValue()
    {
        if (!mSettingsItem) return;
        load(mSettingsItem->defaultValue());
        trigger_setting_updated();
    }

    void SettingsWidgetNew::handleRevertModification()
    {
        if (!mSettingsItem) return;
        load(mSettingsItem->value());
        trigger_setting_updated();
    }

    void SettingsWidgetNew::handleItemDestroyed()
    {
        mSettingsItem = nullptr;
        Q_EMIT triggerRemoveWidget(this);
        close();
    }

    void SettingsWidgetNew::setDirty(bool dirty)
    {
        mDirty = dirty;
        mRevertButton->setEnabled(dirty);

        QStyle* s = style();
        s->unpolish(this);
        s->polish(this);
    }

    bool SettingsWidgetNew::dirty() const
    {
        return mDirty;
    }

    void SettingsWidgetNew::prepare()
    {
        if (!mSettingsItem) return;
        load(mSettingsItem->value());
        setDirty(false);
        mDefaultButton->setEnabled(!mSettingsItem->isDefaultValue());
    }

    void SettingsWidgetNew::enterEvent(QEvent* event)
    {
        Q_UNUSED(event);
        mRevertButton->setVisible(true);
        mDefaultButton->setVisible(true);
        Q_EMIT triggerDescriptionUpdate(mSettingsItem);
    }

    void SettingsWidgetNew::leaveEvent(QEvent* event)
    {
        Q_UNUSED(event);
        mRevertButton->setVisible(false);
        mDefaultButton->setVisible(false);
        Q_EMIT triggerDescriptionUpdate(nullptr);
    }

    void SettingsWidgetNew::acceptValue()
    {
        if (!mSettingsItem) return;
        mSettingsItem->setValue(value());
        setDirty(false);
    }

    void SettingsWidgetNew::restoreDefault()
    {
        if (!mSettingsItem) return;
        load(mSettingsItem->defaultValue());
    }

    void SettingsWidgetNew::loadCurrentValue()
    {
        if (!mSettingsItem) return;
        load(mSettingsItem->value());
    }
}
