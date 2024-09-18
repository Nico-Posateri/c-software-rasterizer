#include "triangle.h"
#include "display.h"
#include "swap.h"

vec3_t get_triangle_normal(vec4_t vertices[3]) {
	// Processes faces to be culled according to cull method
	vec3_t vector_a = vec3_from_vec4(vertices[0]); /*    A    */
	vec3_t vector_b = vec3_from_vec4(vertices[1]); /*   / \   */
	vec3_t vector_c = vec3_from_vec4(vertices[2]); /*  C---B  */
	// Get vector subtraction of B - A, C - A, normalize
	vec3_t vector_ab = vec3_sub(vector_b, vector_a);
	vec3_t vector_ac = vec3_sub(vector_c, vector_a);
	vec3_normalize(&vector_ab);
	vec3_normalize(&vector_ac);
	// Calculate face normal using cross-product to find perpendicular, normalize
	vec3_t normal = vec3_cross(vector_ab, vector_ac);
	vec3_normalize(&normal);

	return normal;
}

///////////////////////////////////////////////////////////////////////////////
// Return the barycentric weights alpha, beta, and gamma for point p
///////////////////////////////////////////////////////////////////////////////
//
//          (B)
//          /|\
//         / | \
//        /  |  \
//       /  (P)  \
//      /  /   \  \
//     / /       \ \
//	  //           \\
//	(A)-------------(C)
//
///////////////////////////////////////////////////////////////////////////////
vec3_t barycentric_weights(vec2_t a, vec2_t b, vec2_t c, vec2_t p) {
	// Find the vectors between the vertices ABC and point p
	vec2_t ac = vec2_sub(c, a);
	vec2_t ab = vec2_sub(b, a);
	vec2_t ap = vec2_sub(p, a);
	vec2_t pc = vec2_sub(c, p);
	vec2_t pb = vec2_sub(b, p);

	// Compute the area of the full parallegram/triangle ABC using 2D cross product
	float area_parallelogram_abc = (ac.x * ab.y - ac.y * ab.x); // || AC x AB ||

	// Alpha is the area of the small parallelogram/triangle PBC divided by the area of the full parallelogram/triangle ABC
	float alpha = (pc.x * pb.y - pc.y * pb.x) / area_parallelogram_abc;

	// Beta is the area of the small parallelogram/triangle APC divided by the area of the full parallelogram/triangle ABC
	float beta = (ac.x * ap.y - ac.y * ap.x) / area_parallelogram_abc;

	// Weight gamma is easily found since barycentric coordinates always add up to 1.0
	float gamma = 1 - alpha - beta;

	vec3_t weights = { alpha, beta, gamma };
	return weights;
}

///////////////////////////////////////////////////////////////////////////////
// Draw a triangle
///////////////////////////////////////////////////////////////////////////////
void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
	draw_line(x0, y0, x1, y1, color);
	draw_line(x1, y1, x2, y2, color);
	draw_line(x2, y2, x0, y0, color);
}

///////////////////////////////////////////////////////////////////////////////
// Draw a solid pixel at position x and y using interpolation
///////////////////////////////////////////////////////////////////////////////
void draw_triangle_pixel(
	int x, int y, uint32_t color,
	vec4_t point_a, vec4_t point_b, vec4_t point_c
) {
	vec2_t p = { x, y };
	vec2_t a = vec2_from_vec4(point_a);
	vec2_t b = vec2_from_vec4(point_b);
	vec2_t c = vec2_from_vec4(point_c);

	vec3_t weights = barycentric_weights(a, b, c, p);

	// Barycentric weights
	float alpha = weights.x;
	float beta = weights.y;
	float gamma = weights.z;

	// Interpolate the value of 1/w for the current pixel
	float interpolated_reciprocal_w = (1 / point_a.w) * alpha + (1 / point_b.w) * beta + (1 / point_c.w) * gamma;

	// Adjust 1/w so the pixels that are closer to the camera have smaller values
	interpolated_reciprocal_w = 1.0 - interpolated_reciprocal_w;

	// Only draw this pixel if its depth value is less than the depth value of the pixel previously stored in the z-buffer
	if (interpolated_reciprocal_w < get_zbuffer_at(x, y)) {
		// Draw a pixel at position (x, y) with the color that comes from the mapped texture
		draw_pixel(x, y, color);

		// Update the z-buffer value with the 1/w of the current pixel
		update_zbuffer_at(x, y, interpolated_reciprocal_w);
	}
}

