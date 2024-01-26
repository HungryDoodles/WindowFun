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

	auto startTime = chrono::steady_clock::now();
	float lastFrameTime = 0.016f;

    lights.push_back(
        {
            float3 {0.6f, 0, 0}, // Light pos
            float3 {1, 0.8f, 0.8f}, // Light Color
            1.2f
        });

    vector<Mesh> mesh(numCubes);

    for (int i = 0; i < numCubes; ++i) 
    {
        float alpha = float(numCubes) / i;
        mesh[i] = Mesh::MakeCube_Tris(0.25f, float3{1.0, 1.0f - alpha, alpha});


    }

	return 0;
}