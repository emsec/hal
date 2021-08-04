#include "gui/settings/settings_widgets/settings_widget.h"

#include <QBoxLayout>
#include <QLabel>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QStyle>
#include "gui/settings/settings_items/settings_item.h"

#include <QDebug>

namespace hal
{
    SettingsWidget::SettingsWidget(SettingsItem* item, QWidget* parent)
        : QFrame(parent), m_layout(new QVBoxLayout()),
          mContainer(new QBoxLayout(QBoxLayout::TopToBottom)), m_top_bar(new QHBoxLayout()),
          mNameLabel(new QLabel(this)), mRevertButton(new QToolButton()),
          mDefaultButton(new QToolButton()), mSettingsItem(item)
    {
        connect(mSettingsItem,&SettingsItem::destroyed,this,&SettingsWidget::handleItemDestroyed);
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
        connect(mRevertButton, &QToolButton::clicked, this, &SettingsWidget::handleRevertModification);
        mDefaultButton->setText("Default");
        mDefaultButton->setToolTip("Load the default value");
        mDefaultButton->setVisible(false);
        mDefaultButton->setEnabled(!mSettingsItem->isDefaultValue());
        QSizePolicy sp_default = mDefaultButton->sizePolicy();
        sp_default.setRetainSizeWhenHidden(true);
        mDefaultButton->setSizePolicy(sp_default);
        connect(mDefaultButton, &QToolButton::clicked, this, &SettingsWidget::handleSetDefaultValue);
        m_top_bar->addWidget(mNameLabel);
        m_top_bar->addStretch();
        m_top_bar->addWidget(mRevertButton);
        m_top_bar->addWidget(mDefaultButton);
        m_layout->addLayout(m_top_bar);
        m_layout->addLayout(mContainer);

        hide();
    }

    void SettingsWidget::reset_labels()
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

    bool SettingsWidget::matchLabel(const QString& needle)
    {
        if (!mSettingsItem) return false;
        return QRegularExpression(needle).match(mSettingsItem->label()).hasMatch();
    }

    void SettingsWidget::trigger_setting_updated()
    {

        if (!mSettingsItem) return;
        QVariant val = value();
        setDirty(mSettingsItem->value() != val);
        mDefaultButton->setEnabled(mSettingsItem->defaultValue() != val);
    }

    void SettingsWidget::handleSetDefaultValue(bool setAll)
    {
        Q_UNUSED(setAll);
        if (!mSettingsItem) return;
        load(mSettingsItem->defaultValue());
        trigger_setting_updated();
    }

    void SettingsWidget::handleRevertModification()
    {
        if (!mSettingsItem) return;
        load(mSettingsItem->value());
        trigger_setting_updated();
    }

    void SettingsWidget::handleItemDestroyed()
    {
        mSettingsItem = nullptr;
        Q_EMIT triggerRemoveWidget(this);
        close();
    }

    void SettingsWidget::setDirty(bool dirty)
    {
        mDirty = dirty;
        mRevertButton->setEnabled(dirty);

        QStyle* s = style();
        s->unpolish(this);
        s->polish(this);
    }

    bool SettingsWidget::dirty() const
    {
        return mDirty;
    }

    void SettingsWidget::prepare()
    {
        if (!mSettingsItem) return;
        load(mSettingsItem->value());
        setDirty(false);
        mDefaultButton->setEnabled(!mSettingsItem->isDefaultValue());
    }

    void SettingsWidget::enterEvent(QEvent* event)
    {
        Q_UNUSED(event);
        mRevertButton->setVisible(true);
        mDefaultButton->setVisible(true);
        Q_EMIT triggerDescriptionUpdate(mSettingsItem);
    }

    void SettingsWidget::leaveEvent(QEvent* event)
    {
        Q_UNUSED(event);
        mRevertButton->setVisible(false);
        mDefaultButton->setVisible(false);
        Q_EMIT triggerDescriptionUpdate(nullptr);
    }

    void SettingsWidget::acceptValue()
    {
        if (!mSettingsItem) return;
        mSettingsItem->setValue(value());
        setDirty(false);
    }

    void SettingsWidget::loadCurrentValue()
    {
        if (!mSettingsItem) return;
        load(mSettingsItem->value());
        trigger_setting_updated();
    }
}
