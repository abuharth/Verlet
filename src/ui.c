// TODO: add a hide UI button
// TODO: add 2 buttons, left and right arrows that switch between
// spawning modes
#include "raylib.h"
#include "common.h"

#define MAX_BUTTONS 32
#define UNIT_SIZE 20

typedef struct Button {
    Rectangle rect;
    char *label;
} Button;

typedef struct Slider {
    Rectangle bounds;
    Rectangle rect;
    char *label;
    float *variable;
    Vector2 range;
} Slider;

static Button buttons[MAX_BUTTONS];
static int numButtons = 0;

static Slider sliders[MAX_BUTTONS];
static int numSliders = 0;

static int buttonMouseHover = -1;
static int sliderFocused = -1;

// global variables to be affected by UI
Vector2 g_mousePos;
bool g_buttonPressed0 = false;
float g_spawnRadius = 10;
float g_spawnRate = 10;
float g_gravity = 1000;

float g_red = 127;
float g_green = 127;
float g_blue = 127;

int g_structType;
bool g_applyConstraint = true;

void CreateButton(Rectangle rect, char *label) {
    if (numButtons >= MAX_BUTTONS) return;

    buttons[numButtons].rect = rect;
    buttons[numButtons].label = label;
    numButtons += 1;
}

void CreateSlider(Rectangle bounds, char *label, float *variable, Vector2 range) {
    if (numSliders >= MAX_BUTTONS) return;
    sliders[numSliders].bounds = bounds;
    // figure out where in between the two ends the slider should go
    float fraction = (*variable - range.x)/(range.y - range.x);
    sliders[numSliders].rect = (Rectangle) {
        bounds.x + (bounds.width - 2*(int)(UNIT_SIZE/3))*fraction,
        bounds.y + bounds.height - (int)(UNIT_SIZE*2),
        2*(int)(UNIT_SIZE/3),
        UNIT_SIZE*2
    };
    sliders[numSliders].label = label;
    sliders[numSliders].variable = variable;
    sliders[numSliders].range = range;
    numSliders += 1;
}

// ---------------------------------------------------------------
// Create all buttons and sliders here
// ---------------------------------------------------------------
void InitUI(void) {
    CreateButton((Rectangle){ 30, 40, 200, 100 }, "Delete Objects");
    CreateSlider((Rectangle){ 30, 180, 200, 60 },
        "Radius", &g_spawnRadius, (Vector2){ 3, 60 });
    CreateSlider((Rectangle){ 30, 280, 200, 60 },
        "Spawn Rate", &g_spawnRate, (Vector2){ 5, 60 });
    CreateSlider((Rectangle){ 30, 380, 200, 60 },
        "Gravity", &g_gravity, (Vector2){ -1000, 1000 });

    CreateButton((Rectangle){ 30, 480, 50, 60 }, "<");
    CreateButton((Rectangle){ 180, 480, 50, 60 }, ">");

    CreateButton((Rectangle){ 30, 580, 200, 60 }, "Toggle Constraint");

    CreateSlider((Rectangle){ g_screenWidth - 230, 180, 200, 60 },
        "Red", &g_red, (Vector2){ 0, 255 });
    CreateSlider((Rectangle){ g_screenWidth - 230, 280, 200, 60 },
        "Green", &g_green, (Vector2){ 0, 255 });
    CreateSlider((Rectangle){ g_screenWidth - 230, 380, 200, 60 },
        "Blue", &g_blue, (Vector2){ 0, 255 });
}

void UpdateSlider(int sliderNum) {
    Slider slider = sliders[sliderNum];
    slider.rect.x = g_mousePos.x - slider.rect.width/2;
    // clamp positions
    if (slider.rect.x < slider.bounds.x) {
        slider.rect.x = slider.bounds.x;
    }
    if (slider.rect.x > slider.bounds.x + slider.bounds.width - slider.rect.width) {
        slider.rect.x = slider.bounds.x + slider.bounds.width - slider.rect.width;
    }
    // update variable based on slider position
    float fraction = 
        (slider.rect.x - slider.bounds.x)
        /(slider.bounds.width - slider.rect.width);
    *slider.variable = slider.range.x + (slider.range.y - slider.range.x)*fraction;

    sliders[sliderNum] = slider;
}

