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

#include "gui/validator/validator.h"

#include <QList>

namespace hal
{
    /**
     * The StackedValidator is a Validator that combines multiple other Validators. Its validate function returns
     * <b>true</b> iff all added Validators accept the string. <br>
     * The fail text will be chosen from the firstly added validator which validation fails.
     */
    class StackedValidator : public Validator
    {
        public:
            /**
             * Constructor.
             * At this point the StackedValidator will accept any string.
             * Therefore one should add validators by calling addValidator.
             */
            StackedValidator();

            /**
             * TODO: Refactor parameter name.
             * Adds a Validator to the StackedValidator.
             *
             * @param Validator - The Validator to add
             */
            void addValidator(Validator* Validator);

            /**
             * TODO: Refactor parameter name.
             * Removes a Validator from the StackedValidator.
             *
             * @param Validator - The Validator to remove
             */
            void removeValidator(Validator* Validator);

            /**
             * Removes all Validators from the StackedValidator.
             */
            void clearValidators();

            /**
             * Given any string this function returns <b>true</b> iff the string is accepted by all registered Validators
             * (added by StackedValidator:addValidator).
             *
             * If no Validator is registered this function always returns true.
             * If one or more Validators reject the string, the fail text of the failing Validator that was added the
             * earliest will be chosen.
             *
             * @param input - The input string
             * @returns <b>true</b> iff the string is considered valid (accepted by all validators)
             */
            bool validate(const QString &input);

        private:
            QList<Validator*> mValidators;
    };
}
