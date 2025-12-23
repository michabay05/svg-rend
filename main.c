#include <assert.h>
#include <string.h>
#include "vendor/include/raylib.h"
#include "vendor/include/raymath.h"
#define ARENA_IMPLEMENTATION
#include "vendor/include/arena.h"

typedef float f32;

typedef struct {
    Vector2 *items;
    int count, capacity;
} Points;

#define CELL_W 5
#define FILL_COLOR BLUE
#define STROKE_COLOR RED
#define MARKER_COLOR DARKGREEN
#define TO_INDEX(r, c) (r*grid_c + c)
Color *pixels;
size_t pixels_count;
int grid_c, grid_r;
Arena a = {0};
Points pts = {0};
f32 pt_r = 8.f;
int drag_ind = -1;
Vector2 cell_size = { CELL_W, CELL_W };

Vector2 qbezier_compute(Vector2 p0, Vector2 c1, Vector2 p2, f32 t)
{
    Vector2 a = Vector2Add(Vector2Subtract(p0, Vector2Scale(c1, 2.f)), p2);
    Vector2 b = Vector2Scale(Vector2Subtract(c1, p0), 2.f);
    Vector2 c = p0;

    return Vector2Add(
        Vector2Add(Vector2Scale(a, t*t), Vector2Scale(b, t)),
        c);
}

int qbezier_count_roots_horz(Vector2 p0, Vector2 c1, Vector2 p2, Vector2 ray)
{
    f32 a = p0.y - 2*c1.y + p2.y;
    f32 b = 2*(c1.y - p0.y);
    f32 c = p0.y - ray.y;
    f32 det = (b*b) - (4*a*c);

    if (det < 0) return 0;

    int count = 0;
    if (a != 0.f) {
        f32 ts[2] = {
            (-b - sqrtf(det)) / (2.f*a),
            (-b + sqrtf(det)) / (2.f*a),
        };
        for (int k = 0; k < 2; k++) {
            f32 t = ts[k];
            Vector2 val = qbezier_compute(p0, c1, p2, t);
            if ((0.f <= t && t <= 1.f) && val.x > ray.x) count++;
        }
        // // Remove double-counted root
        // if (ts[0] == ts[1]) count = (int)fmaxf(count-1, 0.f);
    } else {
        assert(b != 0.f);
        f32 t = -c/b;
        Vector2 val = qbezier_compute(p0, c1, p2, t);
        if ((0.f <= t && t <= 1.f) && val.x > ray.x) count++;
    }

    return count;
}

void render_grid(Vector2 tl, Vector2 cell_size, f32 factor)
{
    assert(0.f <= factor);
    assert(factor <= 1.f);

    Vector2 pos = tl;
    for (int r = 0; r < grid_r; r++) {
        for (int c = 0; c < grid_c; c++) {
            Vector2 size = Vector2Scale(cell_size, factor);
            Vector2 p = Vector2Add(pos, Vector2Scale(size, 0.5f));
            DrawRectangleV(p, size, pixels[TO_INDEX(r, c)]);
            pos.x += cell_size.x;
        }
        pos.y += cell_size.y;
        pos.x = tl.x;
    }
}

void control_points(void)
{
    Vector2 mouse_pos = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (pts.count < 6) {
            arena_da_append(&a, &pts, mouse_pos);
        } else {
            for (int i = 0; drag_ind == -1 && i < pts.count; i++) {
                if (CheckCollisionPointCircle(mouse_pos, pts.items[i], pt_r)) {
                    drag_ind = i;
                    break;
                }
            }
        }
    }

    if (IsMouseButtonUp(MOUSE_BUTTON_LEFT) && drag_ind >= 0) {
        drag_ind = -1;
    }

    if (drag_ind >= 0) {
        pts.items[drag_ind] = mouse_pos;
    }
}

void clear_grid(void)
{
    memset(pixels, 0, pixels_count);
}

void fill_spline_evenodd_grid(void)
{
    if (pts.count != 6) return;
    int count = 0;
    for (int r = 0; r < grid_r; r++) {
        for (int c = 0; c < grid_c; c++) {
            count = 0;
            Vector2 ray = {
                (c+0.5f)*cell_size.x,
                (r+0.5f)*cell_size.y
            };
            for (int i = 0; i+1 < pts.count; i += 2) {
                Vector2 p0 = pts.items[i];
                Vector2 c1 = pts.items[i+1];
                Vector2 p2 = pts.items[(i+2)%pts.count];

                count += qbezier_count_roots_horz(p0, c1, p2, ray);
            }
            // printf("(r = %3d, c = %3d) count = %3d\n", r, c, count);
            if (count % 2 != 0) pixels[TO_INDEX(r, c)] = FILL_COLOR;
        }
    }
}

void stroke_spline_grid(Points pts)
{
    for (int i = 0; i+1 < pts.count; i += 2) {
        Vector2 p0 = pts.items[i];
        Vector2 c1 = pts.items[i+1];
        Vector2 p2 = pts.items[(i+2)%pts.count];
        for (f32 t = 0.f; t < 1.f; t += 0.005f) {
            Vector2 pos = Vector2Divide(qbezier_compute(p0, c1, p2, t), cell_size);
            int c = (int)pos.x;
            int r = (int)pos.y;
            pixels[TO_INDEX(r, c)] = STROKE_COLOR;
        }
    }
}

int main(void)
{
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(800, 600, "svg render - cpu");
    SetTargetFPS(60);

    f32 w = (f32)GetScreenWidth();
    f32 h = (f32)GetScreenHeight();
    f32 fac = 0.5f;
    grid_c = (int)(w / cell_size.x);
    grid_r = (int)(h / cell_size.y);
    printf("Grid size: %d x %d\n", grid_c, grid_r);
    Vector2 tl = {
        .x = 0.5f * ((f32)GetScreenWidth() - (grid_c * cell_size.x)),
        .y = 0.5f * ((f32)GetScreenHeight() - (grid_r * cell_size.y)),
    };

    pixels_count = grid_c * grid_r * sizeof(*pixels);
    pixels = arena_alloc(&a, pixels_count);
    clear_grid();

    Vector2 temp[] = {
        {214.000, 65.000},
        {142.000, 196.000},
        {259.000, 336.000},
        {468.000, 310.000},
        {465.000, 149.000},
        {361.000, 60.000},
    };
    for (int i = 0; i < 6; i++) {
        arena_da_append(&a, &pts, temp[i]);
    }
    printf("Point Count: %d\n", pts.count);

    f32 dt = 0.075f;
    // Vector2 marker_size = Vector2Scale(cell_size, 0.3f);
    f32 marker_size = 5.f;
    while (!WindowShouldClose()) {
        control_points();
        clear_grid();
        stroke_spline_grid(pts);
        fill_spline_evenodd_grid();

        BeginDrawing(); {
            ClearBackground(BLACK);
            render_grid(tl, cell_size, fac);
            for (int i = 0; i < pts.count; i++) {
                DrawCircleV(pts.items[i], pt_r, MARKER_COLOR);
            }
            DrawFPS(10, 10);
        } EndDrawing();
    }

#if 0
    printf("{\n");
    for (int i = 0; i < pts.count; i++) {
        Vector2 p = pts.items[i];
        printf("    {%.3f, %.3f},\n", p.x, p.y);
    }
    printf("}\n");
#endif

    CloseWindow();
    arena_free(&a);
    return 0;
}
