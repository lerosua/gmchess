/*
 * Sound.h
 * Copyright (C) lerosua 2009 <lerosua@gmail.com>
 * 
 */


#ifndef __GM_SOUND_H__
#define __GM_SOUND_H__

#define SOUND_DIR DATA_DIR"/sound/"

enum SOUND_EVENTS
{
	SND_NULL,
	SND_EAT,
	SND_CHECK,
	SND_MOVE,
	SND_CHOOSE,
	SND_WIN,
	SND_LOSS,
	SND_10,
	SND_9,
	SND_8,
	SND_7,
	SND_6,
	SND_5,
	SND_4,
	SND_3,
	SND_2,
	SND_1,
	SND_0
};

namespace CSound
{
	void play (SOUND_EVENTS event);
	void play_file(const char* filename);
};


#endif
