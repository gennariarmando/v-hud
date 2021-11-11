#pragma once

enum eChunks {
    CHUNK_MENU_BACK,
    CHUNK_MENU_SCROLL,
    CHUNK_MENU_SELECT,
    CHUNK_MENU_MAP_MOVE,
    NUM_CHUNKS
};

class CAudio {
public:
    bool bInitialised;   
    unsigned long Chunks[NUM_CHUNKS];
    float fChunksVolume;

public:
    CAudio();
    void Init();
    void Shutdown();
    void Update();

    unsigned long LoadChunkFile(const char* path, const char* name);
    void PlayChunk(int chunk, float volume);
    void CAudio::SetChunksMasterVolume(char vol);
};

extern CAudio Audio;