#include <SDL3/SDL.h>
#include <linalg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define RENDER_SCALE 8

typedef struct {
	float r;
	float g;
	float b;
	float a;
} color;

#define color(r, g, b) (color){ r, g, b, 1.0f }

static SDL_Window* window;
static SDL_Renderer* renderer;
static SDL_Texture* texture;
static color* fbuffer;
static int width = 1280;
static int height = 720;
static double dist_scale = 0.1;

void render(double start_time, double current_time)
{
	float mouse_x, mouse_y;
	SDL_GetMouseState(&mouse_x, &mouse_y);
	vec2 mouse = vec2_div(vec2(mouse_x, mouse_y), vec2(width * RENDER_SCALE, height * RENDER_SCALE));
	mouse = vec2_add_d(mouse, -0.5);
	mouse = vec2_scale(mouse, 2.0);
	mouse.x *= (float)width / height;

	double runtime = current_time - start_time;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			vec2 uv_orig = vec2_div(vec2(x, y), vec2(width, height));
			vec2 uv = vec2_add_d(uv_orig, -0.5);
			uv = vec2_scale(uv, 2.0);
			uv.x *= (float)width / height;

			double d = vec2_length(vec2_sub(mouse, uv));
			double dist = dist_scale / (d * d);
			uv = vec2_lerp(uv, mouse, dist);

			uv = vec2_scale(uv, 2.0);
			uv.x -= floor(uv.x);
			uv.y -= floor(uv.y);
			uv = vec2_add_d(uv, -0.5);
			uv = vec2_scale(uv, 2.0);
			double val = 0.5 - vec2_length(uv);
			val = sin(8.0 * val + runtime * 2.0);
			val = fabs(val);
			val = 0.01 / val;

			vec3 col = vec3_lerp(vec3(0.001, 0.0, 0.01), vec3(0.1, 0.0, 1.0), clamp(0.0, 1.0, val));
			fbuffer[y * width + x] = color(col.x, col.y, col.z);
		}
	}

	SDL_RenderClear(renderer);
	SDL_UpdateTexture(texture, NULL, fbuffer, sizeof(color) * width);
	SDL_RenderTexture(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
}

int main(void)
{
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		fprintf(stderr, "Error: SDL failed to init\n");
		return 1;
	}

	if (!SDL_CreateWindowAndRenderer("shade", width, height, SDL_WINDOW_RESIZABLE, &window, &renderer))
	{
		fprintf(stderr, "Error: Failed to create window or renderer\n");
		return 1;
	}
	width /= RENDER_SCALE;
	height /= RENDER_SCALE;
	SDL_SetRenderVSync(renderer, 1);
	if ((texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA128_FLOAT, SDL_TEXTUREACCESS_STREAMING, width, height)) == NULL) {
		fprintf(stderr, "Error: Failed to create texture\n");
		return 1;
	}
	SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

	fbuffer = malloc(sizeof(color) * width * height);

	struct timespec start_time;
	clock_gettime(CLOCK_MONOTONIC, &start_time);
	double start_time_d = (double)start_time.tv_sec + (double)start_time.tv_nsec / 1000000000.0;
	int quit = 0;
	while (!quit)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_EVENT_QUIT:
					quit = 1;
					break;
				case SDL_EVENT_WINDOW_RESIZED:
					SDL_GetWindowSize(window, &width, &height);
					width /= RENDER_SCALE;
					height /= RENDER_SCALE;
					free(fbuffer);
					fbuffer = malloc(sizeof(color) * width * height);
					SDL_DestroyTexture(texture);
					if ((texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA128_FLOAT, SDL_TEXTUREACCESS_STREAMING, width, height)) == NULL) {
						fprintf(stderr, "Error: Failed to create texture\n");
						return 1;
					}
					SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
					break;
				case SDL_EVENT_MOUSE_WHEEL:
					dist_scale += 0.05 * event.wheel.y;
					if (dist_scale < 0.0) dist_scale = 0.0;
					break;
				default:
					break;
			}
		}

		struct timespec current_time;
		clock_gettime(CLOCK_MONOTONIC, &current_time);
		double current_time_d = (double)current_time.tv_sec + (double)current_time.tv_nsec / 1000000000.0;
		render(start_time_d, current_time_d);
	}

	free(fbuffer);
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
