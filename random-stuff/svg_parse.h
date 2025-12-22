#ifndef _SVG_PARSE_H_
#define _SVG_PARSE_H_

#include "nob.h"
#include "./vendor/include/arena.h"
#include "./vendor/include/raymath.h"

#define UNREACHABLEF(message, ...)                                                             \
    do {                                                                                          \
        fprintf(stderr, "%s:%d: UNREACHABLE: " message "\n", __FILE__, __LINE__, __VA_ARGS__);    \
        abort();                                                                                  \
    } while (0)
#define PRINT_SV(sv) (printf("%s = (" SV_Fmt")\n", #sv, SV_Arg(sv)))
typedef float f32;

typedef enum {
    TK_OPEN,
    TK_CLOSE,
    TK_SINGLE,
} TagKind;

typedef struct {
    Nob_String_View name, value;
} Attrib;

typedef struct {
    Attrib *items;
    int count;
    int capacity;
} AttribList;

typedef struct {
    TagKind kind;
    Nob_String_View name;
    AttribList attribs;
} Tag;

typedef struct {
    Tag *items;
    int count;
    int capacity;
} TagList;

typedef enum {
    ACT_MOVE,
    ACT_CUBIC,
    ACT_HORZ_TO,
    ACT_CLOSE
} ActionKind;

typedef struct {
    Vector2 *items;
    int count;
    int capacity;
} PointList;

typedef struct {
    ActionKind kind;
    PointList points;
    bool absolute;
} Action;

typedef struct {
    Action *items;
    int count;
    int capacity;
} ActionList;

typedef struct {
    Vector2 s, c1, c2, e;
} CBezier;

typedef struct {
    CBezier *items;
    int count;
    int capacity;
} CBezierList;

ActionList get_path_info(Arena *arena);

#endif // _SVG_PARSE_H_
