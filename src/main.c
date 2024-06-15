#include "raylib.h"
#include "raymath.h"
#include "common.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

static int framesCounter = 0;

// function prototype
static void UpdateDrawFrame(void);

int main(void) {
    SetConfigFlags(FLAG_MSAA_4X_HINT);

    InitWindow(g_screenWidth, g_screenHeight, "Verlet Integration Test");
    // load textures / initialize variables
    InitUI();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60);

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    CloseWindow();
    return 0;
}

static void UpdateDrawFrame() {
    // Update
    Color objectColor = { g_red, g_green, g_blue, 255 };

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !IsMouseOnUI()) {
        framesCounter = 100;
    }
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !IsMouseOnUI()) {
        if (framesCounter >= 60/(int)g_spawnRate) {
            framesCounter = 0;
            if (g_structType == DEFAULT) {
                SpawnVerletObject(
                    (Vector2){
                        g_mousePos.x, //+ GetRandomValue(-5, 5),
                        g_mousePos.y //+ GetRandomValue(-5, 5)
                    },
                    (int)g_spawnRadius,
                    (Color){
                        Clamp((int)g_red   + GetRandomValue(-40, 40), 0, 255),
                        Clamp((int)g_green + GetRandomValue(-40, 40), 0, 255),
                        Clamp((int)g_blue  + GetRandomValue(-40, 40), 0, 255),
                        255
                    }
                );
            }
            else if (g_structType == ROPE) {
                SpawnStructureRope(g_mousePos, 35, 25, 8, BOTH, objectColor);
            }
            else if (g_structType == CLOTH) {
                SpawnStructureCloth(g_mousePos, 60, 12, 0, objectColor);
            }
        }
    }
    // cap dt at a reasonable value
    float frameTime = GetFrameTime();
    frameTime = frameTime > MAX_FRAME_TIME? MAX_FRAME_TIME: frameTime;
    float substepTime = frameTime/(float)PHYSICS_SUBSTEPS;

    UpdateVerlet(substepTime);
    UpdateUI();
    framesCounter++;

    // Draw
    BeginDrawing();
        ClearBackground((Color){ 50, 45, 55, 255 });
        if (g_applyConstraint) {
            DrawCircleSector((Vector2){(float)g_screenWidth/2, (float)g_screenHeight/2},
                    400, 0, 360, 128, (Color){ 28, 27, 25, 255 });
        }
        DrawVerlet();
        DrawUI();
        DrawFPS(10, 10);
    EndDrawing();
}
