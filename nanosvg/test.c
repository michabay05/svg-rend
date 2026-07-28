#include <stdio.h>
#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"


int main(void) {
    // Load SVG
	NSVGimage* image;
	image = nsvgParseFromFile("../test.svg", "px", 96);
	printf("size: %f x %f\n", image->width, image->height);
	// // Use...
	// for (NSVGshape *shape = image->shapes; shape != NULL; shape = shape->next) {
	// 	for (NSVGpath *path = shape->paths; path != NULL; path = path->next) {
	// 		for (int i = 0; i < path->npts-1; i += 3) {
	// 			float* p = &path->pts[i*2];
	// 			drawCubicBez(p[0],p[1], p[2],p[3], p[4],p[5], p[6],p[7]);
	// 		}
	// 	}
	// }
	// Delete
	nsvgDelete(image);
}