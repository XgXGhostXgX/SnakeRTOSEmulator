/**
 * @file TUM_Draw.c
 * @author Alex Hoffman
 * @date 27 August 2019
 * @brief A SDL2 based library to implement work queue based drawing of graphical
 * elements. Allows for drawing using SDL2 from multiple threads.
 *
 * @verbatim
   ----------------------------------------------------------------------
    Copyright (C) Alexander Hoffman, 2019
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
   ----------------------------------------------------------------------
@endverbatim
 */
#include <limits.h>
#include <stdlib.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_image.h>

#include <pthread.h>

#include "TUM_Draw.h"
#include "TUM_Font.h"
#include "TUM_Utils.h"

//Starting with (100,100) ^= [1][1]
int array[501][401];
#define ONE_BYTE 8
#define TWO_BYTES 16
#define THREE_BYTES 24
#define MAX_8_BIT 255
#define ALPHA_SOLID MAX_8_BIT
#define FIRST_BYTE 0x000000ff
#define SECOND_BYTE 0x0000ff00
#define THIRD_BYTE 0x00ff0000
#define FOURTH_BYTE 0xff000000
#define RED_PORTION(COLOUR) (COLOUR & 0xFF0000) >> TWO_BYTES
#define GREEN_PORTION(COLOUR) (COLOUR & 0x00FF00) >> ONE_BYTE
#define BLUE_PORTION(COLOUR) (COLOUR & 0x0000FF)
#define ZERO_ALPHA 0

typedef enum {
	DRAW_NONE = 0,
	DRAW_CLEAR,
	DRAW_ARC,
	DRAW_ELLIPSE,
	DRAW_TEXT,
	DRAW_RECT,
	DRAW_FILLED_RECT,
	DRAW_CIRCLE,
	DRAW_LINE,
	DRAW_POLY,
	DRAW_TRIANGLE,
	DRAW_IMAGE,
	DRAW_LOADED_IMAGE,
	DRAW_SCALED_IMAGE,
	DRAW_ARROW,
} draw_job_type_t;

typedef struct loaded_image {
	char *filename;
	FILE *file;
	SDL_Texture *tex;
	SDL_RWops *ops;
	SDL_Surface *surf;
	int w;
	int h;
	float scale;
	//TODO make this atomic
	unsigned int ref_count;
	unsigned char pending_free;

	struct loaded_image *next;
} loaded_image_t;

pthread_mutex_t loaded_images_lock = PTHREAD_MUTEX_INITIALIZER;
loaded_image_t loaded_images_list = { 0 };

typedef struct clear_data {
	unsigned int colour;
} clear_data_t;

typedef struct arc_data {
	signed short x;
	signed short y;
	signed short radius;
	signed short start;
	signed short end;
	unsigned int colour;
} arc_data_t;

typedef struct ellipse_data {
	signed short x;
	signed short y;
	signed short rx;
	signed short ry;
	unsigned int colour;
} ellipse_data_t;

typedef struct rect_data {
	signed short x;
	signed short y;
	signed short w;
	signed short h;
	unsigned int colour;
} rect_data_t;

typedef struct circle_data {
	signed short x;
	signed short y;
	signed short radius;
	unsigned int colour;
} circle_data_t;

typedef struct line_data {
	signed short x1;
	signed short y1;
	signed short x2;
	signed short y2;
	unsigned char thickness;
	unsigned int colour;
} line_data_t;

typedef struct poly_data {
	coord_t *points;
	unsigned int n;
	unsigned int colour;
} poly_data_t;

typedef struct triangle_data {
	coord_t *points;
	unsigned int colour;
} triangle_data_t;

typedef struct image_data {
	char *filename;
	SDL_Texture *tex;
	signed short x;
	signed short y;
} image_data_t;

typedef struct loaded_image_data {
	loaded_image_t *img;
	signed short x;
	signed short y;
} loaded_image_data_t;

typedef struct scaled_image_data {
	image_data_t image;
	float scale;
} scaled_image_data_t;

typedef struct text_data {
	char *str;
	signed short x;
	signed short y;
	unsigned int colour;
	TTF_Font *font;
} text_data_t;

typedef struct arrow_data {
	signed short x1;
	signed short y1;
	signed short x2;
	signed short y2;
	signed short head_length;
	unsigned char thickness;
	unsigned int colour;
} arrow_data_t;

union data_u {
	clear_data_t clear;
	arc_data_t arc;
	ellipse_data_t ellipse;
	rect_data_t rect;
	circle_data_t circle;
	line_data_t line;
	poly_data_t poly;
	triangle_data_t triangle;
	image_data_t image;
	loaded_image_data_t loaded_image;
	scaled_image_data_t scaled_image;
	text_data_t text;
	arrow_data_t arrow;
};

typedef struct draw_job {
	draw_job_type_t type;
	union data_u *data;

	struct draw_job *next;
} draw_job_t;

draw_job_t job_list_head = { 0 };

const int screen_height = SCREEN_HEIGHT;
const int screen_width = SCREEN_WIDTH;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_GLContext context = NULL;

char *error_message = NULL;

/**
 * Method: adding color in our array, only if (x1,y1) inside the Wall
 * Parameter: Coordinates (x1,y1) and the setting color
 */
void addColor(signed short x1, signed short y1, unsigned int color){
	if(x1>100 && x1<600 && y1>100 && y1<400){
		array[x1-100][y1-100]=color;
	}
}

/**
 * Method: Refreshing array, as equal as refresh screen Method
 */
void colorArray(){
	for(int i=0;i<501;i++){
		for(int j=0;j<401;j++){
			array[i][j]=0;
		}
	}
}

static uint32_t SwapBytes(unsigned int x)
{
	return ((x & FIRST_BYTE) << THREE_BYTES) +
	       ((x & SECOND_BYTE) << ONE_BYTE) +
	       ((x & THIRD_BYTE) >> ONE_BYTE) +
	       ((x & FOURTH_BYTE) >> THREE_BYTES);
}

void setErrorMessage(char *msg)
{
	if (error_message) {
		free(error_message);
	}

	error_message = calloc(strlen(msg) + 1, sizeof(char));
	if (error_message == NULL) {
		return;
	}
	strcpy(error_message, msg);
}

