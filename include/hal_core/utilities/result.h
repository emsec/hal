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
#include "hal_core/utilities/error.h"

#include <variant>

namespace hal
{
    template<typename T>
    class Result final
    {
    public:
        Result(const T& value)
        {
            m_has_value = true;
            m_content   = value;
        }

        Result(const Error& error) : std::variant<Error, T>(error)
        {
            m_content = error;
        }

        static const auto Success = Result(std::monostate);

        bool is_valid() const
        {
            return std::holds_alternative<T>(*this);
        }

        bool is_error() const
        {
            return !is_valid();
        }

        bool has_value() const
        {
            return m_has_value;
        }

        T get() const
        {
            return (has_value() ? std::get<T>(*this) : T());
        }

        Error get_error() const
        {
            return (is_error() ? std::get<Error>(*this) : Error());
        }

    private:
        bool m_has_value = false;
        std::variant<Error, T> m_content;

        Result() = default;
    };
}    // namespace hal