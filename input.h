

#include "raylib.h"


typedef struct {
    Vector2 aimDir;
    float aimAngle;
    bool chargePressed;
    bool chargeHeld;
    bool chargeReleased;
    bool confirm, cancel, pause, resetBall;
    float camZoomDelta;
    Vector2 pointerWorld;
    bool usingGamepad;
} InputState;