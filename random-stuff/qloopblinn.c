#include "./vendor/include/raylib.h"
#include "./vendor/include/raymath.h"
#include "./vendor/include/rlgl.h"

// 1. The Single Quadratic Drawer (Reusing your logic)
void DrawQuadraticGL(Vector2 p0, Vector2 p1, Vector2 p2) {
    // P0 -> UV(0, 0)
    rlTexCoord2f(0.0f, 0.0f);
    rlVertex2f(p0.x, p0.y);

    // P1 -> UV(0.5, 0)
    rlTexCoord2f(0.5f, 0.0f);
    rlVertex2f(p1.x, p1.y);

    // P2 -> UV(1, 1)
    rlTexCoord2f(1.0f, 1.0f);
    rlVertex2f(p2.x, p2.y);
}

// 2. The Recursive Cubic Processor
void DrawCubicRecursive(Vector2 p0, Vector2 p1, Vector2 p2, Vector2 p3, int depth) {
    // If we have split enough times, approximate as Quadratic and draw
    // (Depth of 3 or 4 is usually plenty for visual accuracy)
    if (depth > 3) {
        // Calculate the single quadratic control point that approximates this cubic
        // Formula: Q1 = (3*P1 + 3*P2 - P0 - P3) / 4
        Vector2 term1 = Vector2Scale(Vector2Add(p1, p2), 3.0f);
        Vector2 term2 = Vector2Add(p0, p3);
        // Vector2 q1 = Vector2Scale(Vector2Add(term1, Vector2Scale(term2, -1.0f)), 0.25f);
        Vector2 q1 = Vector2Scale(Vector2Subtract(term1, term2), 0.25f);

        DrawQuadraticGL(p0, q1, p3);
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

int main(void)
{
    InitWindow(800, 600, "Loop-Blinn Curve Rendering");

    // Load shader from memory (Raylib default VS, Custom FS)
    Shader curveShader = LoadShader(0, "qloopblinn-fs.glsl");

    Vector2 c0 = { 100, 300 };
    Vector2 c1 = { 250, 100 }; // Pulls up
    Vector2 c2 = { 550, 500 }; // Pulls up
    Vector2 c3 = { 700, 300 };

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // Simple interaction to move the control point
        // c1 = GetMousePosition();

        BeginDrawing();
        ClearBackground(BLACK);

        rlDisableBackfaceCulling();
        rlDisableDepthTest();
        BeginShaderMode(curveShader); {
            rlBegin(RL_TRIANGLES);
            rlColor4ub(255, 0, 0, 255);

            // Call our new function
            // Depth 0 starts the recursion
            DrawCubicRecursive(c0, c1, c2, c3, 0);

            rlEnd();
        } EndShaderMode();
        rlEnableBackfaceCulling();

        // Visualizing the Skeleton (for debug)
        DrawSplineSegmentBezierCubic(c0, c1, c2, c3, 4.0f, WHITE);
        DrawCircleV(c0, 5, BLUE);
        DrawCircleV(c1, 5, GREEN);
        DrawCircleV(c2, 5, BLUE);
        DrawCircleV(c3, 5, BLUE);
        DrawText("Move Mouse to control P1", 10, 10, 20, LIGHTGRAY);

        EndDrawing();
    }

    UnloadShader(curveShader);
    CloseWindow();

    return 0;
}

int main2(void)
{
    InitWindow(800, 600, "Loop-Blinn Curve Rendering");

    // Load shader from memory (Raylib default VS, Custom FS)
    Shader curveShader = LoadShader(0, "qloopblinn-fs.glsl");

    // Define Bezier Control Points
    Vector2 p0 = { 100, 300 }; // Start
    Vector2 p1 = { 400, 100 }; // Control (The peak)
    Vector2 p2 = { 700, 300 }; // End

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // Simple interaction to move the control point
        p1 = GetMousePosition();

        BeginDrawing();
        ClearBackground(BLACK);

        // --- DRAWING THE CURVE ---
        BeginShaderMode(curveShader);
            rlBegin(RL_TRIANGLES);
                rlColor4ub(255, 0, 0, 255); // Red color
                DrawQuadraticGL(p0, p1, p2);
            rlEnd();
        EndShaderMode();
        // -------------------------

        // Visualizing the Skeleton (for debug)
        // DrawSplineSegmentBezierQuadratic(p0, p1, p2, 4.0f, WHITE);
        DrawCircleV(p0, 5, BLUE);
        DrawCircleV(p1, 5, GREEN);
        DrawCircleV(p2, 5, BLUE);
        DrawText("Move Mouse to control P1", 10, 10, 20, LIGHTGRAY);

        EndDrawing();
    }

    UnloadShader(curveShader);
    CloseWindow();

    return 0;
}
