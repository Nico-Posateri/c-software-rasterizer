#include "display.h"

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static uint32_t* color_buffer = NULL;
static float* z_buffer = NULL;
static SDL_Texture* color_buffer_texture = NULL;
static int grid_spacing = 10;	// Current space between grid lines is 10 pixels
static int window_width = 1920;	// Sets window render width (16:9)
static int window_height = 1080;	// Sets window render height (16:9)
static int render_method = 0;
static int cull_method = 0;

int get_window_width(void) {
	return window_width;
}

int get_window_height(void) {
	return window_height;
}

bool initialize_window(void) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "Error initializing SDL.\n");
		return false;
	}

	// SDL query for the monitor's maximum full-screen resolution
	SDL_DisplayMode display_mode;
	SDL_GetCurrentDisplayMode(0, &display_mode);

	int fullscreen_width = display_mode.w;
	int fullscreen_height = display_mode.h;

	// Use below function to divide current monitor resolution by a value
	/*
	window_width = fullscreen_width / 3;
	window_height = fullscreen_height / 3;
	*/

	// Create SDL window of no title, centered, full-screen resolution, and borderless
	window = SDL_CreateWindow(
		NULL,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		fullscreen_width,
		fullscreen_height,
		SDL_WINDOW_BORDERLESS
	);
	if (!window) {
		fprintf(stderr, "Error creating SDL window.\n");
		return false;
	}
	// Create SDL renderer
	renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer) {
		fprintf(stderr, "Error creating SDL renderer.\n");
		return false;
	}
	SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

	// Allocate required memory in bytes to hold the color buffer and z-buffer
	color_buffer = (uint32_t*)malloc(sizeof(uint32_t) * window_width * window_height);
	z_buffer = (float*)malloc(sizeof(float) * window_width * window_height);

	// Create an SDL texture to display the color buffer
	color_buffer_texture = SDL_CreateTexture(
		renderer,
		// Previously SDL_PIXELFORMAT_ARGB8888
		SDL_PIXELFORMAT_RGBA32,
		SDL_TEXTUREACCESS_STREAMING,
		window_width,
		window_height
	);

	return true;
}

void set_render_method(int method) {
	render_method = method;
}

void set_cull_method(int method) {
	cull_method = method;
}

bool is_cull_backface(void) {
	return cull_method == CULL_BACKFACE;
}

bool should_render_filled_triangles(void) {
	return (
		render_method == RENDER_FILL_TRIANGLE || 
		render_method == RENDER_FILL_TRIANGLE_WIRE || 
		render_method == RENDER_FILL_TRIANGLE_WIRE_VERTEX
	);
}

bool should_render_textured_triangles(void) {
	return (
		render_method == RENDER_TEXTURED ||
		render_method == RENDER_TEXTURED_WIRE ||
		render_method == RENDER_TEXTURED_WIRE_VERTEX
	);
}

bool should_render_wireframe(void) {
	return (
		render_method == RENDER_WIRE ||
		render_method == RENDER_WIRE_VERTEX ||
		render_method == RENDER_FILL_TRIANGLE_WIRE ||
		render_method == RENDER_FILL_TRIANGLE_WIRE_VERTEX ||
		render_method == RENDER_TEXTURED_WIRE ||
		render_method == RENDER_TEXTURED_WIRE_VERTEX
	);
}

bool should_render_vertices(void) {
	return (
		render_method == RENDER_WIRE_VERTEX ||
		render_method == RENDER_FILL_TRIANGLE_WIRE_VERTEX ||
		render_method == RENDER_TEXTURED_WIRE_VERTEX
	);
}

// This version draws a dot-matrix on-screen
void draw_grid(void) {
	for (int y = 0; y < window_height; y += grid_spacing) {
		for (int x = 0; x < window_width; x += grid_spacing) {
			color_buffer[(window_width * y) + x] = 0xFF8e918f; // Sets matrix color to grey
		}
	}
}

// This version draws a grid on-screen
/*
void draw_grid(void) {
	for (int y = 0; y < window_height; y++) {
		for (int x = 0; x < window_width; x++) {
			if (x % grid_spacing == 0 || y % grid_spacing == 0) {
				color_buffer[(window_width * y) + x] = 0xFF00FF00; // Sets grid color to green
			}
		}
	}
}
*/

// The function for drawing a pixel on-screen
void draw_pixel(int x, int y, uint32_t color) {
	if (x < 0 || x >= window_width || y < 0 || y >= window_height) {
		return;
	}
	color_buffer[(window_width * y) + x] = color;
}

void draw_line(int x0, int y0, int x1, int y1, uint32_t color) {
	int delta_x = (x1 - x0); // Calculate delta-x
	int delta_y = (y1 - y0); // Calculate delta-y
	// Calculate which side is the longest
	int longest_side_length = (abs(delta_x) >= abs(delta_y)) ? abs(delta_x) : abs(delta_y);
	float x_inc = delta_x / (float)longest_side_length; // Calculate x-increment
	float y_inc = delta_y / (float)longest_side_length; // Calculate y-increment
	float current_x = x0;
	float current_y = y0;
	// Loop to draw edge lines, pixel by pixel
	for (int i = 0; i <= longest_side_length; i++) {
		draw_pixel(round(current_x), round(current_y), color);
		current_x += x_inc;
		current_y += y_inc;
	}
}

// The function for drawing a rectangle on-screen
void draw_rect(int x, int y, int width, int height, uint32_t color) {
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			int current_x = x + i;
			int current_y = y + j;
			draw_pixel(current_x, current_y, color);
		}
	}
}

void render_color_buffer(void) {
	SDL_UpdateTexture(
		color_buffer_texture,
		NULL,
		color_buffer,
		(int)(window_width * sizeof(uint32_t))
	);
	SDL_RenderCopy(renderer, color_buffer_texture, NULL, NULL);	// Scales set window width and height to fit monitor
	SDL_RenderPresent(renderer);
}

void clear_color_buffer(uint32_t color) {
	for (int i = 0; i < window_width * window_height; i++) {
		color_buffer[i] = color;
	}
}

void clear_z_buffer(void) {
	for (int i = 0; i < window_width * window_height; i++) {
		z_buffer[i] = 1.0;
	}
}

float get_zbuffer_at(int x, int y) {
	if (x < 0 || x >= window_width || y < 0 || y >= window_height) {
		return 1.0;
	}
	return z_buffer[(window_width * y) + x];
}

void update_zbuffer_at(int x, int y, float value) {
	if (x < 0 || x >= window_width || y < 0 || y >= window_height) {
		return;
	}
	z_buffer[(window_width * y) + x] = value;
}

void destroy_window(void) {
	free(color_buffer);
	free(z_buffer);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}