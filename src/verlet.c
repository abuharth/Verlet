#include "raylib.h"
#include "raymath.h"
#include "common.h"

#define MAX_OBJECTS 4096
#define MAX_LINKS 8192

static Vector2 gravity = { 0, 1000 };

// TODO: Air pressure for hollow objects
// TODO: maybe let joints be either in tension or compression
typedef struct VerletObject {
    Vector2 currentPos;
    Vector2 oldPos;
    Vector2 acceleration;
    float radius;
    Color color;
    bool isStatic;
    bool isColliding;
} VerletObject;

typedef struct Link {
    VerletObject *object1;
    VerletObject *object2;
    float target_distance;
} Link;

static VerletObject objects[MAX_OBJECTS];
static int numObjects = 0;

static Link links[MAX_LINKS];
static int numLinks = 0;

static float responseCoef = 1.0;

// generate a link between the given positions starting from the
// end of the objects array
// Must be done before spawning verlet objects
void SpawnLink(int pos1, int pos2, float distance) {
        links[numLinks] = (Link) {
            &objects[numObjects + pos1],
            &objects[numObjects + pos2],
            distance
        };
        numLinks++;
}

void SpawnVerletObject(Vector2 position, float radius, Color color) {
    if (numObjects >= MAX_OBJECTS) return;
    VerletObject *ball = &objects[numObjects];
    ball->currentPos = position;
    ball->oldPos = position;
    ball->radius = radius;
    ball->color = color;
    ball->isStatic = false;
    ball->isColliding = true;
    numObjects++;
}

void SpawnVerletObjectStatic(Vector2 position, float radius, Color color) {
    if (numObjects >= MAX_OBJECTS) return;
    VerletObject *ball = &objects[numObjects];
    ball->currentPos = position;
    ball->oldPos = position;
    ball->radius = radius;
    ball->color = color;
    ball->isStatic = true;
    ball->isColliding = true;
    numObjects++;
}

void SpawnVerletObjectNonColliding(Vector2 position, float radius, Color color) {
    if (numObjects >= MAX_OBJECTS) return;
    VerletObject *ball = &objects[numObjects];
    ball->currentPos = position;
    ball->oldPos = position;
    ball->radius = radius;
    ball->color = color;
    ball->isStatic = false;
    ball->isColliding = false;
    numObjects++;
}

void SpawnStructureRope(Vector2 pos, int numJoints, float distance,
        float radius, Anchoring anchoring, Color color) {
    if (numObjects + numJoints >= MAX_OBJECTS) return;
    if (numLinks + numJoints - 1 >= MAX_LINKS) return;
    // joints are too big
    if (radius > distance/2) return;

    // creating links
    for (int i = 0; i < numJoints - 1; i++) {
        SpawnLink(i, i + 1, distance);
    }

    // spawning objects for rope
    if (anchoring == FIRST || anchoring == BOTH) {
        SpawnVerletObjectStatic(pos, radius, color);
    }
    else {
        SpawnVerletObject(pos, radius, color);
    }
    for (int i = 1; i < numJoints - 1; i++) {
        SpawnVerletObject((Vector2){ pos.x + i*distance, pos.y }, radius, color);
    }
    if (anchoring == LAST || anchoring == BOTH) {
        SpawnVerletObjectStatic(
                (Vector2){ pos.x + (numJoints - 1)*distance, pos.y },
                radius, color);
    }
    else {
        SpawnVerletObject(
                (Vector2){ pos.x + (numJoints - 1)*distance, pos.y },
                radius, color);
    }
}

int XYToNum(int x, int y, int width) {
    return y*width + x;
}

void SpawnStructureCloth(Vector2 pos, int numSideJoints, float distance,
        float radius, Color color) {
    if (numObjects + numSideJoints*numSideJoints >= MAX_OBJECTS) return;
    if (numLinks + numSideJoints*numSideJoints*2 >= MAX_LINKS) return;

    for (int i = 0; i < numSideJoints; i++) {
        for (int j = 0; j < numSideJoints; j++) {
            if (i < numSideJoints - 1) {
                SpawnLink(
                        XYToNum(i, j, numSideJoints),
                        XYToNum(i + 1, j, numSideJoints),
                        distance);
            }
            if (j < numSideJoints - 1) {
                SpawnLink(
                        XYToNum(i, j, numSideJoints),
                        XYToNum(i, j + 1, numSideJoints),
                        distance);
            }
        }
    }

    for (int i = 0; i < numSideJoints; i++) {
        for (int j = 0; j < numSideJoints; j++) {
            // anchor the two top corners
            if ((i == 0 && j == 0) || (i == 0 && j == numSideJoints - 1)) {
                SpawnVerletObjectStatic(
                        (Vector2){ pos.x + j*distance - 5, pos.y + i*distance },
                        radius, color);
            }
            else {
                SpawnVerletObjectNonColliding(
                        (Vector2){ pos.x + j*distance, pos.y + i*distance },
                        radius, color);
            }
        }
    }
}

