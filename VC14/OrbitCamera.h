#pragma once
#include "../VC14/Camera.h"
#include "../Externals/Include/GLM/glm/vec3.hpp"

class OrbitCamera :	public Camera
{
public:
	OrbitCamera();
	~OrbitCamera();

	float getR() { return r; }
	float getTheta() { return theta; }
	float getPhi() { return phi; }

	void setR(float newR);
	void setTheta(float newTheta);
	void setPhi(float newPhi);

	void update(float deltaTime) override;

	void onMouse(int button, int state, int x, int y) override;
	void onMotion(int x, int y) override;
	void onMouseWheel(int wheel, int direction, int x, int y) override;
	void onKeyboard(unsigned char key, int x, int y) override;
	void onKeyboardUp(unsigned char key, int x, int y) override;

private:
	float r;
	float theta;
	float phi;

	const static float maxR;
	const static float maxPhi;
	const static float minR;
	const static float minPhi;

	const static float angularVelocity;
	const static float moveVelocity;

	glm::vec3 moveDirection;

	int captureMouseX;
	int captureMouseY;
	float captureTheta;
	float capturePhi;
	bool leftButtonDown;
	bool rightButtonDown;
};

