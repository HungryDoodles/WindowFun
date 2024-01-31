#pragma once

#include "Transform.h"
#include "Windows.h"

class Camera : public Transform 
{
public:
	Camera();
	virtual ~Camera();

	void SetFovDegrees(float degrees);
	void UpdateUserMovement();

	float fov = 3.14159f / 180 * 90;

	virtual void Apply();
};