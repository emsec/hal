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
#include <QString>
#include <QCryptographicHash>
#include "user_action_manager.h"
#include "user_action_object.h"

namespace hal
{
    /**
     * @ingroup user_action
     * @brief The UserAction class is the abstract base class for user interactions
     *
     * It is expected that the exec() method gets called eventually. The derived classes
     * exec() method contains the implementation of the command. The exec function of the base
     * class needs to be called as well in order to store the interaction in the
     * user action managers history. At this point the user action manager is also
     * supposed to take the ownership of an user action object.
     *
     * While derived classes might contain additional arguments the member variable
     * mObject holds a standard argument like a single gate or module.
     *
     * If pointer to mUndoAction is set the interaction might be reversed using
     * Ctrl-Z in a future version of hal
     *
     * The mCompoundOrder number is the index of this action if part of an action
     * compound. It can be used to identify actions which belong together and might
     * be undone in a single step.
     *
     * The mWaitForReady flag causes the execution of a macro to be paused until
     * a handle (e.g. timer handle) resets the flag. It is considered to be a dirty hack and
     * should only be used in the case of extreme desperation.
     */
    class UserAction
    {
    public:

        /**
         * The destructor.
         */
        virtual ~UserAction();

        /**
        * Executes user interaction and returns true on success.
        * Note: Each implementation of exec() must include a call to the exec()
        * method of base class to store the UserAction object in history and
        * to transfer its ownership.
        *
        * @return True on success, False otherwise.
        */
        virtual bool exec();

        /**
         * The xml tagname for the action is purely virtual and needs to be implemented
         * in derived class.
         *
         * @return The tagname.
         */
        virtual QString tagname() const = 0;

        /**
         * Persist user interaction in xml macro file.
         *
         * @param xmlOut - The xml writer.
         */
        virtual void writeToXml(QXmlStreamWriter& xmlOut) const;

        /**
         * Reads user interaction as well as arguments from xml macro file.
         *
         * @param xmlIn - The xml reader.
         */
        virtual void readFromXml(QXmlStreamReader& xmlIn);

        /**
         * Getter for object argument of interaction.
         *
         * @return The object argument.
         */
        virtual UserActionObject object() const { return mObject; }

        /**
         * Setter for object argument of interaction.
         *
         * @param obj - The object.
         */
        virtual void setObject(const UserActionObject& obj);

        /**
         * Pause macro execution until flag gets cleared by handler.
         *
         * @return The WaitForReady flag.
         */
        bool isWaitForReady() const { return mWaitForReady; }

        /**
         * Get the order number in action compound, -1 if not in compound.
         *
         * @return The order number in action compount.
         */
        int compoundOrder() const { return mCompoundOrder; }

        /**
         * Setter for compoundOrder.
         *
         * @param cmpord - The new compound order.
         */
        void setCompoundOrder(int cmpord) { mCompoundOrder = cmpord; }

        /**
         * Get the time stamp for execution time.
         *
         * @return The time stamp.
         */
        qint64 timeStamp() const { return mTimeStamp; }

        /**
         * Get the cryptographic hash to detect manipulations in xml macro file.
         *
         * @param recordNo - A nonce.
         * @return The hash.
         */
        QString cryptographicHash(int recordNo) const;

        /**
         * Hook for derived classes to add parameter to cryptographic hash.
         *
         * @param cryptoHash - Additional hash data.
         */
        virtual void addToHash(QCryptographicHash& cryptoHash) const;

        /**
         * Get the action to reverse last action (nullptr if action cant be undone).
         *
         * @return The reverse action.
         */
        UserAction* undoAction() const { return mUndoAction; }

        /**
         * Dump action in debug window.
         *
         * @return The debug string.
         */
        QString debugDump() const;

        /**
         * Refuse set object requests (e.g. in UserActionCompound)
         *
         * @param lock - Param to set the lock.
         */
        void setObjectLock(bool lock) { mObjectLock = lock; }

    protected:
        UserAction();
        UserActionObject mObject;
        bool mWaitForReady;
        int mCompoundOrder;
        UserAction *mUndoAction;
        qint64 mTimeStamp;
        bool mObjectLock;

        static QString setToText(const QSet<u32>& set);
        static QSet<u32> setFromText(const QString& s);
    };

    /**
     * @ingroup user_action
     * @brief The UserActionFactory is the abstract base class for registration
     *
     * Each class derived from UserAction is supposed to come with a derived factory as
     * well. The factory is used to register the user action constructor with the user
     * action manager. Doing so new interactions can be implemented without tempering
     * with the manager class.
     */
    class UserActionFactory
    {
    protected:
        QString mTagname;
    public:

        /**
         * Register the constructor for user action with tag name provided by derived class.
         *
         * @param nam - The tag name.
         */
        UserActionFactory(const QString& nam);

        /**
         * Get the tag name of derived class.
         *
         * @return The tag name.
         */
        QString tagname() const { return mTagname; }

        /**
         * Call the user action constructor.
         *
         * @return The user action.
         */
        virtual UserAction* newAction() const = 0;
    };
}

