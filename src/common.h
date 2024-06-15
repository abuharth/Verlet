#ifndef COMMON_H
#define COMMON_H

#include "raylib.h"

// Global Variables
#define g_screenWidth 1280
#define g_screenHeight 900
#define PHYSICS_SUBSTEPS 8
#define MAX_FRAME_TIME 0.05

// ---------------------------
// Verlet
// ---------------------------
typedef enum Anchoring {
    FIRST,
    LAST,
    BOTH,
    NONE
} Anchoring;

void SpawnVerletObject(Vector2 pos, float radius, Color color);
void SpawnVerletObjectStatic(Vector2 pos, float radius, Color color);
void SpawnStructureRope(Vector2 pos, int numJoints, float distance,
        float radius, Anchoring anchoring, Color color);
void SpawnStructureCloth(Vector2 pos, int numSideJoints, float distance,
        float radius, Color color);
void SpawnStructureSquare(Vector2 pos, float length, float radius, Color color);
void UpdateVerlet(float dt);
void DrawVerlet(void);
int GetNumObjects(void);

// ---------------------------
// UI
// ---------------------------
typedef enum StructType {
    DEFAULT = 0,
    ROPE,
    CLOTH,
    NUM_STRUCTURES
} StructType;

extern Vector2 g_mousePos;
extern float g_spawnRadius;
extern float g_spawnRate;
extern float g_gravity;
extern bool g_buttonPressed0;
// color control
extern float g_red;
extern float g_green;
extern float g_blue;

extern int g_structType;
extern bool g_applyConstraint;

void InitUI(void);
void UpdateUI(void);
void DrawUI(void);
bool IsMouseOnUI(void);

#endif // COMMON_H
