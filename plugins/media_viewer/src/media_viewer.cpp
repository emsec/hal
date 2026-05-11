// MIT License
//
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "media_viewer/media_viewer.h"

#include "gui/gui_utils/graphics.h"
#include "gui/toolbar/toolbar.h"
#include "hal_core/utilities/log.h"

#include <QAction>
#include <QAudioOutput>
#include <QComboBox>
#include <QCursor>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QMediaPlayer>
#include <QShortcut>
#include <QSizePolicy>
#include <QSlider>
#include <QStackedWidget>
#include <QStyle>
#include <QTime>
#include <QToolButton>
#include <QToolTip>
#include <QUrl>
#include <QVBoxLayout>
#include <QVideoWidget>
#include <QWidget>

namespace hal
{
    MediaViewer::MediaViewer(const QString& pluginName, QObject* /*parent*/)
        : ExternalContentWidget(pluginName, "MediaViewer")
    {
        // ---------------------------------------------------------------
        // Player pipeline
        // ---------------------------------------------------------------
        mPlayer      = new QMediaPlayer(this);
        mAudioOutput = new QAudioOutput(this);
        mPlayer->setAudioOutput(mAudioOutput);
        mAudioOutput->setVolume(0.70f);    // default 70%

        mVideoWidget = new QVideoWidget(this);
        mVideoWidget->setAspectRatioMode(Qt::KeepAspectRatio);
        mVideoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mPlayer->setVideoOutput(mVideoWidget);

        // ---------------------------------------------------------------
        // Volume slider — horizontal, always visible, right of mute button
        // ---------------------------------------------------------------
        mVolumeSlider = new QSlider(Qt::Horizontal, this);
        mVolumeSlider->setRange(0, 100);
        mVolumeSlider->setValue(70);
        mVolumeSlider->setFixedWidth(80);
        mVolumeSlider->setToolTip("Volume");
        connect(mVolumeSlider, &QSlider::valueChanged, this, &MediaViewer::handleVolumeChanged);

        // ---------------------------------------------------------------
        // Play / Pause button
        // ---------------------------------------------------------------
        mPlayPauseButton = new QToolButton(this);
        mPlayPauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        mPlayPauseButton->setToolTip("Play");
        mPlayPauseButton->setAutoRaise(true);
        connect(mPlayPauseButton, &QToolButton::clicked, this, &MediaViewer::handlePlayPauseToggle);

        // ---------------------------------------------------------------
        // Seek slider
        // ---------------------------------------------------------------
        mSeekSlider = new QSlider(Qt::Horizontal, this);
        mSeekSlider->setRange(0, 0);
        mSeekSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        connect(mSeekSlider, &QSlider::valueChanged, this, &MediaViewer::handleSeekSliderMoved);

        // ---------------------------------------------------------------
        // Time label
        // ---------------------------------------------------------------
        mTimeLabel = new QLabel("--:-- / --:--", this);
        mTimeLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        mTimeLabel->setContentsMargins(4, 0, 4, 0);

        // ---------------------------------------------------------------
        // Speed combo
        // ---------------------------------------------------------------
        mSpeedCombo = new QComboBox(this);
        static const double speeds[] = {0.25, 0.5, 0.75, 1.0, 1.25, 1.5, 1.75, 2.0};
        for (double s : speeds)
            mSpeedCombo->addItem(QString("%1x").arg(s), QVariant(s));
        mSpeedCombo->setCurrentIndex(3);    // 1.0x default
        mSpeedCombo->setToolTip("Playback speed");
        connect(mSpeedCombo, &QComboBox::currentIndexChanged, this, &MediaViewer::handleSpeedChanged);

        // ---------------------------------------------------------------
        // Mute button
        // ---------------------------------------------------------------
        mMuteButton = new QToolButton(this);
        mMuteButton->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
        mMuteButton->setToolTip("Mute");
        mMuteButton->setAutoRaise(true);
        connect(mMuteButton, &QToolButton::clicked, this, &MediaViewer::handleMuteToggle);

        // ---------------------------------------------------------------
        // Control bar (bottom strip)
        // ---------------------------------------------------------------
        QWidget*     controlBar    = new QWidget(this);
        QHBoxLayout* controlLayout = new QHBoxLayout(controlBar);
        controlLayout->setContentsMargins(4, 2, 4, 2);
        controlLayout->setSpacing(4);
        controlLayout->addWidget(mPlayPauseButton);
        controlLayout->addWidget(mSeekSlider, 1);
        controlLayout->addWidget(mTimeLabel);
        controlLayout->addSpacing(8);
        controlLayout->addWidget(mSpeedCombo);
        controlLayout->addSpacing(4);
        controlLayout->addWidget(mMuteButton);
        controlLayout->addWidget(mVolumeSlider);

        // ---------------------------------------------------------------
        // Display stack: page 0 = empty state, 1 = video, 2 = error
        // ---------------------------------------------------------------
        mDisplayStack = new QStackedWidget(this);
        mDisplayStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        QLabel* emptyLabel = new QLabel("No file loaded — click 'Open media file' to begin.", mDisplayStack);
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setWordWrap(true);
        emptyLabel->setForegroundRole(QPalette::PlaceholderText);  // muted, theme-aware

        mErrorLabel = new QLabel(mDisplayStack);
        mErrorLabel->setAlignment(Qt::AlignCenter);
        mErrorLabel->setWordWrap(true);
        mErrorLabel->setForegroundRole(QPalette::BrightText);      // contrasting, theme-aware

        mDisplayStack->addWidget(emptyLabel);   // page 0
        mDisplayStack->addWidget(mVideoWidget); // page 1
        mDisplayStack->addWidget(mErrorLabel);  // page 2
        mDisplayStack->setCurrentIndex(PageEmpty);

        // ---------------------------------------------------------------
        // Assemble content layout
        // ---------------------------------------------------------------
        mContentLayout->addWidget(mDisplayStack, 1);
        mContentLayout->addWidget(controlBar, 0);

        // ---------------------------------------------------------------
        // Keyboard shortcuts — fire only when this widget has focus
        // ---------------------------------------------------------------
        setFocusPolicy(Qt::StrongFocus);

        auto bind = [this](const QKeySequence& key, auto slot) {
            auto* sc = new QShortcut(key, this);
            sc->setContext(Qt::WidgetWithChildrenShortcut);
            connect(sc, &QShortcut::activated, this, slot);
        };

        bind(Qt::Key_Space, &MediaViewer::handlePlayPauseToggle);
        bind(Qt::Key_M,     &MediaViewer::handleMuteToggle);
        bind(Qt::Key_Up,    [this] { adjustVolume(+5); });
        bind(Qt::Key_Down,  [this] { adjustVolume(-5); });
        bind(Qt::Key_Left,  [this] { seekRelative(-10000); });
        bind(Qt::Key_Right, [this] { seekRelative(+10000); });


        // ---------------------------------------------------------------
        // Open action (toolbar only)
        // ---------------------------------------------------------------
        mOpenAction = new QAction(this);
        mOpenAction->setToolTip("Open media file");
        mOpenAction->setIcon(gui_utility::getStyledSvgIcon("all->#3192C5", ":/icons/folder"));
        connect(mOpenAction, &QAction::triggered, this, &MediaViewer::handleOpenInputFileDialog);

        // ---------------------------------------------------------------
        // Help button — shows keyboard shortcut list on hover and click
        // ---------------------------------------------------------------
        mHelpAction = new QAction("?", this);
        mHelpAction->setToolTip(shortcutHelpText());
        connect(mHelpAction, &QAction::triggered, this, &MediaViewer::handleHelpTriggered);

        // ---------------------------------------------------------------
        // Player signal connections
        // ---------------------------------------------------------------
        connect(mPlayer, &QMediaPlayer::playbackStateChanged, this, &MediaViewer::handlePlaybackStateChanged);
        connect(mPlayer, &QMediaPlayer::mediaStatusChanged,   this, &MediaViewer::handleMediaStatusChanged);
        connect(mPlayer, &QMediaPlayer::positionChanged,      this, &MediaViewer::handlePositionChanged);
        connect(mPlayer, &QMediaPlayer::durationChanged,      this, &MediaViewer::handleDurationChanged);
        connect(mPlayer, &QMediaPlayer::errorOccurred,        this, &MediaViewer::handleMediaError);
    }

