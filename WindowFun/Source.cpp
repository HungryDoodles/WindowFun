#include <iostream>
#include <math.h>
#include <chrono>
#include <thread>
#include <algorithm>
#include <vector>
#include <thread>
#include <random>

#include "input.h"
#include "ConRender.h"
#include "Mesh.h"
#include "Camera.h"

constexpr int numCubes = 1000;

using namespace std;

struct CameraLook
{
    float3 pos;
    float2 rot;
};

void MoveCamera(const DirectInput& input, Camera& camera, CameraLook& look, float speed, float lookSens, float smoothAlpha = 0.3f);

int main() 
{
    DirectInput dinput;
    if (!dinput.init())
        cout << "Direct input failed" << endl;

	InitConsoleBuffer();

	float lastFrameTime = 0.016f;
    float lastRealFrameTime = 0;
    char screenTextBuf[WIDTH * 2];
    screenTextBuf[WIDTH * 2 - 1] = '\0';

    std::default_random_engine randEngine;
    std::uniform_real_distribution<float> rand(-100.0f, 100.0f);
    std::uniform_real_distribution<float> colorRand(0, 1.0f);


    lights.push_back(
        {
            float3 {0, 0, 0}, // Light pos
            float3 {1.0f, 1.0f, 1.0f}, // Light Color
            1000.f
        });

    lights.push_back(
        {
            float3 {0.6f, 0, 0}, // Light pos
            float3 {1, 0.8f, 0.8f}, // Light Color
            1000.f
        });

    Camera camera;
    CameraLook look;
    vector<Mesh> mesh(numCubes);

    for (int i = 0; i < numCubes; ++i) 
    {
        float alpha = i / float(numCubes);
        mesh[i] = Mesh::MakeCube_Tris(2.0f, float3{ colorRand(randEngine), colorRand(randEngine), colorRand(randEngine) });
        mesh[i].Translate(float3{ rand(randEngine), rand(randEngine), rand(randEngine) });
    }

    camera.SetTransform(float3{0,0,-5});
    
    // Ticking
    while (true) 
    {
        auto startTime = chrono::steady_clock::now();
        dinput.Update();

        /*for (int i = 0; i < mesh.size(); ++i) 
        {
            mesh[i].Rotate(float3{ 0, lastFrameTime, 0 });
        }*/

        // Drawing
        Clear();
        MoveCamera(dinput, camera, look, lastFrameTime * 8, 0.004f);
        lights[0].position = -look.pos;
        camera.Apply();
        for (int i = 0; i < mesh.size(); ++i)
            mesh[i].Draw();
        Finalize();

        sprintf_s(screenTextBuf, "Framerate: %f fps", (1 / lastRealFrameTime));
        DrawTextOnScreen(screenTextBuf, 0, 0);
        Blit();

        auto endTime = chrono::steady_clock::now();
        lastFrameTime = float(chrono::duration_cast<chrono::microseconds>(endTime - startTime).count()) * 1e-6f;
        lastRealFrameTime = lastFrameTime;
        
        if (lastFrameTime < 1.f / 60)
            this_thread::sleep_for(std::chrono::microseconds((int)((1.f/60 - lastFrameTime) * 1e6)));
        lastFrameTime = max(0.0166666f, lastFrameTime);
    }

	return 0;
}








void MoveCamera(const DirectInput& input, Camera& camera, CameraLook& look, float speed, float lookSens, float smoothAlpha)
{
    look.rot.x -= input.MouseDX() * lookSens;
    look.rot.y -= input.MouseDY() * lookSens;

    float3 fwd = float3{ 0, 0, 1 };
    float3 up = float3{ 0, 1, 0 };
    float3 right = float3{ 1, 0, 0 };
    // Screen space coords
    float sX = std::sin(look.rot.x),
        sY = std::sin(look.rot.y),
        cX = std::cos(look.rot.x),
        cY = std::cos(look.rot.y);
    // World space coords
    float3x3 rotY =
    {
        {cX, 0, -sX},
        {0, 1, 0},
        {sX, 0, cX}
    };
    float3x3 rotX =
    {
        {1, 0, 0},
        {0, cY, -sY},
        {0, sY, cY}
    };
    fwd = mul(rotX, fwd);
    fwd = mul(rotY, fwd);
    right = mul(rotX, right);
    right = mul(rotY, right);

    if (input.Key(DIK_LSHIFT))
        speed *= 15;

    if (input.Key(DIK_W))
        look.pos += fwd * speed;
    if (input.Key(DIK_S))
        look.pos -= fwd * speed;
    if (input.Key(DIK_D))
        look.pos -= right * speed;
    if (input.Key(DIK_A))
        look.pos += right * speed;

    if (input.Key(DIK_SPACE))
        look.pos += up * speed;
    if (input.Key(DIK_LCONTROL))
        look.pos -= up * speed;

    float4x4 oldMat = camera.GetMatrix();
    float4x4 newMat = linalg::identity;

    float4x4 rotY4 = 
    {
        {cX, 0, sX, 0},
        {0, 1, 0, 0},
        {-sX, 0, cX, 0},
        {0, 0, 0, 1}
    };
    float4x4 rotX4 =
    {
        {1, 0, 0, 0},
        {0, cY, sY, 0},
        {0, -sY, cY, 0},
        {0, 0, 0, 1}
    };
    newMat = mul(newMat, rotX4);
    newMat = mul(newMat, rotY4);
    newMat = mul(newMat, linalg::translation_matrix(look.pos));

    camera.SetTransform(newMat * (smoothAlpha) + oldMat * (1 - smoothAlpha));
}