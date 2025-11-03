#include "../resources/Model.hpp"

enum class Algorithm {
	QEM,
	EdgeCollapse,
	VertexClustering,
	Naive
};

struct SimplificatorResult {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	Algorithm algorithmUsed;
	double timeTaken;

	size_t originalFaceCount;
	size_t simplifiedFaceCount;
	// float geometricError;
};

class Simplificator {
public:
	Simplificator();
	~Simplificator() = default;

	void setCurrentAlgorithm(Algorithm algorithm);
	Algorithm getCurrentAlgorithm() const { return currentAlgorithm; };

	SimplificatorResult simplify(const Model& model, float targetFaceCountRatio);

private:
	Algorithm currentAlgorithm;

	SimplificatorResult simplifyQEM(const Model& model, size_t targetFaceCount);
	SimplificatorResult simplifyEdgeCollapse(const Model& model, size_t targetFaceCount);
	SimplificatorResult simplifyVertexClustering(const Model& model, size_t targetFaceCount);
	SimplificatorResult simplifyNaive(const Model& model, size_t targetFaceCount);

	size_t computeTargetFaceCount(const Model& model, float ratio);
};