#define PRINT_SDL_ERROR(msg, ...)                                              \
	PRINT_ERROR("[SDL Error] %s\n" #msg, (char *)SDL_GetError(),           \
		    ##__VA_ARGS__)

static draw_job_t *pushDrawJob(void)
{
	draw_job_t *iterator;
	draw_job_t *job = calloc(1, sizeof(draw_job_t));
	if (job == NULL)
		return NULL;

	for (iterator = &job_list_head; iterator->next;
	     iterator = iterator->next)
		;

	iterator->next = job;

	return job;
}

static draw_job_t *popDrawJob(void)
{
	draw_job_t *ret = job_list_head.next;

	if (ret) {
		if (ret->next) {
			job_list_head.next = ret->next;
		} else {
			job_list_head.next = NULL;
		}
	}

	return ret;
}

static int _clearDisplay(unsigned int colour)
{
	SDL_SetRenderDrawColor(renderer, (colour >> 16) & 0xFF,
			       (colour >> 8) & 0xFF, colour & 0xFF,
			       ALPHA_SOLID);
	SDL_RenderClear(renderer);

	return 0;
}

static int _drawRectangle(signed short x, signed short y, signed short w,
			  signed short h, unsigned int colour)
{
	rectangleColor(renderer, x + w, y, x, y + h,
		       SwapBytes((colour << ONE_BYTE) | ALPHA_SOLID));

	return 0;
}

static int _drawFilledRectangle(signed short x, signed short y, signed short w,
				signed short h, unsigned int colour)
{
	boxColor(renderer, x + w, y, x, y + h,
		 SwapBytes((colour << ONE_BYTE) | ALPHA_SOLID));

	return 0;
}

static int _drawArc(signed short x, signed short y, signed short radius,
		    signed short start, signed short end, unsigned int colour)
{
	arcColor(renderer, x, y, radius, start, end,
		 SwapBytes((colour << ONE_BYTE) | ALPHA_SOLID));

	return 0;
}

static int _drawEllipse(signed short x, signed short y, signed short rx,
			signed short ry, unsigned int colour)
{
	ellipseColor(renderer, x, y, rx, ry,
		     SwapBytes((colour << ONE_BYTE) | ALPHA_SOLID));

	return 0;
}

static int _drawCircle(signed short x, signed short y, signed short radius,
		       unsigned int colour)
{
	filledCircleColor(renderer, x, y, radius,
			  SwapBytes((colour << ONE_BYTE) | ALPHA_SOLID));

	return 0;
}

static int _drawLine(signed short x1, signed short y1, signed short x2,
		     signed short y2, unsigned char thickness,
		     unsigned int colour)
{
	thickLineColor(renderer, x1, y1, x2, y2, thickness,
		       SwapBytes((colour << ONE_BYTE) | ALPHA_SOLID));

	return 0;
}

static int _drawPoly(coord_t *points, unsigned int n, signed short colour)
{
	signed short *x_coords = calloc(1, sizeof(signed short) * n);
	signed short *y_coords = calloc(1, sizeof(signed short) * n);
	unsigned int i;

	for (i = 0; i < n; i++) {
		x_coords[i] = points[i].x;
		y_coords[i] = points[i].y;
	}

	polygonColor(renderer, x_coords, y_coords, n,
		     SwapBytes((colour << ONE_BYTE) | ALPHA_SOLID));

	free(x_coords);
	free(y_coords);

	return 0;
}

static int _drawTriangle(coord_t *points, unsigned int colour)
{
	filledTrigonColor(renderer, points[0].x, points[0].y, points[1].x,
			  points[1].y, points[2].x, points[2].y,
			  SwapBytes((colour << ONE_BYTE) | ALPHA_SOLID));

	return 0;
}

static SDL_Texture *loadImage(char *filename, SDL_Renderer *ren)
{
	SDL_Texture *tex = NULL;

	tex = IMG_LoadTexture(ren, filename);

	return tex;
}

static int _renderScaledImage(SDL_Texture *tex, SDL_Renderer *ren,
			      signed short x, signed short y, int w, int h)
{
	SDL_Rect dst;
	dst.w = w;
	dst.h = h;
	dst.x = x;
	dst.y = y;
	return SDL_RenderCopy(ren, tex, NULL, &dst);
}

static int _getImageSize(char *filename, int *w, int *h)
{
	SDL_Texture *tex = loadImage(filename, renderer);
	if (tex == NULL) {
		return -1;
	}
	SDL_QueryTexture(tex, NULL, NULL, w, h);
	SDL_DestroyTexture(tex);

	return 0;
}

static int freeLoadedImage(loaded_image_t **img)
{
	int ret = -1;

	pthread_mutex_lock(&loaded_images_lock);
	loaded_image_t *iterator = &loaded_images_list;
	loaded_image_t *delete;

	for (; iterator->next; iterator = iterator->next)
		if (iterator->next == *img)
			break;

	if (iterator->next == *img) {
		delete = iterator->next;

		if (!iterator->next->next)
			iterator->next = NULL;
		else
			iterator->next = delete->next;

		SDL_FreeSurface(delete->surf);
		SDL_RWclose(delete->ops);
		SDL_DestroyTexture(delete->tex);
		free(delete->filename);
		free(delete);
		*img = (loaded_image_t *)NULL;

		ret = 0;
	}
	pthread_mutex_unlock(&loaded_images_lock);

	return ret;
}

static void vPutLoadedImage(image_handle_t img)
{
	loaded_image_t *loaded_img = (loaded_image_t *)img;

	loaded_img->ref_count--;

	if (loaded_img->pending_free && !loaded_img->ref_count)
		freeLoadedImage((loaded_image_t **)&img);
}

int xDrawLoadedImage(loaded_image_t *img, SDL_Renderer *ren, signed short x,
		     signed short y)
{
	return _renderScaledImage(img->tex, ren, x, y, img->w * img->scale,
				  img->h * img->scale);
}

static int _drawScaledImage(SDL_Texture *tex, SDL_Renderer *ren, signed short x,
			    signed short y, float scale)
{
	int w, h;
	SDL_QueryTexture(tex, NULL, NULL, &w, &h);
	if (!w || !h)
		return -1;

	if (_renderScaledImage(tex, ren, x, y, w * scale, h * scale))
		return -1;

	SDL_DestroyTexture(tex);

	return 0;
}

static int _drawImage(SDL_Texture *tex, SDL_Renderer *ren, signed short x,
		      signed short y)
{
	return _drawScaledImage(tex, ren, x, y, 1);
}

static int _drawText(char *string, signed short x, signed short y,
		     unsigned int colour, TTF_Font *font)
{
	SDL_Color color = { RED_PORTION(colour), GREEN_PORTION(colour),
			    BLUE_PORTION(colour), ZERO_ALPHA };
	SDL_Surface *surface = TTF_RenderText_Solid(font, string, color);
	tumFontPutFont(font);
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_Rect dst = { 0 };
	SDL_QueryTexture(texture, NULL, NULL, &dst.w, &dst.h);
	dst.x = x;
	dst.y = y;
	SDL_RenderCopy(renderer, texture, NULL, &dst);
	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);

	return 0;
}

static int _getTextSize(char *string, int *width, int *height)
{
	SDL_Color color = { 0 };
	TTF_Font *font = tumFontGetCurFont();
	SDL_Surface *surface = TTF_RenderText_Solid(font, string, color);
	tumFontPutFont(font);
	if (surface == NULL)
		goto err_surface;

	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
	if (texture == NULL)
		goto err_texture;

	if (SDL_QueryTexture(texture, NULL, NULL, width, height))
		goto err_query;

	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);

	return 0;

err_query:
	SDL_DestroyTexture(texture);
err_texture:
	SDL_FreeSurface(surface);
err_surface:
	return -1;
}

static int _drawArrow(signed short x1, signed short y1, signed short x2,
		      signed short y2, signed short head_length,
		      unsigned char thickness, unsigned int colour)
{
	// Line vector
	unsigned short dx = x2 - x1;
	unsigned short dy = y2 - y1;

	// Normalize
	float length = sqrt(dx * dx + dy * dy);
	signed short unit_dx = (signed short)(dx / length);
	signed short unit_dy = (signed short)(dy / length);

	signed short head_x1 =
		roundf(x2 - unit_dx * head_length - unit_dy * head_length);
	signed short head_y1 =
		roundf(y2 - unit_dy * head_length + unit_dx * head_length);

	signed short head_x2 =
		roundf(x2 - unit_dx * head_length + unit_dy * head_length);
	signed short head_y2 =
		roundf(y2 - unit_dy * head_length - unit_dx * head_length);

	if (thickLineColor(renderer, x1, y1, x2, y2, thickness,
			   SwapBytes((colour << ONE_BYTE) | ALPHA_SOLID))) {
		return -1;
	}
	if (thickLineColor(renderer, head_x1, head_y1, x2, y2, thickness,
			   SwapBytes((colour << ONE_BYTE) | ALPHA_SOLID))) {
		return -1;
	}
	if (thickLineColor(renderer, head_x2, head_y2, x2, y2, thickness,
			   SwapBytes((colour << ONE_BYTE) | ALPHA_SOLID))) {
		return -1;
	}

	return 0;
}

