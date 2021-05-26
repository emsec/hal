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

#include "gui/validator/validator.h"

#include <QStringList>

namespace hal
{
    /**
     * @ingroup gui
     * @brief Checks if a given string is unique compared to other given strings.
     *
     * The UniqueStringValidator is a Validator which validate function returns <b>true</b> if the candidate string does not
     * appear in the list of unique strings passed in the constructor.
     * It should be used whenever an input must not contain an already used identifier.
     */
    class UniqueStringValidator : public Validator
    {
        public:
            /**
             * Constructor.
             *
             * @param unique_strings - The black list of strings that will cause the validate function to return <b>false</b>
             */
            UniqueStringValidator(const QStringList &unique_strings);

            /**
             * Given any string this function returns <b>true</b> iff the string does not appear in the unique_string
             * list passed to the constructor.
             *
             * @param input - The input string
             * @returns <b>true</b> iff the string is considered valid (unique)
             */
            bool validate(const QString &input);

        private:
            const QStringList &mUniqueStrings;
    };
}
