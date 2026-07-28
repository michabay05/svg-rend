package svgparse

import "core:fmt"
import "core:unicode"
import "core:strconv"

Move :: distinct [2]f32
Line :: distinct [2]f32
QCurve :: distinct [4]f32
CCurve :: distinct [6]f32
Close :: struct {}
Action :: struct {
    absolute: bool,
    val: union {
        Move, Line, CCurve, Close
    }
}

path_parse_main :: proc() {
    content := string(#load("path_d.txt"))
    fmt.println(content, "\n")

    actions := make([dynamic]Action)
    tokenize(content, &actions)
    fmt.println("------------------------------------------")
    fmt.println(actions)
}

tokenize :: proc(content: string, actions: ^[dynamic]Action) {
    consume_while :: proc(content: string, i: ^int, filter: proc(content: u8) -> bool) -> string {
        start := i^
        for i^ < len(content) && filter(content[i^]) {
            i^ += 1
        }
        return content[start:i^]
    }

    num_filter :: proc(content: u8) -> bool {
        return unicode.is_number(rune(content)) || content == '.' || content == '-'
    }

    space_filter :: proc(content: u8) -> bool {
        return unicode.is_space(rune(content))
    }

    i := 0
    for i < len(content) {
        c := content[i]
        switch c {
        case 'M', 'm':
            // Consume 'M'/'m'
            i += 1
            mv: Move
            for k in 0..<2 {
                consume_while(content, &i, space_filter)
                n_str := consume_while(content, &i, num_filter)
                n, n_ok := strconv.parse_f32(n_str)
                assert(n_ok, fmt.tprintf("Unable to parse '%s' to f32", n_str))
                mv[k] = n
            }
            append(actions, Action{absolute = c == 'M', val = mv})

        case 'C', 'c':
            // Consume 'C'/'c'
            i += 1
            curve: CCurve
            for k in 0..<6 {
                consume_while(content, &i, space_filter)
                n_str := consume_while(content, &i, num_filter)
                n, n_ok := strconv.parse_f32(n_str)
                assert(n_ok, fmt.tprintf("Unable to parse '%s' to f32", n_str))
                curve[k] = n
            }
            append(actions, Action{absolute = c == 'C', val = curve})

        case 'L', 'l':
            // Consume 'L'/'l'
            i += 1
            line: Line
            for k in 0..<2 {
                consume_while(content, &i, space_filter)
                n_str := consume_while(content, &i, num_filter)
                n, n_ok := strconv.parse_f32(n_str)
                assert(n_ok, fmt.tprintf("Unable to parse '%s' to f32", n_str))
                line[k] = n
            }
            append(actions, Action{absolute = c == 'L', val = line})

        case 'Z', 'z':
            i += 1
            consume_while(content, &i, space_filter)
            append(actions, Action{absolute = c == 'Z', val = Close{}})
        case:
            fmt.eprintfln("[TODO] handle '%c' action", c)
            return
        }
    }
}