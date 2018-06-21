#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "../Externals/Include/Include.h"
#include <iostream>
#include <fstream>
#include <iomanip>      // std::setprecision

enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN,
	RESET
};

const GLfloat YAW = 0.0f;
const GLfloat PITCH = 0.0f;
const GLfloat SPEED = 3.0f;
const GLfloat MOUSE_SENSITIVTY = 0.05f;
const GLfloat MOUSE_ZOOM = 45.0f;
const float  MAX_PITCH_ANGLE = 89.0f; 

class Camera
{
public:
	Camera(glm::vec3 pos = glm::vec3(0.0, 0.0, 2.0),
		glm::vec3 up = glm::vec3(0.0, 1.0, 0.0),
		GLfloat yaw = YAW, GLfloat pitch = PITCH)
		:position(pos), forward(0.0, 0.0, -1.0), viewUp(up),
		moveSpeed(SPEED), mouse_zoom(MOUSE_ZOOM), mouse_sensitivity(MOUSE_SENSITIVTY),
		yawAngle(yaw), pitchAngle(pitch)
	{
		this->updateCameraVectors();
	}
public:
	void Reset() {
		this->position = vec3(0.0, 0.0, 2.0);
		this->viewUp = vec3(0.0, 1.0, 0.0);
		this->forward = vec3(0.0, 0.0, -1.0);
		this->moveSpeed = SPEED;
		this->mouse_zoom = MOUSE_ZOOM;
		this->mouse_sensitivity = MOUSE_SENSITIVTY;
		this->yawAngle = YAW;
		this->pitchAngle = PITCH;
	}
	glm::mat4 getViewMatrix()
	{
		return glm::lookAt(this->position, this->position + this->forward, this->viewUp);
	}
	
	void handleKeyPress(Camera_Movement direction, GLfloat deltaTime)
	{
		GLfloat velocity = this->moveSpeed * deltaTime;
		vec3 UpDown;
		switch (direction)
		{
		case FORWARD:
			this->position += this->forward * velocity;
			break;
		case BACKWARD:
			this->position -= this->forward * velocity;
			break;
		case LEFT:
			this->position -= this->side * velocity;
			break;
		case RIGHT:
			this->position += this->side * velocity;
			break;
		case UP:
			UpDown = vec3(0, 1, 0);
			printf("Camera Side: (%f, %f, %f)\n", UpDown.x, UpDown.y, UpDown.z);
			this->position += UpDown * velocity;
			break;
		case DOWN:
			UpDown = vec3(0, -1, 0);
			printf("Camera Side: (%f, %f, %f)\n", UpDown.x, UpDown.y, UpDown.z);
			this->position += UpDown * velocity;
			break;
		case RESET: 
			Reset();
			break;
		default:
			break;
		}
	}
	
	void handleMouseMove(GLfloat xoffset, GLfloat yoffset)
	{

		xoffset *= this->mouse_sensitivity; 
		yoffset *= this->mouse_sensitivity;

		this->pitchAngle -= 5 * yoffset;
		this->yawAngle += 5 * xoffset;

		this->normalizeAngle();
		this->updateCameraVectors();
	}
	void handleMouseHoldMove(GLfloat xoffset, GLfloat yoffset) {
		xoffset *= 3 * this->mouse_sensitivity;
		yoffset *= 3 * this->mouse_sensitivity;

		this->pitchAngle -= yoffset;
		this->yawAngle += xoffset;

		this->normalizeAngle();
		this->updateCameraPosition();
	}
	
	void handleMouseScroll(GLfloat yoffset)
	{
		if (this->mouse_zoom >= 1.0f && this->mouse_zoom <= MOUSE_ZOOM)
			this->mouse_zoom -= this->mouse_sensitivity * yoffset;
		if (this->mouse_zoom <= 1.0f)
			this->mouse_zoom = 1.0f;
		if (this->mouse_zoom >= 45.0f)
			this->mouse_zoom = 45.0f;
	}
	
	void normalizeAngle()
	{
		if (this->pitchAngle > MAX_PITCH_ANGLE)
			this->pitchAngle = MAX_PITCH_ANGLE;
		if (this->pitchAngle < -MAX_PITCH_ANGLE)
			this->pitchAngle = -MAX_PITCH_ANGLE;
		if (this->yawAngle < 0.0f)
			this->yawAngle += 360.0f;
	}
	
	void updateCameraVectors()
	{
		glm::vec3 forward;
		forward.x = -sin(glm::radians(this->yawAngle)) * cos(glm::radians(this->pitchAngle));
		forward.y = sin(glm::radians(this->pitchAngle));
		forward.z = -cos(glm::radians(this->yawAngle)) * cos(glm::radians(this->pitchAngle));
		this->forward = glm::normalize(forward);

		glm::vec3 side;
		side.x = cos(glm::radians(this->yawAngle));
		side.y = 0;
		side.z = -sin(glm::radians(this->yawAngle));
		this->side = glm::normalize(side);
	}
	void updateCameraPosition() {
		glm::vec3 newforward;
		newforward.x = -sin(glm::radians(this->yawAngle)) * cos(glm::radians(this->pitchAngle));
		newforward.y = sin(glm::radians(this->pitchAngle));
		newforward.z = -cos(glm::radians(this->yawAngle)) * cos(glm::radians(this->pitchAngle));
		newforward = glm::normalize(newforward);
		this->position = this->position + newforward;

		glm::vec3 side;
		side.x = cos(glm::radians(this->yawAngle));
		side.y = 0;
		side.z = -sin(glm::radians(this->yawAngle));
		this->side = glm::normalize(side);
	}
public:
	glm::vec3 forward, side, viewUp, position; 
	GLfloat yawAngle, pitchAngle;
	GLfloat moveSpeed, mouse_sensitivity, mouse_zoom; 
};

#endif