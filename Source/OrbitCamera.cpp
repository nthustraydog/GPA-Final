#include "../VC14/OrbitCamera.h"
#include "../Externals/Include/FreeGLUT/freeglut.h"
#include "../Externals/Include/GLM/glm/trigonometric.hpp"
#include "../Externals/Include/GLM/glm/gtc/constants.hpp"
#include "../Externals/Include/GLM/glm/matrix.hpp"
#include "../Externals/Include/GLM/glm/geometric.hpp"

const float OrbitCamera::minR = 1.0f;
const float OrbitCamera::maxR = 100.0f;
const float OrbitCamera::minPhi = 0.1f;
const float OrbitCamera::maxPhi = 3.1315f;

const float OrbitCamera::angularVelocity = 0.01f;
const float OrbitCamera::moveVelocity = 500.0f;

OrbitCamera::OrbitCamera() :
	r(3.0f),
	theta(0.0f),
	phi(glm::pi<float>() / 2.0f),
	moveDirection(glm::vec3(0.0f, 0.0f, 0.0f)),
	leftButtonDown(false), 
	rightButtonDown(false)
{
}

OrbitCamera::~OrbitCamera()
{
}

void OrbitCamera::setR(float newR)
{
	if (newR < minR) r = minR;
	else if (newR > maxR) r = maxR;
	else r = newR;
}
void OrbitCamera::setTheta(float newTheta)
{
	theta = newTheta;
}

void OrbitCamera::setPhi(float newPhi)
{
	if (newPhi < minPhi) phi = minPhi;
	else if (newPhi > maxPhi) phi = maxPhi;
	else phi = newPhi;
}

void OrbitCamera::update(float deltaTime)
{
	if (glm::length(moveDirection) > 0.001f)
	{
		glm::vec3 localMovement = glm::normalize(moveDirection) * moveVelocity * deltaTime;
		
		glm::vec3 movement = glm::vec3(glm::transpose(getViewingMatrix()) * glm::vec4(localMovement.x, 0.0f, localMovement.z, 1.0f));
		movement.y += localMovement.y;
		setPosition(position + movement);
	}
	glm::vec3 newTarget = position + glm::vec3(r * glm::cos(theta) * glm::sin(phi),
		r * glm::cos(phi),
		r * glm::sin(theta) * glm::sin(phi));
	setTarget(newTarget);
}

void OrbitCamera::onMouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON) leftButtonDown = (state == GLUT_DOWN);
	if (button == GLUT_RIGHT_BUTTON) rightButtonDown = (state == GLUT_DOWN);

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		captureMouseX = x;
		captureMouseY = y;
		captureTheta = theta;
		capturePhi = phi;
	}
}

void OrbitCamera::onMotion(int x, int y)
{
	if (leftButtonDown)
	{
		int deltaX = x - captureMouseX;
		int deltaY = y - captureMouseY;

		setTheta(captureTheta + deltaX * angularVelocity);
		setPhi(capturePhi + deltaY * angularVelocity);
	}
}

void OrbitCamera::onMouseWheel(int wheel, int direction, int x, int y)
{
}

void OrbitCamera::onKeyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'w':
	case 'W':
		moveDirection.z = -1.0f;
		break;
	case 's':
	case 'S':
		moveDirection.z = 1.0f;
		break;
	case 'a':
	case 'A':
		moveDirection.x = -1.0f;
		break;
	case 'd':
	case 'D':
		moveDirection.x = 1.0f;
		break;
	case 'z':
	case 'Z':
		moveDirection.y = -1.0f;
		break;
	case 'x':
	case 'X':
		moveDirection.y = 1.0f;
		break;
	}
}

void OrbitCamera::onKeyboardUp(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'w':
	case 'W':
	case 's':
	case 'S':
		moveDirection.z = 0.0f;
		break;
	case 'a':
	case 'A':
	case 'd':
	case 'D':
		moveDirection.x = 0.0f;
		break;
	case 'z':
	case 'Z':
	case 'x':
	case 'X':
		moveDirection.y = 0.0f;
		break;
	}
}