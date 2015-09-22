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

#include "core.h"
#include "hypermath/hypermath.h"
#include "dynvector.h"
#include "aux.h"
#include <string.h>
#include <assert.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

/* Utils */

static void push_everything(CT_Texture* target, 
		     float* colour, 
		     float* pos, float scale, float rotation,
		     CT_BlendMode blend_mode)
{
	ct_target_push(target);
	ct_colour_push(colour);
	ct_translation_push(pos, scale, rotation);
	ct_blend_mode_push(blend_mode);
}

static void pop_everything()
{
	ct_target_pop();
	ct_colour_pop();
	ct_translation_pop();
	ct_blend_mode_pop();
}

/* Constants */

static float colour_white[4] = { 1, 1, 1, 1 };

/* Error */

const char* ct_get_error()
{
	return SDL_GetError();
}

void ct_set_error(const char* str)
{
	SDL_SetError(str);
}


/* Default shader */

static const char* vertex_shader_source = 
	"#version 330\n"
	"layout (location = 0) in vec2 vertex; "
	"layout (location = 1) in vec2 coord; "
	"out vec4 f_colour; "
	"out vec2 f_coord; "
	"uniform mat4 modelview; "
	"uniform mat4 projection; "
	"uniform vec4 colour; "
	"void main() { "
		"gl_Position = projection * modelview * vec4(vertex, 0, 1); "
		"f_coord = coord; "
		"f_colour = colour; "
	"}";

static const char* fragment_shader_source = 
	"#version 330\n"
	"uniform sampler2D texture; "
	"in vec4 f_colour; "
	"in vec2 f_coord; "
	"out vec4 fragment; "
	"void main() { "
		"fragment = texture2D(texture, f_coord.st) * f_colour; "
 	"}";

typedef struct
{
	unsigned gl_program_id;
	unsigned gl_vertex_id;
	unsigned gl_fragment_id;
} CT_Shader;

static GLuint compile_shader(const char* source, GLuint type, int* success)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, (const GLchar**)&source, 0);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, success);
	if (*success == GL_FALSE)
	{
	        GLint log_length = 0;
	        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
		char log_string[log_length];
		glGetShaderInfoLog(shader, log_length, &log_length, log_string);
		fprintf(stderr, "GLSL error: %s\n", log_string); 
		glDeleteShader(shader);
		*success = 0;
		return 0;
 	}
	CHECK_GL();
	return shader;
}

static GLuint create_shader_program(GLuint vertex, GLuint fragment, int* success)
{
	GLuint prog = glCreateProgram();
	glAttachShader(prog, vertex);
	glAttachShader(prog, fragment);
	glLinkProgram(prog);
	glGetProgramiv(prog, GL_LINK_STATUS, success);
	if (*success == GL_FALSE)
	{
		ct_set_error("Could not create shader program.");
		glDeleteProgram(prog);
		*success = 0;
		return 0;
	}
	glUseProgram(prog);
	glActiveTexture(GL_TEXTURE0);
	CHECK_GL();
	return prog;
}

static CT_Shader* shader_create(const char* vertex_source,
			 const char* fragment_source)
{
	int success;
	GLuint vertex = compile_shader(vertex_source,
				       GL_VERTEX_SHADER,
				       &success);
	if (!success) return NULL;
	GLuint fragment = compile_shader(fragment_source,
					 GL_FRAGMENT_SHADER,
					 &success);
	if (!success) return NULL;
	GLuint program  = create_shader_program(vertex, fragment, &success);
	if (!success) return NULL;

	CT_Shader* shader = smalloc(sizeof(CT_Shader));
	shader->gl_vertex_id   = vertex;
	shader->gl_fragment_id = fragment;
	shader->gl_program_id  = program;
	CHECK_GL();
	return shader;
}

static void shader_free(CT_Shader* shader)
{
	free(shader);
}


static void shader_upload_colour(CT_Shader* shader, float* colour)
{
	glUniform4fv(glGetUniformLocation(shader->gl_program_id, "colour"),
		     1, colour);
	CHECK_GL();
}

