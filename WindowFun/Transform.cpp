#include "Transform.h"
#include <math.h>

using namespace std;
using namespace linalg;
using namespace linalg::aliases;

Transform::Transform() : transform(identity)
{

}

linalg::aliases::float4x4 Transform::GetMatrix()
{
	return transform;
}

void Transform::SetTransform(const linalg::aliases::float4x4& t)
{
	transform = t;
}

void Transform::SetTransform(const linalg::aliases::float3& pos, const linalg::aliases::float3& rot, const linalg::aliases::float3& scale)
{
	transform = mul(translation_matrix(pos), mul(GetRotEuler(rot), scaling_matrix(scale)));
}

void Transform::Rotate(const linalg::aliases::float3& euler)
{
	transform = mul(transform, GetRotEuler(euler));
}

void Transform::Translate(const linalg::aliases::float3& offset)
{
	transform = mul(transform, translation_matrix(offset));
}

void Transform::Scale(const linalg::aliases::float3& scale)
{
	transform = mul(transform, scaling_matrix(scale));
}
