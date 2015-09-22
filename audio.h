#ifndef __audio_h__
#define __audio_h__

typedef struct Mix_Chunk Mix_Chunk;
typedef struct _Mix_Music Mix_Music;

typedef struct _CT_Sample
{
	Mix_Chunk* mix_chunk;
} CT_Sample;

typedef int CT_Channel;

typedef struct _CT_Track
{
	Mix_Music* mix_music;
} CT_Track;

/* Sample */

extern CT_Sample* ct_sample_load(const char* filename);

extern void ct_sample_free(CT_Sample* sample);

/* 
   Sample radius
   The radius in which a sample can be heard. The volume of a sample
   will decrease linearly. 
*/
extern void ct_sample_radius_set(float value);

extern float ct_sample_radius();

/* Channel */

extern CT_Channel ct_sample_play(CT_Sample* sample, float* position, int loop);

extern int ct_is_channel_active(CT_Channel channel);

extern void ct_channel_position_get(CT_Channel channel, float* position);

extern void ct_channel_position(CT_Channel channel, float* ret);

extern void ct_channel_stop(CT_Channel channel, int fadeout_ms);

/* Track */

extern CT_Track* ct_track_load(const char* filename);

extern void ct_track_free(CT_Track* track);

extern void ct_track_play(CT_Track* track, int fadein_ms);

extern void ct_track_stop(CT_Track* track, int fadeout_ms);

extern int ct_is_track_playing(CT_Track* track);

#endif /* __audio_h__ */
