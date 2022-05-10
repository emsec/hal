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

#include "hal_core/defines.h"

#include <QString>

namespace hal
{
    class PyCodeProvider
    {

    public:

        static QString pyCodeGate(u32 gateId);
        static QString pyCodeGateId(u32 gateId);
        static QString pyCodeGateName(u32 gateId);
        static QString pyCodeGateType(u32 gateId);
        static QString pyCodeGateTypePinDirection(u32 gateID, QString pin);
        static QString pyCodeGateTypePinType(u32 gateID, QString pin);
        static QString pyCodeProperties(u32 gateId);
        static QString pyCodeGateLocation(u32 gateId);
        static QString pyCodeGateModule(u32 gateId);
        static QString pyCodeGateBooleanFunction(u32 gateId, QString booleanFunctionName);
        static QString pyCodeGateAsyncSetResetBehavior(u32 gateId);
        static QString pyCodeGateData(u32 gateId, QString category, QString key);
        static QString pyCodeGateDataMap(u32 gateId);

        static QString pyCodeStateComp(u32 gateId);
        static QString pyCodeStateCompPosState(u32 gateId);
        static QString pyCodeStateCompNegState(u32 gateId);

        static QString pyCodeFFComp(u32 gateId);
        static QString pyCodeFFCompClockFunc(u32 gateId);
        static QString pyCodeFFCompNextStateFunc(u32 gateId);
        static QString pyCodeFFCompAsyncSetFunc(u32 gateId);
        static QString pyCodeFFCompAsyncResetFunc(u32 gateId);
        static QString pyCodeFFCompSetResetBehav(u32 gateId);

        static QString pyCodeLatchComp(u32 gateId);
        static QString pyCodeLatchCompEnableFunc(u32 gateId);
        static QString pyCodeLatchCompDataInFunc(u32 gateId);
        static QString pyCodeLatchCompAsyncSetFunc(u32 gateId);
        static QString pyCodeLatchCompAsyncResetFunc(u32 gateId);
        static QString pyCodeLatchCompSetResetBehav(u32 gateId);


        static QString pyCodeNet(u32 netId);
        static QString pyCodeNetId(u32 netId);
        static QString pyCodeNetName(u32 netId);
        static QString pyCodeNetType(u32 netId);
        static QString pyCodeNetData(u32 netId, QString category, QString key);
        static QString pyCodeNetDataMap(u32 netId);

        static QString pyCodeModule(u32 moduleId);
        static QString pyCodeModuleId(u32 moduleId);
        static QString pyCodeModuleName(u32 moduleId);
        static QString pyCodeModuleType(u32 moduleId);
        static QString pyCodeModuleModule(u32 moduleId);
        static QString pyCodeModuleSubmodules(u32 moduleId);
        static QString pyCodeModuleNets(u32 moduleId);
        static QString pyCodeModuleInputNets(u32 moduleId);
        static QString pyCodeModuleOutputNets(u32 moduleId);
        static QString pyCodeModuleInternalNets(u32 moduleId);
        static QString pyCodeModuleIsTopModule(u32 moduleId);
        static QString pyCodeModuleData(u32 moduleId, QString category, QString key);
        static QString pyCodeModuleDataMap(u32 moduleId);
        static QString pyCodeModulePinGroup(u32 moduleId, QString groupName);
        static QString pyCodeModulePinGroups(u32 moduleId);
        static QString pyCodeModulePinGroupName(u32 moduleId, QString groupName);
        static QString pyCodeModulePinByName(u32 moduleId, QString pinName);
        static QString pyCodeModulePinName(u32 moduleId, QString pinName);
        static QString pyCodeModulePinDirection(u32 moduleId, QString pinName);
        static QString pyCodeModulePinType(u32 moduleId, QString pinName);
        static QString pyCodeModulePins(u32 moduleId);

        static QString pyCodeGrouping(u32 groupingId);
        static QString pyCodeGroupingName(u32 groupingId);
        static QString pyCodeGroupingId(u32 groupingId);

    private:
        static QString buildPyCode(const QString& prefix, const QString& suffix, u32 id);

        static const QString gateCodePrefix;
        static const QString netCodePrefix;
        static const QString moduleCodePrefix;
        static const QString groupingCodePrefix;

    };
}
