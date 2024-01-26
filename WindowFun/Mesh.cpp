#include "Mesh.h"
#include "ConRender.h"

using namespace std;

using namespace linalg;
using namespace linalg::aliases;

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
}

void Mesh::Draw()
{
	auto M = GetMatrix();
	for (auto& idx : lines)
	{
		if (idx.x < 0 || idx.y < 0 || idx.x >= verts.size() || idx.y >= verts.size())
			continue;
		float3 v1 = (mul(M, float4(verts[idx.x], 1))).xyz();
		float3 v2 = (mul(M, float4(verts[idx.y], 1))).xyz();
		DrawLine(v1, v2, colors[idx.x], colors[idx.y]);
	}
	for (auto& idx : tris)
	{
		if (idx.x < 0 || idx.y < 0 || idx.z < 0 || idx.x >= verts.size() || idx.y >= verts.size() || idx.z >= verts.size())
			continue;
		float3 v1 = (mul(M, float4(verts[idx.x], 1))).xyz();
		float3 v2 = (mul(M, float4(verts[idx.y], 1))).xyz();
		float3 v3 = (mul(M, float4(verts[idx.z], 1))).xyz();
		DrawTriangle(v1, v2, v3, colors[idx.x], colors[idx.y], colors[idx.z]);
	}
}

void Mesh::Clear()
{
	verts.clear();
	colors.clear();
	lines.clear();
	tris.clear();
}

void Mesh::SetVerts(
	const linalg::aliases::float3* const verts_in, 
	const linalg::aliases::float3* const colors_in, int count)
{
	if (count < 0) return;
	verts.resize(count);
	colors.resize(count);

	for (int i = 0; i < count; ++i)
	{
		verts[i] = verts_in[i];
		colors[i] = colors_in[i];
	}
}
void Mesh::SetVerts(
	const linalg::aliases::float3* const verts_in,
	const linalg::aliases::float3& color_in, int count)
{
	if (count < 0) return;
	verts.resize(count);
	colors.resize(count);

	for (int i = 0; i < count; ++i)
	{
		verts[i] = verts_in[i];
		colors[i] = color_in;
	}
}


void Mesh::SetLines(const linalg::aliases::uint2* const lines_in, int count) 
{
	if (count < 0) return;
	lines.resize(count);

	for (int i = 0; i < count; ++i) 
	{
		lines[i] = lines_in[i];
	}
}

void Mesh::SetTris(const linalg::aliases::uint3* const tris_in, int count)
{
	if (count < 0) return;
	tris.resize(count);

	for (int i = 0; i < count; ++i)
	{
		tris[i] = tris_in[i];
	}
}

Mesh Mesh::MakeCube_Tris(float s, const linalg::aliases::float3& color)
{
	Mesh mesh;

	const float3 verts[] =
	{
		{ s, s, s },	// 0
		{ s, s, -s },	// 1
		{ s, -s, s },	// 2
		{ s, -s, -s },	// 3
		{ -s, s, s },	// 4
		{ -s, s, -s },	// 5
		{ -s, -s, s },	// 6
		{ -s, -s, -s },	// 7
	};
	const uint3 tris[] =
	{
		//+x
		{0, 1, 2}, {3, 1, 2},
		//-x
		{4, 5, 6}, {7, 5, 6},
		//+y
		{1, 0, 5}, {4, 0, 5},
		//-y
		{3, 2, 7}, {6, 2, 7},
		//+z
		{0, 2, 4}, {6, 2, 4},
		//-z
		{1, 3, 5}, {7, 3, 5}
	};

	mesh.SetVerts(verts, color, 8);

	mesh.SetTris(tris, 12);

	return mesh;
}

Mesh Mesh::MakeCube_Lines(float s, const linalg::aliases::float3& color)
{
	Mesh mesh;

	const float3 verts[] =
	{
		{ s, s, s },	// 0
		{ s, s, -s },	// 1
		{ s, -s, s },	// 2
		{ s, -s, -s },	// 3
		{ -s, s, s },	// 4
		{ -s, s, -s },	// 5
		{ -s, -s, s },	// 6
		{ -s, -s, -s },	// 7
	};
	const uint2 lines[] =
	{
		{0, 1}, {0, 2}, {0, 4},
		{1, 3}, {1, 5},
		{2, 3}, {2, 6},
		{3, 5},
		{4, 5}, {4, 6},
		{5, 7},
		{6, 7}
	};

	mesh.SetVerts(verts, color, 8);

	mesh.SetLines(lines, 12);

	return mesh;
}