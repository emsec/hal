//  MIT License
//
//  Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

#pragma once
#include "user_action.h"
#include <QPoint>
#include <QString>

namespace hal
{
    /**
     * @ingroup user_action
     * @brief Moves a node.
     *
     * Moves a node to another grid position in the GraphicsScene. The Context ID should be provided by passing it in
     * the UserActionObject of type UserActionObjectType::Context.
     *
     * Undo Action: ActionMoveNode
     */
    class ActionMoveNode : public UserAction
    {
        QPoint mFrom, mTo;

        int mContextId;

        static QPoint parseFromString(const QString& s);
    public:
        /**
         * Action constructor.
         *
         * The action object is the graphics context
         *
         * @param from - The initial position of the node to move
         * @param to - The destination of the node
         */
        ActionMoveNode(const QPoint& from = QPoint(), const QPoint& to = QPoint())
            : mFrom(from), mTo(to), mContextId(-1) {;}

        /**
         * Action constructor.
         *
         * Will move the node object (gate or module) in view identified by context
         * A call to setObject is required to identify the node object
         *
         * @param  - The initial position of the node to move
         * @param to - The destination of the node
         */
        ActionMoveNode(u32 ctxId, const QPoint& to)
            : mTo(to), mContextId(ctxId) {;}

        bool exec() override;
        QString tagname() const override;
        void writeToXml(QXmlStreamWriter& xmlOut) const override;
        void readFromXml(QXmlStreamReader& xmlIn) override;
        void addToHash(QCryptographicHash& cryptoHash) const override;
    };

    /**
     * @ingroup user_action
     * @brief UserActionFactory for ActionMoveNode
     */
    class ActionMoveNodeFactory : public UserActionFactory
    {
    public:
        ActionMoveNodeFactory();
        UserAction* newAction() const;
        static ActionMoveNodeFactory* sFactory;
    };
}
