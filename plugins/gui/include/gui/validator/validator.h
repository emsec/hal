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
namespace hal
{
    /**
     * @ingroup gui
     * @brief Base class for any string validator.
     *
     * A <i>Validator</i> can be used to check if a string meets certain requirements. It is intended to be used in
     * and input dialog to dynamically validate the input string of the user. If the input is not considered valid
     * and the validation fails the Validator provides a 'fail text' that can be inserted in the input dialog widget
     * to provide further information about the reason why the validation failed.
     */
    class Validator
    {
        public:
            /**
             * Given any input string this function returns true if the Validator considers this string as valid.
             * It returns false otherwise.
             *
             * @param input - The input string
             * @returns <b>true</b> iff the string is considered valid
             */
            virtual bool validate(const QString &input) = 0;

            /**
             * Configures the fail text that describes the reason for a failed validation. It can be accessed via
             * Validator::failText.
             *
             * @param text - The new fail text
             */
            void setFailText(const QString& text);

            /**
             * Accesses the fail text configured via Validator::setFailText (or a default fail text if no text was
             * configured).
             *
             * @returns the fail text
             */
            QString failText() const;

        protected:
            QString mFailText = "Chosen input is not allowed.";
    };
}
