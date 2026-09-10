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

#include "solve_fsm/configuration.h"

namespace hal
{
    namespace solve_fsm
    {
        Configuration::Configuration(Netlist* nl) : netlist(nl)
        {
        }

        Configuration& Configuration::with_state_register(const std::vector<Gate*>& state_register)
        {
            this->state_register = state_register;
            return *this;
        }

        Configuration& Configuration::with_transition_logic(const std::vector<Gate*>& transition_logic)
        {
            this->transition_logic = transition_logic;
            return *this;
        }

        Configuration& Configuration::with_outputs(const std::vector<std::pair<std::string, std::vector<Net*>>>& outputs)
        {
            this->outputs = outputs;
            return *this;
        }

        Configuration& Configuration::with_initial_state(const std::map<Gate*, bool>& initial_state)
        {
            this->initial_state = initial_state;
            return *this;
        }

        Configuration& Configuration::with_timeout(const u32 timeout)
        {
            this->timeout = timeout;
            return *this;
        }

        Configuration& Configuration::with_brute_force(const bool brute_force)
        {
            this->brute_force = brute_force;
            return *this;
        }
    }    // namespace solve_fsm
}    // namespace hal
