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
#include "hal_core/netlist/boolean_function.h"


namespace hal
{
    /**
     * @ingroup gui
     * @brief Either adds a new function or changes an exisiting function of a given gate.
     *
     * To add a new function to a gate, a function name must be given that is not currently in use.
     * If a function name is given that already exists in the gate, the corresponding function is
     * overwritten with the new one. The undo action is either a DeleteBooleanFunction- or
     * AddBooleanFunction-Action, depending on wether a new one was added or an already existing
     * modified.
     */
    class ActionAddBooleanFunction : public UserAction
    {
    public:
        ActionAddBooleanFunction(QString booleanFuncName = QString(), BooleanFunction func = BooleanFunction(), u32 gateID = 0);

        bool exec() override;
        QString tagname() const override;
        void writeToXml(QXmlStreamWriter& xmlOut) const override;
        void readFromXml(QXmlStreamReader& xmlIn) override;
        void addToHash(QCryptographicHash& cryptoHash) const override;

    private:
        QString mName;
        BooleanFunction mFunction;
    };

    /**
     * @brief The ActionAddBooleanFunctionFactory class
     */
    class ActionAddBooleanFunctionFactory : public UserActionFactory
    {
    public:
        ActionAddBooleanFunctionFactory();
        UserAction * newAction() const override;
        static ActionAddBooleanFunctionFactory* sFactory;

    };
}
