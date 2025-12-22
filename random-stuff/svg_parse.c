#include "svg_parse.h"

void consume_word(Nob_String_Builder sb, size_t *i)
{
    while (*i < sb.count) {
        char c = sb.items[*i];
        if (!isalnum(c) && c != '-' && c != ':') break;
        (*i)++;
    }
}

void consume_string(Nob_String_Builder sb, size_t *i)
{
    while (*i < sb.count) {
        char c = sb.items[*i];
        if (c == '"') break;
        (*i)++;
    }
}

void consume_spaces_sv(Nob_String_View sv, size_t *i)
{
    while (*i < sv.count) {
        char c = sv.data[*i];
        if (c != ' ' && c != '\r' && c != '\n') break;
        (*i)++;
    }
}

void consume_spaces(Nob_String_Builder sb, size_t *i)
{
    consume_spaces_sv(nob_sb_to_sv(sb), i);
}

void consume_number(Nob_String_View sv, size_t *i)
{
    while (*i < sv.count) {
        char c = sv.data[*i];
        if (!isdigit(c) && c != '-' && c != '.') break;
        (*i)++;
    }
}

void print_tag(Tag tag)
{
    printf("<");
    if (tag.kind == TK_CLOSE) printf("/");
    printf(SV_Fmt, SV_Arg(tag.name));
    for (int i = 0; i < tag.attribs.count; i++) {
        Attrib attrib = tag.attribs.items[i];
        printf(" "SV_Fmt"=\""SV_Fmt"\"", SV_Arg(attrib.name), SV_Arg(attrib.value));
    }
    if (tag.kind == TK_SINGLE) printf("/");
    printf(">\n");
}

int find_tag_by_name(TagList tags, const char *tag_name)
{
    Nob_String_View sv = nob_sv_from_cstr(tag_name);
    for (int i = 0; i < tags.count; i++) {
        Tag tag = tags.items[i];
        if (nob_sv_eq(tag.name, sv)) return i;
    }
    return -1;
}

int find_attrib_by_name(Tag tag, const char *attrib_name)
{
    Nob_String_View sv = nob_sv_from_cstr(attrib_name);
    for (int i = 0; i < tag.attribs.count; i++) {
        Attrib attrib = tag.attribs.items[i];
        // NOTE: The only reason, I have added the `end_with` part is because of
        // some attribs like xlink:href
        if (nob_sv_eq(attrib.name, sv) ||
            nob_sv_end_with(attrib.name, attrib_name)) return i;
    }
    return -1;
}

int find_tag_by_id(TagList tags, const char *tag_id)
{
    Nob_String_View id_sv = nob_sv_from_cstr("id");
    Nob_String_View sv = nob_sv_from_cstr(tag_id);
    for (int i = 0; i < tags.count; i++) {
        Tag tag = tags.items[i];
        for (int k = 0; k < tag.attribs.count; k++) {
            Attrib attrib = tag.attribs.items[k];
            if (!nob_sv_eq(attrib.name, id_sv)) continue;
            if (nob_sv_eq(attrib.value, sv)) return i;
        }
    }
    return -1;
}

int find_end_pair(TagList tags, int start)
{
    Tag open = tags.items[start];
    NOB_ASSERT(open.kind == TK_OPEN);
    for (int i = start; i < tags.count; i++) {
        Tag tag = tags.items[i];
        if (tag.kind == TK_CLOSE && nob_sv_eq(tag.name, open.name)) return i;
    }
    return -1;
}

