#include <stdio.h>
#include <string.h>

#include "../src/span.h"
#include "resvg.h"

const char letters[11] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.'
};

Texture texs[11] = {0};
f32 max_h = 0.f;

bool load_letters(void)
{
    resvg_options *opt = resvg_options_create();

    char buf[128] = {0};
    for (int i = 0; i < 11; i++) {
        snprintf(buf, 128, "test-%d.svg", i+1);
        resvg_render_tree *tree;
        int err = resvg_parse_tree_from_file(buf, opt, &tree);
        if (err != RESVG_OK) {
            printf("Error id: %i\n", err);
            return false;
        }
        memset(buf, 0, 128*sizeof(char));

        resvg_size size = resvg_get_image_size(tree);
        int width = (int)size.width;
        int height = (int)size.height;
        if (max_h < (f32)height) {
            max_h = (f32)height;
            printf("Updated max_h at i = %d to %.3f\n", i, max_h);
        }

        Image img = GenImageColor(width, height, BLANK);
        resvg_render(tree, resvg_transform_identity(), img.width, img.height, (char*)img.data);
        texs[i] = LoadTextureFromImage(img);
        SetTextureFilter(texs[i], TEXTURE_FILTER_BILINEAR);
        UnloadImage(img);
        resvg_tree_destroy(tree);
    }

    printf("max_h = %.4f\n", max_h);

    resvg_options_destroy(opt);
    return true;
}

void display_num(Vector2 tl, const char *text, f32 scale, f32 spacing)
{
    size_t n = strlen(text);
    f32 pos_x = tl.x;
    for (size_t i = 0; i < n; i++) {
        char ch = text[i];
        int j;
        for (j = 0; j < 11; j++) {
            if (ch == letters[j]) break;
        }
        Texture tex = texs[j];
        Vector2 p = {pos_x, tl.y + (max_h - tex.height)};
        DrawTextureEx(tex, p, 0.f, scale, WHITE);
        pos_x += tex.width + spacing;
    }

#if 0
    Rectangle r = {
        .x = tl.x,
        .y = tl.y,
        .width = pos_x - tl.x,
        .height = max_h,
    };
    DrawRectangleLinesEx(r, 2.f, RED);
    DrawCircleV(tl, 5.f, BLUE);
#endif
}

int main(void)
{
    // Initialize resvg's library logging system
    resvg_init_log();

    SetTraceLogLevel(LOG_WARNING);
    InitWindow(800, 600, "span - resvg test");
    SetTargetFPS(60);

    if (!load_letters()) return 1;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        Vector2 p = {100, 100};
        display_num(p, "3.14159265358", 1.f, -1.f);
        DrawFPS(10, 10);
        EndDrawing();
    }

    CloseWindow();
}
