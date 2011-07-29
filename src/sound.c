/* $Id$ */

/** @file src/sound.c Sound routines. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "global.h"
#include "os/common.h"
#include "os/strings.h"

#include "sound.h"

#include "file.h"
#include "gui/gui.h"
#include "house.h"
#include "mt32mpu.h"
#include "opendune.h"
#include "string.h"
#include "tile.h"
#include "tools.h"
#include "unknown/unknown.h"


static void *g_variable_3E54[NUM_VOICES];
static uint32 g_variable_3E54_size[NUM_VOICES];
static const char *_currentMusic = NULL;        /*!< Currently loaded music file. */
static uint16 _spokenWords[NUM_SPEECH_PARTS];   /*!< Buffer with speech to play. */

static void Driver_Music_Play(int16 index, uint16 volume)
{
	Driver *music = &g_global->musicDriver;
	MSBuffer *musicBuffer = &g_global->musicBuffer;

	if (index < 0 || index > 120 || g_global->musicEnabled == 0) return;

	if (music->index == 0xFFFF) return;

	if (musicBuffer->index != 0xFFFF) {
		MPU_Stop(musicBuffer->index);
		MPU_ClearData(musicBuffer->index);
		musicBuffer->index = 0xFFFF;
	}

	{
		csip32 nullcsip;
		nullcsip.csip = 0x0;
		musicBuffer->index = MPU_SetData(music->content, index, musicBuffer->buffer, nullcsip);
	}

	MPU_Play(musicBuffer->index);
	MPU_SetVolume(musicBuffer->index, ((volume & 0xFF) * 90) / 256, 0);
}

static void Driver_Music_LoadFile(const char *musicName)
{
	Driver *music = &g_global->musicDriver;
	Driver *sound = &g_global->soundDriver;

	Driver_Music_Stop();

	if (music->index == 0xFFFF) return;

	if (music->content.csip == sound->content.csip) {
		music->content.csip     = 0x0;
		music->variable_1E.csip = 0x0;
		music->filename.csip    = 0x0;
		music->contentMalloced  = 0;
	} else {
		Driver_UnloadFile(music);
	}

	if (sound->filename.csip != 0x0 && musicName != NULL && strcasecmp(Drivers_GenerateFilename(musicName, music), (char *)emu_get_memorycsip(sound->filename)) == 0) {
		g_global->musicDriver.content         = g_global->soundDriver.content;
		g_global->musicDriver.variable_1E     = g_global->soundDriver.variable_1E;
		g_global->musicDriver.filename        = g_global->soundDriver.filename;
		g_global->musicDriver.contentMalloced = g_global->soundDriver.contentMalloced;

		return;
	}

	Driver_LoadFile(musicName, music);
}

/**
 * Plays a music.
 * @param index The index of the music to play.
 */
void Music_Play(uint16 musicID)
{
	csip32 nullcsip;
	nullcsip.csip = 0x0;

	if (musicID == 0xFFFF || musicID >= 38) return;

	if (g_table_musics[musicID].string != _currentMusic) {
		_currentMusic = g_table_musics[musicID].string;

		Driver_Music_Stop();
		Driver_Voice_Play(NULL, nullcsip, 0xFF, 0xFF);
		Driver_Music_LoadFile(NULL);
		Driver_Sound_LoadFile(NULL);
		Driver_Music_LoadFile(_currentMusic);
		Driver_Sound_LoadFile(_currentMusic);
	}

	Driver_Music_Play(g_table_musics[musicID].variable_04, 0xFF);
}

/**
 * Play a voice. Volume is based on distance to position.
 * @param voiceID Which voice to play.
 * @param position Which position to play it on.
 */
void Voice_PlayAtTile(int16 voiceID, tile32 position)
{
	uint16 index;
	uint16 volume;

	if (voiceID < 0) return;
	if (!g_global->soundsEnabled) return;
	assert(voiceID < 120);

	volume = 255;
	if (position.tile != 0) {
		volume = Tile_GetDistancePacked(g_minimapPosition, Tile_PackTile(position));
		if (volume > 64) volume = 64;

		volume = 255 - (volume * 255 / 80);
	}

	index = g_table_voiceMapping[voiceID];

	if (g_config.voiceDrv != 0 && index != 0xFFFF && g_variable_3E54[index] != NULL && g_table_voices[index].variable_04 >= g_global->variable_4060) {
		g_global->variable_4060 = g_table_voices[index].variable_04;
		memmove(emu_get_memorycsip(g_global->readBuffer), g_variable_3E54[index], g_variable_3E54_size[index]);

		Driver_Voice_Play(emu_get_memorycsip(g_global->readBuffer), g_global->readBuffer, g_global->variable_4060, volume);
	} else {
		Driver_Sound_Play(voiceID, volume);
	}
}

/**
 * Play a voice.
 * @param voiceID The voice to play.
 */
void Voice_Play(int16 voiceID)
{
	tile32 tile;

	tile.tile = 0;
	Voice_PlayAtTile(voiceID, tile);
}

/**
 * Load voices.
 * @param voiceSet Voice set to load.
 */
void Voice_LoadVoices(uint16 voiceSet)
{
	static uint16 currentVoiceSet = 0xFFFE;
	uint16 i;
	uint16 voice;

	if (g_config.voiceDrv == 0) return;

	for (voice = 0; voice < NUM_VOICES; voice++) {
		switch (g_table_voices[voice].string[0]) {
			case '%':
				if (g_config.language != LANGUAGE_ENGLISH || currentVoiceSet == voiceSet) {
					if (voiceSet != 0xFFFF && voiceSet != 0xFFFE) break;
				}

				free(g_variable_3E54[voice]);
				g_variable_3E54[voice] = NULL;
				break;

			case '+':
				if (voiceSet != 0xFFFF && voiceSet != 0xFFFE) break;

				free(g_variable_3E54[voice]);
				g_variable_3E54[voice] = NULL;
				break;

			case '-':
				if (voiceSet == 0xFFFF) break;

				free(g_variable_3E54[voice]);
				g_variable_3E54[voice] = NULL;
				break;

			case '/':
				if (voiceSet != 0xFFFE) break;

				free(g_variable_3E54[voice]);
				g_variable_3E54[voice] = NULL;
				break;

			case '?':
				if (voiceSet == 0xFFFF) break;

				/* No free() as there was never a malloc(). */
				g_variable_3E54[voice] = NULL;
				break;

			default:
				break;
		}
	}

	if (currentVoiceSet == voiceSet) return;

	for (voice = 0; voice < NUM_VOICES; voice++) {
		char filename[16];
		const char *str = g_table_voices[voice].string;
		switch (*str) {
			case '%':
				if (g_variable_3E54[voice] != NULL ||
						currentVoiceSet == voiceSet || voiceSet == 0xFFFF || voiceSet == 0xFFFE) break;

				switch (g_config.language) {
					case LANGUAGE_FRENCH: i = 'F'; break;
					case LANGUAGE_GERMAN: i = 'G'; break;
					default: i = g_table_houseInfo[voiceSet].prefixChar;
				}
				snprintf(filename, sizeof(filename), str, i);

				g_variable_3E54[voice] = Sound_Unknown0823(filename, &g_variable_3E54_size[voice]);
				break;

			case '+':
				if (voiceSet == 0xFFFF || g_variable_3E54[voice] != NULL) break;

				switch (g_config.language) {
					case LANGUAGE_FRENCH:  i = 'F'; break;
					case LANGUAGE_GERMAN:  i = 'G'; break;
					default: i = 'Z'; break;
				}
				snprintf(filename, sizeof(filename), str + 1, i);

				g_variable_3E54[voice] = Sound_Unknown0823(filename, &g_variable_3E54_size[voice]);
				break;

			case '-':
				if (voiceSet != 0xFFFF || g_variable_3E54[voice] != NULL) break;

				g_variable_3E54[voice] = Sound_Unknown0823(str + 1, &g_variable_3E54_size[voice]);
				break;

			case '/':
				if (voiceSet != 0xFFFE) break;

				g_variable_3E54[voice] = Sound_Unknown0823(str + 1, &g_variable_3E54_size[voice]);
				break;

			case '?':
				break;

			default:
				if (g_variable_3E54[voice] != NULL) break;

				g_variable_3E54[voice] = Sound_Unknown0823(str, &g_variable_3E54_size[voice]);
				break;
		}
	}
	currentVoiceSet = voiceSet;
}

/**
 * Start playing a sound sample.
 * @param index Sample to play.
 */
void Sound_StartSound(uint16 index)
{
	if (index == 0xFFFF || g_global->soundsEnabled == 0 || (int16)g_table_voices[index].variable_04 < (int16)g_global->variable_4060) return;

	g_global->variable_4060 = g_table_voices[index].variable_04;

	if (g_variable_3E54[index] != NULL) {
		memmove(emu_get_memorycsip(g_global->readBuffer), g_variable_3E54[index], g_variable_3E54_size[index]);
		Driver_Voice_Play(emu_get_memorycsip(g_global->readBuffer), g_global->readBuffer, 0xFF, 0xFF);
	} else {
		char filenameBuffer[16];
		const char *filename;

		filename = g_table_voices[index].string;
		if (filename[0] == '?') {
			snprintf(filenameBuffer, sizeof(filenameBuffer), filename + 1, g_playerHouseID < HOUSE_MAX ? g_table_houseInfo[g_playerHouseID].prefixChar : ' ');

			Driver_Voice_LoadFile(filenameBuffer, (void *)emu_get_memorycsip(g_global->readBuffer), g_global->readBufferSize);

			Driver_Voice_Play(emu_get_memorycsip(g_global->readBuffer), g_global->readBuffer, 0xFF, 0xFF);
		}
	}
}

/**
 * Output feedback about events of the game.
 * @param index Feedback to provide (\c 0xFFFF means do nothing, \c 0xFFFE means stop, otherwise a feedback code).
 * @note If sound is disabled, the main viewport is used to display a message.
 */
void Sound_Output_Feedback(uint16 index)
{
	if (index == 0xFFFF) return;

	if (index == 0xFFFE) {
		uint8 i;

		/* Clear spoken audio. */
		for (i = 0; i < lengthof(_spokenWords); i++) {
			_spokenWords[i] = 0xFFFF;
		}

		Driver_Voice_Stop();

		g_viewportMessageText = NULL;
		if ((g_viewportMessageCounter & 1) != 0) {
			g_global->variable_3A12 = 1;
			g_viewportMessageCounter = 0;
		}
		g_global->variable_4060 = 0;

		return;
	}

	if (g_config.voiceDrv == 0 || g_global->soundsEnabled == 0) {
		Driver_Sound_Play(g_feedback[index].soundId, 0xFF);

		g_viewportMessageText = String_Get_ByIndex(g_feedback[index].messageId);

		if ((g_viewportMessageCounter & 1) != 0) {
			g_global->variable_3A12 = 1;
		}

		g_viewportMessageCounter = 4;

		return;
	}

	/* If nothing is being said currently, load new words. */
	if (_spokenWords[0] == 0xFFFF) {
		uint8 i;

		for (i = 0; i < lengthof(_spokenWords); i++) {
			_spokenWords[i] = (g_config.language == LANGUAGE_ENGLISH) ? g_feedback[index].voiceId[i] : g_translatedVoice[index][i];
		}
	}

	Sound_StartSpeech();
}

/**
 * Start speech.
 * Start a new speech fragment if possible.
 * @return Sound is produced.
 */
bool Sound_StartSpeech()
{
	if (g_global->soundsEnabled == 0) return false;

	if (Driver_Voice_IsPlaying()) return true;

	g_global->variable_4060 = 0;

	if (_spokenWords[0] == 0xFFFF) return false;

	Sound_StartSound(_spokenWords[0]);
	/* Move speech parts one place. */
	memmove(&_spokenWords[0], &_spokenWords[1], sizeof(_spokenWords) - sizeof(_spokenWords[0]));
	_spokenWords[lengthof(_spokenWords) - 1] = 0xFFFF;

	return true;
}

/**
 * ??.
 * @param filename The name of the file to load.
 * @return Where the file is loaded.
 */
void *Sound_Unknown0823(const char *filename, uint32 *retFileSize)
{
	uint8 fileIndex;
	uint32 fileSize;
	void *res;

	if (filename == NULL || !File_Exists(filename)) return NULL;

	fileIndex = File_Open(filename, 1);
	fileSize  = File_GetSize(fileIndex);
	File_Close(fileIndex);

	fileSize += 1;
	fileSize &= 0xFFFFFFFE;

	Driver_Voice_LoadFile(filename, (void *)emu_get_memorycsip(g_global->readBuffer), g_global->readBufferSize);

	*retFileSize = fileSize;
	res = malloc(fileSize);
	memcpy(res, emu_get_memorycsip(g_global->readBuffer), fileSize);

	return res;
}
