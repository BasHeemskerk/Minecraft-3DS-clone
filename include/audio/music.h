#ifndef MUSIC_H
#define MUSIC_H

#include <3ds.h>
#include <stdlib.h> // For rand() and srand()
#include <time.h>   // For time()
#include <stdio.h>
#include <string.h>

#include <ogg/ogg.h>
#include <tremor/ivorbisfile.h>
#include <gui/DebugUI.h>
#include <tremor/ivorbisfile.h>

void init_ogg();
OggVorbis_File* load_ogg_file(const char* path);
void audio_playback_thread(void* arg); // Corrected function signature
bool stop_ogg_file(int channel);
void setup_ogg_player();
void cleanup_ogg_player();
bool random_audio_track(); // Corrected function signature
void stop_audio_track();
//void play_audio_from_loop();

extern bool sFrameReady;

#endif // MUSIC_H