    MediaViewer::~MediaViewer() = default;

    void MediaViewer::setupToolbar(Toolbar* toolbar)
    {
        toolbar->addAction(mOpenAction);
        toolbar->addSeparator();
        toolbar->addAction(mHelpAction);
    }

    MediaViewer* MediaViewer::getInstance()
    {
        const QMap<QString, ExternalContentWidget*>& owMap = ExternalContent::instance()->openWidgets;
        auto it = owMap.find("media_viewer");
        if (it == owMap.end()) return nullptr;
        return dynamic_cast<MediaViewer*>(it.value());
    }

    void MediaViewer::loadMediaFile(const QString& fileName)
    {
        // Cancel any pending first-frame sequence and stop the player before
        // switching source.  Without stop(), rapid source switches cause AVFoundation
        // to throw NSInvalidArgumentException ("output already attached") because
        // the display-link callback from the previous play() call races with the
        // new AVPlayerItem being set up.
        mSeekingFirstFrame = false;
        mPlayer->stop();

        mTimeFormat.clear();
        mTimeLabel->setText("--:-- / --:--");
        mSeekSlider->setValue(0);
        mSeekSlider->setRange(0, 0);

        mSeekingFirstFrame = true;
        mPlayer->setSource(QUrl::fromLocalFile(fileName));
        mDisplayStack->setCurrentIndex(PageVideo);
    }

