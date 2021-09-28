#pragma once
#include "CSprite2d.h"

class CTextureRel {
public:
    RwTexture* texture;
    char path[128];
    char name[128];

public:
    CTextureRel();
    CTextureRel(RwTexture* tex, const char* path, const char* name);
};

class CTextureMgr {
public:
    static CTextureRel* pTex[1024];

public:
    CTextureMgr();
    static void Combine(RwRaster* raster1, RwRaster* raster2);
    static void Init();
    static void Shutdown();
    static void ReloadTextures();
    static void LimitTextureSize(int& w, int& h);
    static RwTexture* LoadPNGTextureCB(const char* path, const char* name);
    static RwTexture* LoadPNGTextureCB(const char* path, const char* name, const char* namea);
    static RwTexture* LoadPNGTextureCB(const char* path, const char* name, const char* mask, const char* col);
    static void Delete(RwTexture* t);
};
