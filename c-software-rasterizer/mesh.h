#ifndef MESH_H
#define MESH_H

#include "vector.h"
#include "triangle.h"
#include "upng.h"

// Defines a struct for dynamically sized meshes with an array of vertices and faces
typedef struct {
	vec3_t* vertices;	// Mesh's dynamic array of vertices
	face_t* faces;		// Mesh's dynamic array of faces
	upng_t* texture;	// Mesh's PNG texture pointer
	vec3_t rotation;	// Mesh rotation with x, y, and z values
	vec3_t scale;		// Mesh scale with x, y, and z values
	vec3_t translation; // Mesh translation with x, y, and z values

} mesh_t;

void load_mesh(char* obj_filename, char* png_filename, vec3_t scale, vec3_t translation, vec3_t rotation);
void load_mesh_obj_data(mesh_t* mesh, char* obj_filename);
void load_mesh_png_data(mesh_t* mesh, char* png_filename);
int get_num_meshes(void);
mesh_t* get_mesh(int index);
void free_meshes(void);

#endif