static void shader_upload_modelview_matrix(CT_Shader* shader, float* matrix)
{
	glUniformMatrix4fv(glGetUniformLocation(shader->gl_program_id, "modelview"),
			   1, GL_FALSE, matrix);
	CHECK_GL();
}

static void shader_upload_projection_matrix(CT_Shader* shader, float* matrix)
{
	glUniformMatrix4fv(glGetUniformLocation(shader->gl_program_id, "projection"),
			   1, GL_FALSE, matrix);
	CHECK_GL();
}


static CT_Shader* _default_shader;

static struct
{
	CT_Shader* stack[CT_STACK_SIZE];
	unsigned size;
} shader_stack;

static void shader_push(CT_Shader* shader)
{
	if (shader_stack.size >= CT_STACK_SIZE)
	{
		/* Stack overflow, resetting stack to prevent
		   crashing if this error is ignored. */
		ct_set_error("Stack overflow");
		shader_stack.size = 0;
	}
	shader_stack.stack[shader_stack.size++] = shader;
	glUseProgram(shader->gl_program_id);
}

void ct_push_default_shader()
{
	shader_push(_default_shader);
}

void ct_shader_pop()
{
	/* First item is default shader pushed by ct_window_init() */
	if (shader_stack.size == 1) 
	{
		/* Stack underflow, setting stack to one
		   so it will set to default value if this
		   error is ignored. */
		ct_set_error("Stack underflow");
		shader_stack.size = 1;
	}
	shader_stack.size--;
}

static CT_Shader* current_shader()
{
	return shader_stack.stack[shader_stack.size-1];
}


/* Window */

static CT_Window window = { NULL, "Coati", 0, 1 };

static CT_Texture _ct_screen_texture;

int ct_window_init()
{
	/* Initialise SDL */
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) return 1;

	/* Open OpenGL context */
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
//	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	window.sdl_window = SDL_CreateWindow(window.title, 
					     SDL_WINDOWPOS_UNDEFINED,
					     SDL_WINDOWPOS_UNDEFINED,
					     800, 600,
					     SDL_WINDOW_OPENGL);
	SDL_GL_CreateContext(window.sdl_window);

	/*  Initialise Glew */
	GLint err = glewInit();
	if (err != GLEW_OK)
	{
		char str[1024];
		sprintf(str, "%s\n", glewGetErrorString(err));
		ct_set_error(str);
		return 1;
	}

	/* Initialise Default Shader */
	_default_shader = shader_create(
		vertex_shader_source, fragment_shader_source);
	if (!_default_shader)
	{
		ct_set_error("Could not create default shader.");
		return 1;
	}
	shader_upload_colour(_default_shader, colour_white);

	/* Make sure first shader is always the default shader */
	/* This one cannot be removed by ct_shader_pop() */
	ct_push_default_shader();

	CHECK_GL();
	return 0;
}

void ct_window_quit()
{
	SDL_DestroyWindow(window.sdl_window);
	shader_free(_default_shader);
	SDL_Quit();
}

void ct_window_resolution_set(unsigned* xy)
{
	window.is_size_changed = 1;
	SDL_SetWindowSize(window.sdl_window, ((int*)xy)[0], ((int*)xy)[1]);
}

void ct_window_resolution(unsigned* ret)
{
	SDL_GetWindowSize(window.sdl_window, (int*)ret, (int*)ret+1);
}

void ct_window_res_set(int x, int y)
{
}

