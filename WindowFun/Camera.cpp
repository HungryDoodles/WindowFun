#include "Camera.h"
#include "ConRender.h"


Camera::Camera()
{
}

Camera::~Camera()
{
}

void Camera::SetFovDegrees(float degrees)
{
	fov = 3.14159 / 180 * degrees;
}

void Camera::UpdateUserMovement()
{
}

void Camera::Apply()
{
	auto V = GetMatrix();
	SetView(V);
	SetPerspective(fov);
}
