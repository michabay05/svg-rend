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
#define TO_INDEX(r, c) (r*grid_c + c)
bool *pixels;
int grid_c, grid_r;
Arena a = {0};
Points pts = {0};
f32 pt_r = 8.f;
int drag_ind = -1;
Vector2 cell_size = { CELL_W, CELL_W };

void render_grid(Vector2 tl, Vector2 cell_size, f32 factor)
{
    assert(0.f <= factor);
    assert(factor <= 1.f);

    Vector2 pos = tl;
    for (int r = 0; r < grid_r; r++) {
        for (int c = 0; c < grid_c; c++) {
            Color clr = pixels[TO_INDEX(r, c)] ? GRAY : (Color){50, 50, 50, 255};
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
        // Remove double-counted root
        if (ts[0] == ts[1]) count = (int)fmaxf(count-1, 0.f);
    } else {
        assert(b != 0.f);
        f32 t = -c/b;
        Vector2 val = qbezier_compute(p0, c1, p2, t);
        if ((0.f <= t && t <= 1.f) && val.x > ray.x) count++;
    }

    return count;
}

void fill_spline(void)
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
            pixels[TO_INDEX(r, c)] = (count % 2) != 0;
        }
    }
}

int main(void)
{
#if 0
    Vector2 p0 = {0.f, 0.f};
    Vector2 c1 = {0.5f, 0.5f};
    Vector2 p2 = {1.f, 1.f};
    for (f32 v = 0.f; v <= 1.1f; v += 0.1f) {
        Vector2 ray = {v, v};
        int count = qbezier_count_roots_horz(p0, c1, p2, ray);
        printf("(%.2f, %.2f): Count: %d\n", ray.x, ray.y, count);
    }
    return 0;
#endif

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

    size_t n = grid_c * grid_r * sizeof(bool);
    pixels = arena_alloc(&a, n);
    memset(pixels, 0, n);

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
        fill_spline();

        BeginDrawing(); {
            ClearBackground(BLACK);
            render_grid(tl, cell_size, fac);
            if (pts.count == 6) {
                for (int i = 0; i+1 < pts.count; i += 2) {
                    Vector2 p0 = pts.items[i];
                    Vector2 c1 = pts.items[i+1];
                    Vector2 p2 = pts.items[(i+2)%pts.count];
                    // DrawSplineSegmentBezierQuadratic(p1, c1, p2, 3.f, MAROON);

#ifdef QBEZIER_PTS
                    for (f32 t = 0.f; t <= 1.f; t += dt) {
                        Vector2 pos = qbezier_compute(p0, c1, p2, t);
                        DrawCircleV(pos, marker_size, MAROON);
                    }
#else
                    DrawSplineSegmentBezierQuadratic(p0, c1, p2, 3.f, MAROON);
#endif
                }
            }

            for (int i = 0; i < pts.count; i++) {
                DrawCircleV(pts.items[i], pt_r, BLUE);
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
