#pragma once
#include "stdio.h"
#include "stdlib.h"
#include "Windows.h"
#undef min
#undef max
#undef near
#undef far
#include <algorithm>
#include <vector>

#include "linalg.h"
using namespace linalg::aliases;

#define WIDTH 100
#define HEIGHT 50

struct LIGHT_INFO 
{
	float3 position;
	float3 color;
	float intensity;
};

HANDLE wHnd; /* write (output) handle */
HANDLE rHnd; /* read (input handle */

/* A CHAR_INFO structure containing data about a single character */
CHAR_INFO consoleBuffer[WIDTH * HEIGHT];
float3 worldSpaceBuffer[WIDTH * HEIGHT];
float3 colorBuffer[WIDTH * HEIGHT];
float depthBuffer[WIDTH * HEIGHT];
std::vector<LIGHT_INFO> lights;
/* Area info */
COORD characterBufferSize = { WIDTH, HEIGHT };
COORD characterPosition = { 0, 0 };
SMALL_RECT consoleWriteArea = { 0, 0, WIDTH - 1, HEIGHT - 1 };

float4x4 view;
float4x4 projection;
float4x4 VP;


void InitConsoleBuffer()
{
    SMALL_RECT windowSize = { 0, 0, WIDTH - 1, HEIGHT - 1 };

    COORD bufferSize = { WIDTH, HEIGHT };


    wHnd = GetStdHandle(STD_OUTPUT_HANDLE);
    rHnd = GetStdHandle(STD_INPUT_HANDLE);
    SetConsoleTitle(L"Ones and zeroes");
    SetConsoleWindowInfo(wHnd, TRUE, &windowSize);
    SetConsoleScreenBufferSize(wHnd, bufferSize);
}

void Clear()
{
    memset(consoleBuffer, 0, WIDTH * HEIGHT * sizeof(CHAR_INFO));
	memset(colorBuffer, 0, WIDTH * HEIGHT * sizeof(float3));
	memset(worldSpaceBuffer, 0, WIDTH * HEIGHT * sizeof(float3));
    for (int i = 0; i < HEIGHT * WIDTH; ++i) depthBuffer[i] = 2.f; // Whatever greater than screen buffer far plane
}

void Blit()
{
    WriteConsoleOutputA(wHnd, consoleBuffer, characterBufferSize, characterPosition, &consoleWriteArea);
}

void SetPerspective(float fovy, float aspect = (float)WIDTH / HEIGHT * 0.5f, float near = 0.01f, float far = 100.f)
{
	projection = linalg::perspective_matrix(fovy, aspect, near, far);
	VP = linalg::mul(projection, view);
}
// Just a shortcut
void SetView(float4x4 viewMat) 
{
	view = viewMat;
	VP = linalg::mul(projection, view);
}

inline WORD _colorAttrib(float3 c) 
{
	float sum = c.x + c.y + c.z;
	if (sum == 0) return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
	WORD attr = 0;
	if (c.x / sum >= 0.3332f) attr |= FOREGROUND_RED;
	if (c.y / sum >= 0.3332f) attr |= FOREGROUND_GREEN;
	if (c.z / sum >= 0.3332f) attr |= FOREGROUND_BLUE;
	return attr;
}
// ░ ▒ ▓ █
inline char _intensityChar(float intensity) 
{
	if (intensity <= 0.05f)
		return ' ';
	else if (intensity <= 0.25f)
		return (char)176; //'░';
	else if (intensity <= 0.5f)
		return (char)177; //'▒';
	else if (intensity <= 0.75f)
		return (char)178; //'▓';
	else
		return (char)219; //'█';
}
void Finalize()
{
    for (int y = 0; y < HEIGHT; ++y)
    {
        for (int x = 0; x < WIDTH; ++x)
        {
			int index = x + WIDTH * y;
			CHAR_INFO& out = consoleBuffer[index];

			float3 worldPos = worldSpaceBuffer[index];
			float3 lightColor = {0, 0, 0};
			for (auto& light : lights) 
			{
				float dist2 = linalg::length2(worldPos - light.position) + 0.0001f;
				lightColor += light.color * light.intensity / dist2;
			}

			out.Attributes = 0;
			float3 finalColor = colorBuffer[index];
			finalColor.x *= lightColor.x;
			finalColor.y *= lightColor.y;
			finalColor.z *= lightColor.z;

			out.Attributes = _colorAttrib(finalColor);
			float finalColorPower = linalg::length(finalColor);

			if (depthBuffer[index] > 0 && depthBuffer[index] < 1)
				out.Char.AsciiChar = _intensityChar(finalColorPower);
        }
    }
}

