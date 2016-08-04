/*************************************************************************
 Author: Zhaoting Weng
 Created Time: Thu 14 Apr 2016 09:29:14 AM CST
 File Name: my_playback.h
 Description: 
 ************************************************************************/

#ifndef MY_PLAYBACK
#define MY_PLAYBACK

#include <alsa/asoundlib.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include <limits.h>

/**************
 * Build macros
 *************/
#define MY_PLAYBACK_DEBUG               // output debug info
#define MY_PLAYBACK_MAIN                // my_playback works as a process

/**************
 * Function
 *************/
void prepare_device(const char *device_name, snd_pcm_t **handle);
void playback(snd_pcm_t *handle, unsigned int duration);
#endif

