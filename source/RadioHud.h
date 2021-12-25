#pragma once
#include "CSprite2d.h"

enum eRadioSprites {
    NUM_RADIO_SPRITES = 15,
};

class CRadioHud {
public:
    static CSprite2d* m_RadioIcons[NUM_RADIO_SPRITES];
    static bool m_bInitialised;
    static int m_nTimeToDisplay;
    static int m_nCurrentRadioId;
    static int m_nPreviousRadioId;
    static bool m_bChangeRadioStation;

public:
    CRadioHud();

    static void Init();
    static void Clear();
    static bool CanRetuneRadioStation();
    static void Process();
    static void Draw();
    static void Shutdown();
};
