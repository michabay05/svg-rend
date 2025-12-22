#include <stdlib.h>
#include "../vendor/include/raylib.h"

int main(void) {
    InitWindow(800, 600, "testing");
    SetTargetFPS(90);

    Image img = GenImageColor(700, 500, BLANK);
    Texture blank_tex = LoadTextureFromImage(img);
    UnloadImage(img);

    Shader shader = LoadShader(NULL, "test.fs");
    int time_loc = GetShaderLocation(shader, "time");
    int res_loc = GetShaderLocation(shader, "res");
    Vector2 res = {700, 500};
    SetShaderValueV(shader, res_loc, &res, SHADER_UNIFORM_VEC2, 1);

    float time = 0.0f;
    SetShaderValue(shader, time_loc, &time, SHADER_UNIFORM_FLOAT);

    while (!WindowShouldClose()) {
        time = (float)GetTime();
        SetShaderValue(shader, time_loc, &time, SHADER_UNIFORM_FLOAT);

        BeginDrawing(); {
            ClearBackground(GetColor(0x181818FF));
            BeginShaderMode(shader); {
                DrawTexture(blank_tex, 50, 50, WHITE);
            } EndShaderMode();

            DrawFPS(10, 10);
        } EndDrawing();
    }

    UnloadTexture(blank_tex);
    UnloadShader(shader);
    CloseWindow();
    return 0;
}
