#include "Camera.h"
#include "ConRender.h"
#include "WinUser.h"

#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")
#include <dinput.h>

IDirectInputDevice8* DIKeyboard;
IDirectInputDevice8* DIMouse;

DIMOUSESTATE mouseLastState;
LPDIRECTINPUT8 DirectInput;


Camera::Camera()
{
}

Camera::Camera(HWND window) : window(window)
{
	HINSTANCE instance = (HINSTANCE)GetWindowLong(window, GWLP_HINSTANCE);
	HRESULT result = DirectInput8Create(instance,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&(DirectInput),
		NULL);
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
