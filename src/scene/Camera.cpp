#include "Camera.hpp"

Camera::Camera()
	: position(0.0f, 0.0f, 3.0f),
	pitch(0.0f),
	yaw(-90.0f),
	front(0.0f, 0.0f, -1.0f),
	up(0.0f, 1.0f, 0.0f)
{
	setPerspective(45.0f, 16.0f / 9.0f, 0.1f, 10.0f);
	setView(position, position + front, up);
}

void Camera::setPerspective(float fov, float aspect, float near, float far)
{
	projectionMatrix = glm::perspective(glm::radians(fov), aspect, near, far);
	// invert Y for Vulkan
	projectionMatrix[1][1] *= -1;
}

void Camera::setView(glm::vec3 pos, glm::vec3 target, glm::vec3 up)
{
	position = pos;
	viewMatrix = glm::lookAt(position, target, up);
}

void Camera::handleInput(GLFWwindow *window, float delta)
{
	float cameraSpeed = 40.5f * delta;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		position += cameraSpeed * front;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		position -= cameraSpeed * front;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		position -= glm::normalize(glm::cross(front, up)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		position += glm::normalize(glm::cross(front, up)) * cameraSpeed;

	setView(position, position + front, up);
}