void ct_window_fullscreen_set(int fullscreen)
{
	SDL_SetWindowFullscreen(window.sdl_window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
	window.fullscreen = fullscreen;
}

int ct_window_fullscreen()
{
	return window.fullscreen;
}
	
int window_is_fullscreen()
{
	return window.fullscreen;
}

CT_Texture* ct_screen_texture()
{
	if (window.is_size_changed) {
		SDL_GetWindowSize(window.sdl_window,
				  (int*)&_ct_screen_texture.w,
				  (int*)&_ct_screen_texture.h);
	}
	return &_ct_screen_texture;
}

void ct_window_update()
{
	SDL_GL_SwapWindow(window.sdl_window);
}

void ct_window_clear(float* colour)
{
	ct_texture_clear(ct_screen_texture(), colour);
}

/* Image */

static CT_Image* image_alloc(SDL_Surface* sur)
{
	CT_Image* image = smalloc(sizeof(CT_Image));
	image->sdl_surface = sur;
	return image;
}

CT_Image* ct_image_load(const char* filename)
{
	SDL_Surface* sur = IMG_Load(filename);
	if (sur == NULL)
	{
		char str[1024];
		sprintf(str, "Could not load image file: %s", filename);
		ct_set_error(str);
		return NULL;
	}
	return image_alloc(sur);
}

CT_Image* ct_image_create(unsigned w, unsigned h)
{
	SDL_Surface* sur = SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0);
	if (sur == NULL)
	{
		fprintf(stderr, "Error could not create image of size %d, %d.\n", w, h);
		return NULL;
	}
	return image_alloc(sur);
}

void ct_image_free(CT_Image* image)
{
	SDL_FreeSurface(image->sdl_surface);
	free(image);
}

unsigned ct_image_bpp(CT_Image* image)
{
	return image->sdl_surface->format->BytesPerPixel;
}

static unsigned ct_image_gl_format(CT_Image* image)
{
	switch(ct_image_bpp(image))
	{
	case 4: return SDL_BYTEORDER == SDL_BIG_ENDIAN ? GL_BGRA : GL_RGBA;
	case 3: return SDL_BYTEORDER == SDL_BIG_ENDIAN ? GL_BGR  : GL_RGB;
	}
	assert(0);
}

void ct_image_size(CT_Image* image, float* vect)
{
	vect[0] = (float)image->sdl_surface->w;
	vect[1] = (float)image->sdl_surface->h;
}

/* Colour */

static struct
{
	float stack[CT_STACK_SIZE*4];
	unsigned size;
} colour_stack;

void ct_colour_push(float* colour)
{
	if (colour_stack.size >= CT_STACK_SIZE)
	{
		/* Stack overflow, resetting stack to prevent
		   crashing if this error is ignored. */
		ct_set_error("Stack overflow");
		colour_stack.size = 0;
	}
	memcpy(colour_stack.stack+(colour_stack.size*4),
	       colour,
	       sizeof(float)*4);
	colour_stack.size++;
	shader_upload_colour(current_shader(), colour);
}

void ct_colour_pop()
{
	if (colour_stack.size == 0)
	{
		/* Stack underflow, setting stack to one
		   so it will set to default value if this
		   error is ignored. */
		ct_set_error("Stack underflow");
		colour_stack.size = 1;
	}
	colour_stack.size--;
	if (colour_stack.size == 0)
	{
		shader_upload_colour(current_shader(), colour_white);
	} else
	{
		shader_upload_colour(current_shader(), colour_stack.stack+(colour_stack.size*4));
	}
}

/* Blending */

static struct
{
	CT_BlendMode stack[CT_STACK_SIZE];
	unsigned size;
} blend_stack;

static void set_blend_mode(CT_BlendMode mode)
{
	if (blend_stack.size == 0)
	{
		glEnable(GL_BLEND);
	}
	switch(mode)
	{
	case CT_BLEND_MODE_NORMAL:
		glBlendFunc(GL_ONE, GL_ZERO);
		break;
	case CT_BLEND_MODE_TRANS:
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case CT_BLEND_MODE_ADD:
		glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case CT_BLEND_MODE_ONE_ONE:
		glBlendFunc(GL_ONE, GL_ONE);
		break;
	}
}

void ct_blend_mode_push(CT_BlendMode mode)
{
	if (blend_stack.size >= CT_STACK_SIZE)
	{
		/* Stack overflow, resetting stack to prevent
		   crashing if this error is ignored. */
		ct_set_error("Stack overflow");
		colour_stack.size = 0;
	}
	blend_stack.stack[blend_stack.size++] = mode;
	glEnable(GL_BLEND);
	set_blend_mode(mode);
}

