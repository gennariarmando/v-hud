#include "plugin.h"

#include "HudColoursNew.h"
#include "PadNew.h"
#include "DeathScreen.h"
#include "FontNew.h"
#include "GPS.h"
#include "HudNew.h"
#include "MenuNew.h"
#include "OverlayLayer.h"
#include "PanelsNew.h"
#include "RadarNew.h"
#include "RadioHud.h"
#include "TextureMgr.h"
#include "Utility.h"
#include "VHud.h"
#include "WeaponSelector.h"

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

VHud::VHud() {
    //OpenConsole(AllocConsole());

    if (!IsSupportedGameVersion())
        Error("This version of GTA: San Andreas is not supported by this plugin.");

    Events::initGameEvent += [] {
        HudColourNew.ReadColorsFromFile();
        CFontNew::Init();
        CGPS::Init();
        MenuNew.Init();
        CHudNew::Init();
        COverlayLayer::Init();
        CRadarNew::Init();
        CWeaponSelector::Init();
    };

    Events::reInitGameEvent += [] {
        CHudNew::ReInit();
        CWeaponSelector::ReInit();
    };

    Events::drawingEvent += [] {
        MenuNew.Update();
    };

    Events::drawHudEvent += [] {
        CHudNew::Draw();
    };

    Events::drawAfterFadeEvent += [] {
        CHudNew::DrawAfterFade();
    };

    Events::shutdownRwEvent += [] {
        CGPS::Shutdown();
        CRadarNew::Shutdown();
        CHudNew::Shutdown();
        COverlayLayer::Shutdown();
        CFontNew::Shutdown();
        CWeaponSelector::Shutdown();
    };
}