    // -----------------------------------------------------------------------
    // Private slots
    // -----------------------------------------------------------------------

    void MediaViewer::handleOpenInputFileDialog()
    {
        if (mPlayer->playbackState() == QMediaPlayer::PlayingState)
            mPlayer->pause();    // pause immediately when dialog opens

        const QString filter = "Media Files (*.mp4 *.m4v *.mov);;All Files (*)";
        const QString fileName = QFileDialog::getOpenFileName(this, "Open media file", QString(), filter);

        if (fileName.isEmpty())
            return;    // cancel — stay paused

        loadMediaFile(fileName);
    }

    void MediaViewer::handlePlayPauseToggle()
    {
        mSeekingFirstFrame = false;    // user is explicitly controlling playback
        if (mPlayer->playbackState() == QMediaPlayer::PlayingState)
            mPlayer->pause();
        else
            mPlayer->play();
    }

    void MediaViewer::handlePlaybackStateChanged(QMediaPlayer::PlaybackState state)
    {
        if (state == QMediaPlayer::PlayingState && mSeekingFirstFrame)
        {
            // Backend has started decoding — immediately pause to freeze on frame 0.
            mSeekingFirstFrame = false;
            mPlayer->pause();
            return;
        }

        if (state == QMediaPlayer::PlayingState)
        {
            mPlayPauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
            mPlayPauseButton->setToolTip("Pause");
        }
        else
        {
            mPlayPauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
            mPlayPauseButton->setToolTip("Play");
        }
    }

    void MediaViewer::handleMediaStatusChanged(QMediaPlayer::MediaStatus status)
    {
        if (mSeekingFirstFrame && status == QMediaPlayer::LoadedMedia)
            mPlayer->play();    // triggers decoding; handlePlaybackStateChanged will pause
    }

    void MediaViewer::handlePositionChanged(qint64 position)
    {
        if (!mSeekSlider->isSliderDown())
        {
            mUpdatingSlider = true;
            mSeekSlider->setValue(static_cast<int>(position));
            mUpdatingSlider = false;
        }
        updateTimeLabel();
    }

    void MediaViewer::handleDurationChanged(qint64 duration)
    {
        mSeekSlider->setRange(0, static_cast<int>(duration));
        mTimeFormat = (duration >= 3600000LL) ? "hh:mm:ss" : "mm:ss";
        updateTimeLabel();
    }

    void MediaViewer::handleSeekSliderMoved(int value)
    {
        if (!mUpdatingSlider)
            mPlayer->setPosition(static_cast<qint64>(value));
    }

    void MediaViewer::handleSpeedChanged(int /*index*/)
    {
        mPlayer->setPlaybackRate(mSpeedCombo->currentData().toDouble());
    }

