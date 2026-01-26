#pragma once
#include <glm/glm.hpp>

class Transform {
public:
	Transform();

	void setPos(glm::vec3 pos);
	void setRot(glm::vec3 rot);
	void setScale(glm::vec3 scl);

	glm::vec3 getPos() { return position; }
	glm::vec3 getRot() { return rotation; }
	glm::vec3 getScale() { return scale; }

	glm::mat4 getModelMatrix() const;

private:
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
};