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

#pragma once
#include "user_action.h"

namespace hal
{
    /**
     * @ingroup user_action
     * @brief Sets the value of an existing typed parameter on a gate.
     *
     * Unlike ActionSetObjectData, a gate's parameters are fixed by its type: they can only
     * be modified, never created, deleted or moved. This action therefore only carries the
     * parameter name and the new value; the target gate is identified by the inherited mObject.
     *
     * The inverse of "set parameter <name> to <value>" is another ActionSetParameterValue that
     * restores the previous value, so exec() builds its own undo action of the same type.
     */
    class ActionSetParameterValue : public UserAction
    {
    public:
        /**
         * Constructor.
         *
         * @param parameterName - The name of the parameter to modify.
         * @param value - The new value, formatted as the core expects it for the parameter's type.
         */
        ActionSetParameterValue(const QString& parameterName = QString(), const QString& value = QString());

        bool exec() override;
        QString tagname() const override;
        void writeToXml(QXmlStreamWriter& xmlOut) const override;
        void readFromXml(QXmlStreamReader& xmlIn) override;
        void addToHash(QCryptographicHash& cryptoHash) const override;

    private:
        QString mParameterName;
        QString mValue;
    };

    /**
     * @brief The ActionSetParameterValueFactory class registers the action with the manager.
     */
    class ActionSetParameterValueFactory : public UserActionFactory
    {
    public:
        ActionSetParameterValueFactory();
        UserAction* newAction() const override;
        static ActionSetParameterValueFactory* sFactory;
    };
}    // namespace hal