    void MediaViewer::handleVolumeChanged(int value)
    {
        // Moving the slider while muted implicitly unmutes
        if (mMuted)
        {
            mMuted = false;
            mAudioOutput->setMuted(false);
            mMuteButton->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
        }
        mAudioOutput->setVolume(static_cast<float>(value) / 100.0f);
    }

    void MediaViewer::handleMuteToggle()
    {
        mMuted = !mMuted;
        mAudioOutput->setMuted(mMuted);
        if (mMuted)
            mMuteButton->setIcon(style()->standardIcon(QStyle::SP_MediaVolumeMuted));
        else
            mMuteButton->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
    }

    void MediaViewer::handleMediaError(QMediaPlayer::Error err, const QString& msg)
    {
        if (err == QMediaPlayer::NoError)
            return;

        log_error("media_viewer", "Playback error ({}): {}", static_cast<int>(err), msg.toStdString());

        QString basename = QFileInfo(mPlayer->source().toLocalFile()).fileName();
        QString displayMsg;
        switch (err)
        {
            case QMediaPlayer::ResourceError:
                displayMsg = basename.isEmpty() ? "Could not open file."
                                                : QString("Could not open file: %1").arg(basename);
                break;
            case QMediaPlayer::FormatError:
                displayMsg = "Unsupported media format.";
                break;
            case QMediaPlayer::AccessDeniedError:
                displayMsg = basename.isEmpty() ? "Access denied."
                                                : QString("Access denied: %1").arg(basename);
                break;
            default:
                displayMsg = msg.isEmpty() ? "Playback error." : msg;
                break;
        }

        mErrorLabel->setText(displayMsg);
        mDisplayStack->setCurrentIndex(PageError);
    }

    // -----------------------------------------------------------------------
    // Private helpers
    // -----------------------------------------------------------------------

    void MediaViewer::adjustVolume(int delta)
    {
        int newVal = qBound(0, mVolumeSlider->value() + delta, 100);
        mVolumeSlider->setValue(newVal);    // triggers handleVolumeChanged
    }

    void MediaViewer::seekRelative(qint64 deltaMs)
    {
        qint64 newPos = qBound(0LL, mPlayer->position() + deltaMs, mPlayer->duration());
        mPlayer->setPosition(newPos);
    }

    QString MediaViewer::formatTime(qint64 ms) const
    {
        return QTime(0, 0).addMSecs(ms).toString(mTimeFormat);
    }

    void MediaViewer::updateTimeLabel()
    {
        if (mTimeFormat.isEmpty())
            return;
        mTimeLabel->setText(formatTime(mPlayer->position()) + " / " + formatTime(mPlayer->duration()));
    }

    void MediaViewer::handleHelpTriggered()
    {
        QToolTip::showText(QCursor::pos(), shortcutHelpText(), this);
    }

    QString MediaViewer::shortcutHelpText()
    {
        // Rich text so Qt renders a table — gives proper column alignment and
        // reliable Unicode arrows via HTML entities (&#8592;=← &#8593;=↑ etc.)
        return QStringLiteral(
            "<b>Keyboard Shortcuts</b><hr/>"
            "<table cellspacing='2'>"
            "<tr>"
              "<td style='padding-right:8px; border-right: 1px solid gray;'><b>Space</b></td>"
              "<td style='padding-left:8px;'>Play / Pause</td>"
            "</tr>"
            "<tr>"
              "<td style='padding-right:8px; border-right: 1px solid gray;'><b>&#8593; / &#8595;</b></td>"
              "<td style='padding-left:8px;'>Volume +5% / -5%</td>"
            "</tr>"
            "<tr>"
              "<td style='padding-right:8px; border-right: 1px solid gray;'><b>M</b></td>"
              "<td style='padding-left:8px;'>Mute toggle</td>"
            "</tr>"
            "<tr>"
              "<td style='padding-right:8px; border-right: 1px solid gray;'><b>&#8592; / &#8594;</b></td>"
              "<td style='padding-left:8px;'>Seek -10s / +10s</td>"
            "</tr>"
            "</table>"
            "<hr/><i>Only when Media Viewer is focused.</i>"
        );
    }

}    // namespace hal