#pragma once
#define VHUD_API __declspec(dllexport)

#include "plugin.h"
#include "LateStaticInit.h"

class VHud {
public:
    static bool bInitialised;
    static HANDLE pThread;
    static bool bRwInitialized;
    static bool bRwQuit;
    static bool bSAMP;

public:
    static bool Init();
    static void Shutdown();
    static void Run();
    static void CheckForMP();
    static bool CheckCompatibility();
    static bool OpenConsole();
};

extern int& gGameState;

