#include "VHud.h"
#include "Audio.h"
#include "MenuNew.h"
#include "Utility.h"

#include "bass.h"

using namespace plugin;

CAudio Audio;

const char* AudioFileFormat = ".wav";

const char* ChunksFileNames[] = {
    "menu_back",
    "menu_scroll",
    "menu_select",
    "menu_map_move",
    "wheel_background",
    "wheel_open_close",
    "wheel_move",
    "td_loading_music",
    "stats_background",
    "wasted_busted",
    "screen_pulse1"
};

static LateStaticInit InstallHooks([]() {

});

void CAudio::Init() {
    if (bInitialised)
        return;

    if (!BASS_Init(-1, 44100, 0, RsGlobal.ps->window, NULL)) {
        int code = BASS_ErrorGetCode();
        if (code != BASS_ERROR_ALREADY) {
            return;
        }
    }

    for (int i = 0; i < NUM_CHUNKS; i++) {
        Chunks[i] = LoadChunkFile(PLUGIN_PATH("VHud\\audio\\frontend"), ChunksFileNames[i]);
        fPreviousChunkVolume[i] = 0.0f;
    }

    bResetMainVolume = GetForegroundWindow() != RsGlobal.ps->window;
    bInitialised = true;
}

void CAudio::Shutdown() {
    if (!bInitialised)
        return;

    for (int i = 0; i < NUM_CHUNKS; i++) {
        BASS_SampleFree(Chunks[i]);
    }

    BASS_Free();
    bInitialised = false;
}

void CAudio::Update() {
    if (!bInitialised)
        return;

    MenuNew.DoFadeTune();

    if (RsGlobal.ps) {
        if (GetForegroundWindow() != RsGlobal.ps->window) {
            if (!bResetMainVolume) {
                for (int i = 0; i < NUM_CHUNKS; i++) {
                    fPreviousChunkVolume[i] = GetVolumeForChunk(i);
                    SetVolumeForChunk(i, 0.0f);
                    bResetMainVolume = true;
                }
            }
        }
        else {
            if (bResetMainVolume) {
                for (int i = 0; i < NUM_CHUNKS; i++) {
                    SetVolumeForChunk(i, fPreviousChunkVolume[i]);
                }
                bResetMainVolume = false;
            }
        }
    }
}

void CAudio::SetChunksMasterVolume(char vol) {
    fChunksVolume = (vol / 64.0f);
}

void CAudio::SetLoop(bool on) {
    bLoop = on;
}

unsigned long CAudio::LoadChunkFile(const char* path, const char* name) {
    char file[512];

    sprintf(file, "%s\\%s.wav", path, name);

    return BASS_SampleLoad(FALSE, file, 0, 0, 3, 0);
}

void CAudio::PlayChunk(int chunk, float volume) {
    if (!bInitialised)
        return;

    chunk = clamp(chunk, 0, NUM_CHUNKS - 1);

    auto c = BASS_SampleGetChannel(Chunks[chunk], NULL);

    if (bLoop) {
        BASS_ChannelFlags(c, BASS_SAMPLE_LOOP, BASS_SAMPLE_LOOP);
    }
    else {
        BASS_ChannelFlags(c, NULL, BASS_SAMPLE_LOOP);
    }

    if (volume < 0.0f) {
        BASS_ChannelSetAttribute(c, BASS_ATTRIB_VOL, ABS(volume));
    }
    else {
        volume = clamp(volume, 0.0f, 1.0f);
        BASS_ChannelSetAttribute(c, BASS_ATTRIB_VOL, volume * (fChunksVolume * 0.5f));
    }
    BASS_ChannelPlay(c, FALSE);
}

float CAudio::GetVolumeForChunk(int chunk) {
    float volume = 0.0f;

    if (!bInitialised)
        return volume;

    chunk = clamp(chunk, 0, NUM_CHUNKS - 1);

    BASS_SAMPLE info;
    BASS_SampleGetInfo(Chunks[chunk], &info);
    if (HCHANNEL* c = (HCHANNEL*)malloc(info.max * sizeof(HCHANNEL))) {
        for (int i = 0; i < BASS_SampleGetChannels(Chunks[chunk], c); i++) {
            BASS_ChannelGetAttribute(c[i], BASS_ATTRIB_VOL, &volume);
        }
        free(c);
    }

    return volume / (fChunksVolume * 0.5f);
}

void CAudio::SetVolumeForChunk(int chunk, float volume) {
    if (!bInitialised)
        return;

    chunk = clamp(chunk, 0, NUM_CHUNKS - 1);
    volume = clamp(volume, 0.0f, 1.0f);

    BASS_SAMPLE info;
    BASS_SampleGetInfo(Chunks[chunk], &info);
    if (HCHANNEL* c = (HCHANNEL*)malloc(info.max * sizeof(HCHANNEL))) {
        for (int i = 0; i < BASS_SampleGetChannels(Chunks[chunk], c); i++) {
            BASS_ChannelSetAttribute(c[i], BASS_ATTRIB_VOL, volume * (fChunksVolume * 0.5f));
        }
        free(c);
    }
}

void CAudio::StopChunk(int chunk) {
    if (!bInitialised)
        return;

    chunk = clamp(chunk, 0, NUM_CHUNKS - 1);

    BASS_SAMPLE info;
    BASS_SampleGetInfo(Chunks[chunk], &info);
    if (HCHANNEL* c = (HCHANNEL*)malloc(info.max * sizeof(HCHANNEL))) {
        for (int i = 0; i < BASS_SampleGetChannels(Chunks[chunk], c); i++) {
            BASS_ChannelSetPosition(c[i], 0, BASS_POS_BYTE);
            BASS_SampleStop(c[i]);
            BASS_ChannelStop(c[i]);
        }
        free(c);
    }
}