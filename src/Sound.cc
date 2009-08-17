/*
 * Sound.cc
 * Copyright (C) lerosua 2009 <lerosua@gmail.com>
 * 
 */

#include "Sound.h"
#include <string>
#include <glib.h>
using namespace std;


void
CSound::play (SOUND_EVENTS event)
{
	const char *filename;
	switch (event)
	{
		case SND_MOVE:
			filename = SOUND_DIR"move.wav";
			break;
		case SND_CHOOSE:
			filename = SOUND_DIR"choose.wav";
			break;
		case SND_CHECK:
			filename = SOUND_DIR"check.wav";
			break;
		case SND_EAT:
			filename = SOUND_DIR"eat.wav";
			break;
		default:
			filename = SOUND_DIR"";
			break;
	}
	play_file(filename);
}

void CSound::play_file(const char* filename)
{
	gchar* argv[3];
	argv[0] = "aplay";
	argv[1] = (gchar*)filename;
	//argv[1] = filename;
	argv[2] = NULL;
	GError* err;
	GSpawnFlags flas = (GSpawnFlags)(G_SPAWN_SEARCH_PATH |
		       	G_SPAWN_STDOUT_TO_DEV_NULL |
		       	G_SPAWN_STDERR_TO_DEV_NULL);
	g_spawn_async(NULL,
			argv,
			NULL,
			flas,
			NULL,
			NULL,
			NULL,
			&err);

}
