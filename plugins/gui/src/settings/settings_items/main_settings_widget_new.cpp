#include "gui/settings/main_settings_widget_new.h"
#include "gui/settings/settings_manager_new.h"

#include "gui/expanding_list/expanding_list_button.h"
#include "gui/expanding_list/expanding_list_widget.h"
#include "gui/gui_globals.h"
//#include "checkbox_setting.h"
//#include "dropdown_setting.h"
#include "gui/settings/settings_widgets/settings_widget_new.h"
//#include "spinbox_setting.h"
//#include "text_setting.h"
#include "gui/searchbar/searchbar.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QTextEdit>

namespace hal
{
    MainSettingsList::~MainSettingsList()
    {
        for (SettingsWidgetNew* widget : *this)
            delete widget;
    }

    void MainSettingsList::registerWidget(const QString& sectionName, SettingsWidgetNew* widget)
    {
        append(widget);
        mSectionMap[sectionName].append(widget);
    }

    QList<const SettingsItem*> MainSettingsList::getItems() const
    {
        QList<const SettingsItem*> retval;
        for (SettingsWidgetNew* widget : *this)
            retval.append(widget->settingsItem());
        return retval;
    }

    void MainSettingsList::unregisterWidget(SettingsWidgetNew* widget)
    {
        for (auto it = mSectionMap.begin(); it != mSectionMap.end(); ++it)
        {
            auto jt = it->begin();
            while (jt != it->end())
                if (*jt == widget)
                    jt = it->erase(jt);
                else
                    ++jt;
        }
        int n = removeAll(widget);
        if (n > 0)
        {
            widget->close();
            widget->deleteLater();
        }
    }

    QStringList MainSettingsList::emptySections() const
    {
        QStringList retval;
        for (auto it = mSectionMap.constBegin(); it != mSectionMap.constEnd(); ++it)
            if (it.value().isEmpty())
                retval.append(it.key());
        return retval;
    }

    MainSettingsWidgetNew::MainSettingsWidgetNew(QWidget* parent)
        : QWidget(parent), mLayout(new QHBoxLayout()), mExpandingListWidget(new ExpandingListWidget()), mVerticalLayout(new QVBoxLayout()), mScrollbar(new QScrollBar()),
          mScrollArea(new QScrollArea()), mContent(new QFrame()),
          mContentLayout(new QHBoxLayout()), mSettingsContainer(new QFrame()),
          mContainerLayout(new QVBoxLayout()), mButtonLayout(new QHBoxLayout()),
          mDescriptionText(new QTextEdit(this)), mRestoreDefaults(new QPushButton()),
          mCancel(new QPushButton()), mOk(new QPushButton())
    {
        mSearchbarContainer = new QFrame(this);
        mSearchbarLayout = new QHBoxLayout();
        mSearchbar = new Searchbar(mSearchbarContainer);


        setWindowModality(Qt::ApplicationModal);

        setLayout(mLayout);
        mLayout->setContentsMargins(0, 0, 0, 0);
        mLayout->setSpacing(0);
        mLayout->addWidget(mExpandingListWidget);
        mLayout->addLayout(mVerticalLayout);
        //mLayout->addWidget(mScrollbar);

        mVerticalLayout->setContentsMargins(0, 0, 0, 0);
        mVerticalLayout->setSpacing(0);
        mVerticalLayout->addWidget(mSearchbarContainer);

        mSearchbarContainer->setObjectName("Searchbar-container");
        mSearchbarContainer->setLayout(mSearchbarLayout);
        mSearchbarLayout->setContentsMargins(0, 0, 0, 0);
        mSearchbarLayout->setSpacing(0);
        mSearchbarLayout->addWidget(mSearchbar);
        mSearchbar->setModeButtonText("Settings");

        mScrollArea->setFrameStyle(QFrame::NoFrame);
        mVerticalLayout->addWidget(mScrollArea);

        mContent->setFrameStyle(QFrame::NoFrame);

        mContentLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        mContentLayout->setContentsMargins(0, 0, 0, 0);
        mContentLayout->setSpacing(0);
        mContent->setLayout(mContentLayout);

        mScrollArea->setWidget(mContent);
        mScrollArea->setWidgetResizable(true);

        mSettingsContainer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        mSettingsContainer->setFrameStyle(QFrame::NoFrame);

        mContainerLayout->setAlignment(Qt::AlignLeft);
        mContainerLayout->setContentsMargins(0, 0, 0, 0);
        mContainerLayout->setSpacing(0);
        mSettingsContainer->setLayout(mContainerLayout);

        mContentLayout->addWidget(mSettingsContainer);

        mButtonLayout->setContentsMargins(10, 10, 10, 10);
        mButtonLayout->setSpacing(20);
        mButtonLayout->setAlignment(Qt::AlignRight);

        mDescriptionText->setMaximumHeight(120);
        mVerticalLayout->addWidget(mDescriptionText);

        mVerticalLayout->addLayout(mButtonLayout);

        mRestoreDefaults->setText("Restore Defaults");
        mRestoreDefaults->setToolTip("Clear user preferences for this page");
        mCancel->setText("Cancel");
        mOk->setText("OK");
        mRestoreDefaults->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        mCancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        mOk->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        QWidget* spacer = new QWidget();
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        mButtonLayout->addWidget(mRestoreDefaults, Qt::AlignLeft);
        mButtonLayout->addWidget(spacer);
        mButtonLayout->addWidget(mCancel, Qt::AlignRight);
        mButtonLayout->addWidget(mOk, Qt::AlignRight);

        connect(mRestoreDefaults, &QPushButton::clicked, this, &MainSettingsWidgetNew::handleRestoreDefaultsClicked);
        connect(mCancel, &QPushButton::clicked, this, &MainSettingsWidgetNew::handleCancelClicked);
        connect(mOk, &QPushButton::clicked, this, &MainSettingsWidgetNew::handleOkClicked);

        connect(mExpandingListWidget, &ExpandingListWidget::buttonSelected, this, &MainSettingsWidgetNew::handleButtonSelected);
        connect(mSearchbar,&Searchbar::textEdited,this,&MainSettingsWidgetNew::searchSettings);
    }

