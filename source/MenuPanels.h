#pragma once
#include "CMenuSystem.h"

class CPanel {
public:
    char title[16];
    char interiorName[16];
    char iconType[16];
    char textureName[128];
};

class CMenuPanels : public CMenuSystem {
public:
    static CPanel Panel[32];
    static CSprite2d ShopUiSprites[32];
    static int NumPanels;
    static bool bActive;

public:
    CMenuPanels();
    static void Init();
    static void Shutdown();
    static void Process(unsigned char panelId);
    static void ReadValuesFromFile();
    static void Draw(unsigned char panelId);
};

