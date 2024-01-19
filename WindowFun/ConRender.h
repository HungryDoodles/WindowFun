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
void _DrawLineScreen(float4 v1, float4 v2, float3 v1Color, float3 v2Color, float3 v1WorldSpace, float3 v2WorldSpace)
{
	float4 p1 = _ToPixelCoords(v1);
	float4 p2 = _ToPixelCoords(v2);

	float& x1 = p1.x;
	float& x2 = p2.x;
	float& y1 = p1.y;
	float& y2 = p2.y;

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

/*
// Assumes y is sorted sorted
void _DrawTriangleFlatBottom(
	float4 v1, float4 v2, float4 v3,
	float3 c1, float3 c2, float3 c3,
	float3 w1, float3 w2, float3 w3) 
{
	float invslope1 = (v2.x - v1.x) / (v2.y - v1.y);
	float invslope2 = (v3.x - v1.x) / (v3.y - v1.y);

	float curx1 = v1.x;
	float curx2 = v1.x;

	int counter = 0;
	for (int scanlineY = v1.y; scanlineY < v2.y && counter <= HEIGHT; scanlineY += 1, counter++)
	{
		float alpha = (scanlineY - v1.y) / (v2.y - v1.y);
		float depth12 = _Lerp(v1.z, v2.z, alpha);
		float depth13 = _Lerp(v1.z, v3.z, alpha);
		float3 c12 = _Lerp(c1, c2, alpha);
		float3 c13 = _Lerp(c1, c3, alpha);
		float3 w12 = _Lerp(w1, w2, alpha);
		float3 w13 = _Lerp(w1, w3, alpha);
		_DrawLineScreen(float4{ curx1, (float)scanlineY, depth12 }, float4{ curx2, (float)scanlineY, depth13 }, c12, c13, w12, w13);
		curx1 += invslope1;
		curx2 += invslope2;
	}
}
// Assumes y is sorted sorted
void _DrawTriangleFlatTop(
	float4 v1, float4 v2, float4 v3,
	float3 c1, float3 c2, float3 c3,
	float3 w1, float3 w2, float3 w3)
{
	float invslope1 = (v3.x - v1.x) / (v3.y - v1.y);
	float invslope2 = (v3.x - v2.x) / (v3.y - v2.y);

	float curx1 = v3.x;
	float curx2 = v3.x;

	int counter = 0;
	for (int scanlineY = v3.y; scanlineY > v1.y && counter <= HEIGHT; scanlineY -= 1, counter++)
	{
		float alpha = (scanlineY - v1.y) / (v3.y - v1.y);
		float depth13 = _Lerp(v1.z, v3.z, alpha);
		float depth23 = _Lerp(v2.z, v3.z, alpha);
		float3 c13 = _Lerp(c1, c3, alpha);
		float3 c23 = _Lerp(c2, c3, alpha);
		float3 w13 = _Lerp(w1, w3, alpha);
		float3 w23 = _Lerp(w2, w3, alpha);
		_DrawLineScreen(float3{ curx1, (float)scanlineY, depth13 }, float3{ curx2, (float)scanlineY, depth23 }, c13, c23, w13, w23);
		curx1 -= invslope1;
		curx2 -= invslope2;
	}
}
void _DrawTriangleScreen(
	float4 v1, float4 v2, float4 v3,
	float3 c1, float3 c2, float3 c3, 
	float3 w1, float3 w2, float3 w3)
{
	if (v2.y < v1.y) { std::swap(v1, v2); std::swap(c1, c2); std::swap(w1, w2); }
	if (v3.y < v1.y) { std::swap(v3, v1); std::swap(c3, c1); std::swap(w3, w1); }
	if (v3.y < v2.y) { std::swap(v3, v2); std::swap(c3, c2); std::swap(w3, w2); }

	if (v2.y == v3.y) 
	{
		_DrawTriangleFlatBottom(v1,v2,v3,c1,c2,c3,w1,w2,w3);
	}
	else if (v1.y == v2.y) 
	{
		_DrawTriangleFlatTop(v1,v2,v3,c1,c2,c3,w1,w2,w3);
	}
	else 
	{
		float alpha = (v2.y - v1.y) / (v3.y - v1.y);
		float3 v4 = float3(_Lerp(v1.x, v3.x, alpha), v2.y, _Lerp(v1.z, v3.z, alpha));
		float3 c4 = _Lerp(c1, c3, alpha);
		float3 w4 = _Lerp(w1, w3, alpha);
		_DrawTriangleFlatBottom(v1,v2,v4,c1,c2,c4,w1,w2,w4);
		_DrawTriangleFlatTop(v2,v4,v3,c2,c4,c3,w2,w4,w3);
	}
}
*/

// Guarantees a triangle, but flickers on Y coordinate
void _DrawTriangleBarycentric(
	float4 v1, float4 v2, float4 v3,
	float3 c1, float3 c2, float3 c3,
	float3 w1, float3 w2, float3 w3) 
{
	float4 p1 = _ToPixelCoords(v1);
	float4 p2 = _ToPixelCoords(v2);
	float4 p3 = _ToPixelCoords(v3);

	float maxX = std::max(p1.x, std::max(p2.x, p3.x));
	float minX = std::min(p1.x, std::min(p2.x, p3.x));
	float maxY = std::max(p1.y, std::max(p2.y, p3.y));
	float minY = std::min(p1.y, std::min(p2.y, p3.y));
	// Failsafe
	maxX = std::min(maxX, float(WIDTH));
	minX = std::max(minX, 0.0f);
	maxY = std::min(maxY, float(HEIGHT));
	minY = std::max(minY, 0.0f);

	float2 d21 = float2(p2.x - p1.x, p2.y - p1.y);
	float2 d31 = float2(p3.x - p1.x, p3.y - p1.y);
	float d1x2_inv = 1.0f / (d21.x * d31.y - d21.y * d31.x);

	for (int x = minX; x <= maxX; ++x) 
	{
		for (int y = minY; y <= maxY; ++y) 
		{
			float2 p = float2(x, y) - float2(p1.x, p1.y);

			float alpha2 = (p.x * d31.y - p.y * d31.x) * d1x2_inv; // alpha towards point 2
			float alpha3 = (d21.x * p.y - d21.y * p.x) * d1x2_inv; // alpha towards point 3
			float alpha1 = 1.0f - alpha2 - alpha3;

			if ((alpha2 >= 0) && (alpha3 >= 0) && (alpha2 + alpha3 <= 1)) 
			{
				//float4 v = v1 * alpha1 + v2 * alpha2 + v3 * alpha3;
				float z = v1.z * alpha1 + v2.z * alpha2 + v3.z * alpha3;
				float3 c = c1 * alpha1 + c2 * alpha2 + c3 * alpha3;
				float3 w = w1 * alpha1 + w2 * alpha2 + w3 * alpha3;

				_SetPixel(x, y, c, z, w);
			}
		}
	}
}




// General Draw functions
void DrawLine(float3 v1, float3 v2, float3 v1Color, float3 v2Color) 
{
	float4 v1Proj = linalg::mul(VP, float4(v1, 1));
	v1Proj /= v1Proj.w;
	float4 v2Proj = linalg::mul(VP, float4(v2, 1));
	v2Proj /= v2Proj.w;

	//v1Proj = _ToPixelCoords(v1Proj);
	//v2Proj = _ToPixelCoords(v2Proj);

	_DrawLineScreen(v1Proj, v2Proj, v1Color, v2Color, v1, v2);
}

void DrawTriangle(float3 v1, float3 v2, float3 v3, float3 v1Color, float3 v2Color, float3 v3Color) 
{
	float4 v1Proj = linalg::mul(VP, float4(v1, 1));
	v1Proj /= v1Proj.w;
	float4 v2Proj = linalg::mul(VP, float4(v2, 1));
	v2Proj /= v2Proj.w;
	float4 v3Proj = linalg::mul(VP, float4(v3, 1));
	v3Proj /= v3Proj.w;

	/*v1Screen = _ToPixelCoords(v1Screen);
	v2Screen = _ToPixelCoords(v2Screen);
	v3Screen = _ToPixelCoords(v3Screen);*/

	// Broken
	//_DrawTriangleScreen(v1Proj, v2Proj, v3Proj, v1Color, v2Color, v3Color, v1, v2, v3);

	_DrawTriangleBarycentric(v1Proj, v2Proj, v3Proj, v1Color, v2Color, v3Color, v1, v2, v3);
}