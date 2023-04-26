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
        class ER
        {
        };
    }    // namespace result_constructor_type

#define OK(...)                                    \
    {                                              \
        result_constructor_type::OK(), __VA_ARGS__ \
    }
#define ERR(message)                                                      \
    {                                                                     \
        result_constructor_type::ER(), Error(__FILE__, __LINE__, message) \
    }
#define ERR_APPEND(prev_error, message)                                               \
    {                                                                                 \
        result_constructor_type::ER(), Error(__FILE__, __LINE__, prev_error, message) \
    }

    template<typename T>
    class [[nodiscard]] Result final
    {
    public:
        //// cannot be instantiated with Error type
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
        Result(result_constructor_type::OK, std::remove_reference_t<T>&& value) : m_result(std::move(value))
        {
        }

        Result(result_constructor_type::ER, const Error& error) : m_result(error)
        {
        }

        /**
         * Construct a valid result carrying a result value.
         * 
         * @param[in] value - The result value.
         * @return The result.
         */
        template<typename... Args, typename U = T, typename std::enable_if_t<!std::is_same_v<U, void>, int> = 0>
        static Result<T> Ok(const T& value)
        {
            return OK(value);
        }

        /**
         * @copydoc Ok(const T&)
         */
        template<typename... Args, typename U = T, typename std::enable_if_t<!std::is_same_v<U, void>, int> = 0>
        static Result<T> Ok(std::remove_reference_t<T>&& value)
        {
            return OK(std::move(value));
        }

        /**
         * Check whether two results are equal.
         *
         * @param[in] other - The result to compare against.
         * @returns True if both results are equal, false otherwise.
         */
        bool operator==(const Result<T>& other)
        {
            return this->m_result == other.m_result;
        }

        /**
         * Check whether two results are unequal.
         *
         * @param[in] other - The result to compare against.
         * @returns True if both results are unequal, false otherwise.
         */
        bool operator!=(const Result<T>& other)
        {
            return !(*this == other);
        }

        /**
         * Check whether the result is valid, i.e., not an error.
         * 
         * @returns `true` if the result is valid, `false` otherwise.
         */
        bool is_ok() const
        {
            return !is_error();
        }

        /**
         * Check whether the result is an error.
         * 
         * @returns `true` if the result is an error, `false` otherwise.
         */
        bool is_error() const
        {
            return std::holds_alternative<Error>(m_result);
        }

        /**
         * Get the value of the result.
         * It is up to the user to check whether the result is valid using `Result::is_ok()` beforehand.
         * 
         * @tparam U - The type of the result value.
         * @returns The result value.
         */
        template<typename U = T, typename std::enable_if_t<!std::is_same_v<U, void>, int> = 0>
        const T& get() const
        {
            return std::get<T>(m_result);
        }

        /**
         * Get the value of the result.
         * It is up to the user to check whether the result is valid using `Result::is_ok()` beforehand.
         * 
         * @tparam U - The type of the result value.
         * @returns The result value.
         */
        template<typename U = T, typename std::enable_if_t<!std::is_same_v<U, void>, int> = 0>
        T&& get()
        {
            return std::get<T>(std::move(m_result));
        }

        /**
         * Get the error of the result.
         * It is up to the user to check whether the result is an error using `Result::is_error()` beforehand.
         * 
         * @returns The error.
         */
        const Error& get_error() const
        {
            return std::get<Error>(m_result);
        }

        /**
         * Get the error of the result.
         * It is up to the user to check whether the result is an error using `Result::is_error()` beforehand.
         * 
         * @returns The error.
         */
        Error&& get_error()
        {
            return std::get<Error>(std::move(m_result));
        }

        /**
         * Map the result to a different user-defined data type using the provided mapping function.
         * 
         * @tparam U - The target data type.
         * @param[in] f - The mapping function.
         * @returns The result mapped to the target data type.
         */
        template<typename U, typename std::enable_if_t<!std::is_same_v<U, void>, int> = 0>
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

        /**
          * Map the result to a different user-defined data type using the provided mapping function.
          * 
          * @tparam U - The target data type.
          * @param[in] f - The mapping function.
          * @returns The result mapped to the target data type.
          */
        template<typename U, typename std::enable_if_t<!std::is_same_v<U, void>, int> = 0>
        Result<U> map(const std::function<Result<U>(T&&)>& f)
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