ActionList parse_path_data(Arena *arena, Nob_String_View sv)
{
    // Official svg - path spec
    // - Source: https://svgwg.org/svg2-draft/paths.html#PathElement

    ActionList al = {0};
    size_t start_i = 0;
    Nob_String_View tsv = {0};
    Vector2 current = {0};
    for (size_t i = 0; i < sv.count;) {
        switch (sv.data[i]) {
            case 'm':
            case 'M': {
                Action action = { .kind = ACT_MOVE, .absolute = isupper(sv.data[i]) };
                // Skip 'M' (or 'm')
                i++;
                consume_spaces_sv(sv, &i);

                // Move command has 2 numbers after it
                Vector2 pt = {0};
                start_i = i;
                consume_number(sv, &i);
                tsv = nob_sv_from_parts(sv.data + start_i, i - start_i);
                pt.x = (f32)atof(nob_temp_sv_to_cstr(tsv));
                consume_spaces_sv(sv, &i);

                start_i = i;
                consume_number(sv, &i);
                tsv = nob_sv_from_parts(sv.data + start_i, i - start_i);
                pt.y = (f32)atof(nob_temp_sv_to_cstr(tsv));
                consume_spaces_sv(sv, &i);

                pt = action.absolute ? pt : Vector2Add(current, pt);
                arena_da_append(arena, &action.points, pt);
                current = pt;

                arena_da_append(arena, &al, action);
                consume_spaces_sv(sv, &i);
            } break;

            case 'c': {
                Action action = { .kind = ACT_CUBIC, .absolute = isupper(sv.data[i]) };
                // Skip 'c'
                i++;
                consume_spaces_sv(sv, &i);

                arena_da_append(arena, &action.points, current);

                // Cubic bezier command has 6 numbers after it
                Vector2 pt = {0};
                for (int z = 0; z < 3; z++) {
                    start_i = i;
                    consume_number(sv, &i);
                    tsv = nob_sv_from_parts(sv.data + start_i, i - start_i);
                    pt.x = (f32)atof(nob_temp_sv_to_cstr(tsv));
                    consume_spaces_sv(sv, &i);

                    start_i = i;
                    consume_number(sv, &i);
                    tsv = nob_sv_from_parts(sv.data + start_i, i - start_i);
                    pt.y = (f32)atof(nob_temp_sv_to_cstr(tsv));
                    consume_spaces_sv(sv, &i);

                    pt = action.absolute ? pt : Vector2Add(current, pt);
                    arena_da_append(arena, &action.points, pt);
                }
                current = pt;

                arena_da_append(arena, &al, action);
                consume_spaces_sv(sv, &i);
            } break;

            case 'h': {
                Action action = { .kind = ACT_HORZ_TO, .absolute = isupper(sv.data[i]) };
                // Skip 'c'
                i++;
                consume_spaces_sv(sv, &i);

                arena_da_append(arena, &action.points, current);

                // Horizontal line to command has 1 numbers after it
                Vector2 pt = {0};
                for (int z = 0; z < 1; z++) {
                    start_i = i;
                    consume_number(sv, &i);
                    tsv = nob_sv_from_parts(sv.data + start_i, i - start_i);
                    pt.x = (f32)atof(nob_temp_sv_to_cstr(tsv));
                    consume_spaces_sv(sv, &i);
                    pt.y = 0.f;

                    pt = action.absolute ? pt : Vector2Add(pt, current);
                    arena_da_append(arena, &action.points, pt);
                }
                current = pt;

                arena_da_append(arena, &al, action);
                consume_spaces_sv(sv, &i);
            } break;

            case 'Z': {
                Action action = { .kind = ACT_CLOSE, .absolute = isupper(sv.data[i]) };
                // Skip 'Z'
                i++;

                arena_da_append(arena, &al, action);
                consume_spaces_sv(sv, &i);
            } break;

            default: {
                char c = sv.data[i];
                UNREACHABLEF("[PATH] Unknown character: '%c' (%d)", c, c);
            } break;
        }
    }

    for (int i = 0; i < al.count; i++) {
        Action a = al.items[i];
        printf("[%s] %d:", a.absolute ? "abs" : "rel", a.kind);
        for (int k = 0; k < a.points.count; k++) {
            printf(" (%8.4f, %8.4f)", a.points.items[k].x, a.points.items[k].y);
        }
        printf("\n");
    }

    return al;
}

// TODO: Apply transformations to <use />

void parse_svg(const char *input_svg, Arena *arena, TagList *tags);
ActionList get_path_info(Arena *arena)
{
    TagList tags = {0};
    parse_svg("test.svg", arena, &tags);

    Tag *tag = &tags.items[find_tag_by_name(tags, "use")];
    Nob_String_View value = tag->attribs.items[find_attrib_by_name(*tag, "href")].value;
    nob_sv_chop_left(&value, 1);
    const char *id = nob_temp_sv_to_cstr(value);
    int open = find_tag_by_id(tags, id);
    tag = &tags.items[open];
    int close = find_end_pair(tags, open);

    ActionList al = {0};
    for (int i = open+1; i <= close-1; i++) {
        Tag tag = tags.items[i];
        NOB_ASSERT(tag.kind != TK_CLOSE);
        NOB_ASSERT(nob_sv_eq(tag.name, nob_sv_from_cstr("path")));
        Nob_String_View value = tag.attribs.items[find_attrib_by_name(tag, "d")].value;
        al = parse_path_data(arena, value);
    }

#if 0
    CBezierList cbl = {0};
    Vector2 start = {0};
    for (int i = 0; i < al.count; i++) {
        Action act = al.items[i];
        switch (act.kind) {
            case ACT_MOVE: {
                NOB_ASSERT(act.points.count == 1);
                if (act.absolute) {
                    start = act.points.items[0];
                } else {
                    start = Vector2Add(act.points.items[0], start);
                }
            } break;

            case ACT_CUBIC: {
                NOB_ASSERT(act.points.count == 3);
                NOB_ASSERT(!act.absolute);
                Vector2 *vs = act.points.items;
                CBezier cb = {
                    .s  = start,
                    .c1 = Vector2Add(vs[0], start),
                    .c2 = Vector2Add(vs[1], start),
                    .e  = Vector2Add(vs[2], start),
                };
                start = cb.e;
                arena_da_append(&arena, &cbl, cb);
            } break;

            case ACT_HORZ_TO: {
                NOB_ASSERT(act.points.count == 1);
                // TODO: add line rendering things
                start = Vector2Add(act.points.items[0], start);
            } break;

            case ACT_CLOSE: break;

            default: {
                UNREACHABLEF("Unknown kind of action: %d", act.kind);
            } break;
        }
    }
#endif

    return al;
}

void parse_svg(const char *input_svg, Arena *arena, TagList *tags)
{
    Nob_String_Builder sb = {0};
    nob_read_entire_file(input_svg, &sb);

    size_t i = 0;
    while (i < sb.count) {
        switch (sb.items[i]) {
            case '<': {
                // Skip '<'
                i++;
                Tag tag = {0};
                tag.kind = TK_OPEN;
                if (sb.items[i] == '/') {
                    tag.kind = TK_CLOSE;
                    // Skip '/'
                    i++;
                }
                size_t start = i;
                consume_word(sb, &i);
                tag.name = nob_sv_from_parts(sb.items + start, i - start);
                consume_spaces(sb, &i);

                while (sb.items[i] != '>') {
                    Attrib attrib = {0};
                    start = i;
                    consume_word(sb, &i);
                    attrib.name = nob_sv_from_parts(sb.items + start, i - start);

                    NOB_ASSERT(sb.items[i] == '=');
                    // Skip '='
                    i++;
                    NOB_ASSERT(sb.items[i] == '"');
                    // Skip '"'
                    i++;

                    start = i;
                    consume_string(sb, &i);
                    attrib.value = nob_sv_from_parts(sb.items + start, i - start);
                    NOB_ASSERT(sb.items[i] == '"');
                    // Skip '"'
                    i++;

                    if (sb.items[i] == '/' && tag.kind == TK_OPEN) {
                        tag.kind = TK_SINGLE;
                        // Skip '/'
                        i++;
                    }
                    consume_spaces(sb, &i);
                    arena_da_append(arena, &tag.attribs, attrib);
                }

                NOB_ASSERT(sb.items[i] == '>');
                arena_da_append(arena, tags, tag);
                i++;
            } break;

            case ' ':
            case '\r':
            case '\n': {
                i++;
            } break;

            default: {
                char c = sb.items[i];
                UNREACHABLEF("[SVG] Unknown character: '%c' (%d)", c, c);
            }
        }
    }

    nob_sb_free(sb);
}