///////////////////////////////////////////////////////////////////////////////
// Draw a textured pixel at position x and y using interpolation
///////////////////////////////////////////////////////////////////////////////
void draw_triangle_texel(
	int x, int y, upng_t* texture,
	vec4_t point_a, vec4_t point_b, vec4_t point_c,
	tex2_t a_uv, tex2_t b_uv, tex2_t c_uv
) {
	vec2_t p = { x, y };
	vec2_t a = vec2_from_vec4(point_a);
	vec2_t b = vec2_from_vec4(point_b);
	vec2_t c = vec2_from_vec4(point_c);

	vec3_t weights = barycentric_weights(a, b, c, p);

	// Barycentric weights
	float alpha = weights.x;
	float beta = weights.y;
	float gamma = weights.z;

	// Variables that store the interpolated values of u/w, v/w, and 1/w per pixel
	float interpolated_u;
	float interpolated_v;
	float interpolated_reciprocal_w;

	// Perform interpolation of all u and v values using barycentric weights and a factor of 1/w
	interpolated_u = (a_uv.u / point_a.w) * alpha + (b_uv.u / point_b.w) * beta + (c_uv.u / point_c.w) * gamma;
	interpolated_v = (a_uv.v / point_a.w) * alpha + (b_uv.v / point_b.w) * beta + (c_uv.v / point_c.w) * gamma;

	// Interpolate the value of 1/w for the current pixel
	interpolated_reciprocal_w = (1 / point_a.w) * alpha + (1 / point_b.w) * beta + (1 / point_c.w) * gamma;

	// Divide all attributes by 1/w
	interpolated_u /= interpolated_reciprocal_w;
	interpolated_v /= interpolated_reciprocal_w;

	// Get the mesh texture width and height
	int texture_width = upng_get_width(texture);
	int texture_height = upng_get_height(texture);

	// Map the uv coordinate to the full texture width and height
	int tex_x = abs((int)(interpolated_u * texture_width)) % texture_width;
	int tex_y = abs((int)(interpolated_v * texture_height)) % texture_height;

	// Adjust 1/w so the pixels that are closer to the camera have smaller values
	interpolated_reciprocal_w = 1.0 - interpolated_reciprocal_w;

	// Only draw this pixel if its depth value is less than the depth value of the pixel previously stored in the z-buffer
	if (interpolated_reciprocal_w < get_zbuffer_at(x, y)) {
		// Get the buffer of colors from the texture
		uint32_t* texture_buffer = (uint32_t*)upng_get_buffer(texture);

		// Draw a pixel at position (x, y) with the color that comes from the mapped texture
		draw_pixel(x, y, texture_buffer[(texture_width * tex_y) + tex_x]);

		// Update the z-buffer value with the 1/w of the current pixel
		update_zbuffer_at(x, y, interpolated_reciprocal_w);
	}
}

