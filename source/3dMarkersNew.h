#pragma once
#include "CSprite2d.h"

class CMarkerSettings {
public:
    bool used;
    int type;
    CVector pos;
    CRGBA col;

public:
    void Clear() {
        used = false;
        type = 0;
        pos = { };
        col = CRGBA(255, 255, 255, 255);
    }

};

class C3dMarkersNew {
public:
    bool bInitialised;
    CSprite2d* Sprite;
    CSprite2d* SpriteAlpha;
    CMarkerSettings Markers[2048];
    int MarkersCount;

public:
    C3dMarkersNew();  
    void Init();
    void Shutdown();
    void DrawArrows();
};

extern C3dMarkersNew MarkersNew;
