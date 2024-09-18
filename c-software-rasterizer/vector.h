#ifndef VECTOR_H
#define VECTOR_H

typedef struct {
	float x, y;
} vec2_t;

typedef struct {
	float x, y, z;
} vec3_t;

typedef struct {
	float x, y, z, w;
} vec4_t;

// Vector 2D functions /////////////////////
// New Vector
vec2_t vec2_new(float x, float y);
// Length
float vec2_length(vec2_t v);
// Addition
vec2_t vec2_add(vec2_t a, vec2_t b);
// Subtraction
vec2_t vec2_sub(vec2_t a, vec2_t b);
// Multiplication
vec2_t vec2_mul(vec2_t v, float factor);
// Division
vec2_t vec2_div(vec2_t v, float factor);
// Normalization
void vec2_normalize(vec2_t* v);
////////////////////////////////////////////

// Vector 3D functions /////////////////////
// New Vector
vec3_t vec3_new(float x, float y, float z);
// Clone
vec3_t vec3_clone(vec3_t* v);
// Length
float vec3_length(vec3_t v);
// Addition
vec3_t vec3_add(vec3_t a, vec3_t b);
// Subtraction
vec3_t vec3_sub(vec3_t a, vec3_t b);
// Multiplication
vec3_t vec3_mul(vec3_t v, float factor);
// Division
vec3_t vec3_div(vec3_t v, float factor);
// Cross Product for Normal Vector
vec3_t vec3_cross(vec3_t a, vec3_t b);
// Dot Product
float vec3_dot(vec3_t a, vec3_t b);
// Normalization
void vec3_normalize(vec3_t* v);
////////////////////////////////////////////

// Rotation functions
vec3_t vec3_rotate_x(vec3_t v, float angle);
vec3_t vec3_rotate_y(vec3_t v, float angle);
vec3_t vec3_rotate_z(vec3_t v, float angle);

// Vector conversion functions /////////////
vec4_t vec4_from_vec3(vec3_t v);
vec3_t vec3_from_vec4(vec4_t v);
vec2_t vec2_from_vec4(vec4_t v);
////////////////////////////////////////////

#endif