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

#include "audio.h"
#include <SDL2/SDL_mixer.h>
#include <math.h>

/* No need to bring in the whole of core.h */
extern const char* ct_get_error();
extern void ct_set_error(const char* str);
extern void* smalloc(size_t size);

#define CAP(v,min,max) ((v)>(max))?(max):((v)<(min))?(min):(v)

static int is_sound_inited = 0;

static int init_sound()
{
	if (!is_sound_inited)
	{
		int flags = MIX_INIT_OGG;
		int is_inited = Mix_Init(flags);
		if (is_inited != flags)
		{
			char str[1024];
			sprintf(str, "Could init mixer: %s", Mix_GetError());
			ct_set_error(str);
			return 1;
		}
		if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == -1)
		{
			printf("!");
			char str[1024];
			sprintf(str, "Could init mixer: %s", Mix_GetError());
			ct_set_error(str);
			return 1;
		}
	}
	is_sound_inited = 1;
	return 0;
}

CT_Sample* ct_sample_load(const char* filename)
{
	if (init_sound()) return NULL;
	Mix_Chunk* chunk = Mix_LoadWAV(filename);
	if (!chunk)
	{
		char str[1024];
		sprintf(str, "Could not load audio file: %s, %s", 
			filename, Mix_GetError());
		ct_set_error(str);
	}
	CT_Sample* sample = smalloc(sizeof(CT_Sample));
	sample->mix_chunk = chunk;
	return sample;
}

void ct_sample_free(CT_Sample* sample)
{
	Mix_FreeChunk(sample->mix_chunk);
	free(sample);
}

/* Sample radius */

static float sample_radius = .5;

void ct_sample_radius_set(float value)
{
	sample_radius = value;
}

float ct_sample_radius()
{
	return sample_radius;
}

static void calc_mix_panning(float* pos, Uint8* left, Uint8* right, Uint8* r_distance)
{
	/* FIXME?
	   Assumes 'microphones' are .5 units away from the centre if the camera.
	   Maybe this should scale when zooming out.
	*/
	float delta_x = pos[0] - .5;
	float delta_y = pos[1] - .5;
	float dist = sqrt(delta_x*delta_x + delta_y*delta_y);
	*r_distance = (Uint8)CAP((255.0 / sample_radius) * dist, 0, 255);
	*right = (Uint8)CAP((128 * (delta_x/.5)) + 128, 0, 255);
	*left = 255-*right;
	*right *= sample_radius - (CAP(dist, 0, 1)/sample_radius);
	*left  *= sample_radius - (CAP(dist, 0, 1)/sample_radius);

	printf("L: %f:%d, R: %f:%d\n", delta_x, (int)*left, delta_x, (int)*right);
}

/* Channel */

CT_Channel ct_sample_play(CT_Sample* sample, float* position, int loop)
{
	Uint8 left;
	Uint8 right;
	Uint8 dist;
	calc_mix_panning(position, &left, &right, &dist);
	CT_Channel channel = Mix_PlayChannel(-1, sample->mix_chunk, loop);
	Mix_SetPanning(channel, left, right);
	return channel;
}
