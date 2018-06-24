#include "../VC14/Camera.h"
#include "../Externals/Include/GLM/glm/gtc/matrix_transform.hpp"
#include "../Externals/Include/GLM/glm/gtc/constants.hpp"

Camera::Camera() : 
	position(glm::vec3(0.0f, 800.0f, 1.0f)),
	target(glm::vec3(0.0f, 5.0f, 0.0f)),
	up(glm::vec3(0.0f, 1.0f, 0.0f)),
	fov(glm::pi<float>() / 3.0f), 
	aspect(16.0f / 9.0f), 
	zNear(30.0f), 
	zFar(5000.0f), 
	isViewingDirty(true), 
	isPerspectiveDirty(true)
{
}

Camera::~Camera()
{
}

void Camera::setPosition(glm::vec3 newPosition)
{
	position = newPosition;
	isViewingDirty = true;
}

void Camera::setTarget(glm::vec3 newTarget)
{
	target = newTarget;
	isViewingDirty = true;
}

void Camera::setUp(glm::vec3 newUp)
{
	up = newUp;
	isViewingDirty = true;
}

void Camera::setFov(float newFov)
{
	fov = newFov;
	isPerspectiveDirty = true;
}

void Camera::setAspect(float newAspect)
{
	aspect = newAspect;
	isPerspectiveDirty = true;
}

void Camera::setNear(float newNear)
{
	zNear = newNear;
	isPerspectiveDirty = true;
}
void Camera::setFar(float newFar)
{
	zFar = newFar;
	isPerspectiveDirty = true;
}

const glm::mat4& Camera::getViewingMatrix() const
{
	if (isViewingDirty)
	{
		viewingMatrix = glm::lookAt(position, target, up);
		isViewingDirty = false;
	}
	return viewingMatrix;
}

const glm::mat4& Camera::getPerspectiveMatrix() const
{
	if (isPerspectiveDirty)
	{
		perspectiveMatrix = glm::perspective(fov, aspect, zNear, zFar);
		isPerspectiveDirty = false;
	}
	return perspectiveMatrix;
}

void Camera::update(float deltaTime)
{
}

void Camera::onMouse(int button, int state, int x, int y)
{
}

void Camera::onMotion(int x, int y)
{
}

void Camera::onMouseWheel(int wheel, int direction, int x, int y)
{
}

void Camera::onKeyboard(unsigned char key, int x, int y)
{
}

void Camera::onKeyboardUp(unsigned char key, int x, int y)
{
}