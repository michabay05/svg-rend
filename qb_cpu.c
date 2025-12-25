// NOTE: This was my first attempt without really looking at any resource in-depth.

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>
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

#define CELL_FILL_FACTOR 0.8f
#define FILL_COLOR BLUE
#define STROKE_COLOR RED
#define MARKER_COLOR DARKGREEN
#define TO_INDEX(r, c) (r*grid_c + c)
Color *pixels;
size_t pixels_count;
int grid_c, grid_r;
int cell_w = 20;
Arena a = {0};
Points pts = {0};
f32 pt_r = 8.f;
int drag_ind = -1;
Vector2 tl, cell_size;

Vector2 qbezier_compute(Vector2 p0, Vector2 c1, Vector2 p2, f32 t)
{
    // Q(A, B, C, t) = (A - 2B + C)t^2 + 2(B - A)t + A
    Vector2 a = Vector2Add(Vector2Subtract(p0, Vector2Scale(c1, 2.f)), p2);
    Vector2 b = Vector2Scale(Vector2Subtract(c1, p0), 2.f);
    Vector2 c = p0;

    return Vector2Add(
        Vector2Add(Vector2Scale(a, t*t), Vector2Scale(b, t)),
        c);
}

Vector2 qbezier_deriv_compute(Vector2 p0, Vector2 c1, Vector2 p2, f32 t)
{
    // Q(A, B, C, t) = (A - 2B + C)t^2 + 2(B - A)t + A
    // Q'(A, B, C, t) = 2(A - 2B + C)t + 2(B - A)

    Vector2 a = Vector2Add(Vector2Subtract(p0, Vector2Scale(c1, 2.f)), p2);
    Vector2 b = Vector2Scale(Vector2Subtract(c1, p0), 2.f);

    return Vector2Add(Vector2Scale(a, 2*t), b);
}

bool f32_close(f32 a, f32 b)
{
    return fmaxf(a, b) - fminf(a, b) < 1e-6;
}

bool is_valid(f32 t, f32 val_x, f32 ray_x)
{
    return ((0.f <= t && t < 1.f) && val_x > ray_x);
}

int qbezier_find_roots(f32 a, f32 b, f32 c, f32 *t0, f32 *t1)
{
    f32 det = (b*b) - (4*a*c);

    if (det < 0.f) return 0;

    int valid_count = 0;
    f32 ts[2] = {0};
    int tn = 0;
    if (f32_close(det, 0.f)) {
        ts[tn++] = (-b + sqrtf(det)) / (2.f*a);
    } else {
        if (f32_close(a, 0.f)) {
            if (f32_close(b, 0.f)) {
                // There is no $t$ variable to solve for.
                // at^2 + bt + c = 0 -(becomes)-> c = 0
            } else {
                ts[tn++] = -c / b;
            }
        } else {
            ts[tn++] = (-b + sqrtf(det)) / (2.f*a);
            ts[tn++] = (-b - sqrtf(det)) / (2.f*a);
        }
    }

    *t0 = ts[0];
    *t1 = ts[1];
    return tn;
}

int qbezier_count_roots_horz(Vector2 p0, Vector2 c1, Vector2 p2, Vector2 ray)
{
    f32 a = p0.y - 2*c1.y + p2.y;
    f32 b = 2*(c1.y - p0.y);
    f32 c = p0.y - ray.y;
    f32 det = (b*b) - (4*a*c);

    if (det < 0.f) return 0;

    int valid_count = 0;
#if 1
    f32 ts[2] = {0};
    int tn = qbezier_find_roots(a, b, c, &ts[0], &ts[1]);
    for (int i = 0; i < tn; i++) {
        f32 t = ts[i];
        Vector2 val = qbezier_compute(p0, c1, p2, t);
        if (is_valid(t, val.x, ray.x)) valid_count++;
    }
#else
    if (f32_close(det, 0)) {
        f32 t = (-b + sqrtf(det)) / (2.f*a);
        Vector2 val = qbezier_compute(p0, c1, p2, t);
        if (is_valid(t, val.x, ray.x)) valid_count = 1;
    } else {
        if (f32_close(a, 0.f)) {
            if (f32_close(b, 0.f)) {
                // There is no $t$ variable to solve for.
                // at^2 + bt + c = 0 -(becomes)-> c = 0
                valid_count = 0;
            } else {
                f32 t = -c / b;
                Vector2 val = qbezier_compute(p0, c1, p2, t);
                if (is_valid(t, val.x, ray.x)) valid_count = 1;
            }
        } else {
            int tn = 2;
            f32 ts[2] = {
                (-b + sqrtf(det)) / (2.f*a),
                (-b - sqrtf(det)) / (2.f*a),
            };
            // Due to multiplicity of 2
            if (ts[0] == ts[1]) tn--;

            for (int k = 0; k < tn; k++) {
                f32 t = ts[k];
                Vector2 val = qbezier_compute(p0, c1, p2, t);
                if (is_valid(t, val.x, ray.x)) valid_count++;
            }
        }
    }
#endif
    return valid_count;
}