    void MainSettingsWidgetNew::initWidgets()
    {
        QSet<const SettingsItem*> registeredItems = QSet<const SettingsItem*>::fromList(mSettingsList.getItems());
        QSet<QString> registeredCategories = QSet<QString>::fromList(mSectionNames.values());
        for (SettingsItem* si : SettingsManagerNew::instance()->mSettingsList)
        {
            if (registeredItems.contains(si)) continue;
            QString catg = si->category();
            if (catg.isEmpty()) catg = "Other";
            if (!registeredCategories.contains(catg))
            {
                makeSection(catg);
                registeredCategories.insert(catg);
            }

            SettingsWidgetNew* widget = si->editWidget();
            connect(widget,&SettingsWidgetNew::triggerDescriptionUpdate,this,&MainSettingsWidgetNew::handleDescriptionUpdate);
            connect(widget,&SettingsWidgetNew::triggerRemoveWidget,this,&MainSettingsWidgetNew::handleWidgetRemove);
            mSettingsList.registerWidget(catg,widget);
            registeredItems.insert(si);
            mContainerLayout->addWidget(widget);
        }

        // show only buttons with non-empty sections
        for (ExpandingListButton* btn : mSectionNames.keys())
            btn->show();
        for (QString sectionName : mSettingsList.emptySections() )
        {
            for (auto it = mSectionNames.begin(); it!= mSectionNames.end(); ++it)
                if (it.value() == sectionName)
                    it.key()->hide();
        }
    }

    void MainSettingsWidgetNew::makeSection(const QString& label)
    {
        int colon = label.indexOf(':');
        QString topLabel;
        QString subLabel;
        int level = 0;
        if (colon<0)
            subLabel = label;
        else
        {
            topLabel = label.left(colon);
            subLabel = label.mid(colon+1);
            if (!mExpandingListWidget->hasGroup(topLabel))
            {
                ExpandingListButton* topBtn = new ExpandingListButton(0);
                topBtn->setObjectName(topLabel);
                topBtn->setText(topLabel);
                topBtn->setDefaultIcon(topLabel);
                mExpandingListWidget->appendItem(topBtn);
                mSectionNames.insert(topBtn,topLabel);
            }
            level = 1;
        }
        ExpandingListButton* btn = new ExpandingListButton(level);
        btn->setObjectName(subLabel);
        btn->setText(subLabel);
        btn->setDefaultIcon(subLabel);
        //btn->setIconPath(iconPath);
        mExpandingListWidget->appendItem(btn,topLabel);
        mSectionNames.insert(btn, label);
    }