void ct_blend_mode_pop()
{
	if (blend_stack.size == 0)
	{
		/* Stack underflow, setting stack to one
		   so it will set to default value if this
		   error is ignored. */
		ct_set_error("Stack underflow");
		blend_stack.size = 1;
	}
	blend_stack.size--;
	if (blend_stack.size == 0)
	{
		set_blend_mode(CT_BLEND_MODE_NORMAL);
	} else {
		set_blend_mode(blend_stack.stack[blend_stack.size]);
	}

}

/* Texture */

static CT_Texture* current_target();

static GLuint create_buffer(GLuint tex_id)
{
	GLuint buf_id; glGenFramebuffers(1, &buf_id);
	glBindFramebuffer(GL_FRAMEBUFFER, buf_id);
	glFramebufferTexture2D(GL_FRAMEBUFFER,
			       GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D, tex_id, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, current_target()->gl_buffer_id);
	CHECK_GL();
	return buf_id;
}

static CT_Texture* new_texture(unsigned w, unsigned h)
{
	GLuint tex_id; glGenTextures(1, &tex_id);
	CT_Texture* tex = smalloc(sizeof(CT_Texture));
	tex->w = w;
	tex->h = h;
	/**/
	glBindTexture(GL_TEXTURE_2D, tex_id);
	/* Use repeat for wrapping-mode */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);	
	/* Don't use mipmapping */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	/**/
	tex->gl_texture_id = tex_id;
	tex->gl_buffer_id  = create_buffer(tex_id);
	CHECK_GL();
	return tex;
}

static CT_Texture* texture_init(CT_Image* image, unsigned format)
{
	void* pixels = image->sdl_surface->pixels;
	unsigned w = image->sdl_surface->w;
	unsigned h = image->sdl_surface->h;
	CT_Texture* tex = new_texture(w, h);
	glBindTexture(GL_TEXTURE_2D, tex->gl_texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
		     w, h,
		     0, format, GL_UNSIGNED_BYTE, pixels);
	return tex;
}

CT_Texture* ct_image_to_texture(CT_Image* image)
{
	return texture_init(image, ct_image_gl_format(image));
}

CT_Texture* ct_texture_create(unsigned w, unsigned h)
{
	CT_Image* image = ct_image_create(w, h);
	CT_Texture* tex = ct_image_to_texture(image);
	if (!image) return NULL; /* ct_image_create already prints error message. */
	ct_image_free(image);
	return tex;
}

CT_Texture* ct_texture_load(const char* filename)
{
	CT_Image* image = ct_image_load(filename);
	if (!image) return NULL; /* ct_image_load already prints error message. */
	CT_Texture* tex = ct_image_to_texture(image);
	ct_image_free(image);
	return tex;
}

CT_Texture* ct_texture_copy(CT_Texture* texture)
{
	/* TODO */
	CT_Texture* tex = ct_texture_create(texture->w, texture->h);
	/**/
	CT_Transformation trans = {
		{ 0, 1, 0, 1 },
		{ 0, 1, 0, 1 },
		{ 0, 0 }, 0,
		-1, -1 };
	float idem[16]; hpmIdentityMat4(idem);
	float pos[] = {0, 0};
	push_everything(tex, 
			colour_white, 
			pos, 1, 0,
			CT_BLEND_MODE_NORMAL);
	ct_texture_render(texture, &trans);
	pop_everything();
	return tex;
}

void ct_texture_free(CT_Texture* tex)
{
	glDeleteTextures(1, &tex->gl_texture_id);
	glDeleteBuffers(1, &tex->gl_buffer_id);
	CHECK_GL();
	free(tex);
}

int ct_is_texture_screen(CT_Texture* tex)
{
	return tex->gl_buffer_id == 0;
}

void ct_texture_size(CT_Texture* tex, float* vect)
{
	if (ct_is_texture_screen(tex)) {
	}
	vect[0] = (float)tex->w;
	vect[1] = (float)tex->h;
}

