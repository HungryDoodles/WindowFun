#pragma once

#include "Transform.h"

class Camera : Transform 
{
public:
	Camera();
	virtual ~Camera();

	void SetFovDegrees(float degrees);

	float fov = 3.14159f / 180 * 60;

	virtual void Apply();
};