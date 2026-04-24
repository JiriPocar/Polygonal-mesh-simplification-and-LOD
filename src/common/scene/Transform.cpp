/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Transform.hpp
 * @brief Transform implementation of managing position, rotation, scale and model matrix of objects in the scene.
 * 
 * References:
 *		- Joey de Vries' "LearnOpenGL"
 *			- @url https://learnopengl.com/Getting-started/Transformations
 */

#include "Transform.hpp"
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

Transform::Transform()
{

}

void Transform::setPos(glm::vec3 pos)
{
	position = pos;
}

void Transform::setRot(glm::vec3 rot)
{
	rotation = rot;
}

void Transform::setScale(glm::vec3 scl)
{
	scale = scl;
}

glm::mat4 Transform::getModelMatrix() const 
{
	glm::mat4 model(1.0f);
	
	// m = T * R * S
	model = glm::translate(model, position);
	model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, scale);
	return model;
}

/* End of the Transform.cpp file */