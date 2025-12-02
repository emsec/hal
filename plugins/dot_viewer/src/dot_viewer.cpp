#include "dot_viewer/dot_viewer.h"

#include "gui/gui_utils/graphics.h"
#include "gui/toolbar/toolbar.h"
#include <QGraphicsView>
#include <QVBoxLayout>
#include <QInputDialog>
#include <QString>
#include <QAction>
#include <QFileDialog>
#include "QGVCore/QGVScene.h"
#include "QGVCore/QGVEdge.h"
#include "dot_viewer/dot_graphics_view.h"
#include <QShortcut>
#include "gui/graph_tab_widget/graph_tab_widget.h"
#include "gui/settings/settings_items/settings_item_keybind.h"
#include "gui/main_window/main_window.h"
#include "gui/settings/settings_items/settings_item_dropdown.h"
#include <QMessageBox>

namespace hal
{
        DotViewer::DotViewer(const QString& pluginName, QObject *parent)
            : ExternalContentWidget(pluginName, "DotViewer")
        {
            mOpenInputfileAction = new QAction(this);
            mOpenInputfileAction->setToolTip("Open dot file");
            connect(mOpenInputfileAction, &QAction::triggered, this, &DotViewer::handleOpenInputFileDialog);
            mOpenInputfileAction->setIcon(gui_utility::getStyledSvgIcon("all->#3192C5",":/icons/folder"));

            mToggleGridAction = new QAction(this);
            mToggleGridAction->setToolTip("Toggle grid");
            connect(mToggleGridAction, &QAction::triggered, this, &DotViewer::handleToggleGrid);
            mToggleGridAction->setIcon(gui_utility::getStyledSvgIcon("all->#ffffff", ":/icons/toggle-grid"));

            mColorSelectAction = new Action(this);
            mColorSelectAction->setToolTip("Select color style");
            connect(mColorSelectAction, &QAction::triggered, this, &DotViewer::handleColorSelect);
            mColorSelectAction->setIcon(gui_utility::getStyledSvgIcon("all->#ffffff", ":/icons/color_select"));

            QShortcut* zoomInShortcut = new QShortcut(GraphTabWidget::sSettingZoomIn->value().toString(), this);
            QShortcut* zoomOutShortcut = new QShortcut(GraphTabWidget::sSettingZoomOut->value().toString(), this);
            connect(GraphTabWidget::sSettingZoomIn, &SettingsItemKeybind::keySequenceChanged, zoomInShortcut, &QShortcut::setKey);
            connect(GraphTabWidget::sSettingZoomOut, &SettingsItemKeybind::keySequenceChanged, zoomOutShortcut, &QShortcut::setKey);
            connect(MainWindow::sSettingStyle, &SettingsItemDropdown::intChanged, this, &DotViewer::handleStyleChanged);

            mDotGraphicsView = new DotGraphicsView(this);

            connect(zoomInShortcut, &QShortcut::activated, mDotGraphicsView, &DotGraphicsView::handleZoomInShortcut);
            connect(zoomOutShortcut, &QShortcut::activated, mDotGraphicsView, &DotGraphicsView::handleZoomOutShortcut);

            mDotScene = new QGVScene(this);
            mDotGraphicsView->setScene(mDotScene);
            mContentLayout->addWidget(mDotGraphicsView);
            handleStyleChanged(MainWindow::sSettingStyle->value().toInt());
        }

        void DotViewer::setupToolbar(Toolbar* toolbar)
        {
            toolbar->addAction(mOpenInputfileAction);
            toolbar->addAction(mToggleGridAction);
            toolbar->addAction(mColorSelectAction);
        }

        DotViewer::~DotViewer()
        {;}

        DotViewer* DotViewer::getDotviewerInstance()
        {
            QMap<QString,ExternalContentWidget*> owMap = ExternalContent::instance()->openWidgets;
            auto jt = owMap.find("dot_viewer");
            if (jt == owMap.end()) return nullptr;
            return dynamic_cast<DotViewer*>(jt.value());
        }

        void DotViewer::handleStyleChanged(int istyle)
        {
            switch (istyle)
            {
                case MainWindow::StyleSheetOption::Dark:
                    QGVStyle::instance()->changeHalStyle(QGVStyle::Dark);
                    break;
                case MainWindow::StyleSheetOption::Light:
                    QGVStyle::instance()->changeHalStyle(QGVStyle::Light);
                    break;
                default:
                    return;
            }
            mDotScene->update();
        }

        void DotViewer::handleToggleGrid()
        {
            mDotScene->toggleDrawGrid();
        }