static float project_matrix[16];

static void texture_bind(CT_Texture* tex)
{
	glViewport(0, 0, tex->w, tex->h);
	hpmOrthoFloat(1, ct_is_texture_screen(tex) ? -1 : 1, -100, 100, project_matrix);
	/* Put origin origin at 0,0 */
	hpmTranslation(-.5, -.5, 0, project_matrix);
	hpmScale2D(2, ct_is_texture_screen(tex) ? -2 : 2, project_matrix);
	shader_upload_projection_matrix(current_shader(), project_matrix);
	glBindFramebuffer(GL_FRAMEBUFFER, tex->gl_buffer_id);
	CHECK_GL();
}


void ct_target_push(CT_Texture* tex);

void ct_target_pop();

void ct_texture_clear(CT_Texture* tex, float* colour)
{
	ct_target_push(tex);
	glClearBufferfv(GL_COLOR, 0, colour);
	ct_target_pop();
	CHECK_GL();
}

static GLushort rect_index_order[] = { 0, 1, 2, 0, 2, 3 };

static void vertex_data(CT_Transformation* tran, float* data);

static float* current_matrix();

void ct_texture_render(CT_Texture* tex, CT_Transformation* trans)
{
	float data[16]; vertex_data(trans, data);
	glUseProgram(current_shader()->gl_program_id);
	glBindTexture(GL_TEXTURE_2D, tex->gl_texture_id);
	shader_upload_modelview_matrix(current_shader(), current_matrix());
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, data);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 16, data+2);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, rect_index_order);
	CHECK_GL();
}

/* Target */

static struct
{
	CT_Texture* stack[CT_STACK_SIZE];
	unsigned size;
} target_stack;

static CT_Texture* current_target()
{
	return target_stack.size
		? target_stack.stack[target_stack.size-1]
		: ct_screen_texture();
}

void ct_target_push(CT_Texture* tex)
{
	if (target_stack.size >= CT_STACK_SIZE)
	{
		/* Stack overflow, resetting stack to prevent
		   crashing if this error is ignored. */
		ct_set_error("Stack overflow");
		target_stack.size = 0;
	}
	texture_bind(tex);
	target_stack.stack[target_stack.size++] = tex;
}

void ct_target_pop()
{
	if (target_stack.size == 0)
	{
		/* Stack underflow, setting stack to one
		   so it will set to default value if this
		   error is ignored. */
		ct_set_error("Stack underflow");
		target_stack.size = 1;
	}
	target_stack.size--;
	texture_bind(target_stack.size
		     ? target_stack.stack[target_stack.size-1]
		     : ct_screen_texture());
}

/* Batch */

CT_Batch* ct_batch_create(unsigned size_hint)
{
	CT_Batch* batch = smalloc(sizeof(CT_Batch));
	batch->vector  = dv_vector_new(16, size_hint);
	batch->indices = smalloc(sizeof(unsigned short)*size_hint*6);
	unsigned i;
	for (i=0; i<size_hint; i++) {
		batch->indices[(i*6)+0] = 0 + (i*4);
		batch->indices[(i*6)+1] = 1 + (i*4);
		batch->indices[(i*6)+2] = 2 + (i*4);
		batch->indices[(i*6)+3] = 0 + (i*4);
		batch->indices[(i*6)+4] = 2 + (i*4);
		batch->indices[(i*6)+5] = 3 + (i*4);
	}
	return batch;
}

void ct_batch_free(CT_Batch* batch)
{
	dv_vector_free(batch->vector);
	free(batch->indices);
	free(batch);
}

