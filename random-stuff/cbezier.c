#include "../vendor/include/raylib.h"

Vector2 foo(float x, float y, float scale, Vector2 offset) {
    return (Vector2){
        (x * scale) + offset.x,
        (y * scale) + offset.y
    };
}

int main(void) {
    const int screenWidth = 800;
    const int screenHeight = 600;

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "Raylib 5.0 Spline Test");
    SetTargetFPS(60);

    float scale = 4.0f;
    Vector2 offset = { 200.0f, 200.0f };
    float thick = 3.0f;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Draw Reference Grid
        DrawLine(offset.x, 0, offset.x, screenHeight, LIGHTGRAY);
        DrawLine(0, offset.y, screenWidth, offset.y, LIGHTGRAY);

        // --- SEGMENT 1 ---
        // M 0 43.17223
        Vector2 p1_start = foo(0.0f, 43.17223f, scale, offset);
        // C 0 43.17223 10.73143 -17.56503 56.72326 5.04273
        Vector2 p1_c1    = foo(0.0f, 43.17223f, scale, offset);
        Vector2 p1_c2    = foo(10.73143f, -17.56503f, scale, offset);
        Vector2 p1_end   = foo(56.72326f, 5.04273f, scale, offset);

        DrawSplineSegmentBezierCubic(p1_start, p1_c1, p1_c2, p1_end, thick, RED);

        // --- SEGMENT 2 ---
        // Start is p1_end
        // C 102.7151 27.65048 106.54776 39.12308 83.9351 67.12971
        Vector2 p2_c1  = foo(102.7151f, 27.65048f, scale, offset);
        Vector2 p2_c2  = foo(106.54776f, 39.12308f, scale, offset);
        Vector2 p2_end = foo(83.9351f, 67.12971f, scale, offset);

        DrawSplineSegmentBezierCubic(p1_end, p2_c1, p2_c2, p2_end, thick, GREEN);

        // --- SEGMENT 3 ---
        // Start is p2_end
        // C 61.32244 95.13635 54.0404 102.22236 23.76245 91.08719
        Vector2 p3_c1  = foo(61.32244f, 95.13635f, scale, offset);
        Vector2 p3_c2  = foo(54.0404f, 102.22236f, scale, offset);
        Vector2 p3_end = foo(23.76245f, 91.08719f, scale, offset);

        DrawSplineSegmentBezierCubic(p2_end, p3_c1, p3_c2, p3_end, thick, BLUE);

        // --- SEGMENT 4 ---
        // Start is p3_end
        // C -6.51551 79.95203 95.43306 23.26391 0 43.17223
        Vector2 p4_c1  = foo(-6.51551f, 79.95203f, scale, offset);
        Vector2 p4_c2  = foo(95.43306f, 23.26391f, scale, offset);
        Vector2 p4_end = foo(0.0f, 43.17223f, scale, offset);

        DrawSplineSegmentBezierCubic(p3_end, p4_c1, p4_c2, p4_end, thick, ORANGE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
