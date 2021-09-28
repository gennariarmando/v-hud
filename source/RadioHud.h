#pragma once
#include "CSprite2d.h"

enum eRadioSprites {
    NUM_RADIO_SPRITES = 14,
};

class CRadioHud {
public:
    static CSprite2d* m_RadioIcons[NUM_RADIO_SPRITES];
    static bool bInitialised;

public:
    CRadioHud();

    static void Init();
    static void Shutdown();
};