void UpdateUI(void) {
    g_mousePos = GetMousePosition();
    // check collision with buttons
    buttonMouseHover = -1;
    for (int i = 0; i < numButtons; i++) {
        if (CheckCollisionPointRec(g_mousePos, buttons[i].rect)) {
            buttonMouseHover = i;
        }
    }
    for (int i = 0; i < numSliders; i++) {
        if (CheckCollisionPointRec(g_mousePos, sliders[i].bounds)) {
            buttonMouseHover = MAX_BUTTONS + i;
        }
    }
    // mouse click response
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && buttonMouseHover >= 0) {
        if (buttonMouseHover == 0) {
            g_buttonPressed0 = true;
        }
        else if (buttonMouseHover == 1) {
            g_structType -= 1;
            if (g_structType < 0) {
                g_structType = NUM_STRUCTURES - 1;
            }
        }
        else if (buttonMouseHover == 2) {
            g_structType += 1;
            if (g_structType > NUM_STRUCTURES - 1) {
                g_structType = 0;
            }
        }
        else if (buttonMouseHover == 3) {
            g_applyConstraint = !g_applyConstraint;
        }

        else if (buttonMouseHover >= MAX_BUTTONS) {
            sliderFocused = buttonMouseHover - MAX_BUTTONS;
        }
    }
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        sliderFocused = - 1;
    }
    // update sliders
    if (sliderFocused >= 0) {
        UpdateSlider(sliderFocused);
    }
}

void DrawButtons(void) {
    for (int i = 0; i < numButtons; i++) {
        if (buttonMouseHover == i) {
            DrawRectangleRec(buttons[i].rect, GRAY);
        }
        else {
            DrawRectangleRec(buttons[i].rect, LIGHTGRAY);
        }
        int textWidth = MeasureText(buttons[i].label, UNIT_SIZE);
        DrawText(
                buttons[i].label,
                buttons[i].rect.x + buttons[i].rect.width/2 - (int)(textWidth/2),
                buttons[i].rect.y + buttons[i].rect.height/2 - (int)(UNIT_SIZE/2),
                UNIT_SIZE, BLACK
        );
    }
}

void DrawSliders(void) {
    for (int i = 0; i < numSliders; i++) {
        // draw a little bar that the slider will slide accross
        DrawRectangleRec(sliders[i].bounds, RED);
        DrawRectangle(
            sliders[i].bounds.x,
            sliders[i].bounds.y + sliders[i].bounds.height - UNIT_SIZE - (int)(UNIT_SIZE/4),
            sliders[i].bounds.width,
            UNIT_SIZE/2,
            LIGHTGRAY
        );
        if (buttonMouseHover == MAX_BUTTONS + i || sliderFocused == i) {
            DrawRectangleRec(sliders[i].rect, GRAY);
        }
        else {
            DrawRectangleRec(sliders[i].rect, LIGHTGRAY);
        }
        int textWidth = MeasureText(sliders[i].label, UNIT_SIZE);
        DrawText(
            sliders[i].label,
            sliders[i].bounds.x + sliders[i].bounds.width/2 - (int)(textWidth/2),
            sliders[i].bounds.y,
            UNIT_SIZE, BLACK
        );
    }
}

bool IsMouseOnUI(void) {
    return buttonMouseHover >= 0 || sliderFocused >= 0;
}

void DrawUI(void) {
    DrawButtons();
    DrawSliders();
    DrawText(TextFormat("Num Objects: %d", GetNumObjects()), 40, 680, 20, RAYWHITE);
    DrawRectangleRec((Rectangle){ g_screenWidth - 230, 40, 200, 100 }, (Color){ (int)g_red, (int)g_green, (int)g_blue, 255 });
    DrawText(g_structType == 0?
            "Ball": g_structType == 1?
            "Rope": g_structType == 2?
            "Cloth":
            "N/A", g_structType == 0? 110: 100, 500, 20, RAYWHITE);
}
