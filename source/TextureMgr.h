#pragma once
#include "CSprite2d.h"

class CTextureMgr {
public:

public:
    static void Combine(RwRaster* raster1, RwRaster* raster2);
    static RwTexture* LoadPNGTextureCB(const char* path, const char* name);
    static RwTexture* LoadPNGTextureCB(const char* path, const char* name, const char* namea);
    static RwTexture* LoadPNGTextureCB(const char* path, const char* name, const char* mask, const char* col);
    static void Delete(RwTexture* t);
};

extern int LoadedTexturesCount;
