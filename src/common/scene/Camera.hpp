#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

class Camera {
public:
	Camera();

	void setPerspective(float fov, float aspect, float near, float far);
	void setView(glm::vec3 pos, glm::vec3 target, glm::vec3 up);
	void handleInput(GLFWwindow* window, float delta);

	void handleMouseInput(double x, double y, bool mouseDisabled);
	void resetMouse();

	glm::mat4 getViewMatrix() const { return viewMatrix; };
	glm::mat4 getProjectionMatrix() const { return projectionMatrix; };
	glm::vec3 getPosition() const { return position; };

	void setPosition(const glm::vec3& pos) { setView(pos, pos + front, up); };

	void processMouseMovement(float xOffset, float yOffset, bool conPitch = true);
	void setMouseSens(float sens) { mouseSens = sens; };

private:
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
	glm::vec3 position;

	float pitch;
	float yaw;
	float mouseSens;
	bool mouseDisabled = false;
	bool inititalMouse = true;
	float xLast;
	float yLast;

	glm::vec3 front;
	glm::vec3 up;
};