// Convert to pixel coords, but don't touch the depth
inline float4 _ToPixelCoords(float4 screenSpace) 
{
    return {
        screenSpace.x = (screenSpace.x + 1.0f) * 0.5f * WIDTH,
        screenSpace.y = (screenSpace.y + 1.0f) * 0.5f * HEIGHT,
        screenSpace.z,
		screenSpace.w
    };
}
// Ignoring depth
inline bool _SameCoords(float3 v1, float3 v2) 
{
    return v1.x == v2.x && v1.y == v2.y;
}
inline bool _PixelClose(float a, float b) 
{
    return std::abs(a - b) > 0.25f;
}
inline void _SetPixel/*Safely*/(float x, float y, float3 color, float depth, float3 worldSpace = {0,0,0})
{
	if (x < 0 || y < 0 || x >= WIDTH || y >= HEIGHT)
		return;

	if (depth < 0) // discard back
		return;

	int index = (int)x + (int)y * WIDTH;

	if (depthBuffer[index] < depth) // discard overlayed
		return;

	depthBuffer[index] = depth;
	colorBuffer[index] = color;
	worldSpaceBuffer[index] = worldSpace;
}

// Geometry functions (screen space in [-1, 1] ranges)
void _DrawLineScreen(float3 v1, float3 v2, float3 v1Color, float3 v2Color, float3 v1WorldSpace, float3 v2WorldSpace)
{
	float& x1 = v1.x;
	float& x2 = v2.x;
	float& y1 = v1.y;
	float& y2 = v2.y;

	float xdiff = (x2 - x1);
	float ydiff = (y2 - y1);

	if (xdiff == 0.0f && ydiff == 0.0f) 
	{
		_SetPixel(x1, y1, v1Color, v1.z, v1WorldSpace);
		return;
	}

	if (fabs(xdiff) > fabs(ydiff)) 
	{
		float xmin, xmax;

		if (x1 < x2) 
		{
			xmin = x1;
			xmax = x2;
		}
		else 
		{
			xmin = x2;
			xmax = x1;
		}

		float slope = ydiff / xdiff;
		int i = 0;
		for (float x = xmin; x <= xmax && i < WIDTH * 2; x += 1.0f, i++)
		{
			float y = y1 + ((x - x1) * slope);
			float alpha = (x - x1) / xdiff;
			float3 color = v1Color + ((v2Color - v1Color) * alpha);
			float3 ws = v1WorldSpace + ((v2WorldSpace - v1WorldSpace) * alpha);
			float depth = v1.z + ((v2.z - v1.z) * alpha);
			_SetPixel(x, y, color, depth, ws);
		}
	}
	else 
	{
		float ymin, ymax;

		if (y1 < y2) 
		{
			ymin = y1;
			ymax = y2;
		}
		else 
		{
			ymin = y2;
			ymax = y1;
		}

		float slope = xdiff / ydiff;
		int i = 0;
		for (float y = ymin; y <= ymax && i < WIDTH * 2; y += 1.0f, i++) 
		{
			float x = x1 + ((y - y1) * slope);
			float alpha = (y - y1) / ydiff;
			float3 color = v1Color + ((v2Color - v1Color) * alpha);
			float3 ws = v1WorldSpace + ((v2WorldSpace - v1WorldSpace) * alpha);
			float depth = v1.z + ((v2.z - v1.z) * alpha);
			_SetPixel(x, y, color, depth, ws);
		}
	}
}




// General Draw functions
void DrawLine(float3 v1, float3 v2, float3 v1Color, float3 v2Color) 
{
	float4 v1Screen = linalg::mul(VP, float4(v1, 1));
	v1Screen /= v1Screen.w;
	float4 v2Screen = linalg::mul(VP, float4(v2, 1));
	v2Screen /= v2Screen.w;

	v1Screen = _ToPixelCoords(v1Screen);
	v2Screen = _ToPixelCoords(v2Screen);

	_DrawLineScreen(v1Screen.xyz(), v2Screen.xyz(), v1Color, v2Color, v1, v2);
}