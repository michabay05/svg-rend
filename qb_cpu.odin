package vg

import "core:fmt"
import "core:math"
import "core:mem"
import rl "vendor:raylib"

CELL_FILL_FACTOR :: 0.8
FILL_COLOR :: rl.BLUE
STROKE_COLOR :: rl.RED
MARKER_COLOR :: rl.DARKGREEN
PT_R :: f32(8.)

State :: struct {
    grid_c, grid_r: int,
    image: rl.Image,
    pts: [dynamic]Vector2,
    drag_ind: int,   
}

control_points_in_image :: proc(state: ^State) {
    mouse_pos := rl.GetMousePosition()
    if rl.IsMouseButtonDown(.LEFT) {
        for i := 0; state.drag_ind == -1 && i < len(state.pts); i += 1 {
            if rl.CheckCollisionPointCircle(mouse_pos, state.pts[i], PT_R) {
                state.drag_ind = i
                break;
            }
        }
        if state.drag_ind == -1 do append(&state.pts, mouse_pos)
    }

    if rl.IsMouseButtonUp(.LEFT) && state.drag_ind >= 0 {
        state.drag_ind = -1
    }

    if state.drag_ind >= 0 {
        state.pts[state.drag_ind] = mouse_pos
    }
}

fill_nonzero :: proc(state: ^State, tl_offset: Vector2) {
    // NOTE: I'm too lazy to rewrite all of these with the `state.` prefix
    pts := &state.pts
    img := &state.image

    if len(pts) <= 3 do return
    if len(pts) % 2 != 0 do return

    winding := 0
    for r in 0..<img.height {
        for c in 0..<img.width {
            winding = 0
            ray := Vector2 {f32(c), f32(r)} + 0.5

            for i := 0; i < len(pts); i += 2 {
                p0 := pts[i+0] - tl_offset
                c1 := pts[(i+1)%len(pts)] - tl_offset
                p2 := pts[(i+2)%len(pts)] - tl_offset

                // ax^2 + bx + c
                // Q(A, B, C, t) = (A - 2B + C)t^2 + (2B - 2A)t + A
                a := p0.y - 2 * c1.y + p2.y
                b := 2 * (c1.y - p0.y)
                c := p0.y - ray.y

                ts: [2]f32
                tn := qbezier_find_roots(a, b, c, &ts)
                for i in 0..<tn {
                    t := ts[i]
                    val := qbezier_compute(p0, c1, p2, t)
                    if (0.0 <= t && t <= 1.0) && val.x > ray.x {
                        deriv := qbezier_derive_compute(p0, c1, p2, t).y
                        if deriv > 0 do winding += 1
                        else if deriv < 0 do winding -= 1
                    }
                }
            }

            if winding > 0 {
                rl.ImageDrawPixel(&state.image, i32(c), i32(r), FILL_COLOR)
            }
        }
    }
}

qbezier_compute :: proc(p0, c1, p2: Vector2, t: f32) -> Vector2 {
    // Q(A, B, C, t) = (A - 2B + C)t^2 + (2B - 2A)t + A
    // Q(A, B, C, t) = (    a     )t^2 + (   b   )t + c
    a := p0 - (c1 * 2) + p2
    b := 2 * (c1 - p0)
    c := p0
    return a*t*t + b*t + c
}

qbezier_find_roots :: proc (a, b, c: f32, ts: ^[2]f32) -> int {
    if f32_close(a, 0.0) {
        // Linear: bt + c = 0
        ts[0] = -c / b
        return 1
    }

    if f32_close(b, 0.0) {
        // at^2 + c = 0
        ts[0] = -math.sqrt(-c / a)
        ts[1] = math.sqrt(-c / a)
        return 2
    }

    if f32_close(c, 0.0) {
        // at^2 + bt = 0
        ts[0] = 0
        ts[1] = -b / a
        return 2
    }

    det := b * b - 4 * a * c
    if f32_close(det, 0.0) {
        ts[0] = -b / (2*a)
        return 1
    } else {
        ts[0] = (-b - math.sqrt(det)) / (2*a)
        ts[1] = (-b + math.sqrt(det)) / (2*a)
        return 2
    }
}

qbezier_derive_compute :: proc(p0, c1, p2: Vector2, t: f32) -> Vector2 {
    // Q(A, B, C, t)  = (A - 2B + C)t^2 + (2B - 2A)t + A
    // Q(A, B, C, t)  = (    a     )t^2 + (   b   )t + c
    // -------
    // Q'(A, B, C, t) = 2(A - 2B + C)t  + (2B - 2A)
    // Q'(A, B, C, t) = (     a     )t  + (   b   )

    a := p0 - (c1 * 2) + p2
    b := (c1 - p0) * 2

    return a * 2*t + b
}

f32_close :: proc(a, b: f32) -> bool {
    return max(a, b) - min(a, b) < 1e-6
}


_to_index :: #force_inline proc(state: State, r, c: int) -> int {
    return r * state.grid_c + c
}