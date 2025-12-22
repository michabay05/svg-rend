#include "vendor/include/raylib.h"
#define NOB_IMPLEMENTATION
#define ARENA_IMPLEMENTATION
#include "svg_parse.h"
#include "vendor/include/raylib.h"

int main(void)
{
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(800, 600, "span - svg_rend");
    SetTargetFPS(90);

    Arena arena = {0};

    while (!WindowShouldClose()) {
        BeginDrawing(); {
            ClearBackground(BLACK);
        } EndDrawing();
    }

    CloseWindow();
}
