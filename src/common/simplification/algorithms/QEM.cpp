/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file QEM.cpp
 * @brief Quadric error metrics implementation.
 *
 * This file contains ...
 */

#include "QEM.hpp"

namespace QEM {

	double Quadric::evalError(const glm::vec3& v) const
	{
		double x = v.x;
		double y = v.y;
		double z = v.z;

		// evaluate v^T * Q * v
		// via https://www.cs.cmu.edu/~garland/Papers/quadrics.pdf [page 3 footnote]
		double error = q11 * x * x + 2 * q12 * x * y + 2 * q13 * x * z + 2 * q14 * x +
										 q22 * y * y + 2 * q23 * y * z + 2 * q24 * y +
														   q33 * z * z + 2 * q34 * z +
																			 q44;

		return error;
	}

	Quadric Quadric::operator+(const Quadric& other) const
	{
		Quadric result;
		result.q11 = q11 + other.q11;
		result.q12 = q12 + other.q12;
		result.q13 = q13 + other.q13;
		result.q14 = q14 + other.q14;

		result.q22 = q22 + other.q22;
		result.q23 = q23 + other.q23;
		result.q24 = q24 + other.q24;

		result.q33 = q33 + other.q33;
		result.q34 = q34 + other.q34;

		result.q44 = q44 + other.q44;

		return result;
	}

	std::vector<Quadric> initQuadrics(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
	{
		std::vector<Quadric> quadrics(vertices.size());

		// for each triangle
		for (int i = 0; i < indices.size(); i += 3)
		{
			uint32_t i1 = indices[i];
			uint32_t i2 = indices[i + 1];
			uint32_t i3 = indices[i + 2];

			glm::vec3& v1 = vertices[i1].pos;
			glm::vec3& v2 = vertices[i2].pos;
			glm::vec3& v3 = vertices[i3].pos;

			Quadric q = createQuadricFromTriangle(v1, v2, v3);

			quadrics[i1] = quadrics[i1] + q;
			quadrics[i2] = quadrics[i2] + q;
			quadrics[i3] = quadrics[i3] + q;
		}

		return quadrics;
	}

	Quadric createQuadricFromTriangle(glm::vec3& v1, glm::vec3& v2, glm::vec3& v3)
	{
		// get plane normal
		glm::vec3 n = glm::normalize(glm::cross(v2 - v1, v3 - v1));
		double a = n.x, b = n.y, c = n.z;
		double d = -glm::dot(n, v1);

		Quadric q;
		q.q11 = a * a;	q.q12 = a * b;	q.q13 = a * c;	q.q14 = a * d;
		q.q22 = b * b;	q.q23 = b * c;	q.q24 = b * d;
		q.q33 = c * c;	q.q34 = c * d;
		q.q44 = d * d;

		return q;
	}

	glm::vec3 computeOptPos(Quadric& q1, Quadric& q2, glm::vec3& v1, glm::vec3& v2, double& outErr)
	{
		Quadric q = q1 + q2;

		// OPTIMAL POSITION - solve for v that minimizes v^T * Q * v
		// since Q is symmetric, we can use the simplified form
		glm::mat3 MAT(
			q.q11, q.q12, q.q13,
			q.q12, q.q22, q.q23,
			q.q13, q.q23, q.q33
		);

		// if determinant is non-zero, we can find optimal position by solving the linear system
		float det = glm::determinant(MAT);
		if (std::abs(det) > 1e-6)
		{
			// vector b is the negation of the last column of Q
			glm::vec3 b(q.q14, q.q24, q.q34);

			// solve for v in MAT * v = -b
			glm::vec3 exactOptPos = -glm::inverse(MAT) * b;

			outErr = q.evalError(exactOptPos);
			return exactOptPos;
		}

		// FALLBACK - choose between v1, v2 and midpoint
		glm::vec3 middle = 0.5f * (v1 + v2);

		// min error wins
		double v1Err = q.evalError(v1);
		double v2Err = q.evalError(v2);
		double midErr = q.evalError(middle);

		if (v1Err <= v2Err && v1Err <= midErr)
		{
			outErr = v1Err;
			return v1;
		}
		else if (v2Err <= v1Err && v2Err <= midErr)
		{
			outErr = v2Err;
			return v2;
		}
		else
		{
			outErr = midErr;
			return middle;
		}
	}

	std::vector<Qedge> createQedges(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, std::vector<Quadric>& quadrics, std::vector<bool>& isBorderVertex)
	{
		std::set<std::pair<uint32_t, uint32_t>> uniqueEdges;

		// find unique edges
		for (int i = 0; i < indices.size(); i += 3)
		{
			uint32_t idxTriplet[3] = { indices[i], indices[i + 1], indices[i + 2] };

			for (int j = 0; j < 3; j++)
			{
				uint32_t v1 = idxTriplet[j];
				uint32_t v2 = idxTriplet[(j + 1) % 3];

				auto edge = std::minmax(v1, v2);
				uniqueEdges.insert(edge);
			}
		}

		// create Qedges
		std::vector<Qedge> qedges;
		qedges.reserve(uniqueEdges.size());

		for (auto& [v1, v2] : uniqueEdges)
		{
			Qedge qe;
			qe.v1 = v1;
			qe.v2 = v2;

			if (isBorderVertex[v1] || isBorderVertex[v2])
			{
				qe.error = FLT_MAX; // do not collapse border edges for now
				qe.optimalPos = vertices[v1].pos;
			}
			else
			{
				qe.optimalPos = computeOptPos(quadrics[v1], quadrics[v2], vertices[v1].pos, vertices[v2].pos, qe.error);
			}
			qedges.push_back(qe);
		}

		return qedges;
	}

	bool getValidMinEdge(std::vector<Qedge>& edges, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, Qedge& outEdge)
	{
		while (true)
		{
			// no more edges to collapse
			if (edges.empty())
			{
				return false;
			}

			auto minErrEdge = std::min_element(edges.begin(), edges.end(),
				[](const Qedge& a, const Qedge& b) {
					return a.error < b.error;
				});

			// if error is too high, stop collapsing
			if (minErrEdge->error > 1e7)
			{
				return false;
			}

			// check if collapse causes face flipping
			bool willFlip = Topology::checkFaceFlipping(
				vertices[minErrEdge->v1].pos,
				minErrEdge->optimalPos,
				minErrEdge->v1,
				indices,
				vertices
			);

			if (willFlip)
			{
				// set high error and continue with next edge
				// TODO: fix this, since it flags indefinitely edges causing flipping
				minErrEdge->error = 1e9;
				continue;
			}

			// found valid edge to collapse
			outEdge = *minErrEdge;
			return true;
		}
	}

	void collapseQedge(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, std::vector<Quadric>& quadrics, Qedge& edge)
	{
		uint32_t keepIdx = edge.v1;
		uint32_t removeIdx = edge.v2;

		// though, extended QEM exists for these purposes, lets interpolate UVs for now
		float d1 = glm::distance(vertices[keepIdx].pos, edge.optimalPos);
		float d2 = glm::distance(vertices[removeIdx].pos, edge.optimalPos);
		float totalDistance = d1 + d2;
		if (totalDistance > 1e-6f)
		{
			float t = d1 / totalDistance;
			vertices[keepIdx].texCoord = glm::mix(vertices[keepIdx].texCoord, vertices[removeIdx].texCoord, t);
		}
		else
		{
			vertices[keepIdx].texCoord = 0.5f * (vertices[keepIdx].texCoord + vertices[removeIdx].texCoord);
		}

		// update vertex position with optimal position
		vertices[keepIdx].pos = edge.optimalPos;

		// update quadric
		quadrics[keepIdx] = quadrics[keepIdx] + quadrics[removeIdx];

		// remap indices
		for (auto& idx : indices)
		{
			if (idx == removeIdx) {
				idx = keepIdx;
			}
		}

		// at last, remove degenerated triangles
		Geometry::removeDegeneratedTriangles(indices);
	}

	void updateAfterCollapse(std::vector<Qedge>& edges, uint32_t idxToRemove, uint32_t idxToKeep, std::vector<Vertex>& vertices, std::vector<Quadric>& quadrics, std::vector<bool>& isBorderVertex)
	{
		// remap edges
		for (auto& edge : edges)
		{
			if (edge.v1 == idxToRemove) edge.v1 = idxToKeep;
			if (edge.v2 == idxToRemove) edge.v2 = idxToKeep;
		}

		// delete edges that involved idxToRemove or are now degenerated
		edges.erase(
			std::remove_if(edges.begin(), edges.end(),
				[idxToKeep](const Qedge& e) {
					// delete edge if it is degenerated
					return e.v1 == idxToKeep && e.v2 == idxToKeep;
				}),
			edges.end()
		);

		// recompute edges involving idxToKeep
		for (auto& edge : edges)
		{
			if (edge.v1 == idxToKeep || edge.v2 == idxToKeep)
			{
				if (isBorderVertex[edge.v1] || isBorderVertex[edge.v2])
				{
					edge.error = FLT_MAX;
					edge.optimalPos = vertices[edge.v1].pos;
				}
				else
				{
					uint32_t otherIdx = (edge.v1 == idxToKeep) ? edge.v2 : edge.v1;
					edge.optimalPos = computeOptPos(quadrics[idxToKeep], quadrics[otherIdx], vertices[idxToKeep].pos, vertices[otherIdx].pos, edge.error);
				}
			}
		}
	}
}

 /* End of the QEM.cpp file */