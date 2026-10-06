// MIT License
//
// Copyright (c) 2019 Ruhr University Bochum, Chair for Embedded Security. All Rights reserved.
// Copyright (c) 2019 Marc Fyrbiak, Sebastian Wallat, Max Hoffmann ("ORIGINAL AUTHORS"). All rights reserved.
// Copyright (c) 2021 Max Planck Institute for Security and Privacy. All Rights reserved.
// Copyright (c) 2021 Jörn Langheinrich, Julian Speith, Nils Albartus, René Walendy, Simon Klix ("ORIGINAL AUTHORS"). All Rights reserved.
// Copyright (c) 2025-2026 Sascha Tommasone. All rights reserved.
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

#include "clock_tree_extractor/branches.h"

#include <algorithm>
#include <igraph/igraph.h>
#include <utility>
#include <variant>

namespace hal
{
    namespace cte
    {
        namespace
        {
            Result<std::monostate> recover_branches_for_root( const ClockTree &clock_tree,
                                                              const std::pair<const void *, PtrType> node,
                                                              Branch &branch,
                                                              std::vector<Branch> &branches )
            {
                branch.push_back( node );

                const auto children = clock_tree.get_neighbors( node.first, IGRAPH_OUT );
                if( children.is_error() )
                {
                    return ERR( children.get_error().get() );
                }

                if( children.get().empty() )
                {
                    branches.push_back( branch );
                }
                else
                {
                    for( const auto &child : children.get() )
                    {
                        const auto res = recover_branches_for_root( clock_tree, child, branch, branches );
                        if( res.is_error() )
                        {
                            return ERR( res.get_error().get() );
                        }
                    }
                }

                branch.pop_back();
                return OK( {} );
            }
        }  // namespace

        Result<std::vector<Branch>> recover_branches( const ClockTree &clock_tree )
        {
            Branch branch;
            std::vector<Branch> branches;

            for( const igraph_int_t root : clock_tree.get_roots() )
            {
                const auto root_ptr = clock_tree.get_ptr_from_vertex( root );
                if( root_ptr.is_error() )
                {
                    return ERR( root_ptr.get_error().get() );
                }

                const auto result = recover_branches_for_root( clock_tree, root_ptr.get(), branch, branches );
                if( result.is_error() )
                {
                    return ERR( result.get_error().get() );
                }
            }

            return OK( std::move( branches ) );
        }
    }  // namespace cte
}  // namespace hal
