/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file VertexDecimation.hpp
 * @brief Vertex decimation algorithm implementation.
 *
 * 
 * Algorithm is based on "Decimation of Triangle Meshes" by William J. Schroeder et al.
 * at https://www.cs.columbia.edu/~allen/PHOTOPAPERS/Schroeder-etal-sg92.pdf
 *
 * This file implements modified version of the algortihm, partly inspired by the Vertex decimation
 * in the VTK library at https://github.com/Kitware/VTK/blob/master/Filters/Core/vtkDecimatePro.cxx.
 *
 * Kept from the original paper:
 *		- Classification of vertices
 *		- Error metrics based on classification
 *		- Concept of vertex removal, hole triangulation, local topology update
 *		- Feature preservation by not removing 'Complex' vertices implicitly or optionally locking border vertices
 *
 * Following parts of the algorithm are inspired by the VTK library implementation:
 *		- Finalizing classification using feature edge count
 *		- Using a priority queue to prevent multiple passes approach
 *
 * Different approaches to the topics in the original paper
 *		- Hole triangulation: originally "Recursive loop splitting" is used, I chose ear clipping instead
 *			- Coming out of: https://www.geometrictools.com/Documentation/TriangulationByEarClipping.pdf
 *			- For 2D projection, using the Newell's method find an average plane of the polygon and project on it
 *			- Ear is defined by three consecutive vertices in the loop, where any other vertex of the loop is not inside
 *			- Best ear is chosen based on (signed area) / (sum of squared edges) for better quality triangles after trianglutaion
 *		- Maintaining the error order using lazy priority queue instead of multiple passes
 *		- Directed edge graph for classification and getting order loop for triangulation
 */

#pragma once
#include <vector>
#include "../../resources/Model.hpp"
#include "../utils/Topology.hpp"
#include "../utils/Geometry.hpp"
#include "../utils/LazyPriorityQueue.hpp"

namespace VertexDecimation {

    enum class VertexClassification {
        Simple,
        Complex,
        Boundary,
        InteriorEdge,
        Corner,
        Undefined
    };

    struct VertexInfo {
        Topology::Neighborhood neighborhood;
        VertexClassification classification = VertexClassification::Undefined;
        bool isActive = true;
        std::vector<uint32_t> orderedLoop;
    };

    struct DecimationCandidate {
        uint32_t vertexIdx;
        double error;

        bool operator>(const DecimationCandidate& other) const {
            return error > other.error;
        }
    };

    struct DecimationCompare {
		bool operator()(const DecimationCandidate& a, const DecimationCandidate& b) const {
			return a.error > b.error;
		}
    };

    double computeVertexError(uint32_t vertexIdx, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, VertexInfo& info, CollapseOptions& options, std::vector<bool>& isLocked);
    VertexClassification classifyVertex(uint32_t vertexIdx, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, VertexInfo& info, CollapseOptions& options);

    std::vector<VertexInfo> computeVertexInfo(std::vector<uint32_t>& indices, size_t vertexCount);
    std::vector<uint32_t> triangulateHole(uint32_t vertexIdx, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, VertexInfo& info);

    void updateLocalTopology(
        std::vector<VertexInfo>& vertexInfo,
        LazyPriorityQueue<DecimationCandidate,
        DecimationCompare>& candidatesQueue,
        std::vector<uint32_t>& newTriangles,
        std::vector<uint32_t>& indices,
        uint32_t removedVertexIdx,
        std::vector<Vertex>& vertices,
        CollapseOptions& options,
        std::vector<bool>& isLocked
    );

}

 /* End of the VertexDecimation.hpp file */