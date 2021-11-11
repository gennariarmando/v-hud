#include "VHud.h"
#include "Audio.h"
#include "MenuNew.h"
#include "Utility.h"

#include "bass.h"

using namespace plugin;

CAudio Audio;

const char* AudioFileFormat = ".wav";

const char* ChunksFileNames[] = {
    "MENU_BACK",
    "MENU_SCROLL",
    "MENU_SELECT",
    "MENU_MAP_MOVE",
};

CAudio::CAudio() {

}

void CAudio::Init() {
    if (bInitialised)
        return;

    if (!BASS_Init(-1, 44100, 0, RsGlobal.ps->window, NULL)) {
        printf("[BASS] Can't initialize device. \n");

        if (BASS_ErrorGetCode() != BASS_ERROR_ALREADY)
            return;
    }

    for (int i = 0; i < NUM_CHUNKS; i++) {
        Chunks[i] = LoadChunkFile(PLUGIN_PATH("VHud\\audio\\frontend"), ChunksFileNames[i]);   
    }

    printf("Initialised BASS Audio. \n");

    bInitialised = true;
}

void CAudio::Shutdown() {
    if (!bInitialised)
        return;

    for (int i = 0; i < NUM_CHUNKS; i++) {
        BASS_SampleFree(Chunks[i]);
    }

    BASS_Free();

    printf("Shutdown BASS Audio. \n");

    bInitialised = false;
}

void CAudio::Update() {
    ;;
}

void CAudio::SetChunksMasterVolume(char vol) {
    fChunksVolume = (vol / 64.0f);
}

unsigned long CAudio::LoadChunkFile(const char* path, const char* name) {
    char file[512];

    strcpy_s(file, path);
    strcat_s(file, "\\");
    strcat_s(file, name);
    strcat_s(file, AudioFileFormat);
    puts(file);

    return BASS_SampleLoad(FALSE, file, 0, 0, 3, 0);
}

void CAudio::PlayChunk(int chunk, float volume) {
    if (!bInitialised)
        return;

    if (GetFocus() != RsGlobal.ps->window)
        return;

    chunk = clamp(chunk, 0, NUM_CHUNKS - 1);

    auto c = BASS_SampleGetChannel(Chunks[chunk], FALSE);

    volume = clamp(volume, 0.0f, 1.0f);
    BASS_ChannelSetAttribute(c, BASS_ATTRIB_VOL, volume * (fChunksVolume * 0.5f));
    BASS_ChannelPlay(c, FALSE);
}