    void MainSettingsWidgetNew::handleWidgetRemove(SettingsWidgetNew* widget)
    {
        mContainerLayout->removeWidget(widget);
        mSettingsList.unregisterWidget(widget);
    }

    bool MainSettingsWidgetNew::handleAboutToClose()
    {
        bool dirty = false;
        for (SettingsWidgetNew* widget : mSettingsList)
        {
            if (widget->dirty())
            {
                dirty = true;
                break;
            }
        }
        if (dirty)
        {
            QMessageBox::StandardButton input = QMessageBox::question(this, "Unsaved settings", "You have unsaved settings that would be discarded.\nSave before leaving the settings page?", QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
            if (input == QMessageBox::Cancel)
                return false;
            if (input == QMessageBox::Yes)
                return saveSettings();
//            if (input == QMessageBox::No)
//                rollbackSettings();
        }
        return true;
    }

    void MainSettingsWidgetNew::handleRestoreDefaultsClicked()
    {
        QList<SettingsWidgetNew*> widgetList = mSettingsList.section(mActiveSection);

        for (SettingsWidgetNew* widget : widgetList)
        {
            widget->restoreDefault();
        }
    }

    void MainSettingsWidgetNew::handleCancelClicked()
    {
        Q_EMIT close();
    }

    void MainSettingsWidgetNew::handleOkClicked()
    {
        saveSettings();
        Q_EMIT close();
    }

    void MainSettingsWidgetNew::activate()
    {
        initWidgets();

        mExpandingListWidget->selectItem(0);
        mExpandingListWidget->repolish();
        for (SettingsWidgetNew* widget : mSettingsList)
        {
            if (widget)
                widget->loadCurrentValue();
        }

    }

    void MainSettingsWidgetNew::handleDescriptionUpdate(SettingsItem* activeSettingsItem)
    {
        if (!activeSettingsItem)
        {
            mDescriptionText->clear();
            return;
        }
        QString html = QString("<html><body><h3>%1</h3><p>%2</p></body></html>")
                .arg(activeSettingsItem->label()).arg(activeSettingsItem->description());
        mDescriptionText->setHtml(html);
    }

    void MainSettingsWidgetNew::handleButtonSelected(ExpandingListButton* button)
    {
        mResetToFirstElement = false;
        mResetToFirstElement = true;

        hideAllSettings();
//        removeAllHighlights();

        if (!button) return;

        QString sectionName  = mSectionNames.value(button);
        mActiveSection       = sectionName;
        QList<SettingsWidgetNew*> widgetList = mSettingsList.section(sectionName);

        for (SettingsWidgetNew* widget : widgetList)
        {
            if (widget)
                widget->show();
        }
    }

    void MainSettingsWidgetNew::hideAllSettings()
    {
        for (SettingsWidgetNew* widget : mSettingsList)
            widget->hide();
    }

    void MainSettingsWidgetNew::showAllSettings()
    {
        for (ExpandingListButton* but : mSectionNames.keys())
            but->setSelected(false);
        for (SettingsWidgetNew* widget : mSettingsList)
            widget->show();
    }

    void MainSettingsWidgetNew::searchSettings(const QString& needle)
    {
        for (ExpandingListButton* but : mSectionNames.keys())
            but->setSelected(false);
        for (SettingsWidgetNew* widget : mSettingsList)
            if (widget->matchLabel(needle))
                widget->show();
            else
                widget->hide();
    }

    bool MainSettingsWidgetNew::saveSettings()
    {
        bool changed = false;
        for (SettingsWidgetNew* widget : mSettingsList)
        {
            if (widget->dirty())
            {
                changed = true;
                widget->acceptValue();
            }
        }
        if (changed)
            SettingsManagerNew::instance()->persistUserSettings();
        return true;
    }
}