///////////////////////////////////////////////////////////////////////////////
// Draw a filled triangle with the flat-top/flat-bottom method
// The original triangle is split in two, half flat-bottom and half flat-top
///////////////////////////////////////////////////////////////////////////////
//
//          (x0,y0)
//            / \
//           /   \
//          /     \
//         /       \
//        /         \
//   (x1,y1)-------(Mx,My)
//       \_           \
//          \_         \
//             \_       \
//                \_     \
//                   \    \
//                     \_  \
//                        \_\
//                           \
//                         (x2,y2)
//
///////////////////////////////////////////////////////////////////////////////
void draw_filled_triangle(
	int x0, int y0, float z0, float w0,
	int x1, int y1, float z1, float w1,
	int x2, int y2, float z2, float w2,
	uint32_t color
) {
	// Sort vertices by ascending y-coordinate (y0 < y1 < y2)
	if (y0 > y1) {
		int_swap(&y0, &y1);
		int_swap(&x0, &x1);
		float_swap(&z0, &z1);
		float_swap(&w0, &w1);
	}
	if (y1 > y2) {
		int_swap(&y1, &y2);
		int_swap(&x1, &x2);
		float_swap(&z1, &z2);
		float_swap(&w1, &w2);
	}
	if (y0 > y1) {
		int_swap(&y0, &y1);
		int_swap(&x0, &x1);
		float_swap(&z0, &z1);
		float_swap(&w0, &w1);
	}

	// Create vector points and texture coordinates after the vertices are sorted
	vec4_t point_a = { x0, y0, z0, w0 };
	vec4_t point_b = { x1, y1, z1, w1 };
	vec4_t point_c = { x2, y2, z2, w2 };

	///////////////////////////////////////////////////////////////////////////////
	// Render the upper triangle (flat-bottom)
	///////////////////////////////////////////////////////////////////////////////
	float inv_slope_1 = 0;
	float inv_slope_2 = 0;

	if (y1 - y0 != 0) inv_slope_1 = (float)(x1 - x0) / abs(y1 - y0);
	if (y2 - y0 != 0) inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);

	if (y1 - y0 != 0) {
		// Loop scanlines from top to bottom
		for (int y = y0; y <= y1; y++) {
			int x_start = x1 + (y - y1) * inv_slope_1;
			int x_end = x0 + (y - y0) * inv_slope_2;

			// Swap x_start and x_end if x_end is to the left of x_start (x_end, x_start --> x_start, x_end)
			if (x_end < x_start) {
				int_swap(&x_start, &x_end);
			}

			// Draw each pixel with a solid color
			for (int x = x_start; x < x_end; x++) {
				draw_triangle_pixel(x, y, color, point_a, point_b, point_c);
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Render the lower triangle (flat-top)
	///////////////////////////////////////////////////////////////////////////////
	inv_slope_1 = 0;
	inv_slope_2 = 0;

	if (y2 - y1 != 0) inv_slope_1 = (float)(x2 - x1) / abs(y2 - y1);
	if (y2 - y0 != 0) inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);

	if (y2 - y1 != 0) {
		// Loop scanlines from top to bottom
		for (int y = y1; y <= y2; y++) {
			int x_start = x1 + (y - y1) * inv_slope_1;
			int x_end = x0 + (y - y0) * inv_slope_2;

			// Swap x_start and x_end if x_end is to the left of x_start (x_end, x_start --> x_start, x_end)
			if (x_end < x_start) {
				int_swap(&x_start, &x_end);
			}

			// Draw each pixel with a solid color
			for (int x = x_start; x < x_end; x++) {
				draw_triangle_pixel(x, y, color, point_a, point_b, point_c);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// Draw a textured triangle with the flat-top/flat-bottom method
// The original triangle is split in two, half flat-bottom and half flat-top
///////////////////////////////////////////////////////////////////////////////
//
//            v0
//            / \
//           /   \
//          /     \
//         /       \
//        /         \
//      v1           \
//       \_           \
//          \_         \
//             \_       \
//                \_     \
//                   \    \
//                     \_  \
//                        \_\
//                           \
//                           v2
//
///////////////////////////////////////////////////////////////////////////////
void draw_textured_triangle(
	int x0, int y0, float z0, float w0, float u0, float v0,
	int x1, int y1, float z1, float w1, float u1, float v1,
	int x2, int y2, float z2, float w2, float u2, float v2,
	upng_t* texture
) {
	// Sort vertices by ascending y-coordinate (y0 < y1 < y2)
	if (y0 > y1) {
		int_swap(&y0, &y1);
		int_swap(&x0, &x1);
		float_swap(&z0, &z1);
		float_swap(&w0, &w1);
		float_swap(&u0, &u1);
		float_swap(&v0, &v1);
	}
	if (y1 > y2) {
		int_swap(&y1, &y2);
		int_swap(&x1, &x2);
		float_swap(&z1, &z2);
		float_swap(&w1, &w2);
		float_swap(&u1, &u2);
		float_swap(&v1, &v2);
	}
	if (y0 > y1) {
		int_swap(&y0, &y1);
		int_swap(&x0, &x1);
		float_swap(&z0, &z1);
		float_swap(&w0, &w1);
		float_swap(&u0, &u1);
		float_swap(&v0, &v1);
	}

	// Flip the v component to account for inverted uv-coordinates (top-left start instead of bottom-left)
	v0 = 1.0 - v0;
	v1 = 1.0 - v1;
	v2 = 1.0 - v2;

	// Create vector points and texture coordinates after the vertices are sorted
	vec4_t point_a = { x0, y0, z0, w0 };
	vec4_t point_b = { x1, y1, z1, w1 };
	vec4_t point_c = { x2, y2, z2, w2 };
	tex2_t a_uv = { u0, v0 };
	tex2_t b_uv = { u1, v1 };
	tex2_t c_uv = { u2, v2 };

	///////////////////////////////////////////////////////////////////////////////
	// Render the upper triangle (flat-bottom)
	///////////////////////////////////////////////////////////////////////////////
	float inv_slope_1 = 0;
	float inv_slope_2 = 0;

	if (y1 - y0 != 0) inv_slope_1 = (float)(x1 - x0) / abs(y1 - y0);
	if (y2 - y0 != 0) inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);

	if (y1 - y0 != 0) {
		// Loop scanlines from top to bottom
		for (int y = y0; y <= y1; y++) {
			int x_start = x1 + (y - y1) * inv_slope_1;
			int x_end = x0 + (y - y0) * inv_slope_2;

			// Swap x_start and x_end if x_end is to the left of x_start (x_end, x_start --> x_start, x_end)
			if (x_end < x_start) {
				int_swap(&x_start, &x_end);
			}

			// Draw each pixel with the color code derived from the texture
			for (int x = x_start; x < x_end; x++) {
				draw_triangle_texel(x, y, texture, point_a, point_b, point_c, a_uv, b_uv, c_uv);
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Render the lower triangle (flat-top)
	///////////////////////////////////////////////////////////////////////////////
	inv_slope_1 = 0;
	inv_slope_2 = 0;

	if (y2 - y1 != 0) inv_slope_1 = (float)(x2 - x1) / abs(y2 - y1);
	if (y2 - y0 != 0) inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);

	if (y2 - y1 != 0) {
		// Loop scanlines from top to bottom
		for (int y = y1; y <= y2; y++) {
			int x_start = x1 + (y - y1) * inv_slope_1;
			int x_end = x0 + (y - y0) * inv_slope_2;

			// Swap x_start and x_end if x_end is to the left of x_start (x_end, x_start --> x_start, x_end)
			if (x_end < x_start) {
				int_swap(&x_start, &x_end);
			}

			// Draw each pixel with the color code derived from the texture
			for (int x = x_start; x < x_end; x++) {
				draw_triangle_texel(x, y, texture, point_a, point_b, point_c, a_uv, b_uv, c_uv);
			}
		}
	}
}