void ApplyLinks() {
    for (int i = 0; i < numLinks; i++) {
        VerletObject *obj1 = links[i].object1;
        VerletObject *obj2 = links[i].object2;

        Vector2 axis = Vector2Subtract(obj1->currentPos, obj2->currentPos);
        float dist = Vector2Length(axis);
        Vector2 n = { axis.x/dist, axis.y/dist };
        float delta = links[i].target_distance - dist;

        // only apply forces if in tension
        if (delta < 0) {
            if (!obj1->isStatic) {
                obj1->currentPos.x += 0.5f*delta*n.x;
                obj1->currentPos.y += 0.5f*delta*n.y;
            }
            if (!obj2->isStatic) {
                obj2->currentPos.x -= 0.5f*delta*n.x;
                obj2->currentPos.y -= 0.5f*delta*n.y;
            }
        }
    }
}

void ApplyAcceleration(Vector2 vector) {
    for (int i = 0; i < numObjects; i++) {
        VerletObject *object = &objects[i];
        object->acceleration.y += vector.y;
    }
}

void AccelerateToPoint(Vector2 vector, float strength) {
    for (int i = 0; i < numObjects; i++) {
        VerletObject *object = &objects[i];
        Vector2 toPoint = Vector2Subtract(vector, object->currentPos);
        float distance = Vector2Length(toPoint);
        if (distance < 0.0001f) continue;
        // toPoint = Vector2Normalize(toPoint);
        object->acceleration.x += toPoint.x*strength/distance;
        object->acceleration.y += toPoint.y*strength/distance;
    }
}

void ApplyConstraintCircle(Vector2 constraintPos, float radius) {
    for (int i = 0; i < numObjects; i++) {
        VerletObject *object = &objects[i];
        Vector2 toObj = Vector2Subtract(object->currentPos, constraintPos);
        float constraintDistance = Vector2Length(toObj);
        if (constraintDistance > radius - object->radius) {
            Vector2 n = (Vector2){
                toObj.x/constraintDistance,
                    toObj.y/constraintDistance
            };
            object->currentPos.x = constraintPos.x + n.x * (radius - object->radius);
            object->currentPos.y = constraintPos.y + n.y * (radius - object->radius);
        }
    }
}

void SolveCollisions(void) {
    for (int i = 0; i < numObjects; i++) {
        VerletObject *object1 = &objects[i];
        if (!object1->isColliding) continue;

        for (int j = i + 1; j < numObjects; j++) {
            VerletObject *object2 = &objects[j];
            if (!object2->isColliding) continue;

            Vector2 v = Vector2Subtract(object1->currentPos, object2->currentPos);
            float dist2 = v.x * v.x + v.y * v.y;
            float min_dist = object1->radius + object2->radius;
            // Check overlapping
            if (dist2 < min_dist * min_dist) {
                float dist  = sqrt(dist2);
                Vector2 n = { v.x/dist, v.y/dist };
                float massRatio1 = object1->radius / (object1->radius + object2->radius);
                float massRatio2 = object2->radius / (object1->radius + object2->radius);
                float delta = 0.5f * responseCoef * (dist - min_dist);
                // Update positions
                if (!object1->isStatic) {
                    object1->currentPos.x -= n.x * (massRatio2 * delta);
                    object1->currentPos.y -= n.y * (massRatio2 * delta);
                }
                if (!object2->isStatic) {
                    object2->currentPos.x += n.x * (massRatio1 * delta);
                    object2->currentPos.y += n.y * (massRatio1 * delta);
                }
            }
        }
    }
}

void UpdatePositions(float dt) {
    for (int i = 0; i < numObjects; i++) {
        VerletObject *object = &objects[i];

        Vector2 displacement = {
            object->currentPos.x - object->oldPos.x,
            object->currentPos.y - object->oldPos.y
        };
        if (!object->isStatic) {
            object->oldPos = object->currentPos;
            object->currentPos.x = object->currentPos.x + displacement.x + object->acceleration.x*dt*dt;
            object->currentPos.y = object->currentPos.y + displacement.y + object->acceleration.y*dt*dt;
            object->acceleration = (Vector2){ 0 };
        }
    }
}

void UpdateVerlet(float dt) {
    // reacting to UI signals
    if (g_buttonPressed0) {
        numObjects = 0;
        numLinks = 0;
        g_buttonPressed0 = false;
    }
    gravity.y = (int)(g_gravity/100)*100;
    Vector2 center = { (float)g_screenWidth/2, (float)g_screenHeight/2 };

    // TODO: Swap delete for fallen objects
    // (Swapping must occur in the links array to maintain links)
    for (int k = 0; k < PHYSICS_SUBSTEPS; k++) {
        ApplyAcceleration(gravity);
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && !IsMouseOnUI()) {
            AccelerateToPoint(g_mousePos, 2000);
        }
        if (g_applyConstraint) {
            ApplyConstraintCircle(center, 400);
        }
        SolveCollisions();
        ApplyLinks();
        UpdatePositions(dt);
    }
}

void DrawVerlet(void) {
    for (int i = 0; i < numLinks; i++) {
        DrawLineEx(links[i].object1->currentPos, links[i].object2->currentPos, 2.0f,
                (Color){ g_red, g_green, g_blue, 255 });
    }
    for (int i = 0; i < numObjects; i++) {
        DrawCircleV(objects[i].currentPos, objects[i].radius, objects[i].color);
    }
}

int GetNumObjects(void) {
    return numObjects;
}
