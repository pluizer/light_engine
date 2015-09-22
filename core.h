/**
Copyright (c) 2014 Richard van Roy (pluizer)

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
**/

#ifndef __core_h__
#define __core_h__

#define CT_STACK_SIZE 32

#include <stdio.h>

typedef struct _DV_Vector DV_Vector;
typedef struct SDL_Surface SDL_Surface;
typedef struct SDL_Window SDL_Window;
typedef struct SDL_RWops SDL_RWops;
typedef struct _TTF_Font TTF_Font;

typedef struct _CT_Image
{
	SDL_Surface* sdl_surface;
} CT_Image;

typedef struct _CT_Texture
{
	unsigned w, h;
	unsigned gl_texture_id;
	unsigned gl_buffer_id;
} CT_Texture;

typedef struct
{
	SDL_Window* sdl_window;
	const char* title;
	int fullscreen;
	int is_size_changed;
} CT_Window;

typedef struct _CT_Batch
{
	DV_Vector* vector;
	unsigned short* indices;
} CT_Batch;

typedef struct _CT_Transformation
{
	float src_rect[4];
	float dst_rect[4];
	float origin[2];
	float rotation;
	float flip_h, flip_v; /* positive = true, negative = false */
} CT_Transformation;

typedef struct _CT_Font
{
	FILE* file;
	SDL_RWops* rw;

	struct CT_FontMapLink
	{
		unsigned size;
		TTF_Font* value;
		struct CT_FontMapLink* next;
	} font_map;

	struct CT_FontMapLink* first;
	
} CT_Font;

/* Error */

extern const char* ct_get_error();

extern void ct_set_error(const char* str);

/* Shader */

extern void ct_push_default_shader();

extern void ct_shader_pop();

/* Window */

extern int ct_window_init();

extern void ct_window_quit();

extern void ct_window_resolution_set(unsigned* xy);

extern void ct_window_resolution(unsigned* ret);

extern void ct_window_fullscreen_set(int fullscreen);

extern int ct_window_fullscreen();

extern void ct_window_update();

extern void ct_window_clear(float* colour);

/* Image */

extern CT_Image* ct_image_load(const char* filename);

extern CT_Image* ct_image_create(unsigned w, unsigned h);

extern void ct_image_free(CT_Image* image);

extern void ct_image_size(CT_Image* image, float* vect);

/* Colour */

extern void ct_colour_push(float* colour);

extern void ct_colour_pop();

/* Blending */

typedef enum _CT_BlendMode
{
	CT_BLEND_MODE_NORMAL,
	CT_BLEND_MODE_ADD,
	CT_BLEND_MODE_TRANS,
	CT_BLEND_MODE_ONE_ONE
} CT_BlendMode;

extern void ct_blend_mode_push(CT_BlendMode mode);

extern void ct_blend_mode_pop();

/* Texture */

extern CT_Texture* ct_image_to_texture(CT_Image* image);

extern CT_Texture* ct_texture_create(unsigned w, unsigned h);

extern CT_Texture* ct_texture_copy(CT_Texture* texture);

extern CT_Texture* ct_texture_load(const char* filename);

extern void ct_texture_free(CT_Texture* tex);

extern int ct_is_texture_screen(CT_Texture* tex);

extern void ct_texture_size(CT_Texture* tex, float* vect);

extern void ct_texture_clear(CT_Texture* tex, float* colour);

extern void ct_texture_render(CT_Texture* tex, CT_Transformation* trans);

/* Target */

extern void ct_target_push(CT_Texture* tex);

extern void ct_target_pop();

/* Batch */

extern CT_Batch* ct_batch_create(unsigned size_hint);

extern void ct_batch_free(CT_Batch* batch);

extern unsigned ct_batch_push(CT_Batch* batch, CT_Transformation* trans);

extern void ct_batch_remove(CT_Batch* batch, unsigned id);

extern void ct_batch_change(CT_Batch* batch, unsigned id, CT_Transformation* trans);

extern void ct_batch_render(CT_Batch* batch, CT_Texture* atlas);

extern unsigned ct_batch_size(CT_Batch* batch);

/* Font */

extern CT_Font* ct_font_load(const char* filename);

extern void ct_font_free(CT_Font* font);

/* TODO: Use colour stack? */
extern CT_Texture* ct_string_to_texture(CT_Font* font,
					unsigned size,
					const char* string,
					float* colour);

/* Translation */
extern void ct_translation_push(float* position, float scale, float rotation);

extern void ct_translation_pop();

#endif /* __core_h__ */	