unsigned ct_batch_push(CT_Batch* batch, CT_Transformation* trans)
{
	
	unsigned grown_by;
	float data[16]; vertex_data(trans, data);
	unsigned id = dv_vector_push(batch->vector, data, &grown_by);
	/* If vector has grown, grow indices array with it. */
	if (grown_by)
	{
		unsigned size = dv_vector_current_capacity(batch->vector);
		batch->indices = srealloc(batch->indices, 
					  sizeof(unsigned short)*size*6);
		unsigned i;
		/* Populate new indicies */
		for (i=size-grown_by; i<size; i++) {
			batch->indices[(i*6)+0] = 0 + (i*4);
			batch->indices[(i*6)+1] = 1 + (i*4);
			batch->indices[(i*6)+2] = 2 + (i*4);
			batch->indices[(i*6)+3] = 0 + (i*4);
			batch->indices[(i*6)+4] = 2 + (i*4);
			batch->indices[(i*6)+5] = 3 + (i*4);
		}
	}
	return id;
}

void ct_batch_remove(CT_Batch* batch, unsigned id)
{
	dv_vector_remove(batch->vector, id);
}

void ct_batch_change(CT_Batch* batch, unsigned id, CT_Transformation* trans)
{
	vertex_data(trans, dv_vector_ref(batch->vector, id));
}

void ct_batch_render(CT_Batch* batch, CT_Texture* atlas)
{
	DV_Vector* vector = batch->vector;
	glUseProgram(current_shader()->gl_program_id);
	glBindTexture(GL_TEXTURE_2D, atlas->gl_texture_id);
	shader_upload_modelview_matrix(current_shader(), current_matrix());
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, vector->data);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 16, vector->data+2);
	glDrawElements(GL_TRIANGLES, vector->size*6, GL_UNSIGNED_SHORT, batch->indices);
	CHECK_GL();
}

unsigned ct_batch_size(CT_Batch* batch)
{
	return batch->vector->size;
}

/* Font */

CT_Font* ct_font_load(const char* filename)
{
	FILE* file = fopen(filename, "r");
	if(!file)
	{
		char str[1024];
		sprintf(str, "%s: file not found.", filename);
		ct_set_error(str);
		return NULL;
	}
	SDL_RWops* rw = SDL_RWFromFP(file, SDL_TRUE);
	if (!rw)
	{
		char str[1024];
		sprintf(str, "could not load file %s.", filename);
		ct_set_error(str);
		return NULL;
	}
	CT_Font* font = smalloc(sizeof(CT_Font));
	font->file = file;
	font->rw = rw;
	font->first = NULL;
	return font;
}

void ct_font_free(CT_Font* font)
{
	SDL_FreeRW(font->rw);
	/* Free font map */
	struct CT_FontMapLink* last = font->first, *tmp;
	if (last)
	{
		while (last)
		{
			tmp = last;
			last = last->next;
			free(tmp);
		}
	}
	free(font);
}

static TTF_Font* load_font_size(CT_Font* font, unsigned size)
{
	if (!TTF_WasInit()) TTF_Init();
	TTF_Font* ttf_font = TTF_OpenFontRW(font->rw, 0, size);
	if (!ttf_font)
	{
		ct_set_error(TTF_GetError());
		return NULL;
	}
	struct CT_FontMapLink* link = smalloc(sizeof(struct CT_FontMapLink));
	link->size  = size;
	link->value = ttf_font;
	link->next  = NULL;
	struct CT_FontMapLink* last = font->first;
	while (last)
	{
		last = last->next;
	}

	last = smalloc(sizeof(struct CT_FontMapLink));
	last->next = link;
	return ttf_font;
}

static TTF_Font* get_ttf_font(CT_Font* font, unsigned size)
{
	struct CT_FontMapLink* last = font->first;
	TTF_Font* ttf_font = NULL;
	while (last)
	{
		if (last->size == size)
		{
			ttf_font = last->value;
			break;
		}	
		last = last->next;
	}
	return ttf_font ? ttf_font : load_font_size(font, size);
}

extern CT_Texture* ct_string_to_texture(CT_Font* font,
					unsigned size,
					const char* string,
					float* colour)
{
	TTF_Font* ttf_font = get_ttf_font(font, size);
	if (!ttf_font) return NULL; /* Error already reported. */
	SDL_Color sdl_colour = { 255*colour[0],
				 255*colour[1],
				 255*colour[2],
				 255*colour[3] };
	int w, h;
	TTF_SizeText(ttf_font, string, &w, &h);
	SDL_Surface* sur = TTF_RenderText_Blended(ttf_font, string, sdl_colour);
	CT_Image* image = image_alloc(sur);
	CT_Texture* tex = ct_image_to_texture(image);
	ct_image_free(image);
	return tex;
}

