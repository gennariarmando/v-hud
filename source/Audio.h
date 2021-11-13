#pragma once

enum eChunks {
    CHUNK_MENU_BACK,
    CHUNK_MENU_SCROLL,
    CHUNK_MENU_SELECT,
    CHUNK_MENU_MAP_MOVE,
    CHUNK_WHEEL_BACKGROUND,
    CHUNK_WHEEL_OPEN_CLOSE,
    CHUNK_WHEEL_MOVE,
    CHUNK_TD_LOADING_MUSIC,
    CHUNK_STATS_BACKGROUND,
    NUM_CHUNKS
};

class CAudio {
public:
    bool bInitialised;   
    unsigned long Chunks[NUM_CHUNKS];
    float fChunksVolume;
    bool loop;

public:
    CAudio();
    void Init();
    void Shutdown();
    void Update();

    unsigned long LoadChunkFile(const char* path, const char* name);
    void PlayChunk(int chunk, float volume);
    void StopChunk(int chunk);
    void SetChunksMasterVolume(char vol);
    void SetLoop(bool on);
};

extern CAudio Audio;