package svgparse

import "core:fmt"
import "core:math"
import "core:math/linalg"
import "core:sort"
import rl "vendor:raylib"

CBezier :: distinct [4]Vector2
QBezier :: distinct [3]Vector2

Vector2 :: rl.Vector2
CB_THICK :: 8.
CB_COLOR :: rl.BLUE
QB_THICK :: 4
QB_COLOR_POOL := []rl.Color{rl.GOLD, rl.VIOLET, rl.WHITE, rl.BROWN, rl.PINK, rl.ORANGE,}
MK_RADIUS :: 6.
MK_COLOR :: rl.RED

split_cubic_main :: proc() {
    rl.SetConfigFlags({.MSAA_4X_HINT})
    rl.InitWindow(800, 600, "Split Cubic")
    defer rl.CloseWindow()
    rl.SetTargetFPS(60)

    orig_pts := CBezier {
        {200, 400},
        {300, 300},
        {400, 300},
        {500, 400},
    }
    cb := orig_pts
    pt_ind := int(-1)
    qbs:= make([dynamic]QBezier)

    for !rl.WindowShouldClose() {
        {
            mpos := rl.GetMousePosition()
            if rl.IsMouseButtonDown(.LEFT) {
                for pt, i in cb {
                    if rl.CheckCollisionPointCircle(mpos, pt, MK_RADIUS) {
                        pt_ind = i
                    }
                }
            } else do pt_ind = -1

            if pt_ind >= 0 do cb[pt_ind] = mpos

            if rl.IsKeyPressed(.R) {
                cb = orig_pts
            }
        }


        clear(&qbs)
        subdivide_cb(cb, &qbs)

        {
            rl.BeginDrawing()
            defer rl.EndDrawing()
            rl.ClearBackground(rl.BLACK)

            rl.DrawSplineSegmentBezierCubic(**cb, CB_THICK, CB_COLOR)
            for qb, i in qbs {
                rl.DrawSplineSegmentBezierQuadratic(**qb, QB_THICK, QB_COLOR_POOL[i % len(QB_COLOR_POOL)])
            }

            for pt in cb {
                rl.DrawCircleV(pt, MK_RADIUS, MK_COLOR)
            }
        }
    }
}

subdivide_cb :: proc(
    cb: CBezier, qbs: ^[dynamic]QBezier,
    eps: f32 = 1e-3, max_error: f32 = 5.0
) {
    p0, p1, p2, p3 := **cb

    // ============== FIND SPLIT POINTS ==============
    // Find inflection point
    u0, v0 := **(3 * (-p0 + 3*p1 - 3*p2 + p3))
    u1, v1 := **(6 * (p0 - 2*p1 + p2))
    u2, v2 := **(3 * (-p0 + p1))

    t_i: [2]f32
    infl_count := quadratic_find_roots(v0*u1 - v1*u0, 2*v0*u2 - 2*u0*v2, v1*u2 - u1*v2, &t_i)

    // Find extremas
    t_x, t_y: [2]f32
    e_x_count := quadratic_find_roots(u0, u1, u2, &t_x)
    e_y_count := quadratic_find_roots(v0, v1, v2, &t_y)

    // fmt.eprintln("inflection:", t_i)
    // fmt.eprintln("extrema x :", t_x)
    // fmt.eprintln("extrema y :", t_y)

    T: [dynamic; 6]f32
    for t in t_i[:infl_count] {
        if 0 < t && t < 1 do append(&T, t)
    }
    for t in t_x[:e_x_count] {
        if 0 < t && t < 1 do append(&T, t)
    }
    for t in t_y[:e_y_count] {
        if 0 < t && t < 1 do append(&T, t)
    }
    sort.quick_sort(T[:])

    // Deduplicate
    for i := 0; i < len(T) - 1; i += 1 {
        if T[i+1] - T[i] < eps {
            ordered_remove(&T, i + 1)
            fmt.println("Deduped")
        }
    }
    // fmt.eprintln("Final:", T)

    // ============== COMPUTE QBEZIER POINTS ==============
    ts := make([]f32, len(T) + 2)
    defer delete(ts)
    // ts = {0.0, **T, 1.0}
    ts[0] = 0.0
    for t, i in T do ts[1 + i] = t
    ts[len(ts) - 1] = 1.0

    for i := 0; i < len(ts)-1; i += 1 {
        approximate_segment_adaptive(cb, ts[i], ts[i+1], qbs, max_error, 0)
    }
}

