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

namespace QEM {

    struct Quadric {
        double q11, q12, q13, q14;
        double      q22, q23, q24;
        double           q33, q34;
        double                q44;

        Quadric() : q11(0), q12(0), q13(0), q14(0),
            q22(0), q23(0), q24(0),
            q33(0), q34(0),
            q44(1) {}

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

    std::vector<Quadric> initQuadrics(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
    Quadric createQuadricFromTriangle(glm::vec3& v1, glm::vec3& v2, glm::vec3& v3);
    glm::vec3 computeOptPos(Quadric& q1, Quadric& q2, glm::vec3& v1, glm::vec3& v2, double& outErr);

    std::vector<Qedge> createQedges(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, std::vector<Quadric>& quadrics, std::vector<bool>& isBorderVertex);
    bool getValidMinEdge(std::vector<Qedge>& edges, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, Qedge& outEdge);

    void collapseQedge(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, std::vector<Quadric>& quadrics, Qedge& edge, std::vector<std::vector<uint32_t>>& twinMap, bool syncUVSeams, std::vector<bool>& vertexDeleted);
    void updateAfterCollapse(std::vector<Qedge>& edges, uint32_t idxToRemove, uint32_t idxToKeep, std::vector<Vertex>& vertices, std::vector<Quadric>& quadrics, std::vector<bool>& isBorderVertex);

}

 /* End of the QEM.hpp file */