void render_grid(Vector2 tl, Vector2 cell_size)
{
    assert(0.f <= CELL_FILL_FACTOR);
    assert(CELL_FILL_FACTOR <= 1.f);

    Vector2 pos = tl;
    for (int r = 0; r < grid_r; r++) {
        for (int c = 0; c < grid_c; c++) {
            Vector2 size = Vector2Scale(cell_size, CELL_FILL_FACTOR);
            Vector2 p = Vector2Add(pos, Vector2Scale(Vector2Subtract(cell_size, size), 0.5f));
            if (pixels[TO_INDEX(r, c)].a != 0) {
                DrawRectangleV(p, size, pixels[TO_INDEX(r, c)]);
            } else {
                DrawRectangleV(p, size, DARKGRAY);
            }
            f32 radius = 0.1*size.x;
            if (radius > 1.f) {
                DrawCircleV(Vector2Add(pos, Vector2Scale(cell_size, 0.5f)), radius, RED);
            }
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
        for (int i = 0; drag_ind == -1 && i < pts.count; i++) {
            if (CheckCollisionPointCircle(mouse_pos, pts.items[i], pt_r)) {
                drag_ind = i;
                break;
            }
        }
        if (drag_ind == -1) arena_da_append(&a, &pts, mouse_pos);
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
    if (pts.count <= 3) return;
    if (pts.count % 2 != 0) return;

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

void fill_spline_nonzero_grid(void)
{
    if (pts.count <= 3) return;
    if (pts.count % 2 != 0) return;

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

void compute_grid(int new_cell_w)
{
    // No change detected
    if (cell_w != new_cell_w) return;

    f32 w = (f32)GetScreenWidth();
    f32 h = (f32)GetScreenHeight();
    cell_size = (Vector2){cell_w, cell_w};
    grid_c = (int)(w / cell_size.x);
    grid_r = (int)(h / cell_size.y);
    printf("Cell size: %.2f x %.2f (%d)\n", cell_size.x, cell_size.y, cell_w);
    printf("Grid size: %d x %d\n\n", grid_c, grid_r);
    tl = (Vector2) {
        .x = 0.5f * ((f32)GetScreenWidth() - (grid_c * cell_size.x)),
        .y = 0.5f * ((f32)GetScreenHeight() - (grid_r * cell_size.y)),
    };
    cell_size = (Vector2){cell_w, cell_w};

    size_t old_px_count = pixels_count;
    pixels_count = grid_c * grid_r * sizeof(*pixels);
    if (pixels == NULL) {
        pixels = arena_alloc(&a, pixels_count);
    } else {
        pixels = arena_realloc(&a, pixels, old_px_count, pixels_count);
    }
}

int main(void)
{
#if 0
    Vector2 p0 = {0,0};
    Vector2 c1 = {0.5, 0};
    Vector2 p2 = {1, 1};
    Vector2 ray = {2e-4, -1e-7};
    int c = qbezier_count_roots_horz(p0, c1, p2, ray);
    printf("Count: %d\n", c);
    return 0;
#endif

    SetTraceLogLevel(LOG_WARNING);
    InitWindow(800, 600, "svg render - cpu");
    SetTargetFPS(60);

    compute_grid(cell_w);
    clear_grid();
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_UP)) {
            cell_w += 2;
            compute_grid(cell_w);
        } else if (IsKeyPressed(KEY_DOWN)) {
            cell_w -= 2;
            compute_grid(cell_w);
        }
        control_points();
        clear_grid();
        // stroke_spline_grid(pts);
        fill_spline_evenodd_grid();
        // fill_spline_nonzero_grid();

        BeginDrawing(); {
            ClearBackground(BLACK);
            render_grid(tl, cell_size);
            for (int i = 0; i < pts.count; i++) {
                DrawCircleV(pts.items[i], pt_r, MARKER_COLOR);
            }
            for (int i = 0; i+1 < pts.count; i += 2) {
                Vector2 p0 = pts.items[i];
                Vector2 c1 = pts.items[i+1];
                Vector2 p2 = pts.items[(i+2)%pts.count];
                DrawSplineSegmentBezierQuadratic(p0, c1, p2, 3.f, GREEN);
            }
            DrawFPS(10, 10);
        } EndDrawing();
    }

    CloseWindow();
    arena_free(&a);
    return 0;
}
