#include "cross_probing/cross_probing_control.h"
#include <QPushButton>
#include <QVBoxLayout>

const char* externalCommands[] = {
    "<SetSelectionFocus>                                            \n"
    "   <modules>3</modules>                                        \n"
    "</SetSelectionFocus>                                           \n",

    "<SetSelectionFocus>                                            \n"
    "   <gates>16,15,14,13,12</gates>                               \n"
    "   <nets>29,28,27,26,25,8,7,6,5,4</nets>                       \n"
    "</SetSelectionFocus>                                           \n",

    "<compound>"
    "   <CreateObject compound=\"0\" id=\"6\" type=\"Module\">      \n"
    "       <objectname>InputModule</objectname>                    \n"
    "       <parentid>1</parentid>                                  \n"
    "       <linkedid>0</linkedid>                                  \n"
    "   </CreateObject>                                             \n"
    "   <AddItemsToObject compound=\"1\" id=\"6\" type=\"Module\">  \n"
    "       <gates>16,15,14,13,12</gates>                           \n"
    "   </AddItemsToObject>                                         \n"
    "</compound>                                                    \n",

    "<MoveNode id=\"6\" type=\"Module\">                            \n"
    "   <context>1</context>                                        \n"
    "   <to>0,2</to>                                                \n"
    "</MoveNode>                                                    \n",

    "<SetSelectionFocus id=\"6\" type=\"Module\">                   \n"
    "   <modules>6</modules>                                        \n"
    "</SetSelectionFocus>                                           \n",

    nullptr};

int commandCounter = 0;

namespace hal {
    CrossProbingControl::CrossProbingControl(QWidget *parent)
        : QDialog(parent)
    {
        setAttribute(Qt::WA_DeleteOnClose);
        setWindowTitle("Fake External Application");
        setMinimumSize(QSize(640,480));
        QVBoxLayout* layout = new QVBoxLayout(this);
        QPushButton* testBut = new QPushButton("test",this);
        connect(testBut, &QPushButton::clicked, this, &CrossProbingControl::testButtonClicked);
        layout->addWidget(testBut);
        mStatus = new QLabel("Press test button for first action", this);
        layout->addWidget(mStatus);
        mSocket = new QLocalSocket(this);
        connect(mSocket, &QLocalSocket::connected, this, &CrossProbingControl::socketCanSend, Qt::QueuedConnection);
        connect(mSocket, &QLocalSocket::readyRead, this, &CrossProbingControl::socketCanRead, Qt::QueuedConnection);
    }

    void CrossProbingControl::testButtonClicked()
    {
        if (!externalCommands[commandCounter]) {
            mStatus->setText("No more test actions");
            return;
        }

        mCurrentCommand = externalCommands[commandCounter++];
        mSocket->abort();
        mSocket->connectToServer("hal_action_pipe");
    }

    void CrossProbingControl::socketCanSend()
    {
        mSocket->write(mCurrentCommand);
        mSocket->flush();
    }

    void CrossProbingControl::socketCanRead()
    {
        QByteArray response = mSocket->readAll();
        mStatus->setText(QString::fromUtf8(response));
        mSocket->close();
    }
}