        void DotViewer::handleOpenInputFileDialog()
        {
            QString filename = QFileDialog::getOpenFileName(this, "Open dot file", ".", "Dot files (*.dot);;All files (*)");
            if (!filename.isEmpty()) handleOpenInputFileByName(filename);
        }

        void DotViewer::handleOpenInputFileByName(const QString& fileName, const QString &creator)
        {
            if (dynamic_cast<PythonThread*>(QThread::currentThread()))
            {
                // call from differnt thread, we cannot create GUI objects directly
                DotViewerCallFromTread* dvcft = new DotViewerCallFromTread;
                dvcft->emitOpenInputFileByName(this, fileName, creator);
                delete dvcft;
                return;
            }

            mFilename = fileName;
            mCreatorPlugin = creator;

            const char* halComment = "\"created by HAL plugin ";

            QObject* toDispose = mDotScene;
            mDotScene = new QGVScene(this);
            mDotGraphicsView->setScene(mDotScene);
            toDispose->deleteLater();

            if (fileName.isEmpty()) return;
            QFile ff(fileName);
            if (!ff.open(QIODevice::ReadOnly)) return;
            QByteArray dotfileContent = ff.readAll();

            // Determine plugin name to construct correct interaction class

            // 1.Try : plugin name from parameter in function call
            QGVInteraction* interact = (mCreatorPlugin.isEmpty() || mCreatorPlugin.toLower() == "(none)")
                ? nullptr
                : QGVInteraction::construct(mCreatorPlugin,mDotScene);

            // 2.Try : plugin name from comment in .dot file
            if (!interact && mCreatorPlugin.toLower() != "(none)")
            {
                int pos0;
                if ((pos0 = dotfileContent.indexOf(halComment)) >= 0)
                {
                    pos0 += strlen(halComment);
                    int pos1 = dotfileContent.indexOf('"', pos0);
                    Q_ASSERT(pos1 > pos0);
                    mCreatorPlugin = dotfileContent.mid(pos0, pos1-pos0);
                }

                interact = QGVInteraction::construct(mCreatorPlugin,mDotScene);

                // 3.Try : plugin name selected by user
                if (!interact)
                {
                    QStringList choices = QGVInteraction::getPlugins();
                    choices.prepend("(none)");
                    mCreatorPlugin = QInputDialog::getItem(this, "Creator of File",
                                                "Select plugin which created the .dot file to enable\n"
                                                "interactions between HAL GUI and Graphviz viewer.",
                                                choices, 0, false);
                    interact = QGVInteraction::construct(mCreatorPlugin,mDotScene);
                    if (!interact) mCreatorPlugin.clear();
                }
            }

            // interact might be nullptr but that is OK (no interactions in this case)
            mDotScene->loadLayout(dotfileContent, interact);
        }

        void DotViewer::handleColorSelect()
        {
            QMenu menu;
            const char* target[] = {"node", "edge", nullptr };
            for (int i=0; target[i]; i++)
            {
                QAction* act = menu.addAction(QString("HAL color style overwrites %1 attibute").arg(target[i]));
                act->setCheckable(true);
                act->setChecked(QGVStyle::instance()->getStyle((QGVStyle::StyleTarget)i) != QGVStyle::Graphviz);
                connect (act, &QAction::triggered, this, [target,i,this](bool checked){
                    std::cerr << QString("HAL color style overwrites %1 attibute => %2").arg(target[i]).arg(checked?"true":"false").toStdString() << std::endl;
                    if (checked)
                        QGVStyle::instance()->setStyle((QGVStyle::StyleTarget)i, (QGVStyle::StyleType) MainWindow::sSettingStyle->value().toInt());
                    else
                        QGVStyle::instance()->setStyle((QGVStyle::StyleTarget)i, QGVStyle::Graphviz);
                    this->mDotScene->update();
                });
            }
            menu.exec(QCursor::pos());
        }

        void DotViewer::disableInteractions()
        {
            for (QObject* obj : mDotScene->children())
            {
                QGVInteraction* interact = dynamic_cast<QGVInteraction*>(obj);
                if (interact) interact->disableHandler();
            }
        }

        void DotViewerCallFromTread::emitOpenInputFileByName(DotViewer* callee, QString filename, QString plugin)
        {
            if (!callee) return;
            connect(this, &DotViewerCallFromTread::callOpenInputFileByName, callee, &DotViewer::handleOpenInputFileByName, Qt::BlockingQueuedConnection);
            Q_EMIT callOpenInputFileByName(filename, plugin);
            disconnect(this, &DotViewerCallFromTread::callOpenInputFileByName, callee, &DotViewer::handleOpenInputFileByName);
        }
}  // namespace hal
