#include "media_viewer/media_viewer.h"

#include "gui/gui_utils/graphics.h"
#include "gui/toolbar/toolbar.h"
#include <QGraphicsView>
#include <QVBoxLayout>
#include <QInputDialog>
#include <QString>
#include <QAction>
#include <QFileDialog>
#include <QShortcut>
#include "gui/graph_tab_widget/graph_tab_widget.h"
#include "gui/settings/settings_items/settings_item_keybind.h"
#include "gui/main_window/main_window.h"
#include "gui/settings/settings_items/settings_item_dropdown.h"
#include <QMessageBox>
#include <QVideoWidget>
#include <QMediaPlayer>
#include <QMediaContent>

namespace hal
{
        MediaViewer::MediaViewer(const QString& pluginName, QObject *parent)
            : ExternalContentWidget(pluginName, "MediaViewer")
        {
            mOpenFileAction  = new QAction(this);
            mCtrlPlayAction  = new QAction(this);
            mCtrlPauseAction = new QAction(this);
            mCtrlStopAction  = new QAction(this);

            mOpenFileAction->setToolTip("Open media file");
            mCtrlPlayAction->setToolTip("Start playing");
            mCtrlPauseAction->setToolTip("Pause");
            mCtrlStopAction->setToolTip("Stop playing");

            connect(mOpenFileAction,  &QAction::triggered, this, &MediaViewer::handleOpenFileDialog);
            connect(mCtrlPlayAction,  &QAction::triggered, this, &MediaViewer::handleControlPlay);
            connect(mCtrlPauseAction, &QAction::triggered, this, &MediaViewer::handleControlPause);
            connect(mCtrlStopAction,  &QAction::triggered, this, &MediaViewer::handleControlStop);

            mOpenFileAction->setIcon (gui_utility::getStyledSvgIcon("all->#3192C5",":/icons/folder"));
            mCtrlPlayAction->setIcon (gui_utility::getStyledSvgIcon("all->#ffffff",":/icons/run", "all->#606162"));
            mCtrlPauseAction->setIcon(gui_utility::getStyledSvgIcon("all->#ffffff",":/icons/circle", "all->#606162"));
            mCtrlStopAction->setIcon (gui_utility::getStyledSvgIcon("all->#ffffff",":/icons/switch", "all->#606162"));

            QShortcut* zoomInShortcut = new QShortcut(GraphTabWidget::sSettingZoomIn->value().toString(), this);
            QShortcut* zoomOutShortcut = new QShortcut(GraphTabWidget::sSettingZoomOut->value().toString(), this);
            connect(GraphTabWidget::sSettingZoomIn, &SettingsItemKeybind::keySequenceChanged, zoomInShortcut, &QShortcut::setKey);
            connect(GraphTabWidget::sSettingZoomOut, &SettingsItemKeybind::keySequenceChanged, zoomOutShortcut, &QShortcut::setKey);
            connect(MainWindow::sSettingStyle, &SettingsItemDropdown::intChanged, this, &MediaViewer::handleStyleChanged);

            mMediaPlayer = new QMediaPlayer(this);
            mVideoWidget = new QVideoWidget(this);
            mMediaPlayer->setVideoOutput(mVideoWidget);


//            connect(zoomInShortcut, &QShortcut::activated, mDotGraphicsView, &DotGraphicsView::handleZoomInShortcut);
//            connect(zoomOutShortcut, &QShortcut::activated, mDotGraphicsView, &DotGraphicsView::handleZoomOutShortcut);

            mContentLayout->addWidget(mVideoWidget);
            handleStyleChanged(MainWindow::sSettingStyle->value().toInt());
        }

        void MediaViewer::setupToolbar(Toolbar* toolbar)
        {
            toolbar->addAction(mOpenFileAction);
            toolbar->addAction(mCtrlPlayAction);
            toolbar->addAction(mCtrlPauseAction);
            toolbar->addAction(mCtrlStopAction);
        }

        MediaViewer::~MediaViewer()
        {;}

        MediaViewer* MediaViewer::getMediaviewerInstance()
        {
            QMap<QString,ExternalContentWidget*> owMap = ExternalContent::instance()->openWidgets;
            auto jt = owMap.find("media_viewer");
            if (jt == owMap.end()) return nullptr;
            return dynamic_cast<MediaViewer*>(jt.value());
        }

        void MediaViewer::handleStyleChanged(int istyle)
        {
            Q_UNUSED(istyle);
            // TODO : react on style switch (dark/light)
        }

        void MediaViewer::handleOpenFileDialog()
        {
            QString filename = QFileDialog::getOpenFileName(this, "Open media file", ".", "MP4 videos (*.mp4);;All files (*)");
            if (!filename.isEmpty()) loadMediaFile(filename);
        }

        bool MediaViewer::loadMediaFile(const QString& fileName)
        {
            if (dynamic_cast<PythonThread*>(QThread::currentThread()))
            {
                // call from differnt thread, we cannot create GUI objects directly
                MediaViewerCallFromTread* mvcft = new MediaViewerCallFromTread;
                bool retval = mvcft->openFile(this, fileName);
                delete mvcft;
                return retval;
            }

            /// TODO : delete old media if any

            if (fileName.isEmpty()) {
                log_warning("media_viewer", "Cannot load media file, no file name provided");
                return false;
            }
            QFile ff(fileName);
            if (!ff.open(QIODevice::ReadOnly)) {
                log_warning("media_viewer", "An error occurred loading media file '{}'", fileName.toStdString());
                return false;
            }

            mFilename = fileName;
            mMediaPlayer->setMedia(QMediaContent(QUrl::fromLocalFile(fileName)));

            return true;
        }

        void MediaViewer::handleControlPlay()
        {
            mMediaPlayer->play();
        }

        void MediaViewer::handleControlPause()
        {
            mMediaPlayer->pause();
        }

        void MediaViewer::handleControlStop()
        {
            mMediaPlayer->stop();
        }

        bool MediaViewerCallFromTread::openFile(MediaViewer* callee, QString filename)
        {
            if (!callee) return false;
            connect(this, &MediaViewerCallFromTread::callOpenFile, callee, &MediaViewer::loadMediaFile, Qt::BlockingQueuedConnection);
            Q_EMIT callOpenFile(filename);
            disconnect(this, &MediaViewerCallFromTread::callOpenFile, callee, &MediaViewer::loadMediaFile);
            return (callee->filename() == filename); // callee will not store filename upon load error
        }
}  // namespace hal
