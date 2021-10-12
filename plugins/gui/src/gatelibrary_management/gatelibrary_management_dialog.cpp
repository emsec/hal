#include "gui/gatelibrary_management/gatelibrary_management_dialog.h"

#include "gui/gatelibrary_management/gatelibrary_table_model.h"
#include "gui/gatelibrary_management/gatelibrary_table_view.h"
#include "gui/gatelibrary_management/gatelibrary_proxy_model.h"

#include "gui/gui_utils/graphics.h"

#include "gui/searchbar/searchbar.h"

#include "hal_core/utilities/utils.h"
#include "hal_version.h"

#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QModelIndexList>

namespace hal
{
    GatelibraryManagementDialog::GatelibraryManagementDialog(QWidget* parent) : QDialog(parent)
    {
        setWindowTitle("Gate Library Management");
        setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

        mLayout = new QVBoxLayout(this);
        mSearchbar = new Searchbar(this);
        mTableModel = new GatelibraryTableModel(this);
        mProxyModel = new GatelibraryProxyModel(this);
        mTableModel->setup();
        mTableView = new GatelibraryTableView(this);
        mHLayout = new QHBoxLayout();
        mLoadButton = new QPushButton(" Load", this);
        mReloadButton = new QPushButton(" Reload", this);
        mRemoveButton = new QPushButton(" Remove", this);

        mProxyModel->setSourceModel(mTableModel);
        mProxyModel->setSortRole(Qt::UserRole);
        mProxyModel->setFilterKeyColumn(0);

        mTableView->setModel(mProxyModel);
        mTableView->setSortingEnabled(true);
        mTableView->sortByColumn(0, Qt::SortOrder::AscendingOrder);

        mLayout->addWidget(mSearchbar);
        mLayout->addWidget(mTableView);
        mLayout->addLayout(mHLayout);

        mHLayout->addWidget(mLoadButton);
        mHLayout->addStretch();
        mHLayout->addWidget(mReloadButton);
        mHLayout->addWidget(mRemoveButton);

        connect(mLoadButton, &QPushButton::clicked, this, &GatelibraryManagementDialog::handleLoadButtonClicked);
        connect(mReloadButton, &QPushButton::clicked, this, &GatelibraryManagementDialog::handleReloadButtonClicked);
        connect(mRemoveButton, &QPushButton::clicked, this, &GatelibraryManagementDialog::handleRemoveButtonClicked);

        connect(mTableView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &GatelibraryManagementDialog::handleCurrentRowChanged);

        connect(mSearchbar, &Searchbar::textEdited, this, &GatelibraryManagementDialog::filter);

        resize(1200, 600);

        mTableView->resizeColumnsToContents();
        mTableView->selectRow(0);

        ensurePolished();
        repolish();
    }

    void GatelibraryManagementDialog::repolish()
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        mSearchbar->repolish();

        s->unpolish(mLoadButton);
        s->polish(mLoadButton);

        s->unpolish(mReloadButton);
        s->polish(mReloadButton);

        s->unpolish(mRemoveButton);
        s->polish(mRemoveButton);

        if (!mLoadIconPath.isEmpty())
            mLoadButton->setIcon(gui_utility::getStyledSvgIcon(mLoadIconStyle, mLoadIconPath).pixmap(QSize(17, 17)));

        if (!mReloadIconPath.isEmpty())
            mReloadButton->setIcon(gui_utility::getStyledSvgIcon(mReloadIconStyle, mReloadIconPath).pixmap(QSize(17, 17)));

        if (!mRemoveIconPath.isEmpty())
            mRemoveButton->setIcon(gui_utility::getStyledSvgIcon(mRemoveIconStyle, mRemoveIconPath).pixmap(QSize(17, 17)));
    }

    QString GatelibraryManagementDialog::loadIconPath()
    {
        return mLoadIconPath;
    }

    QString GatelibraryManagementDialog::loadIconStyle()
    {
        return mLoadIconStyle;
    }

    QString GatelibraryManagementDialog::reloadIconPath()
    {
        return mReloadIconPath;
    }

    QString GatelibraryManagementDialog::reloadIconStyle()
    {
        return mReloadIconStyle;
    }

    QString GatelibraryManagementDialog::removeIconPath()
    {
        return mRemoveIconPath;
    }

    QString GatelibraryManagementDialog::removeIconStyle()
    {
        return mRemoveIconStyle;
    }

    void GatelibraryManagementDialog::setLoadIconPath(const QString& path)
    {
        mLoadIconPath = path;
    }

    void GatelibraryManagementDialog::setLoadIconStyle(const QString& style)
    {
        mLoadIconStyle = style;
    }

    void GatelibraryManagementDialog::setReloadIconPath(const QString& path)
    {
        mReloadIconPath = path;
    }

    void GatelibraryManagementDialog::setReloadIconStyle(const QString& style)
    {
        mReloadIconStyle = style;
    }

    void GatelibraryManagementDialog::setRemoveIconPath(const QString& path)
    {
        mRemoveIconPath = path;
    }

    void GatelibraryManagementDialog::setRemoveIconStyle(const QString& style)
    {
        mRemoveIconStyle = style;
    }

    void GatelibraryManagementDialog::handleLoadButtonClicked()
    {
        QString title = "Load gate library";
        QString text  = "Hal gate libraries (*.hgl)";

        // Non native dialogs does not work on macOS. Therefore do net set DontUseNativeDialog!
        QString path = QDir::currentPath();

        //if (gGuiState->contains("FileDialog/Path/MainWindow"))
        //    path = gGuiState->value("FileDialog/Path/MainWindow").toString();

        QString fileName = QFileDialog::getOpenFileName(nullptr, title, path, text, nullptr);

        if (!fileName.isNull())
        {
            mTableModel->loadFile(fileName);
        }
    }

    void GatelibraryManagementDialog::handleReloadButtonClicked()
    {
        if (mTableView->selectionModel()->hasSelection())
        {
            QModelIndex selected = mTableView->selectionModel()->selectedRows().first();
            mTableModel->reloadIndex(mProxyModel->mapToSource(selected));
        }
    }

    void GatelibraryManagementDialog::handleRemoveButtonClicked()
    {
        if (mTableView->selectionModel()->hasSelection())
        {
            QModelIndex selected = mTableView->selectionModel()->selectedRows().first();
            mTableModel->removeIndex(mProxyModel->mapToSource(selected));
        }
    }

    void GatelibraryManagementDialog::handleCurrentRowChanged(const QModelIndex& current, const QModelIndex& previous)
    {
        /*
        // IF NOTHING SELECTED
        {
            int rows = mTableModel->rowCount();

            if (previous.row() < rows)
                mTableView->selectRow(previous.row());
            else if (rows > 0)
                mTableView->selectRow(rows - 1);
        }
        */
    }

    void GatelibraryManagementDialog::filter(const QString& text)
    {

        QRegularExpression regex(text);

        if (regex.isValid())
        {
            mProxyModel->setFilterRegularExpression(regex);
        }
    }
}
