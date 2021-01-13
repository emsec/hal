//  MIT License
//
//  Copyright (c) 2019 Ruhr-University Bochum, Germany, Chair for Embedded Security. All Rights reserved.
//  Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
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
     * The mCompound flag indicates that the action is part of a compound of several
     * actions. While this flag has no immediate significance it might get used later
     * on to e.g. disable layouting until all actions have been performed
     *
     * The mWaitForReady flag causes the execution of a macro to be paused until
     * a handle (e.g. timer handle) resets the flag. It is considered to be a dirty hack and
     * should only be used in the case of extreme desperation.
     */
    class UserAction
    {
    public:
        /// destructor
        virtual ~UserAction();

        /// executes user interaction. A call to base function must be issued in order
        /// to store object in action manager history.
        virtual void exec();

        /// the xml tagname for the action is purely virtual and needs to be implemented
        /// in derived class
        virtual QString tagname() const = 0;

        /// persist user interaction in xml macro file
        virtual void writeToXml(QXmlStreamWriter& xmlOut) const;

        /// reads user interaction as well as arguments from xml macro file
        virtual void readFromXml(QXmlStreamReader& xmlIn);

        /// getter for object argument of interaction
        virtual UserActionObject object() const { return mObject; }

        /// setter for object argument of interaction
        virtual void setObject(const UserActionObject& o) { mObject = o; }

        /// pause macro execution until flag gets cleared by handler
        bool isWaitForReady() const { return mWaitForReady; }

        /// interaction object is part of a compound of multiple actions
        bool isCompound() const { return mCompound; }

        /// return time stamp for execution time
        qint64 timeStamp() const { return mTimeStamp; }

        /// return cryptographic hash to detect manipulations in xml macro file
        QString cryptographicHash() const;

        /// hook for derived classes to add parameter to cryptographic hash
        virtual void addToHash(QCryptographicHash& cryptoHash) const;
    protected:
        UserAction();
        UserActionObject mObject;
        bool mWaitForReady;
        bool mCompound;
        UserAction *mUndoAction;
        qint64 mTimeStamp;
    };

    /**
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
        /// register the constructor for user action with tag name provided by derived class
        UserActionFactory(const QString& nam);

        /// return the tag name of derived class
        QString tagname() const { return mTagname; }

        ///  call the user action constructor
        virtual UserAction* newAction() const = 0;
    };
}

