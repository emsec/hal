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

/**
 * @file centrality.h 
 * @brief This file contains functions related to centrality metrics in graphs.
 */

#pragma once

#include "graph_algorithm/netlist_graph.h"
#include "hal_core/defines.h"
#include "hal_core/utilities/result.h"

#include <igraph/igraph.h>
#include <set>

namespace hal
{
    class Gate;

    namespace graph_algorithm
    {
        /**
         * @brief Compute the harmonic centrality for a set of gates within a given graph.
         *
         * Harmonic centrality is a measure of centrality in a graph that is computed based on the inverse distances to all other vertices in the graph.
         * In this version, the input vertices are provided as `Gate*` pointers. The specified `direction` parameter
         * determines the edge direction to be considered when computing shortest paths.
         *
         * @param[in] graph - The netlist graph for which the harmonic centrality is to be computed.
         * @param[in] gates - The gates representing vertices in the graph for which centrality should be computed.
         * @param[in] direction - The direction of edges to consider (forward, backward, or both).
         * @param[in] cutoff - The maximum path length to consider during computation. Defaults to `-1` (no cutoff).
         * @returns OK() and a vector of centrality values (in the same order as the input gates) on success, an error otherwise.
         */
        Result<std::vector<double>> get_harmonic_centrality(const NetlistGraph* graph, const std::vector<Gate*>& gates, const NetlistGraph::Direction direction, const i32 cutoff = -1);

        /**
         * @brief Compute the harmonic centrality for a set of vertices within a given graph.
         *
         * Harmonic centrality is a measure of centrality in a graph that is computed based on the inverse distances to all other vertices in the graph.
         * In this version, the input vertices are provided as their vertex IDs. The specified `direction` parameter
         * determines the edge direction to be considered when computing shortest paths.
         *
         * @param[in] graph - The netlist graph for which the harmonic centrality is to be computed.
         * @param[in] vertices - The vertex IDs for which centrality should be computed.
         * @param[in] direction - The direction of edges to consider (forward, backward, or both).
         * @param[in] cutoff - The maximum path length to consider during computation. Defaults to `-1` (no cutoff).
         * @returns OK() and a vector of centrality values (in the same order as the input vertices) on success, an error otherwise.
         */
        Result<std::vector<double>> get_harmonic_centrality(const NetlistGraph* graph, const std::vector<u32>& vertices, const NetlistGraph::Direction direction, const i32 cutoff = -1);

        /**
         * @brief Compute the harmonic centrality for a set of vertices within a given graph.
         *
         * Harmonic centrality is a measure of centrality in a graph that is computed based on the inverse distances to all other vertices in the graph.
         * In this version, the input vertices are provided as an `igraph_vector_int_t`, which is typically used by the underlying igraph library.
         * The specified `direction` parameter determines the edge direction to be considered when computing shortest paths.
         *
         * @param[in] graph - The netlist graph for which the harmonic centrality is to be computed.
         * @param[in] vertices - A pointer to an `igraph_vector_int_t` containing the vertex IDs for which centrality should be computed.
         * @param[in] direction - The direction of edges to consider (forward, backward, or both).
         * @param[in] cutoff - The maximum path length to consider during computation. Defaults to `-1` (no cutoff).
         * @returns OK() and a vector of centrality values (in the same order as the input vertices) on success, an error otherwise.
         */
        Result<std::vector<double>> get_harmonic_centrality(const NetlistGraph* graph, const igraph_vector_int_t* vertices, const NetlistGraph::Direction direction, const i32 cutoff = -1);

        /**
         * @brief Compute the betweenness centrality for a set of gates within a given graph.
         *
         * Betweenness centrality is a measure of centrality in a graph that is based on the number of shortest paths passing through a vertex.
         * In this version, the input vertices are provided as `Gate*` pointers. The `directed` parameter determines whether edges
         * are considered as directed or undirected when computing shortest paths.
         *
         * @param[in] graph - The netlist graph for which the betweenness centrality is to be computed.
         * @param[in] gates - The gates representing vertices in the graph for which centrality should be computed.
         * @param[in] directed - If `true`, consider the graph directed; if `false`, consider it undirected.
         * @param[in] cutoff - The maximum path length to consider during computation. Defaults to `-1` (no cutoff).
         * @returns OK() and a vector of centrality values (in the same order as the input gates) on success, an error otherwise.
         */
        Result<std::vector<double>> get_betweenness_centrality(const NetlistGraph* graph, const std::vector<Gate*>& gates, const bool directed, const i32 cutoff = -1);

        /**
         * @brief Compute the betweenness centrality for a set of vertices within a given graph.
         *
         * Betweenness centrality is a measure of centrality in a graph that is based on the number of shortest paths passing through a vertex.
         * In this version, the input vertices are provided as their vertex IDs. The `directed` parameter determines whether edges
         * are considered as directed or undirected.
         *
         * @param[in] graph - The netlist graph for which the betweenness centrality is to be computed.
         * @param[in] vertices - The vertex IDs for which centrality should be computed.
         * @param[in] directed - If `true`, consider the graph directed; if `false`, consider it undirected.
         * @param[in] cutoff - The maximum path length to consider during computation. Defaults to `-1` (no cutoff).
         * @returns OK() and a vector of centrality values (in the same order as the input vertices) on success, an error otherwise.
         */
        Result<std::vector<double>> get_betweenness_centrality(const NetlistGraph* graph, const std::vector<u32>& vertices, const bool directed, const i32 cutoff = -1);

        /**
         * @brief Compute the betweenness centrality for a set of vertices within a given graph.
         *
         * Betweenness centrality is a measure of centrality in a graph that is based on the number of shortest paths passing through a vertex.
         * In this version, the input vertices are provided as an `igraph_vector_int_t`, which is typically used by the underlying igraph library.
         * The `directed` parameter determines whether edges are considered as directed or undirected.
         *
         * @param[in] graph - The netlist graph for which the betweenness centrality is to be computed.
         * @param[in] vertices - A pointer to an `igraph_vector_int_t` containing the vertex IDs for which centrality should be computed.
         * @param[in] directed - If `true`, consider the graph directed; if `false`, consider it undirected.
         * @param[in] cutoff - The maximum path length to consider during computation. Defaults to `-1` (no cutoff).
         * @returns OK() and a vector of centrality values (in the same order as the input vertices) on success, an error otherwise.
         */
        Result<std::vector<double>> get_betweenness_centrality(const NetlistGraph* graph, const igraph_vector_int_t* vertices, const bool directed, const i32 cutoff = -1);

    }    // namespace graph_algorithm
}    // namespace hal