static int vHandleDrawJob(draw_job_t *job)
{
	int ret = 0;
	if (job == NULL) {
		return -1;
	}

	if (job->data == NULL)
		return -1;

	switch (job->type) {
	case DRAW_CLEAR:
		ret = _clearDisplay(job->data->clear.colour);
		break;
	case DRAW_ARC:
		ret = _drawArc(job->data->arc.x, job->data->arc.y,
			       job->data->arc.radius, job->data->arc.start,
			       job->data->arc.end, job->data->arc.colour);
		break;
	case DRAW_ELLIPSE:
		ret = _drawEllipse(job->data->ellipse.x, job->data->ellipse.y,
				   job->data->ellipse.rx, job->data->ellipse.ry,
				   job->data->ellipse.colour);
		break;
	case DRAW_TEXT:
		ret = _drawText(job->data->text.str, job->data->text.x,
				job->data->text.y, job->data->text.colour,
				job->data->text.font);
		free(job->data->text.str);
		break;
	case DRAW_RECT:
		ret = _drawRectangle(job->data->rect.x, job->data->rect.y,
				     job->data->rect.w, job->data->rect.h,
				     job->data->rect.colour);
		break;
	case DRAW_FILLED_RECT:
		ret = _drawFilledRectangle(job->data->rect.x, job->data->rect.y,
					   job->data->rect.w, job->data->rect.h,
					   job->data->rect.colour);
		break;
	case DRAW_CIRCLE:
		ret = _drawCircle(job->data->circle.x, job->data->circle.y,
				  job->data->circle.radius,
				  job->data->circle.colour);
		break;
	case DRAW_LINE:
		ret = _drawLine(job->data->line.x1, job->data->line.y1,
				job->data->line.x2, job->data->line.y2,
				job->data->line.thickness,
				job->data->line.colour);
		break;
	case DRAW_POLY:
		ret = _drawPoly(job->data->poly.points, job->data->poly.n,
				job->data->poly.colour);
		break;
	case DRAW_TRIANGLE:
		ret = _drawTriangle(job->data->triangle.points,
				    job->data->triangle.colour);
		break;
	case DRAW_IMAGE:
		job->data->image.tex =
			loadImage(job->data->image.filename, renderer);
		ret = _drawImage(job->data->image.tex, renderer,
				 job->data->image.x, job->data->image.y);
		break;
	case DRAW_LOADED_IMAGE:
		ret = xDrawLoadedImage(job->data->loaded_image.img, renderer,
				       job->data->loaded_image.x,
				       job->data->loaded_image.y);
		vPutLoadedImage(job->data->loaded_image.img);
		break;
	case DRAW_SCALED_IMAGE:
		job->data->scaled_image.image.tex = loadImage(
			job->data->scaled_image.image.filename, renderer);
		ret = _drawScaledImage(job->data->scaled_image.image.tex,
				       renderer,
				       job->data->scaled_image.image.x,
				       job->data->scaled_image.image.y,
				       job->data->scaled_image.scale);
		free(job->data->scaled_image.image.filename);
		break;
	case DRAW_ARROW:
		ret = _drawArrow(job->data->arrow.x1, job->data->arrow.y1,
				 job->data->arrow.x2, job->data->arrow.y2,
				 job->data->arrow.head_length,
				 job->data->arrow.thickness,
				 job->data->arrow.colour);
	default:
		break;
	}
	free(job->data);

	return ret;
}

#define INIT_JOB(JOB, TYPE)                                                    \
	draw_job_t *JOB = pushDrawJob();                                       \
	if (!JOB)                                                              \
		return -1;                                                     \
	union data_u *data = calloc(1, sizeof(union data_u));                  \
	if (data == NULL)                                                      \
		logCriticalError("job->data alloc");                           \
	JOB->data = data;                                                      \
	JOB->type = TYPE;

static void logCriticalError(char *msg)
{
	printf("[ERROR] %s\n", msg);
	exit(-1);
}

#define NS_IN_SECOND 1000000000.0
#define MS_IN_SECOND 1000.0
#define NS_IN_MS 1000000.0

static float timespecDiffMilli(struct timespec *start, struct timespec *stop)
{
	if ((stop->tv_nsec - start->tv_nsec) < 0)
		return (stop->tv_sec - start->tv_sec - 1) * MS_IN_SECOND +
		       (stop->tv_nsec - start->tv_nsec + NS_IN_SECOND) /
			       NS_IN_MS;

	return (stop->tv_sec - start->tv_sec) * MS_IN_SECOND +
	       (stop->tv_nsec - start->tv_nsec) / NS_IN_MS;
}

#define FRAMELIMIT 60.0
#define FRAMELIMIT_PERIOD 1000 / FRAMELIMIT

int tumDrawUpdateScreen(void)
{
	static struct timespec last_time = { 0 }, cur_time = { 0 };

	if (clock_gettime(CLOCK_MONOTONIC, &cur_time)) {
		PRINT_ERROR("Failed to get monotonic clock");
		goto err;
	}

	if (timespecDiffMilli(&last_time, &cur_time) < (float)FRAMELIMIT_PERIOD)
		goto err;

	memcpy(&last_time, &cur_time, sizeof(struct timespec));

	if (job_list_head.next == NULL)
		goto err;

	draw_job_t *tmp_job;

	while ((tmp_job = popDrawJob()) != NULL) {
		if (!tmp_job->data)
			return -1;
		if (vHandleDrawJob(tmp_job) == -1) {
			goto draw_error;
		}
		free(tmp_job);
	}

	SDL_RenderPresent(renderer);

	return 0;

draw_error:
	free(tmp_job);
err:
	return -1;
}

char *tumGetErrorMessage(void)
{
	return error_message;
}

int tumDrawInit(char *path) // Should be called from the Thread running main()
{
	/* Relevant for Docker-based toolchain */
#ifdef DOCKER
#ifndef HOST_OS
#warning "HOST_OS undefined! Assuming 'linux'..."
#elif HOST_OS != linux
	setenv("LIBGL_ALWAYS_INDIRECT", "1",
	       1); // speed up drawings a little bit
	setenv("SDL_VIDEO_X11_VISUALID", "",
	       1); // required on windows and macos
#elif HOST_OS == linux
	// nothing
#else
#error "Unexpected value of HOST_OS!"
#endif /* HOST_OS */
#endif /* DOCKER */
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

	if (SDL_Init(SDL_INIT_EVERYTHING)) {
		PRINT_SDL_ERROR("SDL_Init failed");
		goto err_sdl;
	}
	if (TTF_Init()) {
		PRINT_ERROR("TTF_Init failed");
		goto err_ttf;
	}

	if (tumFontInit(path)) {
		PRINT_ERROR("TUM Font init failed");
		goto err_tum_font;
	}

	window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED,
				  SDL_WINDOWPOS_CENTERED, screen_width,
				  screen_height, SDL_WINDOW_OPENGL);

	if (window == NULL) {
		PRINT_SDL_ERROR("Failed to create %d x %d window '%s'",
				screen_width, screen_height, WINDOW_TITLE);
		goto err_window;
	}

	context = SDL_GL_CreateContext(window);

	if (context == NULL) {
		PRINT_SDL_ERROR("Failed to create context");
		goto err_create_context;
	}

	if (SDL_GL_MakeCurrent(window, context) < 0) {
		PRINT_SDL_ERROR("Claiming current context failed");
		goto err_make_current;
	}

	if (SDL_GL_MakeCurrent(window, NULL) < 0) {
		PRINT_SDL_ERROR("Releasing current context failed");
		goto err_make_current;
	}

	tumDrawBindThread();

	atexit(SDL_Quit);

	return 0;

err_make_current:
	SDL_GL_DeleteContext(context);
err_create_context:
	SDL_DestroyWindow(window);
err_window:
	tumFontExit();
err_tum_font:
	TTF_Quit();
err_ttf:
	SDL_Quit();
err_sdl:
	return -1;
}

int tumDrawBindThread(void) // Should be called from the Drawing Thread
{
	if (SDL_GL_MakeCurrent(window, context) < 0) {
		PRINT_SDL_ERROR("Releasing current context failed");
		goto err_make_current;
	}

	if (renderer) {
		SDL_DestroyRenderer(renderer);
		renderer = NULL;
	}

	renderer = SDL_CreateRenderer(window, -1,
				      SDL_RENDERER_ACCELERATED |
					      SDL_RENDERER_TARGETTEXTURE |
					      SDL_RENDERER_PRESENTVSYNC);

	if (renderer == NULL) {
		PRINT_SDL_ERROR("Failed to create renderer");
		goto err_renderer;
	}

	SDL_SetRenderDrawColor(renderer, MAX_8_BIT, MAX_8_BIT, MAX_8_BIT,
			       ALPHA_SOLID);

	SDL_RenderClear(renderer);

	pthread_mutex_lock(&loaded_images_lock);
	loaded_image_t *iterator = &loaded_images_list;

	for (; iterator; iterator = iterator->next)
		if (iterator->tex) {
			SDL_DestroyTexture(iterator->tex);
			iterator->tex = SDL_CreateTextureFromSurface(
				renderer, iterator->surf);
		}

	pthread_mutex_unlock(&loaded_images_lock);

	return 0;

err_renderer:
	SDL_DestroyWindow(window);
err_make_current:
	SDL_GL_DeleteContext(context);
	TTF_Quit();
	SDL_Quit();
	return -1;
}

