#pragma once
#define VHUD_API __declspec(dllexport)

#include "plugin.h"
#include "LateStaticInit.h"


enum eUGEvents {
    UG_EVENT_BEFORE_RENDER2DSTUFF,
    UG_EVENT_AFTER_RENDER2DSTUFF,
    UG_EVENT_BEFORE_RENDERSCENE,
    UG_EVENT_AFTER_RENDERSCENE,
    UG_EVENT_SHUTDOWNFORRESTART,
    UG_EVENT_BEFORE_PROCESSGAME,
    UG_EVENT_AFTER_PROCESSGAME,
    UG_EVENT_SHUTDOWNGAME,
    UG_EVENT_BEFORE_GAMEINIT1,
    UG_EVENT_AFTER_GAMEINIT1,
    UG_EVENT_BEFORE_GAMEINIT2,
    UG_EVENT_AFTER_GAMEINIT2,
    UG_EVENT_BEFORE_GAMEINIT3,
    UG_EVENT_AFTER_GAMEINIT3,
    UG_EVENT_STARTTESTSCRIPT,
    UG_EVENT_UPDATEPOPULATION,
    UG_EVENT_INITPOSTEFFECTS,
    UG_EVENT_CLOSEPOSTEFFECTS,
    UG_EVENT_PROCESSCARGENERATORS,
    UG_EVENT_BEFORE_RENDERFADINGINENTITIES,
    UG_EVENT_AFTER_RENDERFADINGINENTITIES,
    MAX_UG_EVENT_CALLBACKS
};

class VHudSettings {
public:
    char UIMainColor[32];
};

class VHud {
public:
    static bool bInitialised;
    static HANDLE pThread;
    static bool bRwInitialized;
    static bool bRwQuit;
    static bool bSAMP;
    static bool bUG;
    static bool bENB;

    static VHudSettings Settings;

public:
    static bool Init();
    static void Shutdown();
    static void Run();
    static void CheckForENB();
    static void CheckForMP();
    static void CheckForUG();
    static void UG_RegisterEventCallback(int e, void* func);

    static bool CheckCompatibility();
    static bool OpenConsole();
    static void ReadPluginSettings();

};

extern int& gGameState;

