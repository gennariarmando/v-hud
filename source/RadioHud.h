#pragma once
#include "CSprite2d.h"

enum eRadioSprites {
    NUM_RADIO_SPRITES = 15,
};

class CRadioHud {
public:
    static CSprite2d* m_RadioIcons[NUM_RADIO_SPRITES];
    static bool bInitialised;
    static int nTimeToDisplay;

public:
    CRadioHud();

    static void Init();
    static void Draw();
    static void Shutdown();
};
