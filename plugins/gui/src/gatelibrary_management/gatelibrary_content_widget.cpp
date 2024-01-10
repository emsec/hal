#include "gui/gatelibrary_management/gatelibrary_content_widget.h"
#include "gui/gui_globals.h"
#include "gui/gatelibrary_management/gatelibrary_table_model.h"
#include "gui/toolbar/toolbar.h"
#include "gui/searchbar/searchbar.h"
#include "gui/gui_utils/graphics.h"

#include <QHeaderView>
#include <QVBoxLayout>
#include <QMenu>

namespace hal
{

    GatelibraryContentWidget::GatelibraryContentWidget(GatelibraryTableModel *model, QWidget* parent) : QFrame(parent)
    {
        QVBoxLayout* layout = new QVBoxLayout(this);
        setFrameStyle(QFrame::Panel | QFrame::Sunken);
        setLineWidth(2);
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        mSearchbar = new Searchbar(this);

        mTableView = new QTableView(this);
        mTableView->setModel(model);
        mTableView->setContextMenuPolicy(Qt::CustomContextMenu);
        mTableView->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        mTableView->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);
        mTableView->horizontalHeader()->setSectionResizeMode(1,QHeaderView::ResizeToContents);
     //   mTableView->horizontalHeader()->setStretchLastSection(true);
        mTableView->verticalHeader()->hide();

        mTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        mTableView->setSelectionMode(QAbstractItemView::SingleSelection);


        mToolbar = new Toolbar;
        mToolbar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        mToolbar->setIconSize(QSize(18, 18));

        mAddAction = new QAction("Add gate type", this);
        mEditAction = new QAction("Edit gate type", this);
        mSearchAction = new QAction("Search", this);

        //connections
        connect(mTableView, &QTableView::customContextMenuRequested, this, &GatelibraryContentWidget::handleContextMenuRequested);
        connect(mSearchAction, &QAction::triggered, this, &GatelibraryContentWidget::toggleSearchbar);
        connect(mEditAction, &QAction::triggered, this, &GatelibraryContentWidget::handleEditAction);
        //  TODO connect(mSearchbar, &Searchbar::triggerNewSearch, this, ...);

        mToolbar->addAction(mAddAction);
        mToolbar->addAction(mEditAction);
        mToolbar->addAction(mSearchAction);


        layout->addWidget(mToolbar);
        layout->addWidget(mTableView);
        layout->addWidget(mSearchbar);

        mSearchbar->hide();
        mSearchbar->setColumnNames({"Name", "ID"});
    }

    void GatelibraryContentWidget::handleContextMenuRequested(const QPoint& pos)
    {
        QModelIndex idx = mTableView->indexAt(pos);

        if(!idx.isValid())
            return;

        QMenu menu;

        menu.move(mapToGlobal(pos));
        menu.exec();
    }

    void GatelibraryContentWidget::handleEditAction()
    {
        QModelIndex inx = mTableView->currentIndex();
        if (inx.isValid())
            Q_EMIT triggerEditType(inx);
    }

    void GatelibraryContentWidget::toggleSearchbar()
    {
        if (!mSearchAction->isEnabled())
            return;

        if (mSearchbar->isHidden())
        {
            mSearchbar->show();
            mSearchbar->setFocus();
        }
        else
        {
            mSearchbar->hide();
            setFocus();
        }
    }

    void GatelibraryContentWidget::activate()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        mAddAction->setIcon(gui_utility::getStyledSvgIcon(mEnabledIconStyle,mAddTypeIconPath));
        mEditAction->setIcon(gui_utility::getStyledSvgIcon(mEnabledIconStyle,mEditTypeIconPath));
        mSearchAction->setIcon(gui_utility::getStyledSvgIcon(mEnabledIconStyle,mSearchIconPath));
    }

    QString GatelibraryContentWidget::disabledIconStyle() const
    {
        return mDisabledIconStyle;
    }

    QString GatelibraryContentWidget::enabledIconStyle() const
    {
        return mEnabledIconStyle;
    }

    QString GatelibraryContentWidget::addTypeIconPath() const
    {
        return mAddTypeIconPath;
    }

    QString GatelibraryContentWidget::editTypeIconPath() const
    {
        return mEditTypeIconPath;
    }

    QString GatelibraryContentWidget::searchIconPath() const
    {
        return mSearchIconPath;
    }

    QString GatelibraryContentWidget::searchIconStyle() const
    {
        return mSearchIconStyle;
    }

    QString GatelibraryContentWidget::searchActiveIconStyle() const
    {
        return mSearchActiveIconStyle;
    }

    void GatelibraryContentWidget::setDisabledIconStyle(const QString& s)
    {
        mDisabledIconStyle = s;
    }

    void GatelibraryContentWidget::setEnabledIconStyle(const QString& s)
    {
        mEnabledIconStyle = s;
    }

    void GatelibraryContentWidget::setAddTypeIconPath(const QString& s)
    {
        mAddTypeIconPath = s;
    }

    void GatelibraryContentWidget::setEditTypeIconPath(const QString& s)
    {
        mEditTypeIconPath = s;
    }

    void GatelibraryContentWidget::setSearchIconPath(const QString& s)
    {
        mSearchIconPath = s;
    }

    void GatelibraryContentWidget::setSearchIconStyle(const QString& s)
    {
        mSearchIconStyle = s;
    }

    void GatelibraryContentWidget::setSearchActiveIconStyle(const QString& s)
    {
        mSearchActiveIconStyle = s;
    }
}
