#include <iostream>
#include <math.h>
#include <chrono>
#include <thread>
#include <algorithm>
#include <vector>
#include <random>

#include "ConRender.h"
#include "Mesh.h"
#include "Camera.h"

constexpr int numCubes = 20;

using namespace std;

int main() 
{
	InitConsoleBuffer();

	float lastFrameTime = 0.016f;

    std::default_random_engine randEngine;
    std::uniform_real_distribution<float> rand(-10.0f, 10.0f);

    lights.push_back(
        {
            float3 {0.6f, 0, 0}, // Light pos
            float3 {1, 0.8f, 0.8f}, // Light Color
            50.f
        });

    Camera camera;
    vector<Mesh> mesh(numCubes);

    for (int i = 0; i < numCubes; ++i) 
    {
        float alpha = i / float(numCubes);
        mesh[i] = Mesh::MakeCube_Tris(0.25f, float3{1.0, 1.0f - alpha, alpha});
        mesh[i].Translate(float3{ rand(randEngine), rand(randEngine), rand(randEngine) });
    }

    camera.SetTransform(float3{0,0,-5});
    
    // Ticking
    while (true) 
    {
        auto startTime = chrono::steady_clock::now();

        for (int i = 0; i < mesh.size(); ++i) 
        {
            mesh[i].Rotate(float3{ 0, lastFrameTime, 0 });
        }

        // Drawing
        Clear();
        camera.Apply();
        for (int i = 0; i < mesh.size(); ++i)
            mesh[i].Draw();
        Finalize();
        Blit();

        auto endTime = chrono::steady_clock::now();
        lastFrameTime = float(chrono::duration_cast<chrono::microseconds>(endTime - startTime).count()) * 1e-6f;
    }

	return 0;
}