void tumDrawExit(void)
{
	if (window) {
		SDL_DestroyWindow(window);
	}

	if (renderer) {
		SDL_DestroyRenderer(renderer);
	}

	TTF_Quit();
	SDL_Quit();

	exit(EXIT_SUCCESS);
}

int tumDrawText(char *str, signed short x, signed short y, unsigned int colour)
{
	if (strcmp(str, "") == 0) {
		return -1;
	}

	INIT_JOB(job, DRAW_TEXT);

	job->data->text.str = (char *)calloc(strlen(str) + 1, sizeof(char));

	if (job->data->text.str == NULL) {
		printf("Error allocating buffer in tumDrawText\n");
		return -1;
	}

	strcpy(job->data->text.str, str);
	job->data->text.font = tumFontGetCurFont();
	job->data->text.x = x;
	job->data->text.y = y;
	job->data->text.colour = colour;

	return 0;
}

int tumGetTextSize(char *str, int *width, int *height)
{
	if (str == NULL)
		return -1;
	return _getTextSize(str, width, height);
}

int tumDrawEllipse(signed short x, signed short y, signed short rx,
		   signed short ry, unsigned int colour)
{
	INIT_JOB(job, DRAW_ELLIPSE);

	job->data->ellipse.x = x;
	job->data->ellipse.y = y;
	job->data->ellipse.rx = rx;
	job->data->ellipse.ry = ry;
	job->data->ellipse.colour = colour;

	return 0;
}

int tumDrawArc(signed short x, signed short y, signed short radius,
	       signed short start, signed short end, unsigned int colour)
{
	INIT_JOB(job, DRAW_ARC);

	job->data->arc.x = x;
	job->data->arc.y = y;
	job->data->arc.radius = radius;
	job->data->arc.start = start;
	job->data->arc.end = end;
	job->data->arc.colour = colour;

	return 0;
}

int tumDrawFilledBox(signed short x, signed short y, signed short w,
		     signed short h, unsigned int colour)
{
	INIT_JOB(job, DRAW_FILLED_RECT);

	job->data->rect.x = x;
	job->data->rect.y = y;
	job->data->rect.w = w;
	job->data->rect.h = h;
	job->data->rect.colour = colour;

	addColor(x,y,colour);

	return 0;
}

int tumDrawBox(signed short x, signed short y, signed short w, signed short h,
	       unsigned int colour)
{
	INIT_JOB(job, DRAW_RECT);

	job->data->rect.x = x;
	job->data->rect.y = y;
	job->data->rect.w = w;
	job->data->rect.h = h;
	job->data->rect.colour = colour;

	return 0;
}

void tumDrawDuplicateBuffer(void)
{
	SDL_Surface *screen_shot =
		SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
				     0x00ff0000, 0x0000ff00, 0x000000ff,
				     0xff000000);
	SDL_RenderReadPixels(renderer, NULL, 0, screen_shot->pixels,
			     screen_shot->pitch);
	SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, screen_shot);
	SDL_RenderClear(renderer);
	SDL_Rect dest = { .w = SCREEN_WIDTH, .h = SCREEN_HEIGHT };
	SDL_RenderCopy(renderer, tex, NULL, &dest);
    SDL_RenderPresent(renderer);
}

int tumDrawClear(unsigned int colour)
{
	/** INIT_JOB(job, DRAW_CLEAR); */
	draw_job_t *job = pushDrawJob();
	union data_u *data = calloc(1, sizeof(union data_u));
	if (data == NULL) {
		logCriticalError("job->data alloc");
	}
	job->data = data;
	job->type = DRAW_CLEAR;

	job->data->clear.colour = colour;

	return 0;
}

int tumDrawCircle(signed short x, signed short y, signed short radius,
		  unsigned int colour)
{
	INIT_JOB(job, DRAW_CIRCLE);

	job->data->circle.x = x;
	job->data->circle.y = y;
	job->data->circle.radius = radius;
	job->data->circle.colour = colour;

	return 0;
}

int tumDrawLine(signed short x1, signed short y1, signed short x2,
		signed short y2, unsigned char thickness, unsigned int colour)
{
	INIT_JOB(job, DRAW_LINE);

	job->data->line.x1 = x1;
	job->data->line.y1 = y1;
	job->data->line.x2 = x2;
	job->data->line.y2 = y2;
	job->data->line.thickness = thickness;
	job->data->line.colour = colour;

	return 0;
}

int tumDrawPoly(coord_t *points, int n, unsigned int colour)
{
	INIT_JOB(job, DRAW_POLY);

	coord_t *points_cpy = (coord_t *)malloc(sizeof(coord_t) * n);
	if (!points_cpy)
		return -1;

	memcpy(points_cpy, points, sizeof(coord_t) * n);

	job->data->poly.points = points_cpy;
	job->data->poly.n = n;
	job->data->poly.colour = colour;

	return 0;
}

int tumDrawTriangle(coord_t *points, unsigned int colour)
{
	INIT_JOB(job, DRAW_TRIANGLE);

	coord_t *points_cpy = (coord_t *)malloc(sizeof(coord_t) * 3);
	if (!points_cpy)
		return -1;

	memcpy(points_cpy, points, sizeof(coord_t) * 3);

	job->data->triangle.points = points_cpy;
	job->data->triangle.colour = colour;

	return 0;
}

image_handle_t tumDrawLoadScaledImage(char *filename, float scale)
{
	loaded_image_t *ret = calloc(1, sizeof(loaded_image_t));
	if (ret == NULL)
		goto err_alloc;

	ret->filename = strdup(filename);
	if (ret->filename == NULL)
		goto err_filename;

	ret->file = fopen(filename, "r");
	if (ret->file == NULL)
		goto err_file_open;

	ret->ops = SDL_RWFromFP(ret->file, SDL_TRUE);
	if (ret->ops == NULL)
		goto err_ops;

	ret->surf = IMG_Load_RW(ret->ops, 0);
	if (ret->surf == NULL)
		goto err_surf;

	ret->tex = SDL_CreateTextureFromSurface(renderer, ret->surf);
	if (ret->tex == NULL)
		goto err_tex;

	SDL_QueryTexture(ret->tex, NULL, NULL, &ret->w, &ret->h);

	ret->scale = scale;

	pthread_mutex_lock(&loaded_images_lock);

	loaded_image_t *iterator = &loaded_images_list;
	for (; iterator->next; iterator = iterator->next)
		;
	iterator->next = ret;

	pthread_mutex_unlock(&loaded_images_lock);

	return ret;

err_tex:
	SDL_FreeSurface(ret->surf);
err_surf:
	SDL_RWclose(ret->ops);
err_ops:
	fclose(ret->file);
err_file_open:
	free(ret->filename);
err_filename:
	free(ret);
err_alloc:
	return NULL;
}

image_handle_t tumDrawLoadImage(char *filename)
{
	return tumDrawLoadScaledImage(filename, 1);
}

int tumDrawFreeLoadedImage(image_handle_t *img)
{
	int ret = 0;
	loaded_image_t **loaded_img = (loaded_image_t **)img;

	if (!(*loaded_img)->ref_count)
		ret = freeLoadedImage(loaded_img);
	else
		(*loaded_img)->pending_free = 1;

	return ret;
}

int tumDrawLoadedImage(image_handle_t img, signed short x, signed short y)
{
	if (img == NULL)
		return -1;

	INIT_JOB(job, DRAW_LOADED_IMAGE);

	((loaded_image_t *)img)->ref_count++;
	job->data->loaded_image.img = img;
	job->data->loaded_image.x = x;
	job->data->loaded_image.y = y;

	return 0;
}

int tumDrawSetLoadedImageScale(image_handle_t img, float scale)
{
	if (img == NULL)
		return -1;

	((loaded_image_t *)img)->scale = scale;

	return 0;
}

float tumDrawGetLoadedImageScale(image_handle_t img)
{
	if (img == NULL)
		return -1;

	return ((loaded_image_t *)img)->scale;
}

int tumDrawGetLoadedImageWidth(image_handle_t img)
{
	if (img == NULL)
		return -1;

	return ((loaded_image_t *)img)->w * ((loaded_image_t *)img)->scale;
}

