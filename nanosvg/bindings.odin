/*
 * Copyright (c) 2013-14 Mikko Mononen memon@inside.org
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * The SVG parser is based on Anti-Grain Geometry 2.4 SVG example
 * Copyright (C) 2002-2004 Maxim Shemanarev (McSeem) (http://www.antigrain.com/)
 *
 * Arc calculation code based on canvg (https://code.google.com/p/canvg/)
 *
 * Bounding box calculation based on http://blog.hackers-cafe.net/2009/06/how-to-calculate-bezier-curves-bounding.html
 *
 */


package nanosvg

when ODIN_OS == .Linux {
    foreign import lib {
        "./libnanosvg.a",
    }
} else {
	#panic("Other OS's are not supported yet.")
}

import "core:c"

// NanoSVG is a simple stupid single-header-file SVG parse. The output of the parser is a list of cubic bezier shapes.
//
// The library suits well for anything from rendering scalable icons in your editor application to prototyping a game.
//
// NanoSVG supports a wide range of SVG features, but something may be missing, feel free to create a pull request!
//
// The shapes in the SVG images are transformed by the viewBox and converted to specified units.
// That is, you should get the same looking data as your designed in your favorite app.
//
// NanoSVG can return the paths in few different units. For example if you want to render an image, you may choose
// to get the paths in pixels, or if you are feeding the data into a CNC-cutter, you may want to use millimeters.
//
// The units passed to NanoSVG should be one of: 'px', 'pt', 'pc' 'mm', 'cm', or 'in'.
// DPI (dots-per-inch) controls how the unit conversion is done.
//
// If you don't know or care about the units stuff, "px" and 96 should get you going.


/* Example Usage:
	// Load SVG
	NSVGimage* image;
	image = nsvgParseFromFile("test.svg", "px", 96);
	printf("size: %f x %f\n", image->width, image->height);
	// Use...
	for (NSVGshape *shape = image->shapes; shape != NULL; shape = shape->next) {
		for (NSVGpath *path = shape->paths; path != NULL; path = path->next) {
			for (int i = 0; i < path->npts-1; i += 3) {
				float* p = &path->pts[i*2];
				drawCubicBez(p[0],p[1], p[2],p[3], p[4],p[5], p[6],p[7]);
			}
		}
	}
	// Delete
	nsvgDelete(image);
*/

PaintType :: enum c.int {
	Undef = -1,
	None = 0,
	Color = 1,
	Linear_Gradient = 2,
	Radial_Gradient = 3
}

SpreadType :: enum c.int {
	Pad = 0,
	Reflect = 1,
	Repeat = 2
}

LineJoin :: enum c.int {
	Miter = 0,
	Round = 1,
	Bevel = 2
}

LineCap :: enum c.int {
	Butt = 0,
	Round = 1,
	Square = 2
}

FillRule :: enum c.char {
	Nonzero = 0,
	Evenodd = 1
}

Flags :: enum c.int {
	Visible = 0x01
}

PaintOrder :: enum c.int {
	Fill = 0x00,
	Markers = 0x01,
	Stroke = 0x02,
}

GradientStop :: struct {
	color: c.uint,
	offset: c.float,
}

Gradient :: struct {
	xform: [6]c.float,
	spread: c.char,
	fx, fy: c.float,
	nstops: c.int,
	stops: [1]GradientStop,
}

PaintData :: struct #raw_union {
    color:    c.uint,
    gradient: ^Gradient,
}

Paint :: struct {
	type: c.char,
	using data: PaintData
}

Path :: struct {
	pts:    [^]c.float,   // Cubic bezier points: x0,y0, [cpx1,cpx1,cpx2,cpy2,x1,y1], ...
	npts:   c.int,        // Total number of bezier points.
	closed: c.char,       // Flag indicating if shapes should be treated as closed.
	bounds: [4]c.float,   // Tight bounding box of the shape [minx,miny,maxx,maxy].
	next:   ^Path,        // Pointer to next path, or NULL if last element.
}

Shape :: struct {
	id:                 [64]c.char,         // Optional 'id' attr of the shape or its group
    fill:               Paint,          // Fill paint
    stroke:             Paint,          // Stroke paint
	opacity:            c.float,            // Opacity of the shape.
	strokeWidth:        c.float,            // Stroke width (scaled).
	strokeDashOffset:   c.float,            // Stroke dash offset (scaled).
	strokeDashArray:    [8]c.float,         // Stroke dash array (scaled).
	strokeDashCount:    c.char,             // Number of dash values in dash array.
	strokeLineJoin:     c.char,             // Stroke join type.
	strokeLineCap:      c.char,             // Stroke cap type.
	miterLimit:         c.float,            // Miter limit
	fillRule:           FillRule,       // Fill rule, see NSVGfillRule.
    paintOrder:         c.uchar,            // Encoded paint order (3×2-bit fields) see NSVGpaintOrder
	flags:              bit_set[Flags], // Logical or of NSVG_FLAGS_* flags
	bounds:             [4]c.float,         // Tight bounding box of the shape [minx,miny,maxx,maxy].
	fillGradient:       [64]c.char,         // Optional 'id' of fill gradient
	strokeGradient:     [64]c.char,         // Optional 'id' of stroke gradient
	xform:              [6]c.float,         // Root transformation for fill/stroke gradient
	paths:              ^Path,          // Linked list of paths in the image.
	next:               ^Shape,         // Pointer to next shape, or NULL if last element.
}

Image :: struct {
	width:  c.float,      // Width of the image.
	height: c.float,      // Height of the image.
	shapes: ^Shape,   // Linked list of shapes in the image.
}

@(link_prefix="nsvg", default_calling_convention="c")
foreign lib {
    // Parses SVG file from a file, returns SVG image as paths.
    ParseFromFile :: proc(filename: cstring, units: cstring, dpi: c.float) -> ^Image ---

    // Parses SVG file from a null terminated string, returns SVG image as paths.
    // Important note: changes the string.
    Parse :: proc(input: cstring, units: cstring, dpi: c.float) -> ^Image ---

    // Duplicates a path.
    DuplicatePath :: proc(p: ^Path) -> ^Path ---

    // Deletes an image.
    Delete :: proc(image: ^Image) ---
}
