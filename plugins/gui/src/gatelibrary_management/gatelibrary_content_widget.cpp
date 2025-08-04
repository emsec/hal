#include "gui/gatelibrary_management/gatelibrary_content_widget.h"
#include "gui/gui_globals.h"
#include "gui/gatelibrary_management/gatelibrary_table_model.h"
#include "gui/plugin_relay/gui_plugin_manager.h"
#include "gui/toolbar/toolbar.h"
#include "gui/searchbar/searchbar.h"
#include "gui/gui_utils/graphics.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"

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
        mPinProxyModel = new PinProxyModel(this);
        mPinProxyModel->setSourceModel(model);
        mTableView->setModel(mPinProxyModel);
        mTableView->setContextMenuPolicy(Qt::CustomContextMenu);
        mTableView->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        mTableView->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);
        mTableView->horizontalHeader()->setSectionResizeMode(1,QHeaderView::ResizeToContents);
     //   mTableView->horizontalHeader()->setStretchLastSection(true);
        mTableView->verticalHeader()->hide();

        mTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        mTableView->setSelectionMode(QAbstractItemView::SingleSelection);

        mTableView->setSortingEnabled(true);
        mTableView->sortByColumn(0, Qt::AscendingOrder);


        mToolbar = new Toolbar;
        mToolbar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        mToolbar->setIconSize(QSize(18, 18));

        mAddAction = new QAction("Add gate type", this);
        mEditAction = new QAction("Edit gate type", this);
        mDeleteAction = new QAction("Delete gate type", this);
        mSearchAction = new QAction("Search", this);

        //connections
        connect(mTableView, &QTableView::customContextMenuRequested, this, &GatelibraryContentWidget::handleContextMenuRequested);
        connect(mTableView, &QTableView::doubleClicked, this, &GatelibraryContentWidget::handleDoubleClicked);
        connect(mTableView->selectionModel(), &QItemSelectionModel::currentChanged, this, &GatelibraryContentWidget::handleCurrentSelectionChanged);
        connect(mSearchAction, &QAction::triggered, this, &GatelibraryContentWidget::toggleSearchbar);
        connect(mEditAction, &QAction::triggered, this, &GatelibraryContentWidget::handleEditAction);
        connect(mDeleteAction, &QAction::triggered, this, &GatelibraryContentWidget::handleDeleteAction);

        connect(mDeleteAction, &QAction::triggered, this, &GatelibraryContentWidget::handleUnsavedChanges);

        connect(mSearchbar, &Searchbar::triggerNewSearch, mPinProxyModel, &SearchProxyModel::startSearch);

        mToolbar->addAction(mAddAction);
        mToolbar->addAction(mEditAction);
        mToolbar->addAction(mDeleteAction);
        mToolbar->addAction(mSearchAction);


        layout->addWidget(mToolbar);
        layout->addWidget(mTableView);
        layout->addWidget(mSearchbar);

        mSearchbar->hide();
        mSearchbar->setColumnNames({"Name", "ID"});
    }

    bool GatelibraryContentWidget::initialize(GateLibrary* gateLibrary, bool readOnly)
    {
        if(!gateLibrary)
        {
            if(gNetlist && gNetlist->get_gate_library()){
                // readonly flag makes sure mGateLibrary does not get modified, const attribute not needed
                mGateLibrary = const_cast<GateLibrary*>(gNetlist->get_gate_library());
                mReadOnly = true;
            }
            else
            {
                QString title = "Load gate library";
                QString text  = "HAL Gate Library (*.hgl *.lib)";
                QString path  = QDir::currentPath();
                QFile gldpath(":/path/gate_library_definitions");
                if (gldpath.open(QIODevice::ReadOnly))
                    path = QString::fromUtf8(gldpath.readAll());

                //TODO fileDialog throws bad window error
                QString fileName = QFileDialog::getOpenFileName(nullptr, title, path, text, nullptr);
                if (fileName == nullptr)
                    return false;

                if (gPluginRelay->mGuiPluginTable)
                    gPluginRelay->mGuiPluginTable->loadFeature(FacExtensionInterface::FacGatelibParser);

                auto gateLibrary = gate_library_manager::load(std::filesystem::path(fileName.toStdString()));

                if(gateLibrary)
                    mGateLibrary = gateLibrary;
                else
                {
                    //extract gatelibrary name from path
                    mGateLibrary = new GateLibrary(std::filesystem::path(fileName.toStdString()), QFileInfo(fileName).baseName().toStdString());
                }

                mReadOnly = false;
            }

        }
        else
        {
            mReadOnly = readOnly;
            mGateLibrary = gateLibrary;
        }
        QDir dir(QDir::home());
        if(!gFileStatusManager->isGatelibModified()) window()->setWindowTitle(
                QString("GateLibrary %1").arg(QDir::home().relativeFilePath(QString::fromStdString(mGateLibrary->get_path().string()))));
        mPath = mGateLibrary->get_path().generic_string();
        return true;
    }

    void GatelibraryContentWidget::handleContextMenuRequested(const QPoint& pos)
    {
        QModelIndex idx = mTableView->indexAt(pos);

        if(!idx.isValid())
            return;

        QMenu menu;

        menu.addAction(mEditAction);
        menu.addAction(mDeleteAction);

        menu.move(mapToGlobal(pos));
        menu.exec();
    }

    void GatelibraryContentWidget::setGateLibrary(GateLibrary *gl)
    {
        mGateLibrary = gl;
    }

    void GatelibraryContentWidget::setGateLibraryPath(std::filesystem::path p)
    {
        mPath = p;
    }

    void GatelibraryContentWidget::handleEditAction()
    {
        QModelIndex inx = mPinProxyModel->mapToSource(mTableView->currentIndex());
        if (inx.isValid())
            Q_EMIT triggerEditType(inx);
    }

    void GatelibraryContentWidget::handleDeleteAction()
    {
        QModelIndex inx = mPinProxyModel->mapToSource(mTableView->currentIndex());
        if (inx.isValid())
            Q_EMIT triggerDeleteType(inx);
    }

    void GatelibraryContentWidget::handleCurrentSelectionChanged(QModelIndex prevIndex){
        QModelIndex inx = mPinProxyModel->mapToSource(mTableView->currentIndex());
        if (inx.isValid())
            Q_EMIT triggerCurrentSelectionChanged(inx, prevIndex);
    }

    void GatelibraryContentWidget::handleDoubleClicked(QModelIndex index)
    {
        QModelIndex inx = mPinProxyModel->mapToSource(index);
        if (inx.isValid())
            Q_EMIT triggerDoubleClicked(inx);
    }

    void GatelibraryContentWidget::handleSaveAction()
    { 
        if(mCreationMode)
            handleSaveAsAction();
        else
        {
            if (gPluginRelay->mGuiPluginTable)
                gPluginRelay->mGuiPluginTable->loadFeature(FacExtensionInterface::FacGatelibWriter,QString::fromStdString(mPath.extension().string()));
            if(gate_library_manager::save(mPath,mGateLibrary,true))
            {
                gFileStatusManager->gatelibSaved();
                window()->setWindowTitle(mTitle);
            }
        }
        mCreationMode = false;
    }

    void GatelibraryContentWidget::handleSaveAsAction()
    {
        std::filesystem::path oldPath = mGateLibrary->get_path();
        QString path  = QDir::currentPath();
        QFile gldpath(":/path/gate_library_definitions");
        if (gldpath.open(QIODevice::ReadOnly))
            path = QString::fromUtf8(gldpath.readAll());

        if (gPluginRelay->mGuiPluginTable)
            gPluginRelay->mGuiPluginTable->loadFeature(FacExtensionInterface::FacGatelibWriter,".hgl");

        QString filename = "";
        QFileDialog fd(this, "Save as", path, "HGL *.hgl");
        fd.setDefaultSuffix(".hgl");
        fd.setAcceptMode(QFileDialog::AcceptSave);
        if (fd.exec()) {
            filename = fd.selectedFiles().front();
        }

        // order of conditions important:
        //   1. Filename for save as provided
        //   2. Copy was saved under new filename
        //   3. We ar *not* in readOnly Mode
        //       => continue to work with copy
        if (!filename.isEmpty() &&  gate_library_manager::save(filename.toStdString(),mGateLibrary,true) && !mReadOnly)
        {
            mPath = filename.toStdString();
            gFileStatusManager->gatelibSaved();
            window()->setWindowTitle(filename);
            mGateLibrary->set_path(std::filesystem::path(mPath));
            QString glName = filename.mid(filename.lastIndexOf('/')+1); //truncate path until first '/'
            std::string gatelibName = glName.left(glName.length()-4).toStdString(); //remove file extension to get the name
            if(mPath != oldPath)
                mGateLibrary->set_name(gatelibName);
        }
        std::string p = mGateLibrary->get_path().generic_string();
        mCreationMode = false;
    }

    void GatelibraryContentWidget::handleUnsavedChanges()
    {
        if(!(window()->windowTitle().right(1) == "*"))
        {
            mTitle = window()->windowTitle();
            window()->setWindowTitle(mTitle + " *");
        }
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

    void GatelibraryContentWidget::activate(bool readOnly)
    {
        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        mReadOnly = readOnly;
        toggleReadOnlyMode(readOnly);

        mSearchAction->setIcon(gui_utility::getStyledSvgIcon(mEnabledIconStyle,mSearchIconPath));

    }

    void GatelibraryContentWidget::toggleReadOnlyMode(bool readOnly)
    {
        mReadOnly = readOnly;
        mDeleteAction->setEnabled(!readOnly);
        mDeleteAction->setIcon(gui_utility::getStyledSvgIcon(readOnly ? mDisabledIconStyle : mEnabledIconStyle, mDeleteIconPath));
        mEditAction->setEnabled(!readOnly);
        mEditAction->setIcon(gui_utility::getStyledSvgIcon(readOnly ? mDisabledIconStyle : mEnabledIconStyle,mEditTypeIconPath));

        mAddAction->setEnabled(!readOnly);
        mAddAction->setIcon(gui_utility::getStyledSvgIcon(readOnly ? mDisabledIconStyle : mEnabledIconStyle,mAddTypeIconPath));

    }

    void GatelibraryContentWidget::toggleSelection(bool selected)
    {
        mDeleteAction->setEnabled(selected);
        mDeleteAction->setIcon(gui_utility::getStyledSvgIcon(selected ? mEnabledIconStyle : mDisabledIconStyle, mDeleteIconPath));
        mEditAction->setEnabled(selected);
        mEditAction->setIcon(gui_utility::getStyledSvgIcon(selected ? mEnabledIconStyle : mDisabledIconStyle,mEditTypeIconPath));

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

    QString GatelibraryContentWidget::deleteIconPath() const
    {
        return mDeleteIconPath;
    }

    QString GatelibraryContentWidget::deleteIconStyle() const
    {
        return mDeleteIconStyle;
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

    void GatelibraryContentWidget::setDeleteIconPath(const QString& s)
    {
        mDeleteIconPath = s;
    }

    void GatelibraryContentWidget::setDeleteIconStyle(const QString& s)
    {
        mDeleteIconStyle = s;
    }
}
