#include "./vendor/include/raylib.h"
#include "./vendor/include/rlgl.h"
#include "./vendor/include/raymath.h"
#include <math.h>
#define ARENA_IMPLEMENTATION
#include "./vendor/include/arena.h"
#include "nob.h"

#define PRINT_F(f) (printf("%s = %.4f\n", #f, f))
#define PRINT_V2(v) (printf("%s = (%.4f, %.4f)\n", #v, v.x, v.y))

typedef float f32;

typedef struct {
    Vector2 s, c, e;
} QBezier;

typedef struct {
    QBezier *items;
    int count;
    int capacity;
} QBezierList;

typedef struct {
    Vector2 *items;
    int count;
    int capacity;
} PointList;

void render_quadratic_bezier(QBezier qb)
{
    // p_2 -(uv)-> (1, 1)
    rlTexCoord2f(1.0f, 1.0f);
    rlVertex2f(qb.e.x, qb.e.y);

    // p_1 -(uv)-> (0.5, 0)
    rlTexCoord2f(0.5f, 0.0f);
    rlVertex2f(qb.c.x, qb.c.y);

    // p_0 -(uv)-> (0, 0)
    rlTexCoord2f(0.0f, 0.0f);
    rlVertex2f(qb.s.x, qb.s.y);
}

int main(void)
{
    InitWindow(800, 600, "span - qbezier");
    SetTargetFPS(90);

    Shader shader = LoadShader(NULL, "qloopblinn-fs.glsl");

    Arena arena = {0};
    PointList pl = {0};
    QBezierList qbl = {0};

    Vector2 center = {300, 300};
    f32 radius = 150.f;
    int steps = 10;
    f32 dangle = 2 * PI / (f32)steps;
    f32 half_dangle = 0.5f * dangle;
    f32 radius_p1 = radius / cosf(half_dangle);
    for (int i = 0; i < steps; i++) {
        f32 curr = i * dangle;
        f32 next = (i+1) * dangle;
        f32 mid_angle = curr + half_dangle;

        QBezier qb = {
            .s = {
                .x = center.x + radius * cosf(curr),
                .y = center.y + radius * sinf(curr),
            },
            .c = {
                .x = center.x + radius_p1 * cosf(mid_angle),
                .y = center.y + radius_p1 * sinf(mid_angle),
            },
            .e = {
                .x = center.x + radius * cosf(next),
                .y = center.y + radius * sinf(next),
            }
        };
        arena_da_append(&arena, &qbl, qb);
        arena_da_append(&arena, &pl, qb.s);
        arena_da_append(&arena, &pl, qb.e);
    }

    Color c1 = RED, c2 = BLUE;
    while (!WindowShouldClose()) {
        BeginDrawing(); {
            ClearBackground(BLACK);

            rlBegin(RL_TRIANGLES); {
                rlColor4ub(c1.r, c1.g, c1.b, c1.a);

                for (int i = 0; i < pl.count - 1; i++) {
                    // This must be in this order because the winding order must
                    // be in the positive (CCW) orientation.
                    rlVertex2f(center.x, center.y);
                    rlVertex2f(pl.items[i+1].x, pl.items[i+1].y);
                    rlVertex2f(pl.items[i].x, pl.items[i].y);
                }
            } rlEnd();

            BeginShaderMode(shader); {
                rlBegin(RL_TRIANGLES); {
                    // rlColor4ub(c2.r, c2.g, c2.b, c2.a);
                    rlColor4ub(c1.r, c1.g, c1.b, c1.a);
                    for (int i = 0; i < qbl.count; i++) {
                        render_quadratic_bezier(qbl.items[i]);
                    }
                } rlEnd();
            } EndShaderMode();

            DrawFPS(10, 10);
        } EndDrawing();
    }

    CloseWindow();
    return 0;
}

void DrawCubicRecursive(Vector2 p0, Vector2 p1, Vector2 p2, Vector2 p3, int depth)
{
    // If we have split enough times, approximate as Quadratic and draw
    // (Depth of 3 or 4 is usually plenty for visual accuracy)
    if (depth >= 3) {
        // Calculate the single quadratic control point that approximates this cubic
        // Formula: Q1 = (3*P1 + 3*P2 - P0 - P3) / 4
        Vector2 term1 = Vector2Scale(Vector2Add(p1, p2), 3.0f);
        Vector2 term2 = Vector2Add(p0, p3);
        // Vector2 q1 = Vector2Scale(Vector2Add(term1, Vector2Scale(term2, -1.0f)), 0.25f);
        Vector2 q1 = Vector2Scale(Vector2Subtract(term1, term2), 0.25f);

        // DrawSplineSegmentBezierQuadratic(p0, q1, p3, 3.f, RED);
        QBezier qb = { .s = p0, .c = q1, .e = p3 };
        render_quadratic_bezier(qb);
        f32 r = 6.f;
        DrawCircleV(qb.s, r, RED);
        DrawCircleV(qb.c, r, GREEN);
        DrawCircleV(qb.e, r, BLUE);
        return;
    }

    // --- De Casteljau Subdivision at t=0.5 ---

    // Level 1 midpoints
    Vector2 m0 = Vector2Scale(Vector2Add(p0, p1), 0.5f);
    Vector2 m1 = Vector2Scale(Vector2Add(p1, p2), 0.5f);
    Vector2 m2 = Vector2Scale(Vector2Add(p2, p3), 0.5f);

    // Level 2 midpoints
    Vector2 q0 = Vector2Scale(Vector2Add(m0, m1), 0.5f);
    Vector2 q1 = Vector2Scale(Vector2Add(m1, m2), 0.5f);

    // Level 3 midpoint (The actual point on the curve)
    Vector2 split = Vector2Scale(Vector2Add(q0, q1), 0.5f);

    // Recursively draw the two halves
    // Left Cubic: p0, m0, q0, split
    DrawCubicRecursive(p0, m0, q0, split, depth + 1);

    // Right Cubic: split, q1, m2, p3
    DrawCubicRecursive(split, q1, m2, p3, depth + 1);
}
