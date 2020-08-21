#include "gui/gui_globals.h"
#include "gui/selection_details_widget/details_section_widget.h"
#include <QVBoxLayout>
#include <QHeaderView>

namespace hal {

    DetailsSectionWidget::DetailsSectionWidget(QWidget* content, const QString &txt, QWidget *parent)
        : QWidget(parent), m_body(content), m_table(nullptr)
    {
        constructor(txt);
        m_layout->addWidget(m_body);
    }

    DetailsSectionWidget::DetailsSectionWidget(const QString &txt, QTableWidget* tab, QWidget *parent)
        : QWidget(parent), m_body(tab), m_table(tab)
    {
        Q_ASSERT (tab != nullptr);

        constructor(txt);

        QHBoxLayout *hlayout = new QHBoxLayout;
        hlayout->setContentsMargins(3,3,0,0);
        hlayout->setSpacing(10);
        hlayout->addWidget(tab);
        hlayout->addSpacerItem(new QSpacerItem(0,0, QSizePolicy::Expanding, QSizePolicy::Fixed));
        m_layout->addLayout(hlayout);
        m_layout->addSpacerItem(new QSpacerItem(0,7, QSizePolicy::Expanding, QSizePolicy::Fixed));

        setDefaultTableStyle(m_table);
    }

    void DetailsSectionWidget::constructor(const QString &txt)
    {
        Q_ASSERT (m_body != nullptr);

        m_rows        = 0;
        m_bodyVisible = true;
        m_headerText  = txt;
        m_hideEmpty   = g_settings_manager->get("selection_details/hide_empty_sections", false).toBool();
        m_layout      = new QVBoxLayout(this);

        // remove placeholder text from initial view
        m_header      = new QPushButton(txt.endsWith(" (%1)") ? txt.mid(0,txt.size()-5) : txt,this);
        m_layout->setMargin(0);
        m_layout->addWidget(m_header);

        connect(m_header,&QPushButton::clicked,this,&DetailsSectionWidget::toggleBodyVisible);
        connect(g_settings_relay, &SettingsRelay::setting_changed, this, &DetailsSectionWidget::handleGlobalSettingsChanged);
    }

    QTableWidget* DetailsSectionWidget::table() const
    {
        return m_table;
    }

    void DetailsSectionWidget::hideEmpty()
    {
        if (m_hideEmpty && m_rows == 0)
            hide();
        else
        {
            show();
            bodyVisible();
        }
    }

    void DetailsSectionWidget::handleGlobalSettingsChanged(void* sender, const QString& key, const QVariant& value)
    {
        Q_UNUSED(sender)
        if(key == "selection_details/hide_empty_sections")
        {
            m_hideEmpty = value.toBool();
            hideEmpty();
        }
    }

    void DetailsSectionWidget::setRowCount(int rc)
    {
        m_bodyVisible = true; // show body after each update
        m_rows = rc;
        if (m_headerText.contains("%1"))
            m_header->setText(QString(m_headerText).arg(m_rows));
        hideEmpty();
    }

    void DetailsSectionWidget::toggleBodyVisible()
    {
        m_bodyVisible = ! m_bodyVisible;
        bodyVisible();
    }

    void DetailsSectionWidget::bodyVisible()
    {
        if (m_bodyVisible)
            m_body->show();
        else
            m_body->hide();
    }

    void DetailsSectionWidget::setDefaultTableStyle(QTableWidget* tab)
    {
        //tab->horizontalHeader()->setStretchLastSection(true);
        tab->horizontalHeader()->hide();
        tab->verticalHeader()->hide();
        tab->verticalHeader()->setDefaultSectionSize(16);
        tab->resizeColumnToContents(0);
        tab->setShowGrid(false);
        tab->setFocusPolicy(Qt::NoFocus);
        tab->setFrameStyle(QFrame::NoFrame);
        tab->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        tab->setMaximumHeight(tab->verticalHeader()->length());
        tab->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        tab->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        tab->setContextMenuPolicy(Qt::CustomContextMenu);
    }

}