/* Transformation */

static void vertex_data(CT_Transformation* tran, float* data)
{
	float l1 = tran->dst_rect[0];
	float r1 = tran->dst_rect[1];
	float t1 = tran->dst_rect[2];
	float b1 = tran->dst_rect[3];
	/**/
	float l2 = tran->src_rect[0];
	float r2 = tran->src_rect[1];
	float t2 = tran->src_rect[2];
	float b2 = tran->src_rect[3];
	/**/
	float px = tran->origin[0];
	float py = tran->origin[1];
	/**/
	if (tran->flip_h > 0) swap_float(&r2, &l2);
	if (tran->flip_v > 0) swap_float(&b2, &t2);
	/**/
	if (zeroish(tran->rotation))
	{
		float new_data[] = {
			l1-px, t1-py, l2, t2,
			r1-px, t1-py, r2, t2,
			r1-px, b1-py, r2, b2,
			l1-px, b1-py, l2, b2 };
		memcpy(data, new_data, sizeof(float)*16);
	} else
	{
		float ca = cos(tran->rotation);
		float sa = sin(tran->rotation);
		/**/
		float x1 = ((l1 - px) * ca) - ((t1 - py) * sa);
		float y1 = ((l1 - px) * sa) + ((t1 - py) * ca);
		float x2 = ((r1 - px) * ca) - ((t1 - py) * sa);
		float y2 = ((r1 - px) * sa) + ((t1 - py) * ca);
		float x3 = ((r1 - px) * ca) - ((b1 - py) * sa);
		float y3 = ((r1 - px) * sa) + ((b1 - py) * ca);
		float x4 = ((l1 - px) * ca) - ((b1 - py) * sa);
		float y4 = ((l1 - px) * sa) + ((b1 - py) * ca);
		float new_data[] = {
			x1+px, y1+py, l2, t2,
			x2+px, y2+py, r2, t2,
			x3+px, y3+py, r2, b2,
			x4+px, y4+py, l2, b2 };
		memcpy(data, new_data, sizeof(float)*16);
	}
}

/* Translation */

static struct
{
	float stack[16][CT_STACK_SIZE];
	unsigned size;
} matrix_stack;

float _current_matrix[16] = {
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1
};

float* current_matrix()
{
	return _current_matrix;
}

void ct_translation_push(float* position, float scale, float rotation)
{
	if (matrix_stack.size >= CT_STACK_SIZE)
	{
		/* Stack overflow, resetting stack to prevent
		   crashing if this error is ignored. */
		ct_set_error("Stack overflow");
		matrix_stack.size = 0;
	}
	float* trans = matrix_stack.stack[++matrix_stack.size];

	hpmIdentityMat4(trans);
	hpmTranslate(position[0]-.5, position[1]-.5, 0, trans);
	hpmRotateZ(rotation, trans);
	hpmScale2D(scale, scale, trans);
	hpmTranslate(.5, .5, 0, trans);
	float r[16];
	hpmMultMat4(_current_matrix, trans, r);
	memcpy(_current_matrix, r, sizeof(float)*16);
}

void ct_translation_pop()
{
	
	if (matrix_stack.size == 0)
	{
		/* Stack underflow, setting stack to one
		   so it will set to default value if this
		   error is ignored. */
		ct_set_error("Stack underflow");
		matrix_stack.size = 1;
	}
	float* trans = matrix_stack.stack[matrix_stack.size];
	matrix_stack.size--;

	float r1[16];
	float r2[16];
	hpmInverse(trans, r1);
	hpmMultMat4(_current_matrix, r1, r2);
	memcpy(_current_matrix, r2, sizeof(float)*16);
}
