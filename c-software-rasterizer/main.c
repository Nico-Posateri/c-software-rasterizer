#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL.h>
#include "upng.h"
#include "array.h"
#include "display.h"
#include "clipping.h"
#include "vector.h"
#include "matrix.h"
#include "light.h"
#include "camera.h"
#include "triangle.h"
#include "texture.h"
#include "mesh.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Global variables for execution status and game loop
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool is_running = false;
int previous_frame_time = 0;
float delta_time = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Array to store triangles that should be rendered each frame
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define MAX_TRIANGLES 10000
triangle_t triangles_to_render[MAX_TRIANGLES];
int num_triangles_to_render = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Declaration of the global transformation matrices
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
mat4_t world_matrix;
mat4_t proj_matrix;
mat4_t view_matrix;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Set-up function to initialize variables and game objects
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup(void) {
	// Initialize render mode and tri culling method
	set_render_method(RENDER_WIRE);
	set_cull_method(CULL_BACKFACE);

	// Initialize the scene light direction
	init_light(vec3_new(0, 0, 1));

	// Initialize the perspective projection matrix
	float aspect_x = (float)get_window_width() / (float)get_window_height();
	float aspect_y = (float)get_window_height() / (float)get_window_width();
	float fov_y = 3.141592 / 3.0; // Equal to 180/3, or M_PI/3, or 60 degrees
	float fov_x = atan(tan(fov_y / 2) * aspect_x) * 2.0;
	float z_near = 0.1;
	float z_far = 100.0;
	proj_matrix = mat4_make_perspective(fov_y, aspect_y, z_near, z_far);

	// Initialize frustum planes with a point and a normal
	init_frustum_planes(fov_x, fov_y, z_near, z_far);

	/*
	// Loads an .obj, .png, scale, translation, and rotation values into the mesh data structure
	load_mesh("./assets/f22.obj", "./assets/f22.png", vec3_new(1, 1, 1), vec3_new(0, -1.3, +5), vec3_new(0, -M_PI / 2, 0));
	load_mesh("./assets/efa.obj", "./assets/efa.png", vec3_new(1, 1, 1), vec3_new(-2, -1.3, +9), vec3_new(0, -M_PI / 2, 0));
	load_mesh("./assets/f117.obj", "./assets/f117.png", vec3_new(1, 1, 1), vec3_new(+2, -1.3, +9), vec3_new(0, -M_PI / 2, 0));
	load_mesh("./assets/runway.obj", "./assets/runway.png", vec3_new(1, 1, 1), vec3_new(0, -1.5, +23), vec3_new(0, 0, 0));
	*/

	// Needs to come towards camera for a gif
	load_mesh("./assets/f22.obj", "./assets/f22.png", vec3_new(1, 1, 1), vec3_new(0, -1.3, +5), vec3_new(0, -M_PI / 2, 0));
	load_mesh("./assets/efa.obj", "./assets/efa.png", vec3_new(1, 1, 1), vec3_new(-2, -1.3, +9), vec3_new(0, -M_PI / 2, 0));
	load_mesh("./assets/f117.obj", "./assets/f117.png", vec3_new(1, 1, 1), vec3_new(+2, -1.3, +9), vec3_new(0, -M_PI / 2, 0));
	load_mesh("./assets/runway.obj", "./assets/runway.png", vec3_new(1, 1, 1), vec3_new(0, -1.5, +23), vec3_new(0, 0, 0));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function to process keyboard input
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void process_input(void) {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				is_running = false;
				break;
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) {					// "esc": Closes program
					is_running = false;
					break;
				}
				if (event.key.keysym.sym == SDLK_w) {						// "w": Moves the camera forward
					update_camera_forward_velocity(vec3_mul(get_camera_direction(), 3.0 * delta_time));
					update_camera_position(vec3_add(get_camera_position(), get_camera_forward_velocity()));
					break;
				}
				if (event.key.keysym.sym == SDLK_a) {						// "a": Rotates the camera left
					rotate_camera_yaw(-1.0 * delta_time);
					break;
				}
				if (event.key.keysym.sym == SDLK_s) {						// "s": Moves the camera backward
					update_camera_forward_velocity(vec3_mul(get_camera_direction(), 3.0 * delta_time));
					update_camera_position(vec3_sub(get_camera_position(), get_camera_forward_velocity()));
					break;
				}
				if (event.key.keysym.sym == SDLK_d) {						// "d": Rotates the camera right
					rotate_camera_yaw(+1.0 * delta_time);
					break;
				}
				if (event.key.keysym.sym == SDLK_q) {						// "q": Tilts the camera forward
					rotate_camera_pitch(+1.0 * delta_time);
					break;
				}
				if (event.key.keysym.sym == SDLK_z) {						// "z": Tilts the camera back
					rotate_camera_pitch(-1.0 * delta_time);
					break;
				}
				if (event.key.keysym.sym == SDLK_c) {						// "c": Renders the mesh with culled backfaces
					set_cull_method(CULL_BACKFACE);
					break;
				}
				if (event.key.keysym.sym == SDLK_r) {						// "r": Renders the mesh without culled backfaces
					set_cull_method(CULL_NONE);
					break;
				}
				if (event.key.keysym.sym == SDLK_1) {						// "1": Renders the mesh wireframe with vertices
					set_render_method(RENDER_WIRE_VERTEX);
					break;
				}
				if (event.key.keysym.sym == SDLK_2) {						// "2": Renders the mesh wireframe
					set_render_method(RENDER_WIRE);
					break;
				}
				if (event.key.keysym.sym == SDLK_3) {						// "3": Renders the mesh with filled faces
					set_render_method(RENDER_FILL_TRIANGLE);
					break;
				}
				if (event.key.keysym.sym == SDLK_4) {						// "4": Renders the mesh wireframe with filled faces
					set_render_method(RENDER_FILL_TRIANGLE_WIRE);
					break;
				}
				if (event.key.keysym.sym == SDLK_5) {						// "5": Renders the mesh wireframe with filled faces and vertices
					set_render_method(RENDER_FILL_TRIANGLE_WIRE_VERTEX);
					break;
				}
				if (event.key.keysym.sym == SDLK_6) {						// "6": Renders the mesh textured
					set_render_method(RENDER_TEXTURED);
					break;
				}
				if (event.key.keysym.sym == SDLK_7) {						// "7": Renders the mesh textured with a wireframe
					set_render_method(RENDER_TEXTURED_WIRE);
					break;
				}
				if (event.key.keysym.sym == SDLK_8) {						// "8": Renders the mesh textured with a wireframe and vertices
					set_render_method(RENDER_TEXTURED_WIRE_VERTEX);
					break;
				}
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Process the graphics pipeline stages for all the mesh triangles
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// +-------------+
// | Model space |  <-- original mesh vertices
// +-------------+
// |   +-------------+
// `-> | World space |  <-- multiply by world matrix
//     +-------------+
//     |   +--------------+
//     `-> | Camera space |  <-- multiply by view matrix
//         +--------------+
//         |    +------------+
//         `--> |  Clipping  |  <-- clip against the six frustum planes
//              +------------+
//              |    +------------+
//              `--> | Projection |  <-- multiply by projection matrix
//                   +------------+
//                   |    +-------------+
//                   `--> | Image space |  <-- apply perspective divide
//                        +-------------+
//                        |    +--------------+
//                        `--> | Screen space |  <-- ready to render
//                             +--------------+
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void process_graphics_pipeline_stages(mesh_t* mesh) {
	// Create scale, rotation, and translation matrices that will be used to multiply the mesh vertices
	mat4_t scale_matrix = mat4_make_scale(mesh->scale.x, mesh->scale.y, mesh->scale.z);
	mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh->rotation.x);
	mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh->rotation.y);
	mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh->rotation.z);
	mat4_t translation_matrix = mat4_make_translation(mesh->translation.x, mesh->translation.y, mesh->translation.z);

	// Update camera look-at target and initialize the view matrix
	vec3_t target = get_camera_lookat_target();
	vec3_t up_direction = vec3_new(0, 1, 0);
	view_matrix = mat4_look_at(get_camera_position(), target, up_direction);

	// Loop through triangle faces of the mesh
	int num_faces = array_length(mesh->faces);
	for (int i = 0; i < num_faces; i++) {

		face_t mesh_face = mesh->faces[i];

		vec3_t face_vertices[3];
		face_vertices[0] = mesh->vertices[mesh_face.a];
		face_vertices[1] = mesh->vertices[mesh_face.b];
		face_vertices[2] = mesh->vertices[mesh_face.c];

		vec4_t transformed_vertices[3];

		// Loop all 3 vertices of the current face and apply transformations
		for (int j = 0; j < 3; j++) {
			vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);

			// Create a World Matrix combining scale, rotation, and translation matrices
			world_matrix = mat4_identity();

			// Order matters!!! Transformations to the mesh are made with respect to the origin!!!
			// Rotating then translating is NOT the same as translating then rotating!!!
			world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
			world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
			world_matrix = mat4_mul_mat4(rotation_matrix_y, world_matrix);
			world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
			world_matrix = mat4_mul_mat4(translation_matrix, world_matrix);

			// Multiply the World Matrix by the original vector
			transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);

			// Multiply the view matrix by the transformed vector to transform the scene to camera space
			transformed_vertex = mat4_mul_vec4(view_matrix, transformed_vertex);

			// Save transformed vertex in the array of transformed vertices
			transformed_vertices[j] = transformed_vertex;
		}

		// Calculate the triangle facing normal
		vec3_t face_normal = get_triangle_normal(transformed_vertices);

		// Bypass triangles that are looking away from the camera (backfaces)
		if (is_cull_backface()) {

			// Find the vector between a point in the triangle and the camera origin
			vec3_t camera_ray = vec3_sub(vec3_new(0, 0, 0), vec3_from_vec4(transformed_vertices[0]));

			// Calculate dot product of normal vector and camera ray, if it's less than zero, those faces are culled
			float dot_normal_camera = vec3_dot(face_normal, camera_ray);
			if (dot_normal_camera < 0) {
				continue;
			}
		}

		// Clipping implementation ///////////////////////////////////////////////////////////////////////////////

		// Create a polygon from the original transformed triangle to be clipped
		polygon_t polygon = create_polygon_from_triangle(
			vec3_from_vec4(transformed_vertices[0]),
			vec3_from_vec4(transformed_vertices[1]),
			vec3_from_vec4(transformed_vertices[2]),
			mesh_face.a_uv,
			mesh_face.b_uv,
			mesh_face.c_uv
		);

		// Clips the polygon and returns a new polygon
		clip_polygon(&polygon);

		// Break the polygon into triangles after clipping
		triangle_t triangles_after_clipping[MAX_NUM_POLY_TRIANGLES];
		int num_triangles_after_clipping = 0;

		triangles_from_polygon(&polygon, triangles_after_clipping, &num_triangles_after_clipping);

		//////////////////////////////////////////////////////////////////////////////////////////////////////////

		// Loops through the assembled triangles after clipping
		for (int t = 0; t < num_triangles_after_clipping; t++) {
			triangle_t triangle_after_clipping = triangles_after_clipping[t];

			vec4_t projected_points[3];

			// Loop all 3 vertices and project transformed faces onto the display
			for (int j = 0; j < 3; j++) {
				// Project the current vertex
				projected_points[j] = mat4_mul_vec4_project(proj_matrix, triangle_after_clipping.points[j]);

				// Scale projected points into view
				projected_points[j].x *= (get_window_width() / 2.0);
				projected_points[j].y *= (get_window_height() / 2.0);

				// Invert the y values to account for flipped screen y-coordinate
				projected_points[j].y *= -1;

				// Center projected points on screen via translation
				projected_points[j].x += (get_window_width() / 2.0);
				projected_points[j].y += (get_window_height() / 2.0);
			}

			// Calculate the light intensity based on face normal alignment with the inverse of the light ray
			float light_intensity_factor = -vec3_dot(face_normal, get_light_direction());

			// Calculate tri color based on light angle
			uint32_t triangle_color = light_apply_intensity(mesh_face.color, light_intensity_factor);

			triangle_t triangle_to_render = {
				.points = {
					{ projected_points[0].x, projected_points[0].y, projected_points[0].z, projected_points[0].w },
					{ projected_points[1].x, projected_points[1].y, projected_points[1].z, projected_points[1].w },
					{ projected_points[2].x, projected_points[2].y, projected_points[2].z, projected_points[2].w }
				},
				.texcoords = {
					{ triangle_after_clipping.texcoords[0].u, triangle_after_clipping.texcoords[0].v },
					{ triangle_after_clipping.texcoords[1].u, triangle_after_clipping.texcoords[1].v },
					{ triangle_after_clipping.texcoords[2].u, triangle_after_clipping.texcoords[2].v }
				},
				.color = triangle_color,
				.texture = mesh->texture
			};

			// Save the projected triangle in the array of triangles to render
			if (num_triangles_to_render < MAX_TRIANGLES) {
				triangles_to_render[num_triangles_to_render++] = triangle_to_render;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function that updates frame-by-frame with a fixed time step
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void update(void) {

	/* Replaced this while loop with the block below...
	// Locking the execution of update to a framerate determined by a preset FPS, rather than processor speed
	while (!SDL_TICKS_PASSED(SDL_GetTicks(), previous_frame_time + FRAME_TARGET_TIME));
	*/

	/* Replaced AGAIN with the segments below! We're running on variable delta time, baby!!!
	// Wait until the target frame time is reached in milliseconds
	int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);
	// Delay execution if program is running too fast--yay, constant delta-time! Next, variable delta-time...
	if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
		SDL_Delay(time_to_wait);
	}
	*/

	// Get a delta time factor converted to seconds to be used in updating the game objects
	delta_time = (SDL_GetTicks() - previous_frame_time) / 1000.0;

	previous_frame_time = SDL_GetTicks();

	// Initialize the counter of triangles to render for the current frame
	num_triangles_to_render = 0;

	// Loop all the meshes of the scene from the array of meshes
	for (int mesh_index = 0; mesh_index < get_num_meshes(); mesh_index++) {
		mesh_t* mesh = get_mesh(mesh_index);

		// Change the mesh scale, rotation, and translation values per second /////////////////////////////////////
		// For non-incremental manipulations, remove "* delta_time" from the chosen line //////////////////////////
		/*
		mesh.scale.x += 0.0 * delta_time;			// Increments mesh x-scale by 0.0 units each second
		mesh.scale.y += 0.0 * delta_time;			// Increments mesh y-scale by 0.0 units each second
		mesh.scale.z += 0.0 * delta_time;			// Increments mesh z-scale by 0.0 units each second

		mesh.rotation.x += 0.0 * delta_time;		// Increments mesh x-rotation by 0.0 units each second
		mesh.rotation.y += 0.0 * delta_time;		// Increments mesh y-rotation by 0.0 units each second
		mesh.rotation.z += 0.0 * delta_time;		// Increments mesh z-rotation by 0.0 units each second

		mesh.translation.x += 0.0 * delta_time;		// Increments mesh x-translation by 0.0 units each second
		mesh.translation.y += 0.0 * delta_time;		// Increments mesh y-translation by 0.0 units each second
		mesh.translation.z += 0.0 * delta_time;		// Increments mesh z-translation by 0.0 units each second
		*/
		///////////////////////////////////////////////////////////////////////////////////////////////////////////

		process_graphics_pipeline_stages(mesh);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function that renders objects on display
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void render(void) {
	// Clear all arrays in preparation for the next frame
	clear_color_buffer(0xFF000000);
	clear_z_buffer();
	draw_grid();

	// Loop all projected tris and render them
	for (int i = 0; i < num_triangles_to_render; i++) {
		triangle_t triangle = triangles_to_render[i];

		// Filled faces
		if (should_render_filled_triangles()) {
			// Draw filled tris
			draw_filled_triangle(
				triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w, // Vertex A
				triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w, // Vertex B
				triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w, // Vertex C
				triangle.color // White faces
			);
		}

		// Textured faces
		if (should_render_textured_triangles()) {
			// Draw textured tris
			draw_textured_triangle(
				triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w, triangle.texcoords[0].u, triangle.texcoords[0].v, // Vertex A
				triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w, triangle.texcoords[1].u, triangle.texcoords[1].v, // Vertex B
				triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w, triangle.texcoords[2].u, triangle.texcoords[2].v, // Vertex C
				triangle.texture // Textured faces
			);
		}

		// Wireframe
		if (should_render_wireframe()) {
			// Draw edges of tris (wireframe)
			draw_triangle(
				triangle.points[0].x, triangle.points[0].y, // Vertex A
				triangle.points[1].x, triangle.points[1].y, // Vertex B
				triangle.points[2].x, triangle.points[2].y, // Vertex C
				0xFFFFFFFF // White edges
			);
		}

		// Vertices
		if (should_render_vertices()) {
			// Draw verteces of tris
			draw_rect(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, 0xFF008FFF); // Vertex A
			draw_rect(triangle.points[1].x - 3, triangle.points[1].y - 3, 6, 6, 0xFF008FFF); // Vertex B
			draw_rect(triangle.points[2].x - 3, triangle.points[2].y - 3, 6, 6, 0xFF008FFF); // Vertex C
			// Orange vertices
		}
	}

	// Draw the color buffer to the SDL window
	render_color_buffer();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function to free the memory that was dynamically allocated by the program
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void free_resources(void) {
	free_meshes();
	destroy_window();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Main loop
////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(void) {
	is_running = initialize_window();

	setup();

	while (is_running) {
		process_input();
		update();
		render();
	}

	free_resources();

	return 0;
}