// Recursively approximates a subsegment [t0, t1] until the geometric error is within tolerance
approximate_segment_adaptive :: proc(
    cb: CBezier, 
    start_t, end_t: f32,
    qbs: ^[dynamic]QBezier,
    max_error: f32,
    depth: int
) {
    p0, p1, p2, p3 := **cb

    a0, a := cbezier_compute(p0, p1, p2, p3, start_t), cbezier_deriv_compute(p0, p1, p2, p3, start_t)
    b0, b := cbezier_compute(p0, p1, p2, p3, end_t), cbezier_deriv_compute(p0, p1, p2, p3, end_t)

    // Tangent intersection via matrix inversion
    det := a.x * (-b.y) - a.y * (-b.x)
    
    middle: Vector2
    // If tangents are nearly parallel or degenerate, fallback to chord midpoint
    if math.abs(det) < 1e-5 {
        middle = (a0 + b0) * 0.5
    } else {
        inv := linalg.inverse(matrix[2,2]f32{a.x, -b.x, a.y, -b.y})
        sol := inv * Vector2{b0.x - a0.x, b0.y - a0.y}
        middle = a0 + a * sol[0]
    }

    qb := QBezier{a0, middle, b0}

    // Base case: prevent infinite recursion
    if depth >= 4 {
        append(qbs, qb)
        return
    }

    // Measure error at the parameter midpoint t_mid
    t_mid := (start_t + end_t) * 0.5
    cubic_mid := cbezier_compute(p0, p1, p2, p3, t_mid)
    
    // For a quadratic Bézier, parameter t = 0.5 is: 0.25*start + 0.5*middle + 0.25*end
    quad_mid := (a0 + b0) * 0.25 + middle * 0.5
    
    error := linalg.length(cubic_mid - quad_mid)

    // If deviation exceeds tolerance, split the segment in half and recurse
    if error > max_error {
        approximate_segment_adaptive(cb, start_t, t_mid, qbs, max_error, depth + 1)
        approximate_segment_adaptive(cb, t_mid, end_t, qbs, max_error, depth + 1)
    } else {
        append(qbs, qb)
    }
}

cbezier_compute :: #force_inline proc(p0, p1, p2, p3: Vector2, t: f32) -> Vector2 {
    return (-p0 + 3*p1 - 3*p2 + p3)*t*t*t + (3*p0 - 6*p1 + 3*p2)*t*t + (-3*p0 + 3*p1)*t + p0
}

cbezier_deriv_compute :: #force_inline proc(p0, p1, p2, p3: Vector2, t: f32) -> Vector2 {
    return 3*(-p0 + 3*p1 - 3*p2 + p3)*t*t + 6*(p0 - 2*p1 + p2)*t + 3*(-p0 + p1)
}

quadratic_find_roots :: proc(a, b, c: f32, ts: ^[2]f32, eps: f32 = 1e-7) -> int {
    // Degenerate linear case: a approx 0
    if math.abs(a) < eps {
        // 0t^2 + bt + c = 0
        if math.abs(b) < eps {
            // 0t + c = 0 (zero (c == 0) or infinite[c != 0] solutions)
            return 0
        }
        ts[0] = -c / b
        return 1
    }

    det := b * b - 4 * a * c
    if det < -eps {
        // No real roots (complex conjugate roots)
        return 0
    } else if math.abs(det) <= eps {
        // Single repeated root
        ts[0] = -b / (2 * a)
        return 1
    } else {
        // Uses Citardauq formulation (q-method) to prevent catastrophic cancellation.
        sqrt_det := math.sqrt(det)
        q: f32
        if b >= 0 {
            q = -0.5 * (b + sqrt_det)
        } else {
            q = -0.5 * (b - sqrt_det)
        }

        // "quadratic"
        ts[0] = q / a
        // "citardauq" ("quadratic" backwards)
        ts[1] = c / q

        // Ensure roots are returned in ascending order t0 <= t1
        if ts[0] > ts[1] {
            ts[0], ts[1] = ts[1], ts[0]
        }
        return 2
    }
}