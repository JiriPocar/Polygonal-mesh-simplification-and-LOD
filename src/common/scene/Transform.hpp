/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Transform.hpp
 * @brief Transform class for managing position, rotation, scale and model matrix of objects in the scene.
 */

#pragma once
#include <glm/glm.hpp>

class Transform {
public:
	Transform();

	// setters
	void setPos(glm::vec3 pos);
	void setRot(glm::vec3 rot);
	void setScale(glm::vec3 scl);

	// getters
	glm::vec3 getPos() { return position; }
	glm::vec3 getRot() { return rotation; }
	glm::vec3 getScale() { return scale; }

	// gets an object matrix based on the current position, rotation and scale
	glm::mat4 getModelMatrix() const;

private:
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
};

/* End of the Transform.hpp file */