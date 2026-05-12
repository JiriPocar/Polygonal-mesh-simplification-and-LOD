/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file QEM.cpp
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
		// check if triangle is degenerate
		auto cross = glm::cross(v2 - v1, v3 - v1);
		if (glm::length(cross) < 1e-6f) return Quadric();

		// get plane normal
		glm::vec3 n = glm::normalize(cross);
		double a = n.x, b = n.y, c = n.z;
		double d = -glm::dot(n, v1);

		// =============================================================================
		// NOT USED FOR THE MEASURE OF ERROR IN THESIS - FURTHER OPTIMIZATION EXPERIMENT
		// area-weighted quadrics
		// double area = 0.5 * glm::length(cross);

		// multiply the coefficients of the plane equation by the area of the triangle
		// this makes bigger triangles in the mesh more important
		/*Quadric q;
		q.q11 = a * a * area;	q.q12 = a * b * area;	q.q13 = a * c * area;	q.q14 = a * d * area;
		q.q22 = b * b * area;	q.q23 = b * c * area;	q.q24 = b * d * area;
		q.q33 = c * c * area;	q.q34 = c * d * area;
		q.q44 = d * d * area;*/
		// =============================================================================

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
		glm::dmat3 MAT(
			q.q11, q.q12, q.q13,
			q.q12, q.q22, q.q23,
			q.q13, q.q23, q.q33
		);

		// if determinant is non-zero, we can find optimal position by solving the linear system
		double det = glm::determinant(MAT);
		if (std::abs(det) > 1e-12)
		{
			// vector b is the negation of the last column of Q
			glm::dvec3 b(q.q14, q.q24, q.q34);

			// solve for v in MAT * v = -b
			glm::dvec3 exactOptPos = -glm::inverse(MAT) * b;

			// if the optPos is too far away from the edge, indicates unwanted spike
			glm::vec3 optPos(exactOptPos);
			float edgeLen = glm::length(v2 - v1);
			glm::vec3 center = 0.5f * (v1 + v2);
			if (glm::distance(optPos, center) <= edgeLen * 2.0f)
			{
				outErr = q.evalError(exactOptPos);
				if (outErr < 0.0) outErr = 0.0;
				return glm::vec3(exactOptPos);
			}
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
		std::vector<std::pair<uint32_t, uint32_t>> uniqueEdges;
		uniqueEdges.reserve(indices.size());

		// find unique edges
		for (int i = 0; i < indices.size(); i += 3)
		{
			uint32_t idx0 = indices[i];
			uint32_t idx1 = indices[i + 1];
			uint32_t idx2 = indices[i + 2];

			uniqueEdges.push_back(std::minmax(idx0, idx1));
			uniqueEdges.push_back(std::minmax(idx1, idx2));
			uniqueEdges.push_back(std::minmax(idx2, idx0));
		}
		std::sort(uniqueEdges.begin(), uniqueEdges.end());
		uniqueEdges.erase(std::unique(uniqueEdges.begin(), uniqueEdges.end()), uniqueEdges.end());

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
				qe.error = FLT_MAX; // do not collapse border edges
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

	int collapseQedge(QEMContext& context, Qedge& edge)
	{
		uint32_t keepIdx = edge.v1;
		uint32_t removeIdx = edge.v2;
		int outDeletedFaces = 0;

		// set vertex position with optimal position
		context.vertices[keepIdx].pos = edge.optimalPos;

		// update quadric of the kept vertex by adding the quadric of the removed vertex
		context.quadrics[keepIdx] = context.quadrics[keepIdx] + context.quadrics[removeIdx];

		// update all triangles in the neighborhood of the removed vertex
		for (uint32_t tri : context.allNeighborhoods[removeIdx].triangles)
		{
			uint32_t idx0 = context.indices[tri];
			uint32_t idx1 = context.indices[tri + 1];
			uint32_t idx2 = context.indices[tri + 2];

			// skip degenerate triangles
			if (idx0 == idx1 || idx1 == idx2 || idx2 == idx0)
			{
				continue;
			}

			// if we replace removeIdx with keepIdx in this triangle, will it become degenerate?
			bool willDegenerate = (idx0 == keepIdx || idx1 == keepIdx || idx2 == keepIdx);
			if (willDegenerate) outDeletedFaces++;

			// set removeIdx to keepIdx in this triangle
			if (context.indices[tri] == removeIdx) context.indices[tri] = keepIdx;
			if (context.indices[tri + 1] == removeIdx) context.indices[tri + 1] = keepIdx;
			if (context.indices[tri + 2] == removeIdx) context.indices[tri + 2] = keepIdx;

			// if not degenerate, add this triangle to the neighborhood of the kept vertex
			if (!willDegenerate)
			{
				context.allNeighborhoods[keepIdx].triangles.push_back(tri);
			}
		}

		// clear triangle neighborhood of the removed vertex
		context.allNeighborhoods[removeIdx].triangles.clear();

		// remove degenerate triangles from the neighborhood of the kept vertex
		auto& tris = context.allNeighborhoods[keepIdx].triangles;
		tris.erase(std::remove_if(tris.begin(), tris.end(),
			[&context](uint32_t tri) {
				return context.indices[tri] == context.indices[tri + 1] ||
					context.indices[tri + 1] == context.indices[tri + 2] ||
					context.indices[tri + 2] == context.indices[tri];
			}),
			tris.end());
		
		// mark removed vertex as deleted
		context.vertexDeleted[removeIdx] = true;

		return outDeletedFaces;
	}

	int syncSeamTwinsAfterCollapse(QEMContext& context, uint32_t keepIdx, uint32_t removeIdx, const glm::vec3& optimalPos)
	{
		int totalDeletedFaces = 0;

		std::vector<uint32_t> keepCandidates = context.twinMap[keepIdx];
		keepCandidates.push_back(keepIdx);

		std::vector<uint32_t> twinsToRemove = context.twinMap[removeIdx];

		// collapse all twins of the removed vertex to the best matching twin of the kept vertex
		for (uint32_t v2twin : twinsToRemove)
		{
			if (context.vertexDeleted[v2twin]) continue;

			// find the best matching twin of the kept vertex for this twin of the removed vertex
			uint32_t bestV1twin = keepIdx;
			float minDiff = FLT_MAX;
			for (uint32_t candidate : keepCandidates)
			{
				if (context.vertexDeleted[candidate]) continue;

				float diff = glm::length(context.vertices[candidate].texCoord - context.vertices[v2twin].texCoord)
					       + glm::length(context.vertices[candidate].normal - context.vertices[v2twin].normal);

				if (diff < minDiff)
				{
					minDiff = diff;
					bestV1twin = candidate;
				}
			}

			// collapse the twin edge
			Qedge twinEdge;
			twinEdge.v1 = bestV1twin;
			twinEdge.v2 = v2twin;
			twinEdge.optimalPos = optimalPos;
			int deletedByTwin = collapseQedge(context, twinEdge);
			totalDeletedFaces += deletedByTwin;

			// update cross-references in the twin map for the twins of the removed vertex
			for (uint32_t grandTwin : context.twinMap[v2twin])
			{
				if (grandTwin == bestV1twin || context.vertexDeleted[grandTwin]) continue;

				auto& bestTwinTwins = context.twinMap[bestV1twin];
				if (std::find(bestTwinTwins.begin(), bestTwinTwins.end(), grandTwin) == bestTwinTwins.end())
				{
					bestTwinTwins.push_back(grandTwin);
				}

				auto& gtTwins = context.twinMap[grandTwin];
				std::replace(gtTwins.begin(), gtTwins.end(), v2twin, bestV1twin);
			}
			context.twinMap[v2twin].clear();
		}

		// move remaining twins of the removed vertex to the kept vertex and update their references
		std::vector<uint32_t> remainingTwins = context.twinMap[removeIdx];
		for (uint32_t t : remainingTwins)
		{
			if (context.vertexDeleted[t] || t == keepIdx) continue;

			auto& keepIdxTwin = context.twinMap[keepIdx];
			if (std::find(keepIdxTwin.begin(), keepIdxTwin.end(), t) == keepIdxTwin.end())
			{
				keepIdxTwin.push_back(t);
			}

			auto& tTwins = context.twinMap[t];
			std::replace(tTwins.begin(), tTwins.end(), removeIdx, keepIdx);
		}
		context.twinMap[removeIdx].clear();

		// recompute quadrics and positions for all twins of the kept vertex
		for (uint32_t twin : context.twinMap[keepIdx])
		{
			if (!context.vertexDeleted[twin])
			{
				context.quadrics[twin] = context.quadrics[keepIdx];
				context.vertices[twin].pos = context.vertices[keepIdx].pos;
			}
		}
		return totalDeletedFaces;
	}

	void enqueueAffectedEdges(QEMContext& context, uint32_t keepIdx, LazyPriorityQueue<QEM::Qedge, QEM::QedgeCompare>& qedgeQueue, SimplificationOptions options)
	{
		// collect all vertices that might be affected by the collapse
		std::vector<uint32_t> affectedVertices = { keepIdx };
		if (options.resolveUVSeams) // if solving UV seams, also consider all twins of the kept vertex as affected
		{
			for (uint32_t twin : context.twinMap[keepIdx])
			{
				if (!context.vertexDeleted[twin]) affectedVertices.push_back(twin);
			}
		}

		// enqueue all edges in the neighborhood of the affected vertices
		for (uint32_t affectedVertex : affectedVertices)
		{
			for (uint32_t tri : context.allNeighborhoods[affectedVertex].triangles)
			{
				// triangles are saved in triplets of vertex indices 
				uint32_t idx0 = context.indices[tri];
				uint32_t idx1 = context.indices[tri + 1];
				uint32_t idx2 = context.indices[tri + 2];

				// skip degenerate triangles
				if (idx0 == idx1 || idx1 == idx2 || idx2 == idx0) continue;

				// extract edges of this triangle
				uint32_t edges[3][2] = {
					{std::min(idx0, idx1), std::max(idx0, idx1)},
					{std::min(idx1, idx2), std::max(idx1, idx2)},
					{std::min(idx2, idx0), std::max(idx2, idx0)}
				};

				// enqueue each edge if not already deleted or locked
				for (int j = 0; j < 3; j++)
				{
					uint32_t ev1 = edges[j][0];
					uint32_t ev2 = edges[j][1];

					QEM::Qedge newEdge;
					newEdge.v1 = ev1;
					newEdge.v2 = ev2;

					if (context.isLockedVertex[ev1] || context.isLockedVertex[ev2])
					{
						newEdge.error = FLT_MAX;
						newEdge.optimalPos = context.vertices[ev1].pos;
					}
					else
					{
						newEdge.optimalPos = QEM::computeOptPos(context.quadrics[ev1], context.quadrics[ev2], context.vertices[ev1].pos, context.vertices[ev2].pos, newEdge.error);
					}

					// push into lazy priority queue (validated when popped)
					qedgeQueue.push(newEdge);
				}
			}
		}
	}

	bool isEdgeValidForCollapse(QEMContext& context, const QEM::Qedge& e, LazyPriorityQueue<QEM::Qedge, QEM::QedgeCompare>& qedgeQueue, SimplificationOptions options)
	{
		// check if either vertex is already deleted or locked
		if (context.vertexDeleted[e.v1] || context.vertexDeleted[e.v2]) return false;
		if (context.isLockedVertex[e.v1] || context.isLockedVertex[e.v2]) return false;

		// lazy priority queue validation
		double currentError;
		glm::vec3 currentOptPos = QEM::computeOptPos(context.quadrics[e.v1], context.quadrics[e.v2], context.vertices[e.v1].pos, context.vertices[e.v2].pos, currentError);
		if (currentError > e.error + 1e-4)
		{
			// reinsert the edge with the updated error and optimal position
			QEM::Qedge updatedEdge = e;
			updatedEdge.error = currentError;
			updatedEdge.optimalPos = currentOptPos;
			qedgeQueue.push(updatedEdge);
			return false;
		}

		// prevent collapsing edges that would break mesh connectivity
		if (options.checkConnectivity && !Topology::checkConnectivity(e.v1, e.v2, context.indices, context.allNeighborhoods[e.v1], context.allNeighborhoods[e.v2])) return false;

		// prevent face flipping
		if (options.checkFaceFlipping)
		{
			// since it is a full edge collapse, check both vertices for potential face flipping
			bool willFlip = Topology::checkFaceFlipping(context.vertices[e.v1].pos, currentOptPos, e.v1, context.indices, context.vertices, context.allNeighborhoods[e.v1]) ||
				Topology::checkFaceFlipping(context.vertices[e.v2].pos, currentOptPos, e.v2, context.indices, context.vertices, context.allNeighborhoods[e.v2]);
			
			// prevent twin flipping 
			if (options.resolveUVSeams && !willFlip)
			{
				// again - check both vertices, if either has a twin that would flip, the collapse is invalid
				for (uint32_t twin : context.twinMap[e.v1])
				{
					if (!context.vertexDeleted[twin] && Topology::checkFaceFlipping(context.vertices[twin].pos, currentOptPos, twin, context.indices, context.vertices, context.allNeighborhoods[twin]))
					{
						willFlip = true;
						break;
					}
				}
				if (!willFlip)
				{
					for (uint32_t twin : context.twinMap[e.v2])
					{
						if (!context.vertexDeleted[twin] && Topology::checkFaceFlipping(context.vertices[twin].pos, currentOptPos, twin, context.indices, context.vertices, context.allNeighborhoods[twin]))
						{
							willFlip = true;
							break;
						}
					}
				}
			}
			if (willFlip) return false;
		}

		// prevent connectivity issues of UV seam twins
		if (options.resolveUVSeams && options.checkConnectivity)
		{
			std::vector<uint32_t> keepCandidates = context.twinMap[e.v1];
			keepCandidates.push_back(e.v1);

			for (uint32_t v2Twin : context.twinMap[e.v2])
			{
				if (context.vertexDeleted[v2Twin]) continue;

				// find the best matching twin of the kept vertex for this twin of the removed vertex
				uint32_t bestV1Twin = e.v1;
				float minDiff = FLT_MAX;
				for (uint32_t candidate : keepCandidates)
				{
					if (context.vertexDeleted[candidate]) continue;

					float diff = glm::length(context.vertices[candidate].texCoord - context.vertices[v2Twin].texCoord)
						       + glm::length(context.vertices[candidate].normal - context.vertices[v2Twin].normal);
					if (diff < minDiff)
					{
						minDiff = diff;
						bestV1Twin = candidate;
					}
				}

				// if any twin pair fails the connectivity check, the collapse is invalid
				if (!Topology::checkConnectivity(bestV1Twin, v2Twin, context.indices, context.allNeighborhoods[bestV1Twin], context.allNeighborhoods[v2Twin]))
				{
					return false;
				}
			}
		}

		return true;
	}
}

 /* End of the QEM.cpp file */