#pragma once
#include <glm/glm.hpp>

class Transform {
public:
	Transform();

	void setPos(glm::vec3 pos);
	void setRot(glm::vec3 rot);
	void setScale(glm::vec3 scl);

	glm::mat4 getModelMatrix() const;

private:
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
};