int tumDrawGetLoadedImageHeight(image_handle_t img)
{
	if (img == NULL)
		return -1;

	return ((loaded_image_t *)img)->h * ((loaded_image_t *)img)->scale;
}

int tumDrawGetLoadedImageSize(image_handle_t img, int *w, int *h)
{
	if (img == NULL)
		return -1;

	*w = tumDrawGetLoadedImageWidth(img);
	*h = tumDrawGetLoadedImageHeight(img);

	if (*w == -1 || *h == -1)
		return -1;

	return 0;
}

int __attribute_deprecated__ tumDrawImage(char *filename, signed short x,
					  signed short y)
{
	INIT_JOB(job, DRAW_IMAGE);

	char abs_path[PATH_MAX + 1];

	if (realpath(filename, (char *)abs_path) == NULL) {
		return -1;
	}

	job->data->image.filename = calloc(strlen(abs_path) + 1, sizeof(char));
	strcpy(job->data->image.filename, abs_path);
	job->data->image.x = x;
	job->data->image.y = y;

	return 0;
}

int __attribute_deprecated__ tumGetImageSize(char *filename, int *w, int *h)
{
	char full_filename[PATH_MAX + 1];
	realpath(filename, full_filename);
	return _getImageSize(full_filename, w, h);
}

int __attribute_deprecated__ tumDrawScaledImage(char *filename, signed short x,
						signed short y, float scale)
{
	INIT_JOB(job, DRAW_SCALED_IMAGE);

	char abs_path[PATH_MAX + 1];

	if (realpath(filename, (char *)abs_path) == NULL) {
		return -1;
	}

	job->data->scaled_image.image.filename =
		calloc(strlen(abs_path) + 1, sizeof(char));
	strcpy(job->data->scaled_image.image.filename, abs_path);
	job->data->scaled_image.image.x = x;
	job->data->scaled_image.image.y = y;
	job->data->scaled_image.scale = scale;

	return 0;
}

int tumDrawArrow(signed short x1, signed short y1, signed short x2,
		 signed short y2, signed short head_length,
		 unsigned char thickness, unsigned int colour)
{
	INIT_JOB(job, DRAW_ARROW);

	job->data->arrow.x1 = x1;
	job->data->arrow.y1 = y1;
	job->data->arrow.x2 = x2;
	job->data->arrow.y2 = y2;
	job->data->arrow.head_length = head_length;
	job->data->arrow.thickness = thickness;
	job->data->arrow.colour = colour;

	return 0;
}

/*
 * Method: checkColor is checking if the same color on the screen as given at pixel (x1,y1)
 * 		   return 1 if color is same at (x1,y1), return 0 if not
 * Parameter: Coordinates x1 and y1, the checking color
 */
int checkColor(signed short x1, signed short y1, unsigned int color){
	if(color == array[x1-100][y1-100]){
		return 1;
	}
	return 0;
}

/**
 * Method: Initialise Level, as long as intial not changed
 * Parameter: Which level, set Wall true or false
 */

int a=0,b=0,c=0,d=0;
int e=0,f=0,g=0,h=0;

/**
 * Method: Reset Integer a,b,c,d,e,f,g,h for animated drawing
 */
void parameterReset(){
	a=0;
	b=0;
	c=0;
	d=0;
	e=0;
	f=0;
	g=0;
	h=0;
}

int level6field[50][2];

void addInitfield(int x, int y, int i){
	level6field[i][0]=x;
	level6field[i][1]=y;
}
/**
 * Method: Initialise Level
 * Parameter: which level and if wall is acitve or not
 */
void initLevel(int level, int wall, int fieldArray[39][29]){
	if(level==1 && wall){
		if(a!=400){
			checkDraw(tumDrawFilledBox(100+a,100,5,5,Grey), __FUNCTION__);
			a+=10;
		}
		else{
			if(b!=310){
				checkDraw(tumDrawFilledBox(500,100+b,5,5,Grey), __FUNCTION__);
				b+=10;
			}
			else{
				if(c!=400){
					checkDraw(tumDrawFilledBox(500-c,400,5,5,Grey), __FUNCTION__);
				c+=10;
				}
				else{
					if(d!=310){
						checkDraw(tumDrawFilledBox(100,400-d,5,5,Grey), __FUNCTION__);
						d+=10;
					}
				}
			}
		}
	}
	if(level==1 && !wall){
		if(a!=400){
			checkDraw(tumDrawFilledBox(100+a,100,5,5,White), __FUNCTION__);
			a+=5;
		}
		else{
			if(b!=300){
				checkDraw(tumDrawFilledBox(500,100+b,5,5,White), __FUNCTION__);
				b+=5;
			}
			else{
				if(c!=400){
					checkDraw(tumDrawFilledBox(500-c,400,5,5,White), __FUNCTION__);
				c+=5;
				}
				else{
					if(d!=305){
						checkDraw(tumDrawFilledBox(100,400-d,5,5,White), __FUNCTION__);
						d+=5;
					}
				}
			}
		}
	}
	if(level==2 && wall){
		if(a!=400){
			checkDraw(tumDrawFilledBox(100+a,100,5,5,Grey), __FUNCTION__);
			a+=10;
		}
		else{
			if(b!=310){
				checkDraw(tumDrawFilledBox(500,100+b,5,5,Grey), __FUNCTION__);
				b+=10;
			}
			else{
				if(c!=400){
					checkDraw(tumDrawFilledBox(500-c,400,5,5,Grey), __FUNCTION__);
				c+=10;
				}
				else{
					if(d!=310){
						checkDraw(tumDrawFilledBox(100,400-d,5,5,Grey), __FUNCTION__);
						d+=10;
					}
				}
			}
		}
		if(e!=155){
			checkDraw(tumDrawFilledBox(100+e,200,5,5,White), __FUNCTION__);
			e+=5;
		}
		else{
			if(f!=155){
				checkDraw(tumDrawFilledBox(350+f,200,5,5,White), __FUNCTION__);
				f+=5;
			}
			else{
				if(g!=155){
					checkDraw(tumDrawFilledBox(100+g,300,5,5,White), __FUNCTION__);
					g+=5;
				}
				else{
					if(h!=155){
						checkDraw(tumDrawFilledBox(350+h,300,5,5,White), __FUNCTION__);
						h+=5;
					}
				}
			}
		}
	}

	if(level==2 && !wall){
		if(a!=400){
			checkDraw(tumDrawFilledBox(100+a,100,5,5,White), __FUNCTION__);
			a+=5;
		}
		else{
			if(b!=300){
				checkDraw(tumDrawFilledBox(500,100+b,5,5,White), __FUNCTION__);
				b+=5;
			}
			else{
				if(c!=400){
					checkDraw(tumDrawFilledBox(500-c,400,5,5,White), __FUNCTION__);
				c+=5;
				}
				else{
					if(d!=305){
						checkDraw(tumDrawFilledBox(100,400-d,5,5,White), __FUNCTION__);
						d+=5;
					}
				}
			}
		}
		if(e!=155){
			checkDraw(tumDrawFilledBox(100+e,200,5,5,White), __FUNCTION__);
			e+=5;
		}
		else{
			if(f!=155){
				checkDraw(tumDrawFilledBox(350+f,200,5,5,White), __FUNCTION__);
				f+=5;
			}
			else{
				if(g!=155){
					checkDraw(tumDrawFilledBox(100+g,300,5,5,White), __FUNCTION__);
					g+=5;
				}
				else{
					if(h!=155){
						checkDraw(tumDrawFilledBox(350+h,300,5,5,White), __FUNCTION__);
						h+=5;
					}
				}
			}
		}
	}

	if(level==3 && !wall){
		if(a!=400){
			checkDraw(tumDrawFilledBox(100+a,100,5,5,White), __FUNCTION__);
			a+=5;
		}
		else{
			if(b!=300){
				checkDraw(tumDrawFilledBox(500,100+b,5,5,White), __FUNCTION__);
				b+=5;
			}
			else{
				if(c!=400){
					checkDraw(tumDrawFilledBox(500-c,400,5,5,White), __FUNCTION__);
				c+=5;
				}
				else{
					if(d!=305){
						checkDraw(tumDrawFilledBox(100,400-d,5,5,White), __FUNCTION__);
						d+=5;
					}
				}
			}
		}
		if(e!=270){
			checkDraw(tumDrawFilledBox(165+e,200,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(165+e,300,5,5,White), __FUNCTION__);
			e+=5;
		}
		if(d==305 && f!=65){
			checkDraw(tumDrawFilledBox(100+f,100,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,105,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,110,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,115,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,120,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,125,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,130,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,135,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,140,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,145,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,150,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,155,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,160,5,5,White), __FUNCTION__);

			checkDraw(tumDrawFilledBox(100+f,400,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,395,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,390,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,385,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,380,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,375,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,370,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,365,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,360,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,355,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,350,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,345,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,340,5,5,White), __FUNCTION__);
			f+=5;
		}
		if(d==305 && g!=65){
			checkDraw(tumDrawFilledBox(440+g,100,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,105,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,110,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,115,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,120,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,125,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,130,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,135,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,140,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,145,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,150,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,155,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,160,5,5,White), __FUNCTION__);

			checkDraw(tumDrawFilledBox(440+g,400,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,395,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,390,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,385,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,380,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,375,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,370,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,365,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,360,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,355,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,350,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,345,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,340,5,5,White), __FUNCTION__);
			g+=5;
		}
	}
	if(level==3 && wall){
		if(a!=400){
			checkDraw(tumDrawFilledBox(100+a,100,5,5,Grey), __FUNCTION__);
			a+=10;
		}
		else{
			if(b!=310){
				checkDraw(tumDrawFilledBox(500,100+b,5,5,Grey), __FUNCTION__);
				b+=10;
			}
			else{
				if(c!=400){
					checkDraw(tumDrawFilledBox(500-c,400,5,5,Grey), __FUNCTION__);
				c+=10;
				}
				else{
					if(d!=310){
						checkDraw(tumDrawFilledBox(100,400-d,5,5,Grey), __FUNCTION__);
						d+=10;
					}
				}
			}
		}
		if(e!=270){
			checkDraw(tumDrawFilledBox(165+e,200,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(165+e,300,5,5,White), __FUNCTION__);
			e+=5;
		}
		if(d==310 && f!=65){
			checkDraw(tumDrawFilledBox(100+f,100,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,105,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,110,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,115,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,120,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,125,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,130,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,135,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,140,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,145,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,150,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,155,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,160,5,5,White), __FUNCTION__);

			checkDraw(tumDrawFilledBox(100+f,400,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,395,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,390,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,385,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,380,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,375,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,370,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,365,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,360,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,355,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,350,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,345,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(100+f,340,5,5,White), __FUNCTION__);
			f+=5;
		}
		if(d==310 && g!=65){
			checkDraw(tumDrawFilledBox(440+g,100,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,105,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,110,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,115,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,120,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,125,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,130,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,135,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,140,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,145,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,150,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,155,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,160,5,5,White), __FUNCTION__);

			checkDraw(tumDrawFilledBox(440+g,400,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,395,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,390,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,385,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,380,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,375,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,370,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,365,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,360,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,355,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,350,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,345,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(440+g,340,5,5,White), __FUNCTION__);
			g+=5;
		}
	}
	if(level==4 && !wall){
		if(a!=400){
			checkDraw(tumDrawFilledBox(100+a,100,5,5,White), __FUNCTION__);
			a+=5;
		}
		else{
			if(b!=300){
				checkDraw(tumDrawFilledBox(500,100+b,5,5,White), __FUNCTION__);
				b+=5;
			}
			else{
				if(c!=400){
					checkDraw(tumDrawFilledBox(500-c,400,5,5,White), __FUNCTION__);
					c+=5;
				}
				else{
					if(d!=305){
						checkDraw(tumDrawFilledBox(100,400-d,5,5,White), __FUNCTION__);
						d+=5;
					}
				}
			}
		}

		if(e!=110){
			checkDraw(tumDrawFilledBox(145+e,200,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(145+e,300,5,5,White), __FUNCTION__);
			e+=5;
		}
		else{
			if(f!=105){
				checkDraw(tumDrawFilledBox(350+f,200,5,5,White), __FUNCTION__);
				checkDraw(tumDrawFilledBox(350+f,300,5,5,White), __FUNCTION__);
				f+=5;
			}
			else{
				if(g!=50){
					checkDraw(tumDrawFilledBox(250,150+g,5,5,White), __FUNCTION__);
					checkDraw(tumDrawFilledBox(350,150+g,5,5,White), __FUNCTION__);
					g+=5;
				}
				else{
					if(h!=55){
						checkDraw(tumDrawFilledBox(250,300+h,5,5,White), __FUNCTION__);
						checkDraw(tumDrawFilledBox(350,300+h,5,5,White), __FUNCTION__);
						h+=5;
					}
				}
			}
		}
	}

	if(level==4 && wall){
		if(a!=400){
			checkDraw(tumDrawFilledBox(100+a,100,5,5,Grey), __FUNCTION__);
			a+=10;
		}
		else{
			if(b!=310){
				checkDraw(tumDrawFilledBox(500,100+b,5,5,Grey), __FUNCTION__);
				b+=10;
			}
			else{
				if(c!=400){
					checkDraw(tumDrawFilledBox(500-c,400,5,5,Grey), __FUNCTION__);
					c+=10;
				}
				else{
					if(d!=310){
						checkDraw(tumDrawFilledBox(100,400-d,5,5,Grey), __FUNCTION__);
						d+=10;
					}
				}
			}
		}

		if(e!=110){
			checkDraw(tumDrawFilledBox(145+e,200,5,5,White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(145+e,300,5,5,White), __FUNCTION__);
			e+=5;
		}
		else{
			if(f!=105){
				checkDraw(tumDrawFilledBox(350+f,200,5,5,White), __FUNCTION__);
				checkDraw(tumDrawFilledBox(350+f,300,5,5,White), __FUNCTION__);
				f+=5;
			}
			else{
				if(g!=50){
					checkDraw(tumDrawFilledBox(250,150+g,5,5,White), __FUNCTION__);
					checkDraw(tumDrawFilledBox(350,150+g,5,5,White), __FUNCTION__);
					g+=5;
				}
				else{
					if(h!=55){
						checkDraw(tumDrawFilledBox(250,300+h,5,5,White), __FUNCTION__);
						checkDraw(tumDrawFilledBox(350,300+h,5,5,White), __FUNCTION__);
						h+=5;
					}
				}
			}
		}
	}

	if(level==5 && wall){
		if(a!=400){
			checkDraw(tumDrawFilledBox(100+a,100,5,5,Grey), __FUNCTION__);
			a+=10;
		}
		else{
			if(b!=310){
				checkDraw(tumDrawFilledBox(500,100+b,5,5,Grey), __FUNCTION__);
				b+=10;
			}
			else{
				if(c!=400){
					checkDraw(tumDrawFilledBox(500-c,400,5,5,Grey), __FUNCTION__);
					c+=10;
				}
				else{
					if(d!=310){
						checkDraw(tumDrawFilledBox(100,400-d,5,5,Grey), __FUNCTION__);
						d+=10;
					}
				}
			}
			if(d==310 && e!=305){
				checkDraw(tumDrawFilledBox(100, 100+e, 5, 5, White), __FUNCTION__);
				checkDraw(tumDrawFilledBox(200, 100+e, 5, 5, White), __FUNCTION__);
				checkDraw(tumDrawFilledBox(300, 100+e, 5, 5, White), __FUNCTION__);
				checkDraw(tumDrawFilledBox(400, 100+e, 5, 5, White), __FUNCTION__);
				checkDraw(tumDrawFilledBox(500, 100+e, 5, 5, White), __FUNCTION__);
				e+=5;
			}
			if(e==305 && f!=50){
				if(f==0){
					checkDraw(tumDrawFilledBox(150, 170, 5, 5, Purple), __FUNCTION__);
				}
				if(f==5){
					checkDraw(tumDrawFilledBox(250, 170, 5, 5, Purple), __FUNCTION__);
				}
				if(f==10){
					checkDraw(tumDrawFilledBox(350, 170, 5, 5, Purple), __FUNCTION__);
				}
				if(f==15){
					checkDraw(tumDrawFilledBox(450, 170, 5, 5, Purple), __FUNCTION__);
				}
				if(f==30){
					checkDraw(tumDrawFilledBox(450, 330, 5, 5, Purple), __FUNCTION__);
				}
				if(f==35){
					checkDraw(tumDrawFilledBox(350, 330, 5, 5, Purple), __FUNCTION__);
				}
				if(f==40){
					checkDraw(tumDrawFilledBox(250, 330, 5, 5, Purple), __FUNCTION__);
				}
				if(f==45){
					checkDraw(tumDrawFilledBox(150, 330, 5, 5, Purple), __FUNCTION__);
				}
				f++;
			}
		}

	}
	if(level==5 && !wall){
		if(a!=400){
			checkDraw(tumDrawFilledBox(100+a,100,5,5,White), __FUNCTION__);
			a+=5;
		}
		else{
			if(b!=300){
				checkDraw(tumDrawFilledBox(500,100+b,5,5,White), __FUNCTION__);
				b+=5;
			}
			else{
				if(c!=400){
					checkDraw(tumDrawFilledBox(500-c,400,5,5,White), __FUNCTION__);
					c+=5;
				}
				else{
					if(d!=305){
						checkDraw(tumDrawFilledBox(100,400-d,5,5,White), __FUNCTION__);
						d+=5;
					}
				}
			}
		}
		if(d==305 && e!=305){
			checkDraw(tumDrawFilledBox(100, 100+e, 5, 5, White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(200, 100+e, 5, 5, White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(300, 100+e, 5, 5, White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(400, 100+e, 5, 5, White), __FUNCTION__);
			checkDraw(tumDrawFilledBox(500, 100+e, 5, 5, White), __FUNCTION__);
			e+=5;
		}
		if(e==305 && f!=50){
			if(f==0){
				checkDraw(tumDrawFilledBox(150, 170, 5, 5, Purple), __FUNCTION__);
			}
			if(f==5){
				checkDraw(tumDrawFilledBox(250, 170, 5, 5, Purple), __FUNCTION__);
			}
			if(f==10){
				checkDraw(tumDrawFilledBox(350, 170, 5, 5, Purple), __FUNCTION__);
			}
			if(f==15){
				checkDraw(tumDrawFilledBox(450, 170, 5, 5, Purple), __FUNCTION__);
			}
			if(f==30){
				checkDraw(tumDrawFilledBox(450, 330, 5, 5, Purple), __FUNCTION__);
			}
			if(f==35){
				checkDraw(tumDrawFilledBox(350, 330, 5, 5, Purple), __FUNCTION__);
			}
			if(f==40){
				checkDraw(tumDrawFilledBox(250, 330, 5, 5, Purple), __FUNCTION__);
			}
			if(f==45){
				checkDraw(tumDrawFilledBox(150, 330, 5, 5, Purple), __FUNCTION__);
			}
			f++;
		}
	}

	if(level==6 && wall){
		if(a!=400){
			checkDraw(tumDrawFilledBox(100+a,100,5,5,Grey), __FUNCTION__);
			a+=10;
		}
		else{
			if(b!=310){
				checkDraw(tumDrawFilledBox(500,100+b,5,5,Grey), __FUNCTION__);
				b+=10;
			}
			else{
				if(c!=400){
					checkDraw(tumDrawFilledBox(500-c,400,5,5,Grey), __FUNCTION__);
					c+=10;
				}
				else{
					if(d!=310){
						checkDraw(tumDrawFilledBox(100,400-d,5,5,Grey), __FUNCTION__);
						d+=10;
					}
				}
			}
		}

		if(d==310 && e!=50){
			checkDraw(tumDrawFilledBox((level6field[e][0]+11)*10,(level6field[e][1]+11)*10,5,5,White), __FUNCTION__);
			e++;
		}
	}
	if(level==6 && !wall){
		if(a!=400){
			checkDraw(tumDrawFilledBox(100+a,100,5,5,White), __FUNCTION__);
			a+=5;
		}
		else{
			if(b!=300){
				checkDraw(tumDrawFilledBox(500,100+b,5,5,White), __FUNCTION__);
				b+=5;
			}
			else{
				if(c!=400){
					checkDraw(tumDrawFilledBox(500-c,400,5,5,White), __FUNCTION__);
					c+=5;
				}
				else{
					if(d!=305){
						checkDraw(tumDrawFilledBox(100,400-d,5,5,White), __FUNCTION__);
						d+=5;
					}
				}
			}
		}
		if(d==305 && e!=50){
					checkDraw(tumDrawFilledBox((level6field[e][0]+11)*10,(level6field[e][1]+11)*10,5,5,White), __FUNCTION__);
					e++;
				}
	}
}

/**
 * Method: Drawing Level1
 * Parameter: x1 top left corner x, y1 top left corner y
 * x2 bottom right corner x, y2 bottom right corner y, wall true or false
 */
void level1(int x1, int y1, int x2, int y2, int wall){
	int color=White;
	int distance=5;
	if(wall){
		color=Grey;
		distance=10;
	}
	//top wall side
	for(int x=x1; x<=x2;x+=distance)
	{
		checkDraw(tumDrawFilledBox(x,y1,5,5,color), __FUNCTION__);
	}
	//bottom wall side
	for(int x=x1; x<=x2;x+=distance)
	{
		checkDraw(tumDrawFilledBox(x,y2,5,5,color), __FUNCTION__);
	}
	//left wall side
	for(int y=y1; y<=y2;y+=distance)
	{
		checkDraw(tumDrawFilledBox(x1,y,5,5,color), __FUNCTION__);
	}
	//right wall side
	for(int y=y1; y<=y2;y+=distance)
	{
		checkDraw(tumDrawFilledBox(x2,y,5,5,color), __FUNCTION__);
	}
}

/**
 * Method: Drawing Level2
 * Parameter: x1 top left corner x, y1 top left corner y
 * x2 bottom right corner x, y2 bottom right corner y, wall true or false
 */
void level2(int x1, int y1, int x2, int y2, int wall){
	//Outside wall White or Grey
	level1(x1,y1,x2,y2,wall);
	//Wall color
	int color=White;
	//Left Top to Center
	//Left Bottom to Center
	for(int x=x1; x<=250;x+=5)
	{
		checkDraw(tumDrawFilledBox(x,200,5,5,color), __FUNCTION__);
		checkDraw(tumDrawFilledBox(x,300,5,5,color), __FUNCTION__);
	}
	//Right Top to Center
	//Right Bottom to Center
	for(int x=350; x<=x2;x+=5)
	{
		checkDraw(tumDrawFilledBox(x,200,5,5,color), __FUNCTION__);
		checkDraw(tumDrawFilledBox(x,300,5,5,color), __FUNCTION__);
	}
}

/**
 * Method: Drawing Level3
 * Parameter: x1 top left corner x, y1 top left corner y
 * x2 bottom right corner x, y2 bottom right corner y, wall true or false
 */
void level3(int x1, int y1, int x2, int y2, int wall){
	level1(x1,y1,x2,y2,wall);
	//Wall color
	int color=White;
	//Middle Lines
	for(int x=170; x<=430;x+=5)
	{
		//Top
		checkDraw(tumDrawFilledBox(x,200,5,5,color), __FUNCTION__);
		//Bottom
		checkDraw(tumDrawFilledBox(x,300,5,5,color), __FUNCTION__);
	}
	//Left Top Corner
	for(int y=100;y<=160;y+=5){
		for(int x=100;x<=160;x+=5){
				checkDraw(tumDrawFilledBox(x,y,5,5,color), __FUNCTION__);
		}

	}
	//Right Top Corner
	for(int y=100;y<=160;y+=5){
		for(int x=440;x<=500;x+=5){
				checkDraw(tumDrawFilledBox(x,y,5,5,color), __FUNCTION__);
		}
	}
	//Left Bottom Corner
	for(int y=340;y<=400;y+=5){
		for(int x=100;x<=160;x+=5){
				checkDraw(tumDrawFilledBox(x,y,5,5,color), __FUNCTION__);
		}

	}
	//Right Bottom Corner
	for(int y=340;y<=400;y+=5){
		for(int x=440;x<=500;x+=5){
				checkDraw(tumDrawFilledBox(x,y,5,5,color), __FUNCTION__);
		}
	}
}

/**
 * Method: Drawing Level4
 * Parameter: x1 top left corner x, y1 top left corner y
 * x2 bottom right corner x, y2 bottom right corner y, wall true or false
 */
void level4(int x1, int y1, int x2, int y2, int wall){
	//Outside wall White or Grey
	level1(x1,y1,x2,y2,wall);
	//Wall color
	int color=White;
	// Horizon Lines
	for(int x=x1+50; x<=250;x+=5)
	{
		checkDraw(tumDrawFilledBox(x,200,5,5,color), __FUNCTION__);
		checkDraw(tumDrawFilledBox(x,300,5,5,color), __FUNCTION__);
	}
	for(int x=350; x<=x2-50;x+=5)
	{
		checkDraw(tumDrawFilledBox(x,200,5,5,color), __FUNCTION__);
		checkDraw(tumDrawFilledBox(x,300,5,5,color), __FUNCTION__);
	}
	// Vertical Lines
	for(int y=y1+50; y<=200;y+=5)
	{
		checkDraw(tumDrawFilledBox(250,y,5,5,color), __FUNCTION__);
		checkDraw(tumDrawFilledBox(350,y,5,5,color), __FUNCTION__);
	}
	for(int y=300; y<=y2-50;y+=5)
	{
		checkDraw(tumDrawFilledBox(250,y,5,5,color), __FUNCTION__);
		checkDraw(tumDrawFilledBox(350,y,5,5,color), __FUNCTION__);
	}
}

/**
 * Method: Drawing Level5
 * Parameter: x1 top left corner x, y1 top left corner y
 * x2 bottom right corner x, y2 bottom right corner y, wall true or false
 */
void level5(int x1, int y1, int x2, int y2, int wall){
	//Outside wall White or Grey
	level1(x1,y1,x2,y2,wall);
	//Wall color
	int color=White;

	//Teleport Items
	checkDraw(tumDrawFilledBox(150, 170, 5, 5, Purple), __FUNCTION__);
	checkDraw(tumDrawFilledBox(150, 330, 5, 5, Purple), __FUNCTION__);
	checkDraw(tumDrawFilledBox(250, 170, 5, 5, Purple), __FUNCTION__);
	checkDraw(tumDrawFilledBox(250, 330, 5, 5, Purple), __FUNCTION__);
	checkDraw(tumDrawFilledBox(350, 170, 5, 5, Purple), __FUNCTION__);
	checkDraw(tumDrawFilledBox(350, 330, 5, 5, Purple), __FUNCTION__);
	checkDraw(tumDrawFilledBox(450, 170, 5, 5, Purple), __FUNCTION__);
	checkDraw(tumDrawFilledBox(450, 330, 5, 5, Purple), __FUNCTION__);

	//Wall
	for(int i=0;i<305;i+=5){
		checkDraw(tumDrawFilledBox(100, 100+i, 5, 5, color), __FUNCTION__);
		checkDraw(tumDrawFilledBox(200, 100+i, 5, 5, color), __FUNCTION__);
		checkDraw(tumDrawFilledBox(300, 100+i, 5, 5, color), __FUNCTION__);
		checkDraw(tumDrawFilledBox(400, 100+i, 5, 5, color), __FUNCTION__);
		checkDraw(tumDrawFilledBox(500, 100+i, 5, 5, color), __FUNCTION__);
	}
}

/**
 * Method: Drawing Level6
 * Parameter: x1 top left corner x, y1 top left corner y
 * x2 bottom right corner x, y2 bottom right corner y, wall true or false
 */
void level6(int x1, int y1, int x2, int y2, int wall,int fieldArray[39][29]){
	//Outside wall White or Grey
	level1(x1,y1,x2,y2,wall);
	//Wall color
	int color=White;

	for(int i = 0; i<39; i++) {
		for (int j = 0; j<29; j++) {
			if(fieldArray[i][j] == 3) {
				checkDraw(tumDrawFilledBox((i+11)*10, (j+11)*10, 5, 5, color), __FUNCTION__);
			}
		}
	}

}

/**
 * Method: Drawing GameOver if game is over
 */
void gameOverScreen(){
	int letter[500][400];
	//G
	letter[230][200]=1;
	letter[220][200]=1;
	letter[210][200]=1;
	letter[200][200]=1;
	letter[190][200]=1;
	letter[190][210]=1;
	letter[190][220]=1;
	letter[190][230]=1;
	letter[190][240]=1;
	letter[200][240]=1;
	letter[210][240]=1;
	letter[220][240]=1;
	letter[230][240]=1;
	letter[230][230]=1;
	letter[230][220]=1;
	letter[220][220]=1;
	letter[210][220]=1;
	//A
	letter[250][240]=1;
	letter[250][230]=1;
	letter[250][220]=1;
	letter[250][210]=1;
	letter[250][200]=1;
	letter[260][200]=1;
	letter[270][200]=1;
	letter[280][200]=1;
	letter[290][200]=1;
	letter[290][240]=1;
	letter[290][220]=1;
	letter[290][210]=1;
	letter[290][230]=1;
	letter[260][220]=1;
	letter[270][220]=1;
	letter[280][220]=1;
	//M
	letter[310][240]=1;
	letter[310][230]=1;
	letter[310][220]=1;
	letter[310][210]=1;
	letter[310][200]=1;
	letter[320][200]=1;
	letter[330][200]=1;
	letter[330][210]=1;
	letter[330][220]=1;
	letter[330][230]=1;
	letter[340][200]=1;
	letter[350][200]=1;
	letter[350][210]=1;
	letter[350][220]=1;
	letter[350][230]=1;
	letter[350][240]=1;
	//E
	letter[410][200]=1;
	letter[400][200]=1;
	letter[390][200]=1;
	letter[380][200]=1;
	letter[370][200]=1;
	letter[370][210]=1;
	letter[370][220]=1;
	letter[370][230]=1;
	letter[370][240]=1;
	letter[380][240]=1;
	letter[390][240]=1;
	letter[400][240]=1;
	letter[410][240]=1;
	letter[380][220]=1;
	letter[390][220]=1;
	letter[400][220]=1;
	//O
	letter[190][260]=1;
	letter[200][260]=1;
	letter[210][260]=1;
	letter[220][260]=1;
	letter[230][260]=1;
	letter[230][270]=1;
	letter[230][280]=1;
	letter[230][290]=1;
	letter[230][300]=1;
	letter[220][300]=1;
	letter[210][300]=1;
	letter[200][300]=1;
	letter[190][300]=1;
	letter[190][290]=1;
	letter[190][280]=1;
	letter[190][270]=1;
	letter[190][260]=1;
	//V
	letter[250][260]=1;
	letter[250][270]=1;
	letter[250][280]=1;
	letter[260][280]=1;
	letter[260][290]=1;
	letter[270][290]=1;
	letter[280][290]=1;
	letter[270][300]=1;
	letter[280][280]=1;
	letter[290][280]=1;
	letter[290][270]=1;
	letter[290][260]=1;
	//E
	letter[350][260]=1;
	letter[340][260]=1;
	letter[330][260]=1;
	letter[320][260]=1;
	letter[310][260]=1;
	letter[310][270]=1;
	letter[310][280]=1;
	letter[310][290]=1;
	letter[310][300]=1;
	letter[320][300]=1;
	letter[330][300]=1;
	letter[340][300]=1;
	letter[350][300]=1;
	letter[320][280]=1;
	letter[330][280]=1;
	letter[340][280]=1;
	//R
	letter[380][280]=1;
	letter[390][280]=1;
	letter[400][280]=1;
	letter[410][280]=1;
	letter[410][270]=1;
	letter[410][260]=1;
	letter[400][260]=1;
	letter[390][260]=1;
	letter[380][260]=1;
	letter[370][260]=1;
	letter[370][270]=1;
	letter[370][280]=1;
	letter[370][290]=1;
	letter[370][300]=1;
	letter[390][290]=1;
	letter[400][290]=1;
	letter[400][300]=1;
	letter[410][300]=1;

	if(a!=410){
		while(b<150){
			if(letter[100+a][180+b]){
				checkDraw(tumDrawFilledBox(100+a,180+b,25,25,Black), __FUNCTION__);
			}
			else{
				checkDraw(tumDrawFilledBox(100+a,180+b,5,5,White), __FUNCTION__);
			}
			b+=10;
		}
		b=0;
		a+=10;
	}
}
