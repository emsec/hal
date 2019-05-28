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

#ifndef __HAL_GATE_DECORATOR_BDD_H__
#define __HAL_GATE_DECORATOR_BDD_H__

#include "def.h"

#include "../gate_decorator.h"

#include <functional>
#include <map>
#include <memory>
#include <vector>

#include <hal_bdd.h>
#include <kernel.h>


/**
 * @ingroup gate_decorator_system
 */
class gate_decorator_bdd : public gate_decorator, public std::enable_shared_from_this<gate_decorator_bdd>
{
public:
    /**
     * Decorator constructor.
     *
     * @param[in] g - the gate to decorate.
     * @param[in] generator - the generator capable of creating the output bdd of the gate.
    */
    gate_decorator_bdd(const std::shared_ptr<gate> g, gate_decorator_system::bdd_decorator_generator generator);

    ~gate_decorator_bdd() = default;

    /**
     * Get the type of the decorator.
     *
     * @returns The type.
     */
    gate_decorator_system::decorator_type get_type() override;

    /**
     * returns the map of bdds for each output pin type
     *
     * @returns a map from (output pin) to (bdd).
     */
    std::map<std::string, std::shared_ptr<bdd>> get_bdd();

    /**
     * returns the map of bdds for each output pin type with user-defined bdd variable identifiers
     *
     * @param[in] input_pin_type_to_bdd_identifier - the specified variable identifiers
     * @returns a map from (output pin) to (bdd).
     */
    std::map<std::string, std::shared_ptr<bdd>> get_bdd(std::map<std::string, int> input_pin_type_to_bdd_identifier);

    /**
     * returns the map of bdds for each output pin type with user-defined bdd variable identifiers or identifiers for the input pin types
     *
     * @param[in] input_pin_type_to_bdd_or_identifier - the specified variable identifiers or identifiers for the input pin types
     * @returns a map from (output pin) to (bdd).
     */
    std::map<std::string, std::shared_ptr<bdd>> get_bdd(std::map<std::string, std::tuple<std::shared_ptr<bdd>, int>> input_pin_type_to_bdd_or_identifier);

    /**
     * returns the map of bdds for each output pin type with user-defined bdd variables or identifiers for the input pin types
     *
     * @param[in] input_pin_type_to_bdd - the specified variables or identifiers for the input pin types
     * @returns a map from (output pin) to (bdd).
     */
    std::map<std::string, std::shared_ptr<bdd>> get_bdd(std::map<std::string, std::shared_ptr<bdd>> input_pin_type_to_bdd);

    /**
     * Checks whether a bdd is always true.
     *
     * @param[in] bdd_ptr - the bdd
     * @returns the result
     */
    static bool is_tautology(std::shared_ptr<bdd> bdd_ptr);

    /**
     * Checks whether a bdd is always false.
     *
     * @param[in] bdd_ptr - the bdd
     * @returns the result
     */
    static bool is_contradiction(std::shared_ptr<bdd> bdd_ptr);

    /**
     * Creates a human-readable string for a bdd.
     *
     * @param[in] bdd_ptr - the bdd to represent.
     * @returns the string representation.
     */
    static std::string get_bdd_str(std::shared_ptr<bdd> bdd_ptr);

    /**
     *  Turn the bdd into a vector of clauses.
     *
     * @param[in] bdd_ptr - the bdd
     * @returns a vector of maps from input to boolean value
     */
    static std::vector<std::map<int, bool>> get_bdd_clauses(std::shared_ptr<bdd> bdd_ptr);

    /**
     *  Evaluate a given bdd under a certain configuration
     *
     * @param[in] bdd_ptr - the bdd
     * @param[in] input_configuration - configuration of the variables
     * @returns evaluation of the bdd as bool
     */
    static bool evaluate_bdd(std::shared_ptr<bdd> bdd_ptr, const std::map<int, bool>& input_configuration);

    /**
     *  Evaluate a given bdd under a certain configuration
     *
     * @param[in] g - gate, bdd_ptr - the bdd, input_configuration - configuration of the variables
     * @param[in] bdd_ptr - the bdd
     * @param[in] input_configuration - configuration of the variables
     * @returns evaluation of the bdd as bool
     */
    static bool evaluate_bdd(std::shared_ptr<gate> const g, std::shared_ptr<bdd> const bdd_ptr, const std::map<std::string, bool>& input_configuration);

    /**
     *  Evaluate a given bdd under a certain configuration
     *
     * @param[in] clauses - bdd clauses as a vector, input_configuration - configuration of the variables
     * @param[in] input_configuration - configuration of the variables
     * @returns evaluation of the bdd as bool
     */
    static bool evaluate_bdd(const std::vector<std::map<int, bool>>& clauses, const std::map<int, bool>& input_configuration);

    /**
     *  Returns truth for a given function
     *
     * @param[in] bdd_ptr - the bdd
     * @returns truth table
     */
    static std::tuple<std::vector<int>, std::vector<bool>> get_truth_table(std::shared_ptr<bdd> const bdd_ptr);

private:
    gate_decorator_system::bdd_decorator_generator m_generator;
};

#endif /* __HAL_GATE_DECORATOR_BDD_H__ */
