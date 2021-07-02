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

#include "hal_core/defines.h"

#include <QString>

namespace hal
{
    class PyCodeProvider
    {

    public:

        static QString pyCodeGateName(u32 gateId);
        static QString pyCodeGateType(u32 gateId);
        static QString pyCodeProperties(u32 gateId);
        static QString pyCodeGateLocation(u32 gateId);
        static QString pyCodeGateModule(u32 gateId);
        static QString pyCodeGateBooleanFunction(u32 gateId, QString booleanFunctionName);
        static QString pyCodeGateClearPresetBehavior(u32 gateId);
        static QString pyCodeGateData(u32 gateId, QString category, QString key);

        static QString pyCodeNetName(u32 netId);
        static QString pyCodeNetType(u32 netId);
        static QString pyCodeNetData(u32 netId, QString category, QString key);

        static QString pyCodeModuleName(u32 moduleId);
        static QString pyCodeModuleType(u32 moduleId);
        static QString pyCodeModuleModule(u32 moduleId);
        static QString pyCodeModuleData(u32 moduleId, QString category, QString key);

    private:
        static QString buildPyCode(const QString& prefix, const QString& suffix, u32 id);

        static const QString gateCodePrefix;
        static const QString netCodePrefix;
        static const QString moduleCodePrefix;

    };
}
