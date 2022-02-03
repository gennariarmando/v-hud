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

using namespace plugin;

int& gGameState = *(int*)0xC8D4C0;

bool VHud::bInitialised = false;
HANDLE VHud::pThread = NULL;
bool VHud::bRwInitialized = false;
bool VHud::bRwQuit = false;
bool VHud::bSAMP = false;

bool VHud::Init() {
    if (bInitialised)
        return false;

#ifdef DEBUG
    OpenConsole();
#endif
    
    if (!CheckCompatibility()) {
        bRwInitialized = false;
        bRwQuit = true;
        return false;
    }
    else {
        std::function<bool()> b([] { return true; });
        LateStaticInit::TryApplyWithPredicate(b);

        Events::initRwEvent += [] {
            Audio.Init();
            HudColourNew.ReadColorsFromFile();
            CFontNew::Init();
            CRadarNew::InitBeforeGame();
            MenuNew.Init();
            CRadioHud::Init();
            COverlayLayer::Init();
            MarkersNew.Init();

            bRwInitialized = true;
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
            CPadNew::Shutdown();

            bRwQuit = true;
        };
    
        bInitialised = true;
    }

    return true;
}

void VHud::Shutdown() {
    if (!bInitialised)
        return;

    bInitialised = false;
}

void VHud::Run() {
    CPadNew::Init();

    while (!bRwQuit) {
        CheckForMP();

        if (bRwInitialized) {
            if (gGameState && !bSAMP) {
                switch (gGameState) {
                case 7:
                    if (MenuNew.ProcessMenuToGameSwitch(false)) {
                        gGameState = 8;
                        MenuNew.OpenCloseMenu(false);
                    }
                    break;
                case 8:
                    MenuNew.fLoadingPercentage = 100.0f;
                    gGameState = 9;
                    break;
                case 9:
                    if (MenuNew.ProcessMenuToGameSwitch(true)) {
                        MenuNew.OpenCloseMenu(false);
                    }
                    break;
                }
            }

            Audio.Update();
            CPadNew::GInputUpdate();
        }
    }
    CPadNew::GInputRelease();

    CloseHandle(pThread);
};

void VHud::CheckForMP() {
    if (bSAMP)
        return;

    if (const HMODULE h = ModuleList().Get(L"SAMP")) {
        bSAMP = true;
    }
}

bool VHud::CheckCompatibility() {
    if (!IsSupportedGameVersion()) {
        Error("This version of GTA: San Andreas is not supported by this plugin.");
        return false;
    }

    return true;
}

bool VHud::OpenConsole() {
    AllocConsole();
    freopen("conin$", "r", stdin);
    freopen("conout$", "w", stdout);
    freopen("conout$", "w", stderr);

    return true;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        if (VHud::Init())
            VHud::pThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)(void(__cdecl*)())VHud::Run, 0, 0, 0);
    }
    return TRUE;
}