#include "audio/music.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>
#include <tremor/ivorbisfile.h>
#include <tremor/ivorbiscodec.h>

#define AUDIO_CHANNEL 0
#define AUDIO_BUFFER_SIZE (256 * 1024) // Buffer size for audio data

static ndspWaveBuf s_waveBufs[3];
static int16_t *s_audioBuffer = NULL;
static LightEvent s_event;
static volatile bool s_quit = false;

// Define the callbacks explicitly
size_t ov_read_func(void* ptr, size_t size, size_t nmemb, void* datasource) {
    return fread(ptr, size, nmemb, (FILE*)datasource);
}

int ov_seek_func(void* datasource, ogg_int64_t offset, int whence) {
    return fseek((FILE*)datasource, offset, whence);
}

int ov_close_func(void* datasource) {
    return fclose((FILE*)datasource);
}

long ov_tell_func(void* datasource) {
    return ftell((FILE*)datasource);
}

// Define the callbacks structure
ov_callbacks callbacks = {
    .read_func = ov_read_func,
    .seek_func = ov_seek_func,
    .close_func = ov_close_func,
    .tell_func = ov_tell_func
};

void init_ogg() {
    // No specific initialization needed for callbacks
}

OggVorbis_File* load_ogg_file(const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        return NULL;
    }

    OggVorbis_File* ogg_file = (OggVorbis_File*)malloc(sizeof(OggVorbis_File));
    if (ov_open_callbacks(file, ogg_file, NULL, 0, callbacks) < 0) {
        fclose(file);
        free(ogg_file);
        return NULL;
    }
    return ogg_file;
}

bool audioInit(OggVorbis_File* ogg_file) {
    vorbis_info* vi = ov_info(ogg_file, -1);

    // Setup NDSP
    ndspChnReset(AUDIO_CHANNEL);
    ndspSetOutputMode(NDSP_OUTPUT_STEREO);
    ndspChnSetInterp(AUDIO_CHANNEL, NDSP_INTERP_POLYPHASE);
    ndspChnSetRate(AUDIO_CHANNEL, vi->rate);
    ndspChnSetFormat(AUDIO_CHANNEL, vi->channels == 1 ? NDSP_FORMAT_MONO_PCM16 : NDSP_FORMAT_STEREO_PCM16);

    // Allocate audio buffer
    const size_t SAMPLES_PER_BUF = vi->rate * 120 / 1000; // 120ms buffer
    const size_t CHANNELS_PER_SAMPLE = vi->channels;
    const size_t WAVEBUF_SIZE = SAMPLES_PER_BUF * CHANNELS_PER_SAMPLE * sizeof(int16_t);
    const size_t bufferSize = WAVEBUF_SIZE * (sizeof(s_waveBufs) / sizeof(s_waveBufs[0]));
    s_audioBuffer = (int16_t*)linearAlloc(bufferSize);
    if (!s_audioBuffer) {
        printf("Failed to allocate audio buffer\n");
        return false;
    }

    // Setup waveBufs for NDSP
    memset(s_waveBufs, 0, sizeof(s_waveBufs));
    int16_t* buffer = s_audioBuffer;

    for (size_t i = 0; i < sizeof(s_waveBufs) / sizeof(s_waveBufs[0]); ++i) {
        s_waveBufs[i].data_vaddr = buffer;
        s_waveBufs[i].nsamples = SAMPLES_PER_BUF;
        s_waveBufs[i].status = NDSP_WBUF_DONE;

        buffer += SAMPLES_PER_BUF * CHANNELS_PER_SAMPLE;
    }

    return true;
}

void audioExit() {
    ndspChnReset(AUDIO_CHANNEL);
    if (s_audioBuffer) {
        linearFree(s_audioBuffer);
        s_audioBuffer = NULL;
    }
}

bool fillBuffer(OggVorbis_File* ogg_file, ndspWaveBuf* waveBuf) {
    int totalBytes = 0;
    while (totalBytes < waveBuf->nsamples * sizeof(int16_t)) {
        int16_t* buffer = waveBuf->data_pcm16 + (totalBytes / sizeof(int16_t));
        const size_t bufferSize = (waveBuf->nsamples * sizeof(int16_t) - totalBytes);

        int bytesRead = ov_read(ogg_file, (char*)buffer, bufferSize, NULL);
        if (bytesRead <= 0) {
            if (bytesRead == 0) break;
            printf("ov_read error: %d\n", bytesRead);
            break;
        }
        totalBytes += bytesRead;
    }

    if (totalBytes == 0) {
        printf("Playback complete\n");
        return false;
    }

    waveBuf->nsamples = totalBytes / sizeof(int16_t);
    ndspChnWaveBufAdd(AUDIO_CHANNEL, waveBuf);
    DSP_FlushDataCache(waveBuf->data_vaddr, totalBytes);

    return true;
}

void audioCallback(void* nul) {
    if (s_quit) {
        return;
    }

    LightEvent_Signal(&s_event);
}

void audio_playback_thread(void* arg) {
    OggVorbis_File* ogg_file = (OggVorbis_File*)arg;

    if (!ogg_file) {
        return;
    }

    if (!audioInit(ogg_file)) {
        ov_clear(ogg_file);
        free(ogg_file);
        return;
    }

    ndspSetCallback(audioCallback, NULL);

    while (!s_quit) {
        for (size_t i = 0; i < sizeof(s_waveBufs) / sizeof(s_waveBufs[0]); ++i) {
            if (s_waveBufs[i].status != NDSP_WBUF_DONE) {
                continue;
            }
            if (!fillBuffer(ogg_file, &s_waveBufs[i])) {
                return;
            }
        }

        LightEvent_Wait(&s_event);
    }

    audioExit();
    ov_clear(ogg_file);
    free(ogg_file);

    threadExit(0);
}

bool stop_ogg_file(int channel) {
    ndspChnWaveBufClear(channel);
    ndspChnReset(channel);
    return true;
}

void setup_ogg_player() {
    ndspInit();
    romfsInit();
    gfxInitDefault();
}

void cleanup_ogg_player() {
    ndspExit();
    romfsExit();
    gfxExit();
}

const char* path = "romfs:/audio/menu/music/";
const char* default_extension = ".ogg";
const char* tracks[] = {"menu1", "menu2", "menu3", "menu4"};

bool random_audio_track() {
    srand(time(NULL));
    int random = rand() % (sizeof(tracks) / sizeof(tracks[0]));
    char music_name[256];
    char music_path[512];

    strcpy(music_name, tracks[random]);
    strcpy(music_path, path);
    strcat(music_path, music_name);
    strcat(music_path, default_extension);

    printf("Playing music track: %s\n", music_path);

    OggVorbis_File* ogg_file = load_ogg_file(music_path);
    if (ogg_file) {
        Thread threadHandle = threadCreate(audio_playback_thread, ogg_file, 32 * 1024, 0x2F, -2, false);
        threadDetach(threadHandle);
        return true;
    } else {
        printf("Failed to load Ogg Vorbis file\n");
        return false;
    }
}

void stop_audio_track() {
    stop_ogg_file(AUDIO_CHANNEL);
    s_quit = true;
}
