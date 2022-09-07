#include "gui/file_modified_bar/file_modified_bar.h"
#include "gui/gui_globals.h"
#include "gui/python/python_context.h"
#include "gui/python/python_thread.h"
#include <QFileInfo>
#include <sys/types.h>
#include <signal.h>
#include <QDebug>

namespace hal
{
    FileModifiedBar::FileModifiedBar(QWidget* parent)
    {
        Q_UNUSED(parent);
        mLayout = new QHBoxLayout();
        setLayout(mLayout);

        mMessageLabel = new QLabel("");
        mLayout->addWidget(mMessageLabel);

        mReloadButton = new QPushButton("Reload");
        mReloadButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        mLayout->addWidget(mReloadButton);

        mIgnoreButton = new QPushButton("Ignore");
        mIgnoreButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        mLayout->addWidget(mIgnoreButton);

        mOkButton = new QPushButton("Ok");
        mOkButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        mLayout->addWidget(mOkButton);

        mAbortButton = new QPushButton("Abort");
        mAbortButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        mLayout->addWidget(mAbortButton);


        connect(mReloadButton, &QPushButton::clicked, this, &FileModifiedBar::handleReloadClicked);
        connect(mIgnoreButton, &QPushButton::clicked, this, &FileModifiedBar::handleIgnoreClicked);
        connect(mOkButton, &QPushButton::clicked, this, &FileModifiedBar::handleOkClicked);
        connect(mAbortButton, &QPushButton::clicked, this, &FileModifiedBar::handleAbortClicked);
    }

    void FileModifiedBar::handleFileChanged(QString path)
    {
        QFile file(path);

        if(file.exists())
            handleFileContentModified(path);
        else
            handleFileMovOrDel(path);
    }

    void FileModifiedBar::handleScriptExecute(QString path)
    {
        mAbortButton->setHidden(false);
        mOkButton->setHidden(true);
        mReloadButton->setHidden(true);
        mIgnoreButton->setHidden(true);

        mMessageLabel->setText("Executing script " + path + "...");
    }


    void FileModifiedBar::handleFileContentModified(QString path)
    {
        mAbortButton->setHidden(true);
        mOkButton->setHidden(true);
        mReloadButton->setHidden(false);
        mIgnoreButton->setHidden(false);

        mMessageLabel->setText(path + " has been modified on disk.");
    }

    void FileModifiedBar::handleFileMovOrDel(QString path)
    {
        mAbortButton->setHidden(true);
        mOkButton->setHidden(false);
        mReloadButton->setHidden(true);
        mIgnoreButton->setHidden(true);

        mMessageLabel->setText(path + " has been moved on disk.");
    }

    // FIXME why are we using FileModifiedBar for aborting a python script?
    void FileModifiedBar::handleAbortClicked()
    {
        if (gPythonContext->currentThread())
        {
            gPythonContext->currentThread()->interrupt();
        }
    }

    
    void FileModifiedBar::handleReloadClicked()
    {
        Q_EMIT reloadClicked();
    }

    void FileModifiedBar::handleIgnoreClicked()
    {
        Q_EMIT ignoreClicked();
    }

    void FileModifiedBar::handleOkClicked()
    {
        Q_EMIT okClicked();
    }
}
