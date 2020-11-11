#include "gui/file_modified_bar/file_modified_bar.h"

#include <QFileInfo>

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

        connect(mReloadButton, &QPushButton::clicked, this, &FileModifiedBar::handleReloadClicked);
        connect(mIgnoreButton, &QPushButton::clicked, this, &FileModifiedBar::handleIgnoreClicked);
        connect(mOkButton, &QPushButton::clicked, this, &FileModifiedBar::handleOkClicked);
    }

    void FileModifiedBar::handleFileChanged(QString path)
    {
        QFile file(path);

        if(file.exists())
            handleFileContentModified(path);
        else
            handleFileMovOrDel(path);
    }

    void FileModifiedBar::handleFileContentModified(QString path)
    {
        mOkButton->setHidden(true);
        mReloadButton->setHidden(false);
        mIgnoreButton->setHidden(false);

        mMessageLabel->setText(path + " has been modified on disk.");
    }

    void FileModifiedBar::handleFileMovOrDel(QString path)
    {
        mOkButton->setHidden(false);
        mReloadButton->setHidden(true);
        mIgnoreButton->setHidden(true);

        mMessageLabel->setText(path + " has been moved on disk.");
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
