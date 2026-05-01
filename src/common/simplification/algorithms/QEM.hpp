/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file QEM.hpp
 * @brief Quadric error metrics implementation.
 *
 * This file contains the imeplemmentation of the QEM simplification algorithm, including
 * the definition of quadrics, quadric edges meant to be collapsed and overall functions
 * for initializing quadrics, creating quadric edges, collapsing edges and updating the mesh.
 * 
 * =======================================================================================
 * 
 * Inspirations and sources:
 * 	- The overall algorithm and mathematical definitions of quadrics
 *      - "Surface simplification using quadric error metrics." by Michael Garland and Paul Heckbert
 *          - @url https://www.cs.cmu.edu/~garland/Papers/quadrics.pdf
 *      - "Simplifying surfaces with color and texture using quadric error metrics" by Michael Garland and Paul Heckbert
 *          - @url https://www.cs.cmu.edu/~garland/Papers/quadric2.pdf
 *      - "Quadric-Based Polygonal Surface Simplification" by Michael Garland
 *          - @url https://www.cs.cmu.edu/~garland/thesis/thesis-onscreen.pdf
 *  - Edge collapse
 *      - "Mesh Optimization" by Hugues Hoppe
 *          - @url https://hhoppe.com/meshopt.pdf
 *		- Validity of edge collapse
 *			- @url [answer]https://stackoverflow.com/questions/27049163/mesh-simplification-edge-collapse-conditions
 * 
 * =======================================================================================
 */

#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <cstdint>
#include <algorithm>
#include <set>
#include "common/resources/Model.hpp"
#include "common/simplification/utils/Topology.hpp"
#include "common/simplification/utils/Geometry.hpp"
#include "common/simplification/utils/LazyPriorityQueue.hpp"

namespace QEM {
    
	// quadric definiton, a symmetric 4x4 matrix represented by its upper triangular part
    struct Quadric {
        double q11, q12, q13, q14;
        double      q22, q23, q24;
        double           q33, q34;
        double                q44;

        Quadric() : q11(0), q12(0), q13(0), q14(0),
            q22(0), q23(0), q24(0),
            q33(0), q34(0),
            q44(0) {}

        Quadric operator+(const Quadric& other) const;
        double evalError(const glm::vec3& v) const;
    };

	// edge meant to be collapsed, containing the optimal position to collapse to and the error of the collapse
    struct Qedge {
        uint32_t v1, v2;
        glm::vec3 optimalPos;
        double error;

        bool operator>(const Qedge& other) const {
            return error > other.error;
        }
    };

	// comparator for the priority queue of edges
    struct QedgeCompare {
        bool operator()(const Qedge& a, const Qedge& b) const {
            return a.error > b.error;
        }
    };

	// context struct to pass around necessary data during edge collapses and updates
    struct QEMContext {
        std::vector<Vertex>& vertices;
        std::vector<uint32_t>& indices;
        std::vector<Quadric>& quadrics;
        std::vector<bool>& vertexDeleted;
        std::vector<bool>& isLockedVertex;
        std::vector<std::vector<uint32_t>>& twinMap;
        std::vector<Topology::Neighborhood>& allNeighborhoods;
    };

    /**
	* @brief Initializes the quadrics for each vertex based on the input mesh.
    * 
	* @param vertices The vertices of the mesh
	* @param indices The triangle indices of the mesh
    * 
	* @return vector of quadrics corresponing to each vertex in the mesh
    */
    std::vector<Quadric> initQuadrics(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);

    /**
	* @brief Creates a quadric for a triangle defined by its three vertices.
    * 
	* @param v1 First vertex of the triangle
	* @param v2 Second vertex of the triangle
	* @param v3 Third vertex of the triangle
    * 
    * @return quadric structure representing the plane of the triangle
    */
    Quadric createQuadricFromTriangle(glm::vec3& v1, glm::vec3& v2, glm::vec3& v3);

    /**
	* @brief Computes the optimal position for collapsing an edge defined by two vertices and their quadrics.
    * 
	* @param q1 Quadric of the first vertex
	* @param q2 Quadric of the second vertex
	* @param v1 Position of the first vertex
	* @param v2 Position of the second vertex
	* @param outErr Output parameter to store the error of collapsing to the optimal position
    * 
    * @return optimal position to collapse the edge to
    */
    glm::vec3 computeOptPos(Quadric& q1, Quadric& q2, glm::vec3& v1, glm::vec3& v2, double& outErr);

    /**
	* @brief Creates a list of unique quadric edges in the mesh. Calculates optimal collapse positions and errors.
    * 
	* @param vertices The vertices of the mesh
	* @param indices The triangle indices of the mesh
	* @param quadrics The quadrics corresponding to each vertex
	* @param isBorderVertex Vector indicating which vertices are on the border of the mesh
    * 
	* @return vector of unique quadric edges with their optimal collapse positions and errors
    */
    std::vector<Qedge> createQedges(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, std::vector<Quadric>& quadrics, std::vector<bool>& isBorderVertex);
    
    /**
	* @brief Synchronizes twin tracking after collapsing an edge. Updates the twin map to reflect the new vertex index and optimal position.
    * 
	* @param context The QEM context structure
	* @param keepIdx The index of the vertex that is kept after the collapse
	* @param removeIdx The index of the vertex that is removed after the collapse
	* @param optimalPos The optimal position that the kept vertex is moved to after the collapse
    * 
	* @return number of deleted twin faces that were collapsed due to the synchronization
    */
    int syncSeamTwinsAfterCollapse(QEMContext& context, uint32_t keepIdx, uint32_t removeIdx, const glm::vec3& optimalPos);
    
    /**
    * @brief Collapses a quadric edge by merging one vertex to another and then setting this vertex's positon to optimal position.
	*        Updated the neighborhood of affected vertices. Sets the removed vertex as deleted and updates the quadric of the kept vertex.
    * 
	* @param context The QEM context structure
	* @param edge The quadric edge to be collapsed
	* @param outDeletedFaces Output parameter to store the number of faces that were deleted due to the collapse
    * 
	* @return index of the vertex that was kept after the collapse
    */
    uint32_t collapseQedge(QEMContext& context, Qedge& edge, int& outDeletedFaces);

    /**
    * @brief Checks if a quadric edge is valid for collapse. Face flipping and mesh connectivity checks.
    * 
	* @param context The QEM context structure
	* @param e The quadric edge to be checked
	* @param qedgeQueue The priority queue of quadric edges, used for checking if the edge is still valid in the queue
	* @param options The collapse options
    * 
	* @return true if the edge is valid for collapse, false otherwise
    */
    bool isEdgeValidForCollapse(QEMContext& context, const QEM::Qedge& e, LazyPriorityQueue<QEM::Qedge, QEM::QedgeCompare>& qedgeQueue, SimplificationOptions options);

    /**
	* @brief Enqueues the edges affected by the collapse. After collapsing an edge, the quadrics
    *        of the affected vertices are updated, and the edges connected to these vertices need
    *        to be enqueued to the priority queue with new optimal position and error.
    * 
	* @param context The QEM context structure
	* @param keepIdx The index of the vertex that was kept after the collapse
	* @param qedgeQueue The priority queue of quadric edges, to which the affected edges will be enqueued
	* @param options The collapse options
    */
    void enqueueAffectedEdges(QEMContext& context, uint32_t keepIdx, LazyPriorityQueue<QEM::Qedge, QEM::QedgeCompare>& qedgeQueue, SimplificationOptions options);
}

 /* End of the QEM.hpp file */