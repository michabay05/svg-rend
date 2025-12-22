#include <assert.h>
#include "vendor/include/raylib.h"
#include "vendor/include/raymath.h"

typedef float f32;

#define GRID_COLS 140
#define GRID_ROWS 140
#define TO_INDEX(r, c) (r*GRID_COLS + c)
bool pixels[GRID_ROWS*GRID_COLS] = {0};

void render_grid(Vector2 tl, Vector2 cell_size, f32 factor)
{
    assert(0.f <= factor);
    assert(factor <= 1.f);

    Vector2 pos = tl;
    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            Color clr = pixels[TO_INDEX(r, c)] ? RED : (Color){50, 50, 50, 255};
            Vector2 size = Vector2Scale(cell_size, factor);
            Vector2 p = Vector2Add(pos, Vector2Scale(size, 0.5f));
            DrawRectangleV(p, size, clr);
            pos.x += cell_size.x;
        }
        pos.y += cell_size.y;
        pos.x = tl.x;
    }
}

Vector2 get_px_pos(Vector2 tl, Vector2 cell_size, int r, int c)
{
    Vector2 ind = {c, r};
    Vector2 px_tl = Vector2Add(tl, Vector2Multiply(cell_size, ind));
    return Vector2Add(px_tl, Vector2Scale(cell_size, 0.5f));
}

int main(void)
{
    InitWindow(800, 600, "svg render - cpu");

    f32 h = (f32)GetScreenHeight() * 0.95;
    f32 w = h;
    Vector2 tl = {
        .x = 0.5f * ((f32)GetScreenWidth() - w),
        .y = 0.5f * ((f32)GetScreenHeight() - h),
    };
    f32 fac = 0.5f;
    Vector2 cell_size = {
        .x = w / (f32)GRID_COLS,
        .y = h / (f32)GRID_ROWS,
    };

    Vector2 center = get_px_pos(tl, cell_size, GRID_ROWS / 2, GRID_COLS / 2);
    f32 radius = 50;
    while (!WindowShouldClose()) {
        for (int r = 0; r < GRID_ROWS; r++) {
            for (int c = 0; c < GRID_ROWS; c++) {
                Vector2 pos = get_px_pos(tl, cell_size, r, c);
                f32 dist = Vector2LengthSqr(Vector2Subtract(pos, center));

                if (dist <= radius * radius) {
                    pixels[TO_INDEX(r, c)] = true;
                } else {
                    pixels[TO_INDEX(r, c)] = false;
                }
            }
        }

        BeginDrawing(); {
            ClearBackground(BLACK);
            render_grid(tl, cell_size, fac);
            DrawCircleV(center, radius, ColorAlpha(BLUE, 0.5));
            // DrawRectangleLinesEx(r, 3.f, DARKGRAY);
        } EndDrawing();
    }

    CloseWindow();
    return 0;
}
