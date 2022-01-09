#include "VHud.h"
#include "Audio.h"
#include "CellPhone.h"
#include "HudColoursNew.h"
#include "PadNew.h"
#include "FontNew.h"
#include "GPS.h"
#include "HudNew.h"
#include "MenuNew.h"
#include "MenuPanels.h"
#include "OverlayLayer.h"
#include "RadarNew.h"
#include "RadioHud.h"
#include "TextureMgr.h"
#include "Utility.h"
#include "WeaponSelector.h"
#include "3dMarkersNew.h"

#include "VHudAPI.h"

#include "CTimer.h"

using namespace plugin;

VHud pluginVHud;

inline int OpenConsole(bool console) {
    if (!freopen("conin$", "r", stdin))
        return false;

    if (!freopen("conout$", "w", stdout))
        return false;

    if (!freopen("conout$", "w", stderr))
        return false;

    return false;
}


HANDLE thread = NULL;
bool rwInitialized = false;
bool rwQuit = false;
int& gGameState = *(int*)0xC8D4C0;

bool SAMP = false;

VHud::VHud() {
#ifdef DEBUG
    OpenConsole(AllocConsole());
#endif

    if (!IsSupportedGameVersion())
        Error("This version of GTA: San Andreas is not supported by this plugin.");

    auto VHudLoop = []() {
        CPadNew::Init();

        while (!rwQuit) {
            CheckForMP();

            if (rwInitialized) {
                if (gGameState && !SAMP) {
                    switch (gGameState) {
                    case 7:
                        if (MenuNew.ProcessMenuToGameSwitch(false)) {
                            gGameState = 8;
                            MenuNew.OpenCloseMenu(false, true);
                        }
                        break;
                    case 8:
                        MenuNew.fLoadingPercentage = 100.0f;
                        gGameState = 9;
                        break;
                    case 9:
                        if (MenuNew.ProcessMenuToGameSwitch(true)) {
                            MenuNew.OpenCloseMenu(false, true);
                        }
                        break;
                    }
                }

                Audio.Update();
                CPadNew::GInputUpdate();
            }
        }
        CPadNew::GInputRelease();

        CloseHandle(thread);
    };

    thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)(void(__cdecl*)())VHudLoop, 0, 0, 0);

    Events::initRwEvent += [] {
        Audio.Init();
        HudColourNew.ReadColorsFromFile();
        CFontNew::Init();
        MenuNew.Init();
        CRadioHud::Init();
        COverlayLayer::Init();
        MarkersNew.Init();

        rwInitialized = true;
    };

    Events::initGameEvent += [] {
        CGPS::Init();
        CHudNew::Init();
        CellPhone.Init();
        CRadarNew::Init();
        CWeaponSelector::Init();
        CMenuPanels::Init();
    };

    Events::reInitGameEvent += [] {
        CHudNew::ReInit();
        CWeaponSelector::ReInit();
    };

    CdeclEvent<AddressList<0x53EB9D, H_CALL>, PRIORITY_BEFORE, ArgPickNone, void()> beforeFading;
    beforeFading += [] {
        CHudNew::Draw();
    };

    CdeclEvent<AddressList<0x53EB9D, H_CALL>, PRIORITY_AFTER, ArgPickNone, void()> afterFading;
    afterFading += [] {
        CHudNew::DrawAfterFade();
    };

    Events::d3dLostEvent += [] {
        CFontNew::Lost();
    };

    Events::d3dResetEvent += [] {
        CFontNew::Reset();
    };

    Events::shutdownRwEvent += [] {
        MenuNew.Shutdown();
        CGPS::Shutdown();
        CRadarNew::Shutdown();
        CHudNew::Shutdown();
        CellPhone.Shutdown();
        CRadioHud::Shutdown();
        COverlayLayer::Shutdown();
        CFontNew::Shutdown();
        CWeaponSelector::Shutdown();
        CMenuPanels::Shutdown();
        Audio.Shutdown();
        MarkersNew.Shutdown();

        rwQuit = true;
    };
}

void CheckForMP() {
    if (SAMP)
        return;

    const HMODULE h = ModuleList().Get(L"SAMP");

    if (h) {
        SAMP = true;
    }
}
