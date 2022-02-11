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

#include <functional>
#include <type_traits>
#include <variant>

namespace hal
{
    namespace result_constructor_type
    {
        // Constructor type for macro: OK()
        class OK
        {
        };
        // Constructor type for macro: ERROR()
        class ERR
        {
        };
    }    // namespace result_constructor_type

#define OK(...)                                    \
    {                                              \
        result_constructor_type::OK(), __VA_ARGS__ \
    }
#define ERR(message)                                     \
    {                                                    \
        result_constructor_type::ERR(), Error(message) \
    }

    template<typename T>
    class [[nodiscard]] Result final
    {
    public:
        static_assert(!std::is_same<T, Error>(), "Cannot initialize a Result<Error>.");

        template<typename... Args, typename U = T, typename std::enable_if_t<std::is_same_v<U, void>, int> = 0>
        Result(result_constructor_type::OK) : m_result()
        {
        }

        template<typename... Args, typename U = T, typename std::enable_if_t<!std::is_same_v<U, void>, int> = 0>
        Result(result_constructor_type::OK, const T& value) : m_result(value)
        {
        }

        template<typename... Args, typename U = T, typename std::enable_if_t<!std::is_same_v<U, void>, int> = 0>
        Result(result_constructor_type::OK, T&& value) : m_result(std::move(value))
        {
        }

        Result(result_constructor_type::ERR, const Error& error)
        {
            m_result = error;
        }

        template<typename... Args, typename U = T, typename std::enable_if_t<!std::is_same_v<U, void>, int> = 0>
        static Result<T> Ok(const T& value)
        {
            return OK(value);
        }

        template<typename... Args, typename U = T, typename std::enable_if_t<!std::is_same_v<U, void>, int> = 0>
        static Result<T> Ok(T&& value)
        {
            return OK(std::move(value));
        }

        bool is_ok() const
        {
            return !is_error();
        }

        bool is_error() const
        {
            return std::holds_alternative<Error>(m_result);
        }

        template<typename U = T, typename std::enable_if_t<!std::is_same_v<U, void>, int> = 0>
        const T& get() const
        {
            return std::get<T>(m_result);
        }

        template<typename U = T, typename std::enable_if_t<!std::is_same_v<U, void>, int> = 0>
        T&& get() 
        {
            return std::get<T>(std::move(m_result));
        }

        Error get_error() const
        {
            return std::get<Error>(m_result);
        }

        template<typename U = T, typename std::enable_if_t<!std::is_same_v<U, void>, int> = 0>
        Result<U> map(const std::function<Result<U>(const T&)>& f) const
        {
            if (this->is_ok())
            {
                return f(this->get());
            }
            else
            {
                return ERR(this->get_error());
            }
        }

    private:
        std::variant<T, Error> m_result;
    };
}    // namespace hal