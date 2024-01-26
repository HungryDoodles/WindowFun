#pragma once
#include "linalg.h"
#include <vector>

#include "Transform.h"

class Mesh : Transform
{
public:
	Mesh();
	virtual ~Mesh();

	virtual void Draw();
	virtual void Clear();

	void SetVerts(const linalg::aliases::float3* const verts, 
		const linalg::aliases::float3* const colors,
		int count);
	void SetVerts(const linalg::aliases::float3* const verts,
		const linalg::aliases::float3& color,
		int count);

	void SetLines(const linalg::aliases::uint2* const lines, int count);
	void SetTris(const linalg::aliases::uint3* const tris, int count);


protected:
	std::vector<linalg::aliases::float3> verts;
	std::vector<linalg::aliases::float3> colors;
	std::vector<linalg::aliases::uint2> lines;
	std::vector<linalg::aliases::uint3> tris;



public:
	static Mesh MakeCube_Tris(float size, const linalg::aliases::float3& color);
	static Mesh MakeCube_Lines(float size, const linalg::aliases::float3& color);
};