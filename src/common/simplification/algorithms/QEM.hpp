/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file QEM.hpp
 * @brief Quadric error metrics implementation.
 *
 * This file contains ...
 */

#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <cstdint>
#include <algorithm>
#include <set>
#include "../../resources/Model.hpp"
#include "../utils/Topology.hpp"
#include "../utils/Geometry.hpp"
#include "../utils/LazyPriorityQueue.hpp"

namespace QEM {
    
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

    struct Qedge {
        uint32_t v1, v2;
        glm::vec3 optimalPos;
        double error;

        bool operator>(const Qedge& other) const {
            return error > other.error;
        }
    };

    struct QedgeCompare {
        bool operator()(const Qedge& a, const Qedge& b) const {
            return a.error > b.error;
        }
    };

    struct QEMContext {
        std::vector<Vertex>& vertices;
        std::vector<uint32_t>& indices;
        std::vector<Quadric>& quadrics;
        std::vector<bool>& vertexDeleted;
        std::vector<bool>& isLockedVertex;
        std::vector<std::vector<uint32_t>>& twinMap;
        std::vector<Topology::Neighborhood>& allNeighborhoods;
    };

    std::vector<Quadric> initQuadrics(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
    Quadric createQuadricFromTriangle(glm::vec3& v1, glm::vec3& v2, glm::vec3& v3);
    glm::vec3 computeOptPos(Quadric& q1, Quadric& q2, glm::vec3& v1, glm::vec3& v2, double& outErr);

    std::vector<Qedge> createQedges(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, std::vector<Quadric>& quadrics, std::vector<bool>& isBorderVertex);
    int syncSeamTwinsAfterCollapse(QEMContext& context, uint32_t keepIdx, uint32_t removeIdx, const glm::vec3& optimalPos);
    uint32_t collapseQedge(QEMContext& context, Qedge& edge, int& outDeletedFaces);

    bool isEdgeValidForCollapse(QEMContext& context, const QEM::Qedge& e, LazyPriorityQueue<QEM::Qedge, QEM::QedgeCompare>& qedgeQueue, CollapseOptions options);

    void enqueueAffectedEdges(QEMContext& context, uint32_t keepIdx, LazyPriorityQueue<QEM::Qedge, QEM::QedgeCompare>& qedgeQueue, CollapseOptions options);
}

 /* End of the QEM.hpp file */