/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Camera.hpp
 * @brief Camera class for managing the view, projection matrices and handling user input.
 *
 * This file implements the Camera class, which is responsible for managing the camera's position,
 * orientation, view and projection matrices, and handling user input for camera movement and mouse look.
 */

#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

class Camera {
public:
	Camera();
	
	/**
	* @brief Handles keyboard input for camera movement.
	* 
	* @param window The GLFW window to check for input
	* @param delta The time elapsed since the last frame
	*/
	void handleInput(GLFWwindow* window, float delta);

	/**
	* @brief Handles raw mouse input from GLFW.
	*
	* @param x The current x position of the mouse
	* @param y The current y position of the mouse
	*/
	void handleMouseInput(double x, double y);

	/**
	* @brief Handles mouse movement input for camera orientation.
	* 
	* @param xOffset The horizontal offset of the mouse movement
	* @param yOffset The vertical offset of the mouse movement
	* @param conPitch Constrain the pitch angle to prevent flipping
	*/
	void processMouseMovement(float xOffset, float yOffset, bool conPitch = true);

	// toggles mouse input for the camera
	void resetMouse();

	// getters and setters
	glm::mat4 getViewMatrix() const { return viewMatrix; };
	glm::mat4 getProjectionMatrix() const { return projectionMatrix; };
	glm::vec3 getPosition() const { return position; };
	void setView(glm::vec3 pos, glm::vec3 target, glm::vec3 up);
	void setPosition(const glm::vec3& pos) { setView(pos, pos + front, up); };
	void setPerspective(float fov, float aspect, float near, float far);
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
	float movementSpeed;

	glm::vec3 front;
	glm::vec3 up;
};

/* End of the Camera.hpp file */