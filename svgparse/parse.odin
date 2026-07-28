package svgparse

import "core:strconv"
import "core:strings"
import "core:fmt"
import "core:encoding/xml"

SVG_Rect :: [4]f32

Fill_Rule :: enum { EvenOdd, NonZero }
Use_Info :: struct {
    ref_id: string,
    x, y: f32,
    fill: string,
    fill_rule: Fill_Rule,
}
Group :: struct {
    transform: string,
    uses: [dynamic]Use_Info
}

Path :: struct { d: string }
Symbol :: struct {
    id: string,
    items: [dynamic]Path
}

SVG_Info :: struct {
    // Unit in pt
    view_box: SVG_Rect,
    groups: [dynamic]Group,
    symbols: [dynamic]Symbol,
    current_block: enum { Svg, Group, Def, Symbol }
}

main :: proc() {
    // path_parse_main()
    split_cubic_main()
}

main2 :: proc() {
    content := #load("test.svg")
    doc, err := xml.parse(content)
    
    for el in doc.elements {
        fmt.println(el, "\n")
    }

    svg: SVG_Info
    process_tags(&svg, doc.elements[:], 0)
    fmt.println(svg)
}

process_tags :: proc(svg: ^SVG_Info, elements: []xml.Element, index: int) {
    process_values :: proc(svg: ^SVG_Info, elements: []xml.Element, values: []xml.Value) {
        block := svg.current_block
        for value in values {
            switch v in value {
            case xml.Element_ID:
                svg.current_block = block
                process_tags(svg, elements, int(v))
            case string:
                unimplemented()
            }
        }
    }

    el := elements[index]
    assert(el.kind == .Element)
    switch el.ident {
    case "svg":
        svg.current_block = .Svg
        for attr in el.attribs {
            switch attr.key {
            case "viewBox":
                parts, err := strings.split(attr.val, " ")
                assert(err == nil)
                assert(len(parts) == 4)
                for part, i in parts {
                    parsed_val, parse_ok := strconv.parse_f32(part)
                    assert(parse_ok)
                    svg.view_box[i] = parsed_val
                }
            }
        }
        process_values(svg, elements, el.value[:])

    case "g":
        assert(svg.current_block == .Svg)
        svg.current_block = .Group
        group := Group{}
        for attr in el.attribs {
            switch attr.key {
            case "transform":
                // TODO: Parse transform command
                group.transform = attr.val
            case: fmt.eprintfln("[WARN] skipping attribute in group: %s", attr.key)
            }
        }
        append(&svg.groups, group)
        process_values(svg, elements, el.value[:])
    
    case "use":
        assert(svg.current_block == .Group)
        info: Use_Info
        for attr in el.attribs {
            switch attr.key {
            case "xlink:href":
                // TODO: Check that the id has a # at the beginning before slicing it off.
                info.ref_id = attr.val[1:]
            case "x":
                x_val, x_ok := strconv.parse_f32(attr.val)
                assert(x_ok)
                info.x = x_val
            case "y":
                y_val, y_ok := strconv.parse_f32(attr.val)
                assert(y_ok)
                info.y = y_val
            case "fill":
                info.fill = attr.val
            case "fill-rule":
                switch attr.val {
                case "nonzero": info.fill_rule = .NonZero
                case "evenodd": info.fill_rule = .EvenOdd
                case: panic(fmt.tprintf("Unknown fill rule: %s", attr.val))
                }
            }
        }
        append(&svg.groups[len(svg.groups) - 1].uses, info)
        process_values(svg, elements, el.value[:])

    case "defs":
        assert(svg.current_block == .Svg)
        svg.current_block = .Def
        process_values(svg, elements, el.value[:])

    case "path":
        assert(svg.current_block == .Symbol)
        path: Path
        for attr in el.attribs {
            switch attr.key {
            case "d":
                path.d = attr.val
            case:
                fmt.eprintfln(">> '%s'", attr.key)
            }
        }
        sym := &svg.symbols[len(svg.symbols) - 1]
        append(&sym.items, path)
        process_values(svg, elements, el.value[:])

    case "symbol":
        assert(svg.current_block == .Def)
        svg.current_block = .Symbol
        symbol: Symbol
        for attr in el.attribs {
            switch attr.key {
            case "id": symbol.id = attr.val 
            case: fmt.eprintfln("[WARN] skipping symbol attribute: '%s'", attr.key)
            }
        }
        append(&svg.symbols, symbol)
        process_values(svg, elements, el.value[:])

    case:
        fmt.eprintfln("Unhandled el type: '%s'", el.ident)
        unreachable()
    }
}

_parse_transform :: proc(svg: ^SVG_Info, val: string) {}
