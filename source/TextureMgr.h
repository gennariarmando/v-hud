#pragma once
#include "CSprite2d.h"

class CTextureMgr {
public:

public:
    CTextureMgr();
    static void Combine(RwRaster* raster1, RwRaster* raster2);
    static void Init();
    static void Shutdown();
    static RwTexture* LoadDDSTextureCB(const char* path, const char* name);
    static RwTexture* LoadBMPTextureCB(const char* path, const char* name);
    static RwTexture* LoadPNGTextureCB(const char* path, const char* name);
    static RwTexture* LoadPNGTextureCB(const char* path, const char* name, const char* namea);
    static RwTexture* LoadPNGTextureCB(const char* path, const char* name, const char* mask, const char* col);
};
