#pragma once
#include "linalg.h"

class Transform
{
public:
	Transform();

	linalg::aliases::float4x4 GetMatrix();
	void SetTransform(const linalg::aliases::float4x4& t);
	void SetTransform(const linalg::aliases::float3& pos, const linalg::aliases::float3& rotation = {0, 0, 0}, const linalg::aliases::float3& scale = { 1, 1, 1 });
	void Rotate(const linalg::aliases::float3& euler);
	void Translate(const linalg::aliases::float3& offset);
	void Scale(const linalg::aliases::float3& scale);

protected:
	linalg::aliases::float4x4 transform;

	inline linalg::aliases::float4x4 GetRotEuler(const linalg::aliases::float3& eulers) 
	{
		float sx = sin(eulers.x), cx = cos(eulers.x);
		float sy = sin(eulers.y), cy = cos(eulers.y);
		float sz = sin(eulers.z), cz = cos(eulers.z);
		return linalg::aliases::float4x4
		{
			{ cx * cy,	cx * sy * sz - sx * cz,	cx * sy * cz + sx * sz, 0},
			{ sx * cy,	sx * sy * sz + cx * cz,	sx * sy * cz - cx * sz, 0},
			{ -sy,		cy * sz,				cy * cz,				0},
			{ 0,		0,						0,						1}
		};
	}
};