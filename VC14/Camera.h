#pragma once
#include "../Externals/Include/GLM/glm/vec3.hpp"
#include "../Externals/Include/GLM/glm/mat4x4.hpp"

class Camera
{
public:
	Camera();
	virtual ~Camera();

	void setPosition(glm::vec3 newPosition);
	const glm::vec3& getPosition() { return position; };
	void setTarget(glm::vec3 newTarget);
	void setUp(glm::vec3 newUp);

	void setFov(float newFov);
	void setAspect(float newAspect);
	void setNear(float newNear);
	void setFar(float newFar);

	const glm::mat4& getViewingMatrix() const;
	const glm::mat4& getPerspectiveMatrix() const;

	virtual void update(float deltaTime);

	virtual void onMouse(int button, int state, int x, int y);
	virtual void onMotion(int x, int y);
	virtual void onMouseWheel(int wheel, int direction, int x, int y);
	virtual void onKeyboard(unsigned char key, int x, int y);
	virtual void onKeyboardUp(unsigned char key, int x, int y);

protected:
	glm::vec3 position;
	glm::vec3 target;
	glm::vec3 up;

	float fov;
	float aspect;
	float zNear;
	float zFar;

	mutable glm::mat4 viewingMatrix;
	mutable glm::mat4 perspectiveMatrix;

	mutable bool isViewingDirty;
	mutable bool isPerspectiveDirty;
};

