package vg

import "core:fmt"
import "core:math"
import rl "vendor:raylib"
import nsvg "nanosvg"

Color :: rl.Color
Vector2 :: rl.Vector2
Rectangle :: rl.Rectangle

main :: proc() {
    rl.SetTraceLogLevel(.WARNING)
    rl.InitWindow(800, 600, "svg render - cpu")
    defer rl.CloseWindow()
    rl.SetTargetFPS(60)

    state := State {
        pts = make([dynamic]Vector2),
        drag_ind = -1,
        image = rl.GenImageColor(700, 500, rl.RED)
    }

    img := nsvg.ParseFromFile("./test.svg", "px", 96)
    fmt.println(img^)

    pad := f32(10.)
    tex_rect, border_rect: Rectangle
    tex_size: Vector2
    for !rl.WindowShouldClose() {
        {
            if rl.IsKeyPressed(.C) {
                clear(&state.pts)
            }
        }

        {
            screen_size := Vector2 {f32(rl.GetScreenWidth()), f32(rl.GetScreenHeight())}
            tex_size = Vector2 {f32(state.image.width), f32(state.image.height)}
            tex_rect = Rectangle {
                **(0.5 * (screen_size - tex_size)),
                **tex_size,
            }
            border_rect = Rectangle {
                x = tex_rect.x - pad,
                y = tex_rect.y - pad,
                width = tex_rect.width + 2. * pad,
                height = tex_rect.height + 2. * pad,
            }

            mouse_pos := rl.GetMousePosition()
            if rl.CheckCollisionPointRec(mouse_pos, tex_rect) {
                control_points_in_image(&state)
                
            }
            rl.ImageClearBackground(&state.image, rl.RED)
            fill_nonzero(&state, Vector2{tex_rect.x, tex_rect.y})
        }

        {
            rl.BeginDrawing()
            defer rl.EndDrawing()
            rl.ClearBackground(rl.BLACK)

            tex := rl.LoadTextureFromImage(state.image)
            rl.DrawRectangleLinesEx(border_rect, 2.0, rl.WHITE)
            rl.DrawTexturePro(tex, Rectangle{0, 0, **tex_size}, tex_rect, Vector2{}, 0.0, rl.WHITE)

            for pt in state.pts {
                rl.DrawCircleV(pt, PT_R, MARKER_COLOR)
            }

            for i := 0; i+1 < len(state.pts); i += 2 {
                p0 := state.pts[i+0]
                c1 := state.pts[i+1]
                p2 := state.pts[(i+2)%len(state.pts)]
                rl.DrawSplineSegmentBezierQuadratic(p0, c1, p2, 3., rl.GREEN)
            }
            rl.DrawFPS(10, 10)
        }
    }

    fmt.println("Last point arrangement")
    for pt, i in state.pts {
        fmt.printf("Vector2%w", pt)
        if i < len(state.pts) - 1 do fmt.printf(", ")
    }
    fmt.println()
}