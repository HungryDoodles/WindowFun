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

#define WIDTH 400
#define HEIGHT 200

struct LIGHT_INFO 
{
	float3 position;
	float3 color;
	float intensity;
};


inline HANDLE wHnd; /* write (output) handle */
inline HANDLE rHnd; /* read (input handle */

/* A CHAR_INFO structure containing data about a single character */
inline CHAR_INFO consoleBuffer[WIDTH * HEIGHT];
inline float3 worldSpaceBuffer[WIDTH * HEIGHT];
inline float3 colorBuffer[WIDTH * HEIGHT];
inline float depthBuffer[WIDTH * HEIGHT];
inline std::vector<LIGHT_INFO> lights;
/* Area info */
inline COORD characterBufferSize = { WIDTH, HEIGHT };
inline COORD characterPosition = { 0, 0 };
inline SMALL_RECT consoleWriteArea = { 0, 0, WIDTH - 1, HEIGHT - 1 };

inline float4x4 view;
inline float4x4 projection;
inline float4x4 VP;


void InitConsoleBuffer();

void Clear();

void Blit();

void SetPerspective(float fovy, float aspect = (float)WIDTH / HEIGHT * 0.5f, float near = 0.01f, float far = 1000.f);
// Just a shortcut
void SetView(float4x4 viewMat);

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
void Finalize();

template<typename T>
inline T _Lerp(T a, T b, float alpha)
{
	return (1.0f - alpha) * a + alpha * b;
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
inline bool _SetPixel/*Safely*/(float x, float y, float3 color, float depth, float3 worldSpace = { 0,0,0 })
{
	if (x < 0 || y < 0 || x >= WIDTH || y >= HEIGHT)
		return false;

	if (depth < 0) // discard back
		return false;

	int index = (int)x + (int)y * WIDTH;

	if (depthBuffer[index] < depth) // discard overlayed
		return false;

	depthBuffer[index] = depth;
	colorBuffer[index] = color;
	worldSpaceBuffer[index] = worldSpace;

	return true;
}

// Geometry functions (screen space in [-1, 1] ranges)
void _DrawLineScreen(float4 v1, float4 v2, float3 v1Color, float3 v2Color, float3 v1WorldSpace, float3 v2WorldSpace);


// Guarantees a triangle, but flickers on Y coordinate
void _DrawTriangleBarycentric(
	float4 v1, float4 v2, float4 v3,
	float3 c1, float3 c2, float3 c3,
	float3 w1, float3 w2, float3 w3);




// General Draw functions
void DrawLine(float3 v1, float3 v2, float3 v1Color, float3 v2Color);

void DrawTriangle(float3 v1, float3 v2, float3 v3, float3 v1Color, float3 v2Color, float3 v3Color);

void DrawTextOnScreen(const char* text, int x, int y);