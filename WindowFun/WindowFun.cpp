#include <iostream>
#include <math.h>
#include <chrono>
#include <thread>
#include <algorithm>

#include "ConRender.h"

using namespace std;

int main_old()
{
    InitConsoleBuffer();

    auto startTime = chrono::steady_clock::now();
    float lastFrameTime = 0.016f;

    float4x4 model = linalg::identity;
    SetPerspective(1.0472f * 1.5f); // About 60 degrees
    SetView(linalg::translation_matrix(float3{ 0, 0, -1.5f }));
    lights.push_back(
        {
            float3 {0.6f, 0, 0}, // Light pos
            float3 {1, 0.8f, 0.8f}, // Light Color
            1.2f
        });
    float3 verts[] =
    {
        {+1,-1,-1}, //0
        {+1,-1,+1}, //1
        {+1,+1,-1}, //2
        {+1,+1,+1}, //3
        {-1,-1,-1}, //4
        {-1,-1,+1}, //5
        {-1,+1,-1}, //6
        {-1,+1,+1}, //7
    };
    float3 colors[] =
    {
        {+1,    0.8f,    0.8f},
        {+1,    0.8f,    +1},
        {+1,    +1,     0.8f},
        {+1,    +1,     +1},
        {0.8f,   0.8f,    0.8f},
        {0.8f,   0.8f,    +1},
        {0.8f,   +1,     0.8f},
        {0.8f,   +1,     +1},
    };
    uint2 idxs[] =
    {
        // y = -1 section
        {0, 1}, {1, 5}, {5, 4}, {4, 0},
        // y = 1 section
        {2, 3}, {3, 7}, {7, 6}, {6, 2},
        // supports
        {0, 2}, {1, 3}, {4, 6}, {5, 7}
    };
    uint3 tris[] =
    {
        // x+
        {0, 1, 2}, {3,1,2}
    };


    for(;;)
    {
        auto frameStartTime = chrono::steady_clock::now();
        float secondsSinceStart = chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - startTime).count() * 1e-6f;

        float rotAngle = secondsSinceStart;
        const float scale = 1.5f;
        model = linalg::identity;
        model = linalg::mul(model, linalg::translation_matrix(float3{ 0, -0.4f, 0.0f }));
        model = linalg::mul(model, linalg::scaling_matrix(float3{ scale,scale,scale }));
        model = linalg::mul(model, linalg::rotation_matrix(linalg::rotation_quat(float3{0,1,0}, rotAngle*0.2f)));
        //float4x4 viewMat = linalg::translation_matrix(float3{ std::sinf(secondsSinceStart * 4.9f) * 0.1f + 0.2f, std::cosf(secondsSinceStart * 7.71f) * 0.1f + 0.5f, -1.5f });
        float4x4 viewMat = linalg::translation_matrix(float3{ 0.2f, 0.5f, -1.5f });
        viewMat = linalg::mul(viewMat, linalg::rotation_matrix(linalg::rotation_quat(float3{ 0,1,0 }, 0.2f)));
        SetView(viewMat);

        /* Render */
        Clear();
        for (auto& idx : idxs)
        {
            float4 p1, p2;
            p1 = linalg::mul(model, float4(verts[idx.x], 1));
            p2 = linalg::mul(model, float4(verts[idx.y], 1));
            DrawLine(p1.xyz(), p2.xyz(), colors[idx.x], colors[idx.y]);
        }
        for (auto& idx : tris)
        {
            float4 p1, p2, p3;
            p1 = linalg::mul(model, float4(verts[idx.x], 1));
            p2 = linalg::mul(model, float4(verts[idx.y], 1));
            p3 = linalg::mul(model, float4(verts[idx.z], 1));
            DrawTriangle(p1.xyz(), p2.xyz(), p3.xyz(), colors[idx.x], colors[idx.y], colors[idx.z]);
        }
        DrawLine(float3{ -1, 0, 0 }, float3{ 1, 0, 0 }, float3{ 1,0,0 }, float3{ 1,0,0 });
        DrawLine(float3{ 0, -1, 0 }, float3{ 0, 1, 0 }, float3{ 0,1,0 }, float3{ 0,1,0 });
        DrawLine(float3{ 0, 0, -1 }, float3{ 0, 0, 1 }, float3{ 0,0,1 }, float3{ 0,0,1 });
        Finalize();
        Blit();

        float frameTime = chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - frameStartTime).count() * 1e-6f;

        this_thread::sleep_for(16ms);
    }

    return 0;
}

