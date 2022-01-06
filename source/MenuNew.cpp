#include "VHud.h"
#include "CTimer.h"
#include "CMenuManager.h"
#include "CWorld.h"
#include "CClock.h"
#include "CGeneral.h"
#include "CDraw.h"
#include "CRenderer.h"
#include "Fx_c.h"
#include "CGamma.h"
#include "CAudioEngine.h"
#include "CText.h"
#include "CScene.h"
#include "CAERadioTrackManager.h"
#include "CAECutsceneTrackManager.h"
#include "CGame.h"
#include "CLoadingScreen.h"
#include "CGenericGameStorage.h"
#include "C_PcSave.h"
#include "CRadar.h"
#include "CMessages.h"
#include "CStats.h"
#include "CTimeCycle.h"

#include "Audio.h"
#include "GPS.h"
#include "PedNew.h"
#include "MenuNew.h"
#include "FontNew.h"
#include "HudNew.h"
#include "HudColoursNew.h"
#include "TextureMgr.h"
#include "PadNew.h"
#include "OverlayLayer.h"
#include "Utility.h"
#include "TextNew.h"
#include "RadarNew.h"
#include "RadioHud.h"
#include "WeaponSelector.h"

#include "dx\VidMemViaD3D9.h"

#include "pugixml.hpp"

#include <d3d9.h>

using namespace plugin;
using namespace pugi;

CMenuNew MenuNew;

char* MenuSpritesFileNames[] = {
    "mouse_arrow",
    "mouse_fucku",
    "mouse_grab",
    "mouse_hand",
    "rockstarlogo256",
    "xbox_gamepad",
    "arrow_left",
    "arrow_right",
    "menu_bought",
    "menu_bought",
    "menu_car",
    "menu_car_b",
    "menu_clothes",
    "menu_clothes_b",
    "menu_weapons",
    "menu_weapons_b",
    "menu_map_crosshair_line_right",
    "menu_map_crosshair_line_up",
    "menu_selector",
};

char* MiscSpritesFileNames[] = {
    "rect_grad",
    "rect_grad_centered",
    "skip_icon",
    "spinner",
};

char* FrontendSpritesFileNames[] = {
    "back",
    "front",
    "infobox",
    "gtalogo"
};

char* UserFilesFolder = "VHud\\ufiles\\";
char* SettingsFileName = "VHud\\ufiles\\settings.xml";

inline CRect GetMenuBarRect() { return CRect(MENU_X(311.0f), MENU_Y(181.0f), SCREEN_COORD(214.0f), SCREEN_COORD(38.0f)); }
inline CRect GetMenuTabRect() { return CRect(MENU_X(311.0f), MENU_Y(240.0f), SCREEN_COORD(432.0f), SCREEN_COORD(38.0f)); }
inline CRect GetMenuEntryRect() { return CRect(MENU_X(746.0f), MENU_Y(240.0f), SCREEN_COORD(864.0f), SCREEN_COORD(38.0f)); }
inline CRect GetMenuScreenRect() { return CRect(MENU_X(311.0f), MENU_Y(240.0f), SCREEN_COORD(1300.0f), SCREEN_COORD(645.0f)); }
inline float GetMenuHorSpacing() { return SCREEN_COORD(3.0f); };

inline float GetMenuCenterX() { return GetMenuScreenRect().left + (GetMenuScreenRect().right * 0.5f); }
inline float GetMenuCenterY() { return GetMenuScreenRect().top + (GetMenuScreenRect().bottom * 0.5f); }

bool bSaveScreenHasBeenOpened = false;

// Used for printing stats strings
char* gGxtString = (char*)0xC1B100;
char* gGxtString2 = (char*)0xC1AED8;

// Help text actions
auto HelpText_GoThrough = []() { MenuNew.ProcessGoThrough(-99); };
auto HelpText_GoBack = []() { MenuNew.ProcessGoBack(-99); };
auto HelpText_SetWaypoint = []() { MenuNew.SetWaypoint(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2); Audio.PlayChunk(CHUNK_MENU_SELECT, 1.0f); };
auto HelpText_ShowHideLegend = []() { MenuNew.bShowLegend = MenuNew.bShowLegend == false; Audio.PlayChunk(CHUNK_MENU_SELECT, 1.0f); };
auto HelpText_DeleteSave = []() { MenuNew.SetMenuMessage(MENUMESSAGE_DELETE_GAME); Audio.PlayChunk(CHUNK_MENU_SELECT, 1.0f); };
auto HelpText_ApplyChanges = []() { if (MenuNew.nMenuAlert == MENUALERT_PENDINGCHANGES) MenuNew.ApplyGraphicsChanges(); Audio.PlayChunk(CHUNK_MENU_SELECT, 1.0f); };

CMenuNew::CMenuNew() {
    patch::Set<BYTE>(0x53E797, 0xEB);
    patch::Nop(0x53EB85, 2);
    patch::Nop(0x53E826, 2);
    patch::Nop(0x748CE1, 10);

    // Lock gGameState to 6
    patch::Set<BYTE>(0xC8D4C0, 5);
    patch::Nop(0x747483, 6);
    patch::Nop(0x748C23, 5);
    patch::Nop(0x748C2B, 5);
    patch::Nop(0x748C9A, 5);

    patch::Nop(0x748CF1, 10);

    patch::RedirectJump(0x748995, (void*)0x7489D4);

    auto openSavePage = [](int, int, int, int, int) {
        if (!bSaveScreenHasBeenOpened) {
            MenuNew.SetSavePageBehaviour(true);
            bSaveScreenHasBeenOpened = true;
        }
    };
    patch::RedirectCall(0x48547E, (void(__fastcall*)(int, int, int, int, int))openSavePage);
    patch::Nop(0x485457, 7);

    auto processFrontend = [](int _this, int) {
        MenuNew.Process();
    };
    patch::RedirectJump(0x57B440, (void(__fastcall*)(int, int))processFrontend);

    auto drawFrontend = [](int, int) {
        MenuNew.Draw();
    };
    patch::RedirectJump(0x57C290, (void(__fastcall*)(int, int))drawFrontend);

    auto loadSettings = [](int, int) {
        MenuNew.Settings.Load();
    };
    patch::RedirectJump(0x57C8F0, (void(__fastcall*)(int, int))loadSettings);

    auto saveSettings = [](int, int) {
        MenuNew.Settings.Save();
    };
    patch::RedirectJump(0x57C660, (void(__fastcall*)(int, int))saveSettings);

    CdeclEvent<AddressList<0x61907A, H_CALL>, PRIORITY_AFTER, ArgPickNone, void()> onSavingGame;
    onSavingGame += [] {
        MenuNew.TempSettings.saveSlot = MenuNew.nCurrentEntryItem;
        MenuNew.ApplyChanges();
    };

    auto preRenderEntity = []() {
        if (CTimer::m_UserPause || CTimer::m_CodePause)
            return;

        CRenderer::PreRender();
    };
    patch::RedirectCall(0x53E9FE, (void(__cdecl*)())preRenderEntity);
}

void CMenuNew::Init() {
    if (bInitialised)
        return;

    Clear();

    for (int i = 0; i < NUM_MENU_SPRITES; i++) {
        MenuSprites[i] = new CSprite2d();
        MenuSprites[i]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\menus"), MenuSpritesFileNames[i]);
    }

    for (int i = 0; i < NUM_MISC_SPRITES; i++) {
        MiscSprites[i] = new CSprite2d();
        MiscSprites[i]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\misc"), MiscSpritesFileNames[i]);
    }

    for (int i = 0; i < NUM_FRONTEND_SPRITES; i++) {
        FrontendSprites[i] = new CSprite2d();
        FrontendSprites[i]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\frontend"), FrontendSpritesFileNames[i]);    
    }

    BuildMenuBar();
    BuildMenuScreen();

    VideoModeList = GetVideoModeList();

    MenuNew.Settings.Load();
    TempSettings = Settings;

    if (!SAMP) {
        bLandingPage = Settings.landingPage;
        bStartOrLoadGame = !bLandingPage;

        if (bLandingPage) {
            SetLandingPageBehaviour();
            PlayLoadingTune();
        }
    }

    ScanGalleryPictures(true);

    nOpenCloseWaitTime = CTimer::m_snTimeInMillisecondsPauseMode + 500;

    bInitialised = true;
}

void CMenuNew::Shutdown() {
    if (!bInitialised)
        return;

    for (int i = 0; i < NUM_MENU_SPRITES; i++) {
        if (MenuSprites[i]) {
            MenuSprites[i]->Delete();
            delete MenuSprites[i];
        }
    }

    for (int i = 0; i < NUM_MISC_SPRITES; i++) {
        if (MiscSprites[i]) {
            MiscSprites[i]->Delete();
            delete MiscSprites[i];
        }
    }

    for (int i = 0; i < NUM_FRONTEND_SPRITES; i++) {
        if (FrontendSprites[i]) {
            FrontendSprites[i]->Delete();
            delete FrontendSprites[i];
        }
    }

    for (int i = 0; i < MAX_GALLERY_PICTURES; i++) {
        if (Gallery[i]) {
            Gallery[i]->Delete();
            delete Gallery[i];
        }
    }

    delete[] VideoModeList;

    bInitialised = false;
    Clear();
}

void CMenuNew::Clear() {
    const int n = 0;

    nMouseType = MOUSE_ARROW;
    bDrawMouse = false;
    bMenuActive = false;
    nCurrentInputType = MENUINPUT_BAR;

    nPreviousBarItem = n;
    nCurrentBarItem = n;
    nCurrentBarItemHover = MENU_HOVER_NONE;
    nPreviousBarItemHover = MENU_HOVER_NONE;

    nPreviousTabItem = n;
    nCurrentTabItem = n;
    nCurrentTabItemHover = MENU_HOVER_NONE;
    nPreviousTabItemHover = MENU_HOVER_NONE;

    nPreviousEntryItem = n;
    nCurrentEntryItem = n;
    nCurrentEntryItemHover = MENU_HOVER_NONE;
    nPreviousEntryItemHover = MENU_HOVER_NONE;

    nPreviousScreen = MENUSCREEN_NONE;
    nCurrentScreen = MENUSCREEN_NONE;

    bRequestScreenUpdate = false;
    fScreenAlpha = 0;

    nLoadingTime = 0;

    nOpenCloseWaitTime = 0;

    nUsedVidMemory = 0;

    unsigned int mem;
    GetVideoMemoryViaD3D9(GetPrimaryMonitorHandle(), &mem);
    nFreeVidMemory = mem / 1024 / 1024;

    bRadioEnabled = false;

    nPreviousMessage = MENUMESSAGE_NONE;
    nCurrentMessage = MENUMESSAGE_NONE;

    nMenuAlert = MENUALERT_NONE;

    bShowMenuExtraText = true;
    bShowMenuBar = true;
    bShowMenu = true;
    bNoTransparentBackg = false;
    bLandingPage = false;
    bInvertInput = false;

    nNumOfSaveGames = 0;
    bSaveSlotsPopulated = false;

    for (int i = 0; i < 9; i++) {
        nSaveSlots[i][0] = NULL;
        nSaveSlotsDate[i][0] = NULL;
    }

    nControlsHelperCount = 0;

    bSavePage = false;
    bRequestMenuClose = false;

    bSaveScreenHasBeenOpened = false;
    bCleanMapScreenNextFrame = false;
    bDrawMenuMap = false;
    fMapZoom = MENU_MAP_ZOOM_MIN;
    vMapBase.x = 0.0f;
    vMapBase.y = 0.0f;
    vTempMapBase = vMapBase;
    nMapZoomTime = 0;
    bShowLegend = true;

    nTimeForSafeZonesToShow = 0;

    ResetMap();

    bScanGallery = false;
    bPreviouslyInGallery = false;
    nGalleryCount = 0;

    nTimePassedSinceLastKeyBind = 0;

    bLoadingPage = false;
    fLoadingPercentage = 0.0f;
    nLoadingTimeInMs = 0;
    bLoad = false;
    nSlot = 0;
}

void CMenuNew::BuildMenuBar() {
    AddNewBarItem("FE_MAP", MENUSCREEN_MAP);
    AddNewBarItem("FE_BRF", MENUSCREEN_BRIEF);
    AddNewBarItem("FE_STA", MENUSCREEN_STATS);
    AddNewBarItem("FE_SET", MENUSCREEN_SETTINGS);
    AddNewBarItem("FE_GAM", MENUSCREEN_GAME);
    AddNewBarItem("FE_GAL", MENUSCREEN_GALLERY);
}

void CMenuNew::BuildMenuScreen() {
    AddNewScreen("BLANK");
    AddNewScreen("FE_MAP");
    AddNewScreen("FE_BRF");

    // MENUSCREEN_STATS
    if (auto stats = AddNewScreen("FE_STA")) {
        if (!SAMP) {
            AddNewTab(stats, MENUENTRY_STAT, "FE_STAT_1", NULL, false);
            AddNewTab(stats, MENUENTRY_STAT, "FE_STAT_2", NULL, false);
            AddNewTab(stats, MENUENTRY_STAT, "FE_STAT_3", NULL, false);
            AddNewTab(stats, MENUENTRY_STAT, "FE_STAT_4", NULL, false);
            AddNewTab(stats, MENUENTRY_STAT, "FE_STAT_5", NULL, false);
            AddNewTab(stats, MENUENTRY_STAT, "FE_STAT_6", NULL, false);
            AddNewTab(stats, MENUENTRY_STAT, "FE_STAT_7", NULL, false);
            AddNewTab(stats, MENUENTRY_STAT, "FE_STAT_8", NULL, false);
        }
    }

    // MENUSCREEN_SETTINGS
    if (auto settings = AddNewScreen("FE_SET")) {
        if (auto pad = AddNewTab(settings, MENUENTRY_PAD, "FE_PAD", NULL, false)) {
            AddNewEntry(pad, MENUENTRY_SHOWCONTROLSFOR, "FE_SCF", 0, 7);
            AddNewEntry(pad, MENUENTRY_INVERTPADX1, "FE_ILSX", 0, 0);
            AddNewEntry(pad, MENUENTRY_INVERTPADY1, "FE_ILSY", 0, 0);
            AddNewEntry(pad, MENUENTRY_INVERTPADX2, "FE_ILA", 0, 0);
            AddNewEntry(pad, MENUENTRY_INVERTPADY2, "FE_IRSX", 0, 0);
            AddNewEntry(pad, MENUENTRY_SWAPPADAXIS1, "FE_IRSY", 0, 0);
            AddNewEntry(pad, MENUENTRY_SWAPPADAXIS2, "FE_IRA", 0, 0);
            AddNewEntry(pad, MENUENTRY_RESTOREDEFAULTS, "FE_RDEF", 0, 0);
        }

        if (auto key = AddNewTab(settings, MENUENTRY_CHANGETAB, "FE_KEY", NULL, false)) {
            AddNewEntry(key, MENUENTRY_MOUSESENSITIVITY, "FE_MVEL", 0, 0);
            AddNewEntry(key, MENUENTRY_INVERTMOUSEY, "FE_MINY", 0, 0);
            AddNewEntry(key, MENUENTRY_MOUSESTEER, "FE_MSTE", 0, 0);
            AddNewEntry(key, MENUENTRY_MOUSEFLYING, "FE_MFLY", 0, 0);
            AddNewEntry(key, MENUENTRY_RESTOREDEFAULTS, "FE_RDEF", 0, 0);
        }

        if (auto kbin = AddNewTab(settings, MENUENTRY_ACTION, "FE_KBIN", "FE_KBIN1", false)) {

        }

        if (auto audio = AddNewTab(settings, MENUENTRY_CHANGETAB, "FE_AUD", NULL, false)) {
            AddNewEntry(audio, MENUENTRY_SFXVOLUME, "FE_SFXV", 0, 0);
            AddNewEntry(audio, MENUENTRY_RADIOVOLUME, "FE_RADV", 0, 0);
            AddNewEntry(audio, MENUENTRY_RADIOSTATION, "FE_RSTA", 0, 0);
            AddNewEntry(audio, MENUENTRY_RADIOAUTOSELECT, "FE_RAUT", 0, 0);
            AddNewEntry(audio, MENUENTRY_RADIOEQ, "FE_RAEQ", 0, 0);
            AddNewEntry(audio, MENUENTRY_TRACKSAUTOSCAN, "FE_TAUT", 0, 0);
            //AddNewEntry(audio, MENUENTRY_RADIOMODE, "FE_RMOD", 0, 0);
            AddNewEntry(audio, MENUENTRY_RESTOREDEFAULTS, "FE_RDEF", 0, 0);
        }

        if (auto display = AddNewTab(settings, MENUENTRY_CHANGETAB, "FE_DIS", NULL, false)) {
            AddNewEntry(display, MENUENTRY_SHOWRADAR, "FE_RAD", 0, 0);
            AddNewEntry(display, MENUENTRY_SHOWHUD, "FE_HUD", 0, 0);
            AddNewEntry(display, MENUENTRY_WEAPONTARGET, "FE_TAR", 0, 0);
            AddNewEntry(display, MENUENTRY_GPSROUTE, "FE_GPS", 0, 0);
            AddNewEntry(display, MENUENTRY_BRIGHTNESS, "FE_BRI", 0, 0);
            AddNewEntry(display, MENUENTRY_GAMMA, "FE_GMA", 0, 0);
            AddNewEntry(display, MENUENTRY_SAFEZONESIZE, "FE_SAFE", 0, 0);
            AddNewEntry(display, MENUENTRY_SUBTITLES, "FE_SUB", 0, 0);
            AddNewEntry(display, MENUENTRY_MEASUREMENTSYS, "FE_MSYS", 0, 0);
            AddNewEntry(display, MENUENTRY_LANGUAGE, "FE_LANG", 0, 0);
            AddNewEntry(display, MENUENTRY_RESTOREDEFAULTS, "FE_RDEF", 0, 0);
        }

        if (auto graphics = AddNewTab(settings, MENUENTRY_GFX, "FE_GFX", NULL, false)) {
            AddNewEntry(graphics, MENUENTRY_SCREENTYPE, "FE_STP", 0, 2);
            AddNewEntry(graphics, MENUENTRY_CHANGERES, "FE_RES", 0, 0);
            //AddNewEntry(graphics, MENUENTRY_ASPECTRATIO, "FE_AR", 0, 0);
            AddNewEntry(graphics, MENUENTRY_FRAMELIMITER, "FE_FRL", 0, 0);

            AddNewEntry(graphics, MENUENTRY_MSAA, "FE_MSA", 0, 1);
            AddNewEntry(graphics, MENUENTRY_DRAWDISTANCE, "FE_DD", 0, 0);
            AddNewEntry(graphics, MENUENTRY_VISUALQUALITY, "FE_FXQ", 0, 0);
            AddNewEntry(graphics, MENUENTRY_RESTOREDEFAULTS, "FE_RDEF", 0, 0);
        }

        if (auto startup = AddNewTab(settings, MENUENTRY_CHANGETAB, "FE_SAS", NULL, false)) {
            AddNewEntry(startup, MENUENTRY_LANDINGPAGE, "FE_LAND", 0, 0);
        }
    }

    // MENUSCREEN_GAME
    if (auto game = AddNewScreen("FE_GAM")) {
        if (!SAMP) {
            if (auto loadGame = AddNewTab(game, MENUENTRY_POPULATESAVESLOT, "FE_LGAM", NULL, false)) {
                AddNewEntry(loadGame, MENUENTRY_LOADGAME, "FE_NOSAV", 0, 1);
                AddNewEntry(loadGame, MENUENTRY_LOADGAME, "FE_NOSAV", 0, 0);
                AddNewEntry(loadGame, MENUENTRY_LOADGAME, "FE_NOSAV", 0, 0);
                AddNewEntry(loadGame, MENUENTRY_LOADGAME, "FE_NOSAV", 0, 0);
                AddNewEntry(loadGame, MENUENTRY_LOADGAME, "FE_NOSAV", 0, 0);
                AddNewEntry(loadGame, MENUENTRY_LOADGAME, "FE_NOSAV", 0, 0);
                AddNewEntry(loadGame, MENUENTRY_LOADGAME, "FE_NOSAV", 0, 0);
                AddNewEntry(loadGame, MENUENTRY_LOADGAME, "FE_NOSAV", 0, 0);
            }

            if (auto newGame = AddNewTab(game, MENUENTRY_ACTION, "FE_NGAM", "FE_NGAM1", false)) {

            }
        }

        if (auto exitGame = AddNewTab(game, MENUENTRY_ACTION, "FE_EXIT", "FE_EXIT1", false)) {

        }
    }

    // MENUSCREEN_GALLERY
    AddNewScreen("FE_GAL");

    // MENUSCREEN_LANDING
    if (auto landing = AddNewScreen("BLANK")) {
        AddNewTab(landing, MENUENTRY_STORYMODE, "FE_STORY", NULL, false);
        AddNewTab(landing, MENUENTRY_SETTINGS, "FE_SET", NULL, false);
        AddNewTab(landing, MENUENTRY_QUIT, "FE_QUIT", NULL, false);
    }

    // MENUSCREEN_SAVE
    if (auto save = AddNewScreen("FE_SGAM")) {
        AddNewTab(save, MENUENTRY_SAVEGAME, "FE_NOSAV", NULL, true);
        AddNewTab(save, MENUENTRY_SAVEGAME, "FE_NOSAV", NULL, true);
        AddNewTab(save, MENUENTRY_SAVEGAME, "FE_NOSAV", NULL, true);
        AddNewTab(save, MENUENTRY_SAVEGAME, "FE_NOSAV", NULL, true);
        AddNewTab(save, MENUENTRY_SAVEGAME, "FE_NOSAV", NULL, true);
        AddNewTab(save, MENUENTRY_SAVEGAME, "FE_NOSAV", NULL, true);
        AddNewTab(save, MENUENTRY_SAVEGAME, "FE_NOSAV", NULL, true);
        AddNewTab(save, MENUENTRY_SAVEGAME, "FE_NOSAV", NULL, true);
    }

    // MENUSCREEN_KEYBIND
    if (auto key = AddNewScreen("FE_KBIN")) {
        if (auto foot = AddNewTab(key, MENUENTRY_KEYBINDING, "FE_GEN", NULL, false)) {
            for (int i = 0; i < PHONE_ENTER; i++) {
                AddNewEntry(foot, MENUENTRY_REDEFINEKEY, "BLANK", 0, i == 0 ? 1 : 0);
            }
        }
    }
}

void CMenuNew::DrawBackground() {
    CSprite2d::DrawRect(CRect(-5.0f, -5.0f, SCREEN_WIDTH + 5.0f, SCREEN_HEIGHT + 5.0f), CRGBA(0, 0, 0, 255));

    if (bStylizedBackground) {
        RwRenderStateSet(rwRENDERSTATETEXTUREPERSPECTIVE, (void*)FALSE);
        RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*)rwTEXTUREADDRESSCLAMP);
        MenuNew.FrontendSprites[FRONTEND_BACK]->Draw(CRect(MENU_X(0.0f), MENU_Y(0.0f), MENU_RIGHT(0.0f), MENU_BOTTOM(0.0f)), CRGBA(255, 255, 255, 255));
        MenuNew.FrontendSprites[FRONTEND_FRONT]->Draw(CRect(MENU_X(0.0f), MENU_BOTTOM(1078.0f), MENU_X(1080.0f), MENU_BOTTOM(-2.0f)), CRGBA(255, 255, 255, 255));
        MenuNew.FrontendSprites[FRONTEND_GTALOGO]->Draw(CRect(HUD_X(96.0f), HUD_BOTTOM(312.0f), HUD_X(96.0f + 280.0f), HUD_BOTTOM(312.0f - 278.0f)), CRGBA(255, 255, 255, 255));
    }
}

void CMenuNew::SetLandingPageBehaviour() {
    if (nCurrentScreen == MENUSCREEN_LANDING)
        return;

    bLandingPage = true;
    bLoadingPage = false;
    bShowMenu = false;
    bShowMenuExtraText = false;
    bShowMenuBar = false;
    bInvertInput = true;
    bNoTransparentBackg = true;
    bStylizedBackground = true;

    OpenMenuScreen(MENUSCREEN_LANDING);
    SetInputTypeAndClear(MENUINPUT_TAB, 0);
}

void CMenuNew::SetLoadingPageBehaviour() {
    if (SAMP) {
        DoSettingsBeforeStartingAGame(bLoad, nSlot);
        return;
    }

    if (nCurrentScreen == MENUSCREEN_LOADING)
        return;

    bLandingPage = true;
    bLoadingPage = true;
    bShowMenu = false;
    bShowMenuExtraText = false;
    bShowMenuBar = false;
    bInvertInput = false;
    bNoTransparentBackg = true;
    bStylizedBackground = true;
    bDrawMouse = false;

    OpenMenuScreen(MENUSCREEN_LOADING);
    bRequestScreenUpdate = false;
}

void CMenuNew::PlayLoadingTune() {
    if (!bLoadingTuneStarted) {
        Audio.SetLoop(true);
        Audio.PlayChunk(CHUNK_TD_LOADING_MUSIC, 0.5f);
        Audio.SetLoop(false);

        bLoadingTuneStarted = true;
        fLoadingTuneVolume = 1.0f;
    }
}

void CMenuNew::StopLoadingTune() {
    ;;
}

void CMenuNew::DoFadeTune() {
    float p = 100.0f - fLoadingPercentage;
    p /= 100.0f;
    if (bLoadingTuneStarted && p < 0.5f) {
        fLoadingTuneVolume = lerp(p, 0.0f, 1.0f);

        Audio.SetVolumeForChunk(CHUNK_TD_LOADING_MUSIC, fLoadingTuneVolume);

        if (isNearlyEqualF(fLoadingTuneVolume, 0.0f, 0.01f)) {
            Audio.StopChunk(CHUNK_TD_LOADING_MUSIC);
            bLoadingTuneStarted = false;
        }
    }
}

void CMenuNew::SetSavePageBehaviour(bool background) {
    if (nCurrentScreen == MENUSCREEN_SAVE)
        return;

    bSavePage = true;
    bShowMenu = true;
    bShowMenuExtraText = true;
    bShowMenuBar = false;
    bInvertInput = false;
    bNoTransparentBackg = background;
    bStylizedBackground = false;
    bLoadingPage = false;

    OpenMenuScreen(MENUSCREEN_SAVE);
    SetInputTypeAndClear(MENUINPUT_TAB, 0);
}

void CMenuNew::SetDefaultPageBehaviour() {
    bShowMenu = true;
    bShowMenuExtraText = false;
    bShowMenuBar = false;
    bInvertInput = false;
    bNoTransparentBackg = true;
    bStylizedBackground = true;
    bLoadingPage = false;
}

void CMenuNew::AddNewBarItem(char* name, int screen) {
    for (int i = 0; i < MAX_MENU_BAR_ITEMS; i++) {
        CMenuBar& b = MenuBar[i];

        if (b.barName[0] != '\0')
            continue;

        b.AddItem(name, screen);
        return;
    }
}

CMenuScreen* CMenuNew::AddNewScreen(char* name) {
    for (int i = 0; i < MAX_MENU_SCREENS; i++) {
        CMenuScreen* s = &MenuScreen[i];

        if (s->screenName[0] != '\0')
            continue;

        s->AddScreen(name);
        return s;
    }
}

CMenuScreen* CMenuNew::GetMenuScreen(char* name) {
    for (int i = 0; i < MAX_MENU_SCREENS; i++) {
        CMenuScreen* s = &MenuScreen[i];

        if (s->screenName[0] != '\0' &&
            !faststrcmp(s->screenName, name))
            return s;
    }
}

CMenuTab* CMenuNew::GetMenuTab(CMenuScreen* s, char* name) {
    for (int i = 0; i < MAX_MENU_SCREENS; i++) {
        CMenuTab* t = &s->Tab[i];

        if (t->tabName[0] != '\0' &&
            !faststrcmp(t->tabName, name))
            return t;
    }
}

CMenuEntry* CMenuNew::GetMenuEntry(CMenuTab* t, char* name) {
    for (int i = 0; i < MAX_MENU_SCREENS; i++) {
        CMenuEntry* e = &t->Entries[i];

        if (e->entryName[0] != '\0' &&
            !faststrcmp(e->entryName, name))
            return e;
    }
}

CMenuEntry* CMenuNew::GetMenuEntry(CMenuTab* t, int i) {
    CMenuEntry* e = &t->Entries[i];
    return e;
}

CMenuTab* CMenuNew::AddNewTab(CMenuScreen* s, int type, char* tabName, char* actionName, bool full) {
    for (int j = 0; j < MAX_MENU_TABS; j++) {
        CMenuTab* t = &s->Tab[j];

        if (t->tabName[0] != '\0')
            continue;

        t->AddTab(type, tabName, actionName, full);
        return t;
    }
}

CMenuEntry* CMenuNew::AddNewEntry(CMenuTab* t, int type, char* entryName, int x, int y) {
    for (int k = 0; k < MAX_MENU_ENTRIES; k++) {
        CMenuEntry* e = &t->Entries[k];

        if (e->entryName[0] != '\0')
            continue;

        e->AddEntry(type, entryName, x, y);
        return e;
    }
}

void CMenuNew::SetInputTypeAndClear(int input, int n) {
    if (input > 0) {
        nPreviousInputType = nCurrentInputType;
        nCurrentInputType = input;

        if (n != -1) {
            if (bPreviouslyInGallery && bScanGallery && nCurrentScreen != MENUSCREEN_GALLERY) {
                bScanGallery = false;
                nGalleryCount = 0;
                bInvertInput = false;
                bPreviouslyInGallery = false;
            }

            if (bShowPictureBigSize) {
                bShowPictureBigSize = false;
                //bShowMenu = true;
            }

            switch (input) {
            case MENUINPUT_BAR:
                nPreviousBarItem = nCurrentBarItem;
                nCurrentBarItem = n;
                nCurrentBarItemHover = MENU_HOVER_NONE;
                nPreviousBarItemHover = MENU_HOVER_NONE;

                if (bCleanMapScreenNextFrame) {
                    ResetMap();
                    bShowMenu = true;
                    bDrawMenuMap = false;
                    bCleanMapScreenNextFrame = false;
                }
                break;
            case MENUINPUT_TAB:
                nPreviousTabItem = nCurrentTabItem;
                nCurrentTabItem = n;
                nCurrentTabItemHover = MENU_HOVER_NONE;
                nPreviousTabItemHover = MENU_HOVER_NONE;
                break;
            case MENUINPUT_ENTRY:
                if (n == 0)
                    n = GetFirstMenuScreenEntry();
                nPreviousEntryItem = nCurrentEntryItem;
                nCurrentEntryItem = n;
                nCurrentEntryItemHover = MENU_HOVER_NONE;
                nPreviousEntryItemHover = MENU_HOVER_NONE;
                break;
            default:
                break;
            }
        }
    }
}

int CMenuNew::GetLastMenuBarItem() {
    int result = 0;
    for (int i = 0; i < MAX_MENU_BAR_ITEMS; i++) {
        if (MenuBar[i].barName[0] != '\0')
            result++;
    }
    return result;
}

int CMenuNew::GetLastMenuScreenTab() {
    int result = 0;
    if (nCurrentScreen == MENUSCREEN_GALLERY) {
        result = 24;
    }
    else {
        for (int i = 0; i < MAX_MENU_TABS; i++) {
            if (MenuScreen[nCurrentScreen].Tab[i].tabName[0] != '\0')
                result++;
        }
    }
    return result;
}

int CMenuNew::GetFirstMenuScreenEntry() {
    for (int i = 0; i < MAX_MENU_ENTRIES; i++) {
        if (HasToContinueLoopInverse(i))
            return i;
    }
    return 0;
}

int CMenuNew::GetLastMenuScreenEntry() {
    int result = 0;
    for (int i = 0; i < MAX_MENU_ENTRIES; i++) {
        if (HasToContinueLoop(i))
            continue;

        result++;
    }
    return result;
}

bool CMenuNew::HasToContinueLoop(int i) {
    if (MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].Entries[i].type == MENUENTRY_LOADGAME) {
        char* str = nSaveSlots[i];

        if (str[0] == '\0')
            return true;
    }
    else {
        if (MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].Entries[i].entryName[0] == '\0')
            return true;
    }
    return false;
}

bool CMenuNew::HasToContinueLoopInverse(int i) {
    if (MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].Entries[i].type == MENUENTRY_LOADGAME) {
        char* str = nSaveSlots[i];

        if (str && str[0] != '\0')
            return true;
    }
    else {
        char* str = MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].Entries[i].entryName;

        if (str && str[0] != '\0')
            return true;
    }
    return false;
}

int CMenuNew::GetEntryBackHeight() {
    return GetLastMenuScreenEntry() - 1;
}

void CMenuNew::OpenCloseMenu(bool on, bool force) {
    if (!force) {
        if (nOpenCloseWaitTime > GetTimeInMillisecondsRight() || IsLoading())
            return;
    }

    CPadNew* pad = CPadNew::GetPad(0);
    if (on) {
        if (SAMP) {
            CWeaponSelector::DisableCameraMovement();
        }
        else
            CTimer::StartUserPause();
        AudioEngine.Service();

        pad->Clear(0, 1);
        //pad->ClearKeyBoardHistory();
        pad->ClearMouseHistory();
        PreviousPlayerControls = pad->DisablePlayerControls;
        CurrentPlayerControls = true;

        AudioEngine.StopRadio(NULL, 0);

        Audio.PlayChunk(CHUNK_MENU_BACK, 1.0f);
    }
    else {
        if (SAMP) {
            CWeaponSelector::ResetCameraMovement();
        }
        else
            CTimer::EndUserPause();

        pad->Clear(1, 1);
        //pad->ClearKeyBoardHistory();
        pad->ClearMouseHistory();
        CurrentPlayerControls = PreviousPlayerControls;
        pad->DisablePlayerControls = PreviousPlayerControls;

        Clear();
        SetInputTypeAndClear(MENUINPUT_BAR);
        bRequestScreenUpdate = true;
    }

    nLoadingTime = GetTimeInMillisecondsRight() + MENU_SCREEN_CHANGE_WAIT_TIME;
    nOpenCloseWaitTime = GetTimeInMillisecondsRight() + MENU_OPEN_CLOSE_WAIT_TIME;
    bMenuActive = on;

    CenterCursor();
}

void CMenuNew::OpenMenuScreen(int screen) {
    CTimer::StartUserPause();
    nLoadingTime = 0;
    nOpenCloseWaitTime = 0;
    bMenuActive = true;
    bRequestScreenUpdate = true;

    nPreviousScreen = nCurrentScreen;
    nCurrentScreen = screen;

    PreviousPlayerControls = CPadNew::GetPad(0)->DisablePlayerControls;
}

void CMenuNew::CenterCursor() {
    if (SAMP)
        return;

    POINT p;
    p.x = SCREEN_WIDTH / 2;
    p.y = SCREEN_HEIGHT / 2;
    if (ClientToScreen(RsGlobal.ps->window, &p)) {
        SetCursorPos(p.x, p.y);

        RsGlobal.ps->lastMousePos.x = p.x;
        RsGlobal.ps->lastMousePos.y = p.y;
    }
}

void CMenuNew::DoMapZoomInOut(bool out) {
    if (nMapZoomTime > CTimer::m_snTimeInMillisecondsPauseMode)
        return;

    const float value = (!out ? 1.1f : 1.0f / 1.1f);

    if (out && fMapZoom > MENU_MAP_ZOOM_MIN || !out && fMapZoom < MENU_MAP_ZOOM_MAX) {
        fMapZoom *= value;

        const float halfMapSize = GetMenuMapWholeSize() / 2;
        if ((vTempMapBase.x + halfMapSize < MENU_X(0.0f))
            || (vTempMapBase.x - halfMapSize > MENU_RIGHT(0.0f))
            || (vTempMapBase.y + halfMapSize < MENU_Y(0.0f))
            || (vTempMapBase.y - halfMapSize > MENU_BOTTOM(0.0f))) {

        }
        else {
            vMapBase.x += (SCREEN_HALF_WIDTH - vMapBase.x) * (1.0f - value);
            vMapBase.y += (SCREEN_HALF_HEIGHT - vMapBase.y) * (1.0f - value);
            vTempMapBase = vMapBase;
        }

        nMapZoomTime = CTimer::m_snTimeInMillisecondsPauseMode + 25;
    }
}

void CMenuNew::RemoveUnusedControllerSettings() {
    static bool checkGInput = false;
    if (!checkGInput) {
        CMenuScreen* s = GetMenuScreen("FE_SET");
        CMenuTab* t = GetMenuTab(s, "FE_PAD");

        if (GINPUT) {
            for (int i = 1; i < 8; i++) {
                CMenuEntry* e = GetMenuEntry(t, i);
                e->RemoveEntry();
            }
        }
        else {
            CMenuEntry* e = GetMenuEntry(t, 0);
            e->RemoveEntry();

            e = GetMenuEntry(t, 0);
            e->y = 7;
        }

        checkGInput = true;
    }
}

void CMenuNew::ProcessGoThrough(int input) {
    if (input == -99)
        input = nCurrentInputType;

    switch (input) {
    case MENUINPUT_BAR:
        if (GetLastMenuBarItem() > 0)
            Audio.PlayChunk(CHUNK_MENU_SELECT, 1.0f);

        SetInputTypeAndClear(MENUINPUT_TAB, nCurrentTabItem);
        break;
    case MENUINPUT_TAB:
        if (GetLastMenuScreenTab() > 0)
            Audio.PlayChunk(CHUNK_MENU_SELECT, 1.0f);

        ProcessTabStuff();
        break;
    case MENUINPUT_ENTRY:
        ProcessEntryStuff(true, 0);
        Audio.PlayChunk(CHUNK_MENU_SELECT, 1.0f);
        break;
    case MENUINPUT_MESSAGE:
        Audio.PlayChunk(CHUNK_MENU_SELECT, 1.0f);
        ProcessMessagesStuff(true, false, false, false);
        break;
    case MENUINPUT_GALLERYPIC:
        break;
    }
}

void CMenuNew::ProcessGoBack(int input) {
    if (input == -99)
        input = nCurrentInputType;

    switch (input) {
    case MENUINPUT_BAR:
        Audio.PlayChunk(CHUNK_MENU_BACK, 1.0f);
        OpenCloseMenu(false, false);
        break;
    case MENUINPUT_TAB:
        if (nCurrentScreen != MENUSCREEN_LANDING && nCurrentScreen != MENUSCREEN_LOADING) {
            Audio.PlayChunk(CHUNK_MENU_BACK, 1.0f);
        }

        switch (nCurrentScreen) {
        case MENUSCREEN_KEYBIND:
            if (int p = nPreviousScreen) {
                nPreviousScreen = nCurrentScreen;
                nCurrentScreen = p;
                bRequestScreenUpdate = true;
                SetInputTypeAndClear(nCurrentInputType, tabItemBeforeScreenChange);
            }
            break;
        case MENUSCREEN_LANDING:
            break;
        case MENUSCREEN_LOADING:
            break;
        case MENUSCREEN_SAVE:
            bRequestMenuClose = true;
            bRequestScreenUpdate = true;
            break;
        case MENUSCREEN_SETTINGS:
            if (bLandingPage) {
                bShowMenu = false;
                bShowMenuBar = false;
                bInvertInput = true;
                OpenMenuScreen(MENUSCREEN_LANDING);
                SetInputTypeAndClear(MENUINPUT_TAB, 1);
                break;
            }
            [[fallthrough]];
        default:
        def:
            SetInputTypeAndClear(MENUINPUT_BAR, nCurrentBarItem);
            break;
        }
        break;
    case MENUINPUT_ENTRY:
        Audio.PlayChunk(CHUNK_MENU_BACK, 1.0f);

        if (nMenuAlert == MENUALERT_NONE) {
            SetInputTypeAndClear(MENUINPUT_ENTRY, 0);
            SetInputTypeAndClear(MENUINPUT_TAB, nCurrentTabItem);
        }
        else {
            ProcessAlertStuff();
        }
        break;
    case MENUINPUT_MESSAGE:
        Audio.PlayChunk(CHUNK_MENU_BACK, 1.0f);
        ProcessMessagesStuff(false, true, false, false);
        break;
    case MENUINPUT_GALLERYPIC:
        Audio.PlayChunk(CHUNK_MENU_BACK, 1.0f);

        SetInputTypeAndClear(MENUINPUT_ENTRY, 0);
        SetInputTypeAndClear(MENUINPUT_TAB, nCurrentTabItem);
        break;
    }
}

void CMenuNew::Process() {
    if (nTimePassedSinceLastKeyBind > CTimer::m_snTimeInMillisecondsPauseMode)
        return;

    float x = CPadNew::GetMouseInput(256.0f).x;
    float y = CPadNew::GetMouseInput(256.0f).y;

    vOldMousePos.x = vMousePos.x;
    vOldMousePos.y = vMousePos.y;

    if (nCurrentInputType != MENUINPUT_REDEFINEKEY) {
        if (x < 0.01f || x > 0.01f)
            vMousePos.x += x;

        if (y < 0.01f || y > 0.01f)
            vMousePos.y += y;
    }

    if (vMousePos.x < 0)
        vMousePos.x = 0;
    if (vMousePos.x > SCREEN_WIDTH)
        vMousePos.x = SCREEN_WIDTH;
    if (vMousePos.y < 0)
        vMousePos.y = 0;
    if (vMousePos.y > SCREEN_HEIGHT)
        vMousePos.y = SCREEN_HEIGHT;

    // Input
    CPadNew* pad = CPadNew::GetPad(0);

    bool Up = false;
    bool Down = false;
    bool Left = false;
    bool Right = false;

    bool UpPressed = false;
    bool DownPressed = false;
    bool LeftPressed = false;
    bool RightPressed = false;

    bool Enter = false;
    bool Back = false;
    bool Space = false;
    bool WheelUp = false;
    bool WheelDown = false;
    bool LeftMouseDown = false;
    bool LeftMouseJustDown = false;
    bool LeftMouseJustUp = false;
    bool MiddleMouseJustDown = false;
    bool LeftMouseDoubleClickJustDown = false;

    bool MapZoomIn = false;
    bool MapZoomOut = false;
    bool ShowHideMapLegend = false;

    bool OpenClose = false;

    if (nOpenCloseWaitTime < GetTimeInMillisecondsRight()) {
        Up = pad->GetMenuUpJustDown();
        Down = pad->GetMenuDownJustDown();
        Left = pad->GetMenuLeftJustDown();
        Right = pad->GetMenuRightJustDown();

        UpPressed = pad->GetMenuUp();
        DownPressed = pad->GetMenuDown();
        LeftPressed = pad->GetMenuLeft();
        RightPressed = pad->GetMenuRight();

        Enter = pad->GetMenuEnterJustDown();
        Back = pad->GetMenuBackJustDown() || (pad->GetRightMouseJustDown() && nCurrentInputType != MENUINPUT_BAR);
        Space = pad->GetMenuSpaceJustDown();
        WheelUp = pad->GetMenuMapZoomInJustDown();
        WheelDown = pad->GetMenuMapZoomOutJustDown();
        LeftMouseDown = pad->GetLeftMouseDown();
        LeftMouseJustDown = pad->GetLeftMouseJustDown();
        LeftMouseJustUp = pad->GetLeftMouseJustUp();
        MiddleMouseJustDown = pad->GetMiddleMouseJustDown();
        LeftMouseDoubleClickJustDown = pad->GetLeftMouseDoubleClickJustDown();

        MapZoomIn = pad->GetMenuMapZoomIn();
        MapZoomOut = pad->GetMenuMapZoomOut();
        ShowHideMapLegend = pad->GetMenuShowHideLegendJustDown();

        OpenClose = pad->GetOpenCloseMenuJustDown();

        if (bInvertInput) {
            if (nCurrentScreen == MENUSCREEN_GALLERY) {
                Up = Left;
                Down = Right;
            }
            else {
                Up = Right;
                Down = Left;
            }
        }
    }

    if (bMenuActive) {
        if (!bLoadingPage) {
            if (pad->CheckForMouseInput())
                bDrawMouse = true;
            else if (Up || Down || Left || Right || Enter || Back)
                bDrawMouse = false;
        }

        pad->DisablePlayerControls = CurrentPlayerControls;

        if (HAS_PAD_IN_HANDS(0))
            OpenClose |= pad->GetMenuBackJustDown();

        if (MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].type == MENUENTRY_POPULATESAVESLOT ||
            bSavePage) {
            if (!bSaveSlotsPopulated) {
                PcSaveHelper.PopulateSlotInfo();
                nNumOfSaveGames = GetNumOfSaveGames();
                bSaveSlotsPopulated = true;
            }
        }
        else if (nCurrentScreen == MENUSCREEN_GALLERY) {
            MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].type = MENUENTRY_SHOWPICTURE;
            ScanGalleryPictures(false);
            bPreviouslyInGallery = true;
        }
        else {
            nNumOfSaveGames = 0;
            for (int i = 0; i < 9; i++) {
                nSaveSlots[i][0] = NULL;
                nSaveSlotsDate[i][0] = NULL;
            }
            bSaveSlotsPopulated = false;
        }

        tabItemBeforeScreenChange = nCurrentTabItem;

        switch (nCurrentInputType) {
        case MENUINPUT_BAR:
            if (bShowMenuBar) {
                if (Left) {
                    if (GetLastMenuBarItem() > 0)
                        Audio.PlayChunk(CHUNK_MENU_SCROLL, 1.0f);

                    nPreviousBarItem = nCurrentBarItem;
                    nCurrentBarItem--;

                    if (nCurrentBarItem < 0)
                        nCurrentBarItem = GetLastMenuBarItem() - 1;

                    SetInputTypeAndClear(MENUINPUT_TAB);
                    SetInputTypeAndClear(MENUINPUT_ENTRY);
                    SetInputTypeAndClear(MENUINPUT_BAR, nCurrentBarItem);

                    bRequestScreenUpdate = true;
                }
                else if (Right) {
                    if (GetLastMenuBarItem() > 0)
                        Audio.PlayChunk(CHUNK_MENU_SCROLL, 1.0f);

                    nPreviousBarItem = nCurrentBarItem;
                    nCurrentBarItem++;

                    if (nCurrentBarItem > GetLastMenuBarItem() - 1)
                        nCurrentBarItem = 0;

                    SetInputTypeAndClear(MENUINPUT_TAB);
                    SetInputTypeAndClear(MENUINPUT_ENTRY);
                    SetInputTypeAndClear(MENUINPUT_BAR, nCurrentBarItem);

                    bRequestScreenUpdate = true;
                }
                else if (Enter) {
                    ProcessGoThrough(-99);
                }
                else if (OpenClose) {
                    ProcessGoBack(-99);
                    return;
                }

                if (nCurrentScreen != MenuBar[nCurrentBarItem].targetScreen) {
                    nPreviousScreen = nCurrentScreen;
                    nCurrentScreen = MenuBar[nCurrentBarItem].targetScreen;
                }
            }
            break;
        case MENUINPUT_TAB:
            if (!IsLoading()) {
                if (Up) {
                    if (GetLastMenuScreenTab() > 0)
                        Audio.PlayChunk(CHUNK_MENU_SCROLL, 1.0f);

                    nPreviousTabItem = nCurrentTabItem;
                    nCurrentTabItem--;

                    if (nCurrentTabItem < 0) {
                        nCurrentTabItem = GetLastMenuScreenTab() - 1;

                        if (nCurrentScreen == MENUSCREEN_GALLERY) {
                            nPreviousGalleryPage = nCurrentGalleryPage;
                            nCurrentGalleryPage--;

                            if (nCurrentGalleryPage < 0)
                                nCurrentGalleryPage = MAX_GALLERY_PAGES - 1;

                            bRequestScreenUpdate = true;
                        }
                    }

                    SetInputTypeAndClear(MENUINPUT_ENTRY, 0);
                    SetInputTypeAndClear(MENUINPUT_TAB, nCurrentTabItem);
                }
                else if (Down) {
                    if (GetLastMenuScreenTab() > 0)
                        Audio.PlayChunk(CHUNK_MENU_SCROLL, 1.0f);

                    nPreviousTabItem = nCurrentTabItem;
                    nCurrentTabItem++;

                    if (nCurrentTabItem > GetLastMenuScreenTab() - 1) {
                        nCurrentTabItem = 0;

                        if (nCurrentScreen == MENUSCREEN_GALLERY) {
                            nPreviousGalleryPage = nCurrentGalleryPage;
                            nCurrentGalleryPage++;

                            if (nCurrentGalleryPage > MAX_GALLERY_PAGES - 1)
                                nCurrentGalleryPage = 0;

                            bRequestScreenUpdate = true;
                        }
                    }

                    SetInputTypeAndClear(MENUINPUT_ENTRY, 0);
                    SetInputTypeAndClear(MENUINPUT_TAB, nCurrentTabItem);
                }
                else if (Enter) {
                    ProcessGoThrough(-99);
                }
                else if (Back) {
                    ProcessGoBack(-99);
                }

                if (!bShowMenu) {
                    if (nCurrentScreen == MENUSCREEN_MAP) {
                        CVector2D prevMapBase = vTempMapBase;
                        static int timeLeftMousePressed = 0;

                        if (LeftMouseDown) {
                            if (vMousePos.x != vOldMousePos.x || vMousePos.y != vOldMousePos.y)
                                timeLeftMousePressed = CTimer::m_snTimeInMillisecondsPauseMode + 200;
                        }

                        if (LeftMouseJustUp)
                            timeLeftMousePressed = 0;

                        if (timeLeftMousePressed > CTimer::m_snTimeInMillisecondsPauseMode) {
                            nMouseType = MOUSE_GRAB;

                            vTempMapBase.x += (vMousePos.x - vOldMousePos.x);
                            vTempMapBase.y += (vMousePos.y - vOldMousePos.y);
                        }
                        else {
                            float value = SCREEN_COORD(8.0f);
                            vTempMapBase.x += (LeftPressed ? value : RightPressed ? -value : 0.0f);
                            vTempMapBase.y += (UpPressed ? value : DownPressed ? -value : 0.0f);

                            if (WheelDown)
                                DoMapZoomInOut(true);
                            if (WheelUp)
                                DoMapZoomInOut(false);

                            if (MapZoomIn)
                                DoMapZoomInOut(false);
                            if (MapZoomOut)
                                DoMapZoomInOut(true);

                            if (LeftMouseDoubleClickJustDown) {
                                Audio.PlayChunk(CHUNK_MENU_SELECT, 1.0f);

                                SetWaypoint(vMousePos.x, vMousePos.y);
                            }

                            if (Enter) {
                                Audio.PlayChunk(CHUNK_MENU_SELECT, 1.0f);

                                SetWaypoint(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
                            }

                            if (ShowHideMapLegend) {
                                bShowLegend = bShowLegend == false;
                                Audio.PlayChunk(CHUNK_MENU_SELECT, 1.0f);
                            }
                        }

                        const float halfMapSize = GetMenuMapWholeSize() / 2;
                        if (vTempMapBase.x + halfMapSize < SCREEN_HALF_WIDTH ||
                            vTempMapBase.x - halfMapSize > SCREEN_HALF_WIDTH)
                            vTempMapBase.x = prevMapBase.x;

                        if (vTempMapBase.y + halfMapSize < SCREEN_HALF_HEIGHT ||
                            vTempMapBase.y - halfMapSize > SCREEN_HALF_HEIGHT)
                            vTempMapBase.y = prevMapBase.y;

                        vMapBase.x = interpF(vMapBase.x, vTempMapBase.x, 0.2f);
                        vMapBase.y = interpF(vMapBase.y, vTempMapBase.y, 0.2f);

                        static int mapMoveTime = 0;
                        static bool mapPlayShot = false;
                        if (vTempMapBase.x != prevMapBase.x || vTempMapBase.y != prevMapBase.y) {
                            if (!mapPlayShot && mapMoveTime < CTimer::m_snTimeInMillisecondsPauseMode) {
                                mapMoveTime = CTimer::m_snTimeInMillisecondsPauseMode + 100;
                                mapPlayShot = true;
                            }

                            if (mapPlayShot) {
                                Audio.PlayChunk(CHUNK_MENU_MAP_MOVE, 0.5f);
                                mapPlayShot = false;
                            }
                        }
                    }
                }
            }
            break;
        case MENUINPUT_ENTRY:
            if (!IsLoading()) {
                if (Up) {
                    Audio.PlayChunk(CHUNK_MENU_SCROLL, 1.0f);

                    while (true) {
                        nPreviousEntryItem = nCurrentEntryItem;
                        nCurrentEntryItem--;

                        if (nCurrentEntryItem < 0)
                            nCurrentEntryItem = MAX_MENU_ENTRIES - 1;

                        if (HasToContinueLoopInverse(nCurrentEntryItem))
                            break;
                    }
                }
                else if (Down) {
                    Audio.PlayChunk(CHUNK_MENU_SCROLL, 1.0f);

                    while (true) {
                        nPreviousEntryItem = nCurrentEntryItem;
                        nCurrentEntryItem++;

                        if (nCurrentEntryItem > MAX_MENU_ENTRIES - 1)
                            nCurrentEntryItem = 0;

                        if (HasToContinueLoopInverse(nCurrentEntryItem))
                            break;
                    }
                }
                else if (Back) {
                    ProcessGoBack(-99);
                }

                if (Enter || Left || Right) {
                    ProcessEntryStuff(Enter, Left ? -1 : Right ? 1 : 0);
                    Audio.PlayChunk(CHUNK_MENU_SELECT, 1.0f);
                }

                if (Space) {
                    Audio.PlayChunk(CHUNK_MENU_SELECT, 1.0f);

                    if (!faststrcmp(MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].tabName, "FE_GFX")) {
                        if (nMenuAlert == MENUALERT_PENDINGCHANGES)
                            ApplyGraphicsChanges();
                    }
                    if (!faststrcmp(MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].tabName, "FE_LGAM")) {
                        SetMenuMessage(MENUMESSAGE_DELETE_GAME);
                    }
                }
            }
            break;
        case MENUINPUT_MESSAGE:
            if (!IsLoading()) {
                if (Back)
                    Audio.PlayChunk(CHUNK_MENU_BACK, 1.0f);
                else if (Enter)
                    Audio.PlayChunk(CHUNK_MENU_SELECT, 1.0f);

                ProcessMessagesStuff(Enter, Back, Space, Left ? -1 : Right ? 1 : 0);
            }
            break;
        case MENUINPUT_GALLERYPIC:
            if (Back) {
                ProcessGoBack(-99);
            }
            break;
        case MENUINPUT_REDEFINEKEY:
            if (pad->GetMenuBackJustDown()) {
                Audio.PlayChunk(CHUNK_MENU_BACK, 1.0f);
                SetInputTypeAndClear(MENUINPUT_ENTRY, nCurrentEntryItem);
            }
            else {
                bool pressed = false;
                int key = rsNULL;

                for (int i = 0; i < ARRAY_SIZE(controlKeysStrings); i++) {
                    key = CPadNew::StringToKey(controlKeysStrings[i]);

                    if (CPadNew::GetPad(0)->GetKeyJustDown(key)) {
                        pressed = true;
                        break;
                    }
                }

                if (!pressed) {
                    const char* charMap[] = {
                        "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
                        "A", "B", "C", "D", "E", "F", "G", "H", "I", "J",
                        "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T",
                        "U", "V", "W", "X", "Y", "Z", ".", ",", ":", ";",
                        " ", "'", "{", "}", "[", "]", "-", "=", "_", "+"
                        "`",
                    };
                    for (int i = 0; i < ARRAY_SIZE(charMap); i++) {
                        key = CPadNew::StringToKey(charMap[i]);

                        if (CPadNew::GetPad(0)->GetKeyJustDown(key)) {
                            pressed = true;
                            break;
                        }
                    }
                }

                if (pressed && key != rsNULL && key != rsESC) {
                    Controls[nCurrentEntryItem].key = key;
                    Audio.PlayChunk(CHUNK_MENU_SELECT, 1.0f);
                    SetInputTypeAndClear(MENUINPUT_ENTRY, nCurrentEntryItem);
                    pressed = false;
                    CPadNew::GetPad(0)->Clear(false, 0);
                    ApplyChanges();

                    bDrawMouse = false;
                    nTimePassedSinceLastKeyBind = CTimer::m_snTimeInMillisecondsPauseMode + 500;
                }
            }
            break;
        }

        if (CRadioHud::CanRetuneRadioStation()) {
            switch (MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].Entries[nCurrentEntryItem].type) {
            case MENUENTRY_RADIOSTATION:
                StartRadio();
                break;
            default:
                StopRadio();
                break;
            }
        }

        if (nCurrentMessage == MENUMESSAGE_NONE) {
            switch (nCurrentScreen) {
            case MENUSCREEN_NONE:
                break;
            case MENUSCREEN_LANDING:
                break;
            case MENUSCREEN_LOADING:
                break;
            case MENUSCREEN_MAP:
                if (!bShowMenu) {
                    AppendHelpText("H_LG", HelpText_ShowHideLegend);
                    AppendHelpText("H_BAC", HelpText_GoBack);
                    AppendHelpText("H_WAYP", HelpText_SetWaypoint);
                    break;
                }
                [[fallthrough]];
            case MENUSCREEN_GAME:
                if (nCurrentInputType == MENUINPUT_ENTRY && MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].type == MENUENTRY_POPULATESAVESLOT) {
                    AppendHelpText("H_DEL", HelpText_DeleteSave);
                }
                [[fallthrough]];
            case MENUSCREEN_SETTINGS:
                if (nMenuAlert == MENUALERT_PENDINGCHANGES) {
                    AppendHelpText("H_APCH", HelpText_ApplyChanges);
                }
                [[fallthrough]];
            default:
                AppendHelpText("H_BAC", HelpText_GoBack);
                AppendHelpText("H_SEL", HelpText_GoThrough);
                break;
            }
        }
        else {
            switch (nCurrentMessage) {
            case MENUMESSAGE_NONE:
                break;
            default:
                AppendHelpText("H_BAC", HelpText_GoBack);
                AppendHelpText("H_SEL", HelpText_GoThrough);
                break;
            }
        }

        FindOutUsedMemory();
        RemoveUnusedControllerSettings();

        if (bRequestScreenUpdate) {
            fScreenAlpha = 0;
            nLoadingTime = GetTimeInMillisecondsRight() + MENU_SCREEN_CHANGE_WAIT_TIME;
            bRequestScreenUpdate = false;
            tabItemBeforeScreenChange = nPreviousTabItem;
        }

        if (!IsLoading()) {
            if (fScreenAlpha < 255.0f) {
                fScreenAlpha += 0.02f * 255.0f;
                fScreenAlpha = clamp(fScreenAlpha, 0, 255);
            }
            else
                fScreenAlpha = 255.0f;

            nLoadingTime = 0;

            if (bRequestMenuClose) {
                OpenCloseMenu(false, true);
                bRequestMenuClose = false;
            }
        }
    }
    else {
        if (TheCamera.GetScreenFadeStatus() != 2 
            && COverlayLayer::GetCurrentEffect() == EFFECT_NONE) {
            if (OpenClose) {
                OpenCloseMenu(true, false);
                return;
            }
        }
    }

    if (bStartOrLoadGame) {
        bLoad = true;
        nSlot = Settings.saveSlot;
        SetLoadingPageBehaviour();
        bStartOrLoadGame = false;
    }
}

void CMenuNew::ScanGalleryPictures(bool force) {
    if (!bScanGallery) {
        if (IsLoading() || force) {
            char name[16];
            char buff[128];

            for (int i = 0; i < MAX_GALLERY_PICTURES; i++) {
                sprintf(name, "gallery%d", i);
                sprintf(buff, "VHud\\gallery\\gallery%d.bmp", i);

                if (FileCheck(PLUGIN_PATH(buff))) {
                    if (!Gallery[i]) {
                        Gallery[i] = new CSprite2d;
                        Gallery[i]->m_pTexture = CTextureMgr::LoadBMPTextureCB(PLUGIN_PATH("VHud\\gallery"), name);
                    }
                    ++nGalleryCount;
                }
            }

            bScanGallery = true;
            bInvertInput = true;
        }
        else {
            bRequestScreenUpdate = true;
        }
    }
}

unsigned int CMenuNew::GetTimeInMillisecondsRight() {
    return CTimer::m_snTimeInMillisecondsPauseMode;
}

unsigned char CMenuNew::FadeIn(unsigned char alpha) {
    if (nCurrentInputType == MENUINPUT_BAR) {
        return (unsigned char)clamp(fScreenAlpha, 0, alpha / 3);
    }
    else
        return (unsigned char)clamp(fScreenAlpha, 0, alpha);
}

void CMenuNew::ProcessTabStuff() {
    ProcessEntryStuff(1, 0);
}

void CMenuNew::DoSettingsBeforeStartingAGame(bool load, int slot) {
    if (load) {
        if (CGenericGameStorage::CheckSlotDataValid(slot == -1 ? nCurrentEntryItem : slot, false)) {
            FrontEndMenuManager.m_bDontDrawFrontEnd = true;
            CGame::bMissionPackGame = false;
            FrontEndMenuManager.m_bLoadingData = true;
            FrontEndMenuManager.field_1B3C = false;
        }
        else {
            // Error
            // Start new game
        }
    }

    FrontEndMenuManager.DoSettingsBeforeStartingAGame();
    Clear();
}

void CMenuNew::DoStuffBeforeSavingAGame(int slot) {
    PcSaveHelper.SaveSlot(slot);
    bRequestMenuClose = true;
    bRequestScreenUpdate = true;
}

bool CMenuNew::ProcessMenuToGameSwitch(bool force) {
    if (bLoadingPage) {
        if (fLoadingPercentage >= 100.0f || force) {
            DoSettingsBeforeStartingAGame(bLoad, nSlot);
            return true;
        }
    }

    return false;
}

void CMenuNew::ProcessMessagesStuff(int enter, int esc, int space, int input) {
    if (!enter && !esc && !space && !input)
        return;

    switch (nCurrentMessage) {
    case MENUMESSAGE_NEW_GAME:
        if (enter) {
            bLoad = false;
            nSlot = -1;
            SetLoadingPageBehaviour();
        }
        else if (esc) {
            UnSetMenuMessage();
        }
        break;
    case MENUMESSAGE_LOAD_GAME:
        if (enter) {
            bLoad = true;
            nSlot = -1;
            SetLoadingPageBehaviour();
        }
        else if (esc) {
            UnSetMenuMessage();
        }
        break;
    case MENUMESSAGE_SAVE_GAME:
        if (enter) {
            DoStuffBeforeSavingAGame(nCurrentTabItem);
            UnSetMenuMessage();
        }
        else if (esc) {
            UnSetMenuMessage();
        }
        break;
    case MENUMESSAGE_EXIT_GAME:
        if (enter) {
            RsEventHandler(rsQUITAPP, (void*)FALSE);
        }
        else if (esc) {
            UnSetMenuMessage();
        }
        break;
    case MENUMESSAGE_DELETE_GAME:
        if (enter) {
            if (CGenericGameStorage::CheckSlotDataValid(nCurrentEntryItem, false)) {
                PcSaveHelper.DeleteSlot(nCurrentEntryItem);
                bSaveSlotsPopulated = false;
                UnSetMenuMessage();
                SetInputTypeAndClear(MENUINPUT_TAB, -1);
            }
        }
        else if (esc) {
            UnSetMenuMessage();
        }
        break;
    case MENUMESSAGE_LOSE_CHANGES_ASK:
        if (enter) {
            ApplyGraphicsChanges();
            UnSetMenuMessage();
        }
        else if (esc) {
            RestorePreviousSettings();
            UnSetMenuMessage();
        }
        break;
    }
}

void CMenuNew::ProcessAlertStuff() {
    switch (nMenuAlert) {
    case MENUALERT_PENDINGCHANGES:
        SetMenuMessage(MENUMESSAGE_LOSE_CHANGES_ASK);
        break;
    default:
        break;
    }
}

char** CMenuNew::GetVideoModeList() {
    int numModes = RwEngineGetNumVideoModes();
    char** list = new char* [numModes];

    NumVideoModes = numModes - 1;
    for (int i = 0; i < numModes; i++) {
        RwVideoMode vm;
        RwEngineGetVideoModeInfo(&vm, i);

        char tmp[100] = { NULL };
        if ((vm.flags & rwVIDEOMODEEXCLUSIVE)
            && vm.width >= 640 && vm.height >= 480 && vm.depth == 32) {
            sprintf(tmp, "%dx%d", vm.width, vm.height);
        }

        list[i] = new char[100];
        strcpy(list[i], tmp);
    }

    return list;
}

void CMenuNew::ProcessEntryStuff(int enter, int input) {
    int type = -1;

    switch (nCurrentInputType) {
    case MENUINPUT_BAR:
    case MENUINPUT_TAB:
        type = MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].type;
        break;
    case MENUINPUT_ENTRY:
        type = MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].Entries[nCurrentEntryItem].type;
        break;
    }

    if (type == -1)
        return;

    if (!enter && !input)
        return;

    switch (type) {
    case MENUENTRY_NONE:
        break;
    case MENUENTRY_ACTION:
        if (!faststrcmp(MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].tabName, "FE_NGAM")) {
            SetMenuMessage(MENUMESSAGE_NEW_GAME);
        }
        else if (!faststrcmp(MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].tabName, "FE_EXIT")) {
            SetMenuMessage(MENUMESSAGE_EXIT_GAME);
        }
        else if (!faststrcmp(MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].tabName, "FE_KBIN")) {
            nPreviousScreen = nCurrentScreen;
            nCurrentScreen = MENUSCREEN_KEYBIND;
            bRequestScreenUpdate = true;
            SetInputTypeAndClear(MENUINPUT_TAB);
        }
        break;
    case MENUENTRY_STORYMODE:
        bLoad = true;
        nSlot = 0;
        SetLoadingPageBehaviour();
        break;
    case MENUENTRY_SETTINGS:
        SetInputTypeAndClear(MENUINPUT_TAB, 0);
        OpenMenuScreen(MENUSCREEN_SETTINGS);
        SetDefaultPageBehaviour();
        break;
    case MENUENTRY_QUIT:
        SetMenuMessage(MENUMESSAGE_EXIT_GAME);
        break;
    case MENUENTRY_KEYBINDING:
    case MENUENTRY_PAD:
    case MENUENTRY_GFX:
    case MENUENTRY_POPULATESAVESLOT:
    case MENUENTRY_STAT:
        [[fallthrough]];
    case MENUENTRY_CHANGETAB:
        if (GetLastMenuScreenEntry() - 1 != -1)
            SetInputTypeAndClear(MENUINPUT_ENTRY, nPreviousEntryItem);
        break;
    case MENUENTRY_LOADGAME:
        if (enter) {
            SetMenuMessage(MENUMESSAGE_LOAD_GAME);
        }
        break;
    case MENUENTRY_SAVEGAME:
        if (enter) {
            SetMenuMessage(MENUMESSAGE_SAVE_GAME);
        }
        break;
    case MENUENTRY_SCREENTYPE:
        break;
    case MENUENTRY_CHANGERES:
        if (char** modes = VideoModeList) {
            if (input < 0) {
                while (true) {
                    TempSettings.videoMode--;
                    if (TempSettings.videoMode < 0)
                        TempSettings.videoMode = NumVideoModes;

                    if (modes[TempSettings.videoMode][0])
                        break;
                }
                ApplyChanges();
            }
            else if (input > 0) {
                while (true) {
                    TempSettings.videoMode++;
                    if (TempSettings.videoMode > NumVideoModes)
                        TempSettings.videoMode = 0;

                    if (modes[TempSettings.videoMode][0])
                        break;
                }
                ApplyChanges();
            }
        }
        break;
    case MENUENTRY_MIPMAPPING:
        TempSettings.mipMapping = TempSettings.mipMapping == false;
        ApplyChanges();
        break;
    case MENUENTRY_ASPECTRATIO:
        if (input < 0) {
            TempSettings.aspectRatio--;

            if (TempSettings.aspectRatio < 0)
                TempSettings.aspectRatio = 2;
        }
        else if (input > 0) {
            TempSettings.aspectRatio++;

            if (TempSettings.aspectRatio > 2)
                TempSettings.aspectRatio = 0;
        }
        ApplyChanges();
        break;
    case MENUENTRY_REFRESHRATE:
        break;
    case MENUENTRY_MSAA:
        if (int levels = RwD3D9EngineGetMaxMultiSamplingLevels()) {
            levels = clamp(levels, 1, 4);

            if (input < 0) {
                TempSettings.antiAliasing--;

                if (TempSettings.antiAliasing < 1)
                    TempSettings.antiAliasing = levels;
            }
            else if (input > 0) {
                TempSettings.antiAliasing++;

                if (TempSettings.antiAliasing > levels)
                    TempSettings.antiAliasing = 1;
            }
        }
        ApplyChanges();
        break;
    case MENUENTRY_FRAMELIMITER:
        TempSettings.frameLimiter = TempSettings.frameLimiter == false;
        ApplyChanges();
        break;
    case MENUENTRY_VISUALQUALITY:
        if (input < 0) {
            TempSettings.visualQuality--;

            if (TempSettings.visualQuality < 0)
                TempSettings.visualQuality = 3;
        }
        else if (input > 0) {
            TempSettings.visualQuality++;

            if (TempSettings.visualQuality > 3)
                TempSettings.visualQuality = 0;
        }
        ApplyChanges();
        break;
    case MENUENTRY_SHOWHUD:
        TempSettings.showHUD = TempSettings.showHUD == false;
        ApplyChanges();
        break;
    case MENUENTRY_SHOWRADAR:
        TempSettings.showRadar = TempSettings.showRadar == false;
        ApplyChanges();
        break;
    case MENUENTRY_WEAPONTARGET:
        if (input < 0) {
            TempSettings.weaponTarget--;

            if (TempSettings.weaponTarget < 0)
                TempSettings.weaponTarget = 1;
        }
        else if (input > 0) {
            TempSettings.weaponTarget++;

            if (TempSettings.weaponTarget > 1)
                TempSettings.weaponTarget = 0;
        }
        ApplyChanges();
        break;
    case MENUENTRY_GPSROUTE:
        TempSettings.gpsRoute = TempSettings.gpsRoute == false;
        ApplyChanges();
        break;
    case MENUENTRY_MEASUREMENTSYS:
        TempSettings.measurementSys = TempSettings.measurementSys == false;
        ApplyChanges();
        break;
    case MENUENTRY_SUBTITLES:
        TempSettings.subtitles = TempSettings.subtitles == false;
        ApplyChanges();
        break;
    case MENUENTRY_INVERTMOUSEY:
        TempSettings.invertMouseY = TempSettings.invertMouseY == false;
        ApplyChanges();
        break;
    case MENUENTRY_MOUSESTEER:
        TempSettings.mouseSteering = TempSettings.mouseSteering == false;
        ApplyChanges();
        break;
    case MENUENTRY_MOUSEFLYING:
        TempSettings.mouseFlying = TempSettings.mouseFlying == false;
        ApplyChanges();
        break;
    case MENUENTRY_RADIOSTATION:
        if (CRadioHud::CanRetuneRadioStation()) {
            if (input < 0) {
                TempSettings.radioStation--;

                if (TempSettings.radioStation < 1)
                    TempSettings.radioStation = 13;
            }
            else if (input > 0) {
                TempSettings.radioStation++;

                if (TempSettings.radioStation > 13)
                    TempSettings.radioStation = 1;
            }

            RetuneRadio(TempSettings.radioStation);
            ApplyChanges();
        }
        break;
    case MENUENTRY_RADIOAUTOSELECT:
        TempSettings.radioAutoSelect = TempSettings.radioAutoSelect == false;
        ApplyChanges();
        break;
    case MENUENTRY_RADIOEQ:
        TempSettings.radioEQ = TempSettings.radioEQ == false;
        ApplyChanges();
        break;
    case MENUENTRY_TRACKSAUTOSCAN:
        TempSettings.tracksAutoScan = TempSettings.tracksAutoScan == false;
        ApplyChanges();
        break;
    case MENUENTRY_RADIOMODE:
        TempSettings.radioMode = TempSettings.radioMode == false;
        ApplyChanges();
        break;
    case MENUENTRY_SHOWCONTROLSFOR:
        TempSettings.showControlsFor = TempSettings.showControlsFor == false;
        break;
    case MENUENTRY_INVERTPADX1:
        TempSettings.invertPadX1 = TempSettings.invertPadX1 == false;
        ApplyChanges();
        break;
    case MENUENTRY_INVERTPADY1:
        TempSettings.invertPadY1 = TempSettings.invertPadY1 == false;
        ApplyChanges();
        break;
    case MENUENTRY_INVERTPADX2:
        TempSettings.invertPadX2 = TempSettings.invertPadX2 == false;
        ApplyChanges();
        break;
    case MENUENTRY_INVERTPADY2:
        TempSettings.invertPadY2 = TempSettings.invertPadY2 == false;
        ApplyChanges();
        break;
    case MENUENTRY_SWAPPADAXIS1:
        TempSettings.swapPadAxis1 = TempSettings.swapPadAxis1 == false;
        ApplyChanges();
        break;
    case MENUENTRY_SWAPPADAXIS2:
        TempSettings.swapPadAxis2 = TempSettings.swapPadAxis2 == false;
        ApplyChanges();
        break;
    case MENUENTRY_RESTOREDEFAULTS:
        if (enter) {
            if (!faststrcmp(MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].tabName, "FE_PAD"))
                RestoreDefaults(&TempSettings, SETTINGS_GAMEPAD);
            else if (!faststrcmp(MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].tabName, "FE_KEY"))
                RestoreDefaults(&TempSettings, SETTINGS_KEYBOARD);
            else if (!faststrcmp(MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].tabName, "FE_AUD"))
                RestoreDefaults(&TempSettings, SETTINGS_AUDIO);
            else if (!faststrcmp(MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].tabName, "FE_DIS"))
                RestoreDefaults(&TempSettings, SETTINGS_DISPLAY);
            else if (!faststrcmp(MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].tabName, "FE_GFX"))
                RestoreDefaults(&TempSettings, SETTINGS_GRAPHICS);
            ApplyChanges();
        }
        break;
    case MENUENTRY_LANDINGPAGE:
        TempSettings.landingPage = TempSettings.landingPage == false;
        ApplyChanges();
        break;
    case MENUENTRY_SHOWPICTURE:
        if (Gallery[nCurrentTabItem] && Gallery[nCurrentTabItem]->m_pTexture) {
            SetInputTypeAndClear(MENUINPUT_GALLERYPIC);
            bShowPictureBigSize = true;
            //bShowMenu = false;
        }
        break;
    case MENUENTRY_REDEFINEKEY:
        SetInputTypeAndClear(MENUINPUT_REDEFINEKEY);
        break;
    case MENUENTRY_LANGUAGE:
        if (input < 0) {
            TempSettings.language--;

            if (TempSettings.language < 0)
                TempSettings.language = TextNew.TextSwitch.count;
        }
        else if (input > 0) {
            TempSettings.language++;

            if (TempSettings.language > TextNew.TextSwitch.count)
                TempSettings.language = 0;
        }
        ApplyChanges();
        break;
    
        // Sliders
    case MENUENTRY_DRAWDISTANCE:
    case MENUENTRY_BRIGHTNESS:
    case MENUENTRY_SFXVOLUME:
    case MENUENTRY_RADIOVOLUME:
    case MENUENTRY_MOUSESENSITIVITY:
    case MENUENTRY_GAMMA:
    case MENUENTRY_SAFEZONESIZE:
        CheckSliderMovement(input);
        ApplyChanges();
        break;
    }
}

void CMenuNew::RetuneRadio(char id) {
    AudioEngine.RetuneRadio(id);

    CRadioHud::m_nPreviousRadioId = CRadioHud::m_nCurrentRadioId;
    CRadioHud::m_nCurrentRadioId = id;

    MenuNew.TempSettings.radioStation = CRadioHud::m_nCurrentRadioId;
    MenuNew.Settings.radioStation = CRadioHud::m_nCurrentRadioId;
}

void CMenuNew::StartRadio() {
    if (bRadioEnabled || AudioEngine.IsRadioRetuneInProgress() || nCurrentInputType != MENUINPUT_ENTRY)
        return;

    if (!AECutsceneTrackManager.GetCutsceneTrackStatus()) {
        AERadioTrackManager.StartRadio(Settings.radioStation, 0, 0, 0);
        bRadioEnabled = true;
    }

    if (bLoadingTuneStarted) {
        fLoadingTuneVolume = 0.0f;
        Audio.SetVolumeForChunk(CHUNK_TD_LOADING_MUSIC, fLoadingTuneVolume);
    }
}

void CMenuNew::StopRadio() {
    if (!bRadioEnabled || AudioEngine.IsRadioRetuneInProgress())
        return;

    AERadioTrackManager.StopRadio(NULL, 0);
    bRadioEnabled = false;

    if (bLoadingTuneStarted) {
        fLoadingTuneVolume = 1.0f;
        Audio.SetVolumeForChunk(CHUNK_TD_LOADING_MUSIC, fLoadingTuneVolume);
    }
}

void CMenuNew::CheckSliderMovement(double value) {
    CMenuSettings& ts = MenuNew.TempSettings;

    if (value == 0)
        return;

    value = clamp(value, -1, 1);
    value /= 10.0;
    switch (MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].Entries[nCurrentEntryItem].type) {
    case MENUENTRY_DRAWDISTANCE:
        ts.drawDist += double(value * 1.8);
        ts.drawDist = clamp(ts.drawDist, 0.8, 1.8);
        break;
    case MENUENTRY_BRIGHTNESS:
        ts.brightness += int(value * 512);
        ts.brightness = clamp(ts.brightness, 0, 512);
        break;
    case MENUENTRY_GAMMA:
        ts.gamma += double(value * 1.0);
        ts.gamma = clamp(ts.gamma, 0, 1.0);
        gamma.SetGamma(ts.gamma, 1);
        break;
    case MENUENTRY_SAFEZONESIZE:
        ts.safeZoneSize += double(value * 96.0);
        ts.safeZoneSize = clamp(ts.safeZoneSize, 0, 96.0);
        break;
    case MENUENTRY_SFXVOLUME:
        ts.sfxVolume += char(value * 64);
        ts.sfxVolume = clamp(ts.sfxVolume, 0, 64);
        AudioEngine.SetEffectsMasterVolume(ts.sfxVolume);
        Audio.SetChunksMasterVolume(ts.sfxVolume);
        Audio.SetVolumeForChunk(CHUNK_TD_LOADING_MUSIC, fLoadingTuneVolume);
        break;
    case MENUENTRY_RADIOVOLUME:
        ts.radioVolume += char(value * 64);
        ts.radioVolume = clamp(ts.radioVolume, 0, 64);
        AudioEngine.SetMusicMasterVolume(ts.radioVolume);
        break;
    case MENUENTRY_MOUSESENSITIVITY:
        ts.mouseSensitivity += double(value * (1.0 / 200.0));
        ts.mouseSensitivity = clamp(ts.mouseSensitivity, 1.0 / 3200.0, 1.0 / 200.0);
        TheCamera.m_fMouseAccelHorzntl = (float)ts.mouseSensitivity;
        TheCamera.m_fMouseAccelVertical = (float)ts.mouseSensitivity;
        break;
    }
}

void CMenuNew::DrawPauseMenuExtraText() {
    CPlayerInfo playa = CWorld::Players[CWorld::PlayerInFocus];

    // Header
    CFontNew::SetBackground(false);
    CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
    CFontNew::SetAlignment(CFontNew::ALIGN_LEFT);
    CFontNew::SetWrapX(SCREEN_COORD(640.0f));
    CFontNew::SetFontStyle(CFontNew::FONT_4);
    CFontNew::SetDropShadow(SCREEN_COORD(2.0f));
    CFontNew::SetOutline(0.0f);
    CFontNew::SetDropColor(CRGBA(0, 0, 0, 255));
    CFontNew::SetColor(CRGBA(255, 255, 255, 255));
    CFontNew::SetScale(SCREEN_MULTIPLIER(1.2f), SCREEN_MULTIPLIER(2.76f));

    char* header = NULL;
    switch (nCurrentScreen) {
    case MENUSCREEN_SAVE:
        header = TextNew.GetText("FE_SGAM").text;
        break;
    default:
        if (SAMP)
            header = TextNew.GetText("FE_GTAO").text;
        else
            header = TextNew.GetText("FE_GTA").text;
        break;
    }

    if (header)
        CFontNew::PrintString(MENU_X(311.0f), MENU_Y(111.0f), header);

    // Player info
    if (playa.m_pPed) {
        CRect portrait;
        portrait.left = MENU_RIGHT(311.0f + 69.0f);
        portrait.top = MENU_Y(96.0f);
        portrait.right = portrait.left + SCREEN_COORD(69.0f);
        portrait.bottom = portrait.top + SCREEN_COORD(69.0f);

        CSprite2d::DrawRect(portrait, HudColourNew.GetRGB(HUD_COLOUR_BLACK, 255));
        CHudNew::CheckPlayerPortrait(CWorld::PlayerInFocus);
        CHudNew::PlayerPortrait[CWorld::PlayerInFocus][1]->Draw(portrait, CRGBA(255, 255, 255, 255));

        CFontNew::SetDropShadow(SCREEN_COORD(2.0f));
        CFontNew::SetAlignment(CFontNew::ALIGN_RIGHT);
        CFontNew::SetScale(SCREEN_MULTIPLIER(0.58f), SCREEN_MULTIPLIER(1.42f));

        char playerName[16];
        sprintf(playerName, "%s", PedNameList[playa.m_pPed->m_nModelIndex]);
        TextNew.UpperCase(playerName);
        CFontNew::PrintString(MENU_RIGHT(311.0f + 69.0f + 8.0f), MENU_Y(92.0f), playerName);

        char daytmp[32];
        char* day = NULL;
        sprintf(daytmp, "DAY_%d", CClock::CurrentDay);
        day = TextNew.GetText(daytmp).text;
        
        if (day) {
            TextNew.UpperCase(day);
            CFontNew::PrintString(MENU_RIGHT(311.0f + 69.0f + 58.0f), MENU_Y(92.0f + 23.0), day);
        }

        char time[16];
        sprintf(time, "%02d:%02d", CClock::ms_nGameClockHours, CClock::ms_nGameClockMinutes);
        CFontNew::PrintString(MENU_RIGHT(311.0f + 69.0f + 8.0f), MENU_Y(92.0f + 23.0), time);

        char money[32];
        sprintf(money, "$%d", playa.m_nMoney);
        CFontNew::PrintString(MENU_RIGHT(311.0f + 69.0f + 8.0f), MENU_Y(92.0f + 46.0f), money);
    }
}

void CMenuNew::Draw() {
    if (!bMenuActive)
        return;

    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)(rwFILTERLINEARMIPLINEAR));
    RwRenderStateSet(rwRENDERSTATETEXTUREPERSPECTIVE, (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*)rwTEXTUREADDRESSCLAMP);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);

    CFontNew::SetClipX(SCREEN_WIDTH);
    CFontNew::SetWrapX(SCREEN_WIDTH);

    // Set
    if (bNoTransparentBackg) {
        DrawBackground();
    }
    else {
        COverlayLayer::SetEffect(EFFECT_BLUR_COLOR);
    }

    if (bShowMenu) {
        if (bShowMenuExtraText)
            DrawPauseMenuExtraText();

        if (bShowMenuBar) {
            // Menubar
            CRect menuBar;
            CRect menuBarSelected;
            CRGBA menuBarColor;
            CRGBA menuBarSelectedColor;
            CRGBA menuBarTextColor;
            const float spacing = SCREEN_COORD(3.0f);

            menuBar = GetMenuBarRect();

            menuBarSelected.left = menuBar.left;
            menuBarSelected.top = menuBar.top + SCREEN_COORD(-8.0f);
            menuBarSelected.right = menuBar.right;
            menuBarSelected.bottom = SCREEN_COORD(8.0f);

            CPadNew* pad = CPadNew::GetPad(0);

            nCurrentBarItemHover = MENU_HOVER_NONE;
            for (int i = 0; i < MAX_MENU_BAR_ITEMS; i++) {
                if (MenuBar[i].barName[0] == '\0')
                    continue;

                if (bDrawMouse && CheckHover(menuBar.left, menuBar.left + menuBar.right, menuBar.top, menuBar.top + menuBar.bottom)) {
                    nCurrentBarItemHover = i;

                    if (pad->GetLeftMouseJustUp()) {
                        Audio.PlayChunk(CHUNK_MENU_SELECT, 1.0f);

                        if (nCurrentBarItemHover != nCurrentBarItem)
                            bRequestScreenUpdate = true;

                        SetInputTypeAndClear(MENUINPUT_TAB);
                        SetInputTypeAndClear(MENUINPUT_ENTRY);
                        SetInputTypeAndClear(MENUINPUT_BAR, nCurrentBarItemHover);         
                    }
                }

                menuBarColor = { 0, 0, 0, 180 };
                menuBarSelectedColor = { 0, 0, 0, 0 };
                menuBarTextColor = HudColourNew.GetRGB(HUD_COLOUR_WHITE, 255);

                if (i == nCurrentBarItem) {
                    menuBarColor = { 255, 255, 255, 255 };
                    menuBarSelectedColor = HudColourNew.GetRGB(Settings.uiMainColor, 255);
                    menuBarTextColor = HudColourNew.GetRGB(HUD_COLOUR_BLACK, 255);
                }
                else if (i == nCurrentBarItemHover) {
                    menuBarColor = { 50, 50, 50, 180 };
                    menuBarSelectedColor = { 0, 0, 0, 0 };
                    menuBarTextColor = HudColourNew.GetRGB(HUD_COLOUR_WHITE, 255);
                }

                // Back
                CSprite2d::DrawRect(CRect(menuBar.left, menuBar.top, menuBar.left + menuBar.right, menuBar.top + menuBar.bottom), menuBarColor);

                // Selected line
                CSprite2d::DrawRect(CRect(menuBarSelected.left, menuBarSelected.top, menuBarSelected.left + menuBarSelected.right, menuBarSelected.top + menuBarSelected.bottom), menuBarSelectedColor);

                CFontNew::SetBackground(false);
                CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
                CFontNew::SetAlignment(CFontNew::ALIGN_CENTER);
                CFontNew::SetWrapX(SCREEN_COORD(640.0f));
                CFontNew::SetFontStyle(CFontNew::FONT_1);
                CFontNew::SetDropShadow(0.0f);
                CFontNew::SetOutline(0.0f);
                CFontNew::SetDropColor(CRGBA(0, 0, 0, 0));
                CFontNew::SetColor(menuBarTextColor);
                CFontNew::SetScale(SCREEN_MULTIPLIER(0.6f), SCREEN_MULTIPLIER(1.2f));

                char* str = TextNew.GetText(MenuBar[i].barName).text;
                TextNew.UpperCase(str);
                CFontNew::PrintString(menuBar.left + (menuBar.right * 0.5f), menuBar.top + SCREEN_COORD(6.0f), str);

                menuBar.left += menuBar.right + spacing;
                menuBarSelected.left += menuBarSelected.right + spacing;
            }
        }
        else {
            char* str = MenuScreen[nCurrentScreen].screenName;

            if (faststrcmp(str, "BLANK")) {
                str = TextNew.GetText(str).text;

                CRect bar;
                CRGBA barColor;
                CRect barSelected;
                CRGBA barSelectedColor;
                CRGBA barTextColor;

                bar = GetMenuBarRect();
                bar.right = GetMenuScreenRect().right;

                barColor = { 255, 255, 255, 255 };
                barSelectedColor = HudColourNew.GetRGB(Settings.uiMainColor, 255);
                barTextColor = HudColourNew.GetRGB(HUD_COLOUR_BLACK, 255);

                // Bar
                CSprite2d::DrawRect(CRect(bar.left, bar.top, bar.left + bar.right, bar.top + bar.bottom), barColor);

                barSelected.left = bar.left;
                barSelected.top = bar.top + SCREEN_COORD(-8.0f);
                barSelected.right = bar.right;
                barSelected.bottom = SCREEN_COORD(8.0f);

                // Selected line
                CSprite2d::DrawRect(CRect(barSelected.left, barSelected.top, barSelected.left + barSelected.right, barSelected.top + barSelected.bottom), barSelectedColor);

                CFontNew::SetBackground(false);
                CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
                CFontNew::SetAlignment(CFontNew::ALIGN_CENTER);
                CFontNew::SetWrapX(SCREEN_COORD(640.0f));
                CFontNew::SetFontStyle(CFontNew::FONT_1);
                CFontNew::SetDropShadow(0.0f);
                CFontNew::SetOutline(0.0f);
                CFontNew::SetDropColor(CRGBA(0, 0, 0, 0));
                CFontNew::SetColor(barTextColor);
                CFontNew::SetScale(SCREEN_MULTIPLIER(0.6f), SCREEN_MULTIPLIER(1.2f));

                TextNew.UpperCase(str);
                CFontNew::PrintString(bar.left + (bar.right * 0.5f), bar.top + SCREEN_COORD(6.0f), str);
            }
        }
    }

    if (nCurrentMessage == MENUMESSAGE_NONE) {
        if (IsLoading()) {
            const float mult = 1.0f;
            const float scale = 32.0f * mult;

            if (!bLandingPage)
                DrawSpinningWheel(SCREEN_COORD_CENTER_LEFT(0.0f), SCREEN_COORD_CENTER_DOWN(0.0f), SCREEN_COORD(scale), SCREEN_COORD(scale));
        }
        else {
            switch (nCurrentScreen) {
            case MENUSCREEN_LOADING:
                DrawLoadingPage();
                break;
            case MENUSCREEN_BRIEF:
                PrintBrief();
                break;
            case MENUSCREEN_MAP:
                DrawMap();
                break;
            case MENUSCREEN_GALLERY:
                DrawGallery();
                break;
            case MENUSCREEN_LANDING:
                DrawLandingPage();
                break;
            case MENUSCREEN_STATS:
                PrintStats();
                [[fallthrough]];
            default:
                DrawDefault();
                break;
            }
        }

        // Eh
        const CVector2D currScreen = { SCREEN_WIDTH, SCREEN_HEIGHT };
        static CVector2D prevScreen = currScreen;
        if (currScreen.x != prevScreen.x || currScreen.y != prevScreen.y) {
            vMapBase.x = vMapBase.x * currScreen.x / prevScreen.x;
            vMapBase.y = vMapBase.y * currScreen.y / prevScreen.y;
            vTempMapBase = vMapBase;

            prevScreen.x = currScreen.x;
            prevScreen.y = currScreen.y;
        }

        if (MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].Entries[nCurrentEntryItem].type == MENUENTRY_SAFEZONESIZE) {
            static double previousSafeZone = TempSettings.safeZoneSize;
            if (TempSettings.safeZoneSize != previousSafeZone) {
                nTimeForSafeZonesToShow = 1500 + CTimer::m_snTimeInMillisecondsPauseMode;
                previousSafeZone = TempSettings.safeZoneSize;
            }
        }

        if (nTimeForSafeZonesToShow > CTimer::m_snTimeInMillisecondsPauseMode) {
            DrawSafeZoneAngle(HUD_X(96.0f), HUD_Y(96.0f * SAFE_ZONE_HEIGHT_MULT), 1, 1);
            CSprite2d::DrawRect(CRect(HUD_X(96.0f), HUD_Y(96.0f * SAFE_ZONE_HEIGHT_MULT), 0.0f, 0.0f), CRGBA(0, 0, 0, 100));
            CSprite2d::DrawRect(CRect(0.0f, HUD_Y(96.0f * SAFE_ZONE_HEIGHT_MULT), HUD_X(96.0f), HUD_BOTTOM(96.0f * SAFE_ZONE_HEIGHT_MULT)), CRGBA(0, 0, 0, 100));

            DrawSafeZoneAngle(HUD_RIGHT(96.0f), HUD_Y(96.0f * SAFE_ZONE_HEIGHT_MULT), -1, 1);
            CSprite2d::DrawRect(CRect(HUD_RIGHT(96.0f), HUD_Y(96.0f * SAFE_ZONE_HEIGHT_MULT), SCREEN_WIDTH, 0.0f), CRGBA(0, 0, 0, 100));
            CSprite2d::DrawRect(CRect(HUD_RIGHT(96.0f), HUD_Y(96.0f * SAFE_ZONE_HEIGHT_MULT), SCREEN_WIDTH, HUD_BOTTOM(96.0f * SAFE_ZONE_HEIGHT_MULT)), CRGBA(0, 0, 0, 100));

            DrawSafeZoneAngle(HUD_X(96.0f), HUD_BOTTOM(96.0f * SAFE_ZONE_HEIGHT_MULT), 1, -1);
            CSprite2d::DrawRect(CRect(HUD_X(96.0f), HUD_BOTTOM(96.0f * SAFE_ZONE_HEIGHT_MULT), 0.0f, SCREEN_HEIGHT), CRGBA(0, 0, 0, 100));
            CSprite2d::DrawRect(CRect(HUD_X(96.0f), 0.0f, HUD_RIGHT(96.0f), HUD_Y(96.0f * SAFE_ZONE_HEIGHT_MULT)), CRGBA(0, 0, 0, 100));

            DrawSafeZoneAngle(HUD_RIGHT(96.0f), HUD_BOTTOM(96.0f * SAFE_ZONE_HEIGHT_MULT), -1, -1);
            CSprite2d::DrawRect(CRect(HUD_RIGHT(96.0f), HUD_BOTTOM(96.0f * SAFE_ZONE_HEIGHT_MULT), SCREEN_WIDTH, SCREEN_HEIGHT), CRGBA(0, 0, 0, 100));
            CSprite2d::DrawRect(CRect(HUD_X(96.0f), HUD_BOTTOM(96.0f * SAFE_ZONE_HEIGHT_MULT), HUD_RIGHT(96.0f), SCREEN_HEIGHT), CRGBA(0, 0, 0, 100));
        }
    }
    else {
        char* header = NULL;
        char* msg = NULL;
        switch (nCurrentMessage) {
        case MENUMESSAGE_NEW_GAME:
            header = "FE_NGAM";
            msg = "FE_NGAM2";
            break;
        case MENUMESSAGE_LOAD_GAME:
            header = "FE_LGAM";
            msg = "FE_LGAM1";
            break;
        case MENUMESSAGE_SAVE_GAME:
            header = "FE_SGAM";
            msg = "FE_SGAM1";
            break;
        case MENUMESSAGE_EXIT_GAME:
            header = "FE_EXITW";
            msg = "FE_EXITW1";
            break;
        case MENUMESSAGE_DELETE_GAME:
            header = "FE_DGAM";
            msg = "FE_DGAM1";
            break;
        case MENUMESSAGE_LOSE_CHANGES_ASK:
            header = "FE_ALERT";
            msg = "FE_PENDSET";
            break;
        }

        CSprite2d::DrawRect(CRect(-5.0f, -5.0f, SCREEN_WIDTH + 5.0f, SCREEN_HEIGHT + 5.0f), CRGBA(0, 0, 0, 255));

        CFontNew::SetBackground(false);
        CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
        CFontNew::SetAlignment(CFontNew::ALIGN_CENTER);
        CFontNew::SetWrapX(SCREEN_COORD(1200.0f));
        CFontNew::SetFontStyle(CFontNew::FONT_3);
        CFontNew::SetDropShadow(0.0f);
        CFontNew::SetOutline(0.0f);
        CFontNew::SetDropColor(CRGBA(0, 0, 0, 0));
        CFontNew::SetColor(HudColourNew.GetRGB(HUD_COLOUR_YELLOW, 255));
        CFontNew::SetScale(SCREEN_MULTIPLIER(3.2f), SCREEN_MULTIPLIER(6.3f));

        if (header) {
            CFontNew::PrintStringFromBottom(SCREEN_COORD_CENTER_X, SCREEN_COORD_CENTER_Y + SCREEN_COORD(-116.0f), TextNew.GetText(header).text);
        }

        if (msg) {
            CFontNew::SetFontStyle(CFontNew::FONT_1);
            CFontNew::SetColor(HudColourNew.GetRGB(HUD_COLOUR_WHITE, 255));
            CFontNew::SetScale(SCREEN_MULTIPLIER(0.86f), SCREEN_MULTIPLIER(1.80f));

            char* str = TextNew.GetText(msg).text;
            int n = CFontNew::PrintString(SCREEN_COORD_CENTER_X, SCREEN_COORD_CENTER_Y + SCREEN_COORD(36.0f), str);

            CRect r;
            r.left = SCREEN_COORD_CENTER_RIGHT(-64.0f) - clamp(CFontNew::GetStringWidth(str, true), 0, CFontNew::Details.wrapX) / 2;
            r.top = SCREEN_COORD_CENTER_Y + SCREEN_COORD(28.0f);
            r.right = SCREEN_COORD_CENTER_RIGHT(64.0f) + clamp(CFontNew::GetStringWidth(str, true), 0, CFontNew::Details.wrapX) / 2;
            r.bottom = SCREEN_COORD(3.0f);
            CSprite2d::DrawRect(CRect(r.left, r.top, r.right, r.top + r.bottom), CRGBA(HudColourNew.GetRGB(HUD_COLOUR_WHITE, 255)));

            r.top += SCREEN_COORD(22.0f);
            r.top += CFontNew::GetHeightScale(CFontNew::Details.scale.y) * (n);
            CSprite2d::DrawRect(CRect(r.left, r.top, r.right, r.top + r.bottom), CRGBA(HudColourNew.GetRGB(HUD_COLOUR_WHITE, 255)));
        }
    }

    DrawControlsHelper();

    if (bDrawMouse) {
        if (nMouseType <= MOUSE_HAND) {
            RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*)rwTEXTUREADDRESSCLAMP);
            MenuSprites[nMouseType]->Draw(vMousePos.x, vMousePos.y, SCREEN_COORD(30.0f), SCREEN_COORD(32.0f), CRGBA(255, 255, 255, 255));
            nMouseType = MOUSE_ARROW;
        }
    }
}

void CMenuNew::DrawControlsHelper() {
    CRect r;
    r.left = HUD_RIGHT(96.0f);
    r.right = SCREEN_COORD(32.0f);
    r.top = HUD_BOTTOM(90.0f);
    r.bottom = SCREEN_COORD(36.0f);

    static float x = 0.0f;
    CSprite2d::DrawRect(CRect(x, r.top, r.left, r.top + r.bottom), CRGBA(0, 0, 0, 150));
    x = r.left;

    CFontNew::SetBackground(false);
    CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
    CFontNew::SetAlignment(CFontNew::ALIGN_RIGHT);
    CFontNew::SetWrapX(SCREEN_COORD(980.0f));
    CFontNew::SetFontStyle(CFontNew::FONT_1);
    CFontNew::SetDropShadow(0.0f);
    CFontNew::SetOutline(0.0f);
    CFontNew::SetDropColor(CRGBA(0, 0, 0, 0));
    CFontNew::SetColor(HudColourNew.GetRGB(HUD_COLOUR_WHITE, 255));
    CFontNew::SetScale(SCREEN_MULTIPLIER(0.6f), SCREEN_MULTIPLIER(1.2f));

    float spacing = SCREEN_COORD(24.0f);
    float offset = SCREEN_COORD(10.0f);
    float ro = SCREEN_COORD(6.0f);
    for (int i = nControlsHelperCount; i >= 0; i--) {
        char* str = nControlsHelper[i].text;

        if (str) {
            str = TextNew.GetText(str).text;

            CPadNew* pad = CPadNew::GetPad(0);
            if (bDrawMouse && CheckHover((x - offset - ro) - CFontNew::GetStringWidth(str, true), (x - offset + ro), r.top, r.top + r.bottom)) {
                CSprite2d::DrawRect(CRect(x - offset - ro - CFontNew::GetStringWidth(str, true), r.top, (x - offset + ro), r.top + r.bottom), CRGBA(150, 150, 150, 150));

                if (pad->GetLeftMouseJustUp()) {
                    if (nControlsHelper[i].hasFunc)
                        nControlsHelper[i].func();
                }
            }

            CFontNew::PrintString(x - offset, r.top + SCREEN_COORD(4.0f), str);
            x -= CFontNew::GetStringWidth(str, true);
            x -= spacing;
        }

        nControlsHelper[i].text = NULL;
        nControlsHelper[i].hasFunc = false;
        nControlsHelper[i].func = NULL;
    }
    nControlsHelperCount = 0;
}

void CMenuNew::AppendHelpText(const char* text, funcCall func) {
    nControlsHelper[nControlsHelperCount].text = (char*)text;

    if (func) {
        nControlsHelper[nControlsHelperCount].hasFunc = true;
        nControlsHelper[nControlsHelperCount].func = func;
    }
    nControlsHelperCount++;
}

void CMenuNew::SetMenuMessage(int type) {
    if (type != nCurrentMessage) {
        nPreviousMessage = nCurrentMessage;
        nCurrentMessage = type;
        SetInputTypeAndClear(MENUINPUT_MESSAGE);
    }
}

void CMenuNew::UnSetMenuMessage() {
    if (nCurrentMessage > MENUMESSAGE_NONE) {
        nPreviousMessage = nCurrentMessage;
        nCurrentMessage = MENUMESSAGE_NONE;
        SetInputTypeAndClear(nPreviousInputType, -1);
    }
}

bool CMenuNew::IsLoading() {
    if (nLoadingTime > GetTimeInMillisecondsRight())
        return true;

    nLoadingTime = 0;
    return false;
}

int CMenuNew::GetNumOfSaveGames() {
    int result = 0;
    for (int i = 0; i < 9; i++) {
        char* str = CGenericGameStorage::GetNameOfSavedGame(i);

        strcpy(nSaveSlots[i], str);

        char* dat = CGenericGameStorage::ms_SlotSaveDate[i].m_sSavedGameDateAndTime;
        strcpy(nSaveSlotsDate[i], dat);

        if (str[0] != '\0')
            result++;
    }

    return result;
}

void CMenuNew::DrawSafeZoneAngle(float x, float y, int w, int h) {
    CRect rect;
    rect.left = x;
    rect.top = y;
    rect.right = x + (SCREEN_COORD(14.0f) * w);
    rect.bottom = y + (SCREEN_COORD(50.0f) * h);
    CSprite2d::DrawRect(rect, HudColourNew.GetRGB(HUD_COLOUR_WHITE, 200));

    rect.left = rect.right;
    rect.right = x + (SCREEN_COORD(50.0f) * w);
    rect.bottom = y + (SCREEN_COORD(14.0f) * h);
    CSprite2d::DrawRect(rect, HudColourNew.GetRGB(HUD_COLOUR_WHITE, 200));
}

void CMenuNew::DrawDefault() {
    CPadNew* pad = CPadNew::GetPad(0);

    {
        CRect menuTab;
        CRGBA menuTabColor;
        CRGBA menuTabTextColor;

        menuTab = GetMenuTabRect();
        nCurrentTabItemHover = -1;
        for (int i = 0; i < MAX_MENU_TABS; i++) {
            char* leftText = NULL;
            char* rightText = NULL;
            char strTmp[32];
            if (MenuScreen[nCurrentScreen].Tab[i].tabName[0] == '\0')
                continue;

            if (MenuScreen[nCurrentScreen].Tab[i].full)
                menuTab.right = GetMenuScreenRect().right;

            if (MenuScreen[nCurrentScreen].Tab[i].type == MENUENTRY_SAVEGAME) {
                leftText = nSaveSlots[i];

                if (leftText[0] != '\0') {
                    sprintf(strTmp, "%02d - %s", i + 1, leftText);
                }
                else
                    sprintf(strTmp, "%s %d/8", TextNew.GetText("FE_NEWSAV").text, i + 1);

                leftText = strTmp;
                rightText = nSaveSlotsDate[i];
            }
            else {
                leftText = TextNew.GetText(MenuScreen[nCurrentScreen].Tab[i].tabName).text;
            }

            if (bDrawMouse && CheckHover(menuTab.left, menuTab.left + menuTab.right, menuTab.top, menuTab.top + menuTab.bottom)) {
                nCurrentTabItemHover = i;

                if (pad->GetLeftMouseJustUp()) {
                    Audio.PlayChunk(CHUNK_MENU_SELECT, 1.0f);

                    if (nCurrentTabItemHover == nCurrentTabItem)
                        ProcessTabStuff();
                    else
                        SetInputTypeAndClear(MENUINPUT_TAB, nCurrentTabItemHover);
                }
            }
            menuTabColor = { 0, 0, 0, FadeIn(180) };
            menuTabTextColor = HudColourNew.GetRGB(HUD_COLOUR_WHITE, FadeIn(255));

            if (i == nCurrentTabItemHover) {
                menuTabColor = { 100, 100, 100, FadeIn(180) };
                menuTabTextColor = HudColourNew.GetRGB(HUD_COLOUR_WHITE, FadeIn(255));
            }

            if (nCurrentInputType != MENUINPUT_BAR) {
                if (i == nCurrentTabItem) {
                    menuTabColor = { 255, 255, 255, FadeIn(255) };
                    menuTabTextColor = HudColourNew.GetRGB(HUD_COLOUR_BLACK, FadeIn(255));
                }
            }

            CSprite2d::DrawRect(CRect(menuTab.left, menuTab.top, menuTab.left + menuTab.right, menuTab.top + menuTab.bottom), menuTabColor);

            CFontNew::SetBackground(false);
            CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
            CFontNew::SetAlignment(CFontNew::ALIGN_LEFT);
            CFontNew::SetWrapX(SCREEN_COORD(640.0f));
            CFontNew::SetFontStyle(CFontNew::FONT_1);
            CFontNew::SetDropShadow(0.0f);
            CFontNew::SetOutline(0.0f);
            CFontNew::SetDropColor(CRGBA(0, 0, 0, 0));
            CFontNew::SetColor(menuTabTextColor);
            CFontNew::SetScale(SCREEN_MULTIPLIER(0.6f), SCREEN_MULTIPLIER(1.2f));

            CFontNew::PrintString(menuTab.left + SCREEN_COORD(12.0f), menuTab.top + SCREEN_COORD(6.0f), leftText);

            if (MenuScreen[nCurrentScreen].Tab[i].full && rightText) {
                CFontNew::SetAlignment(CFontNew::ALIGN_RIGHT);
                CFontNew::PrintString(menuTab.left + menuTab.right + SCREEN_COORD(-12.0f), menuTab.top + SCREEN_COORD(6.0f), rightText);
            }

            menuTab.top += menuTab.bottom + GetMenuHorSpacing();
        }
    }

    if (MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].type == MENUENTRY_ACTION) {
        CRect rect = GetMenuEntryRect();
        rect.bottom = GetMenuScreenRect().bottom;
        DrawPatternBackground(CRect(rect.left, rect.top, rect.left + rect.right, rect.top + rect.bottom), CRGBA(0, 0, 0, FadeIn(180)));

        CFontNew::SetBackground(false);
        CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
        CFontNew::SetAlignment(CFontNew::ALIGN_LEFT);
        CFontNew::SetWrapX(SCREEN_COORD(720.0f));
        CFontNew::SetFontStyle(CFontNew::FONT_1);
        CFontNew::SetDropShadow(0.0f);
        CFontNew::SetOutline(0.0f);
        CFontNew::SetDropColor(CRGBA(0, 0, 0, 0));
        CFontNew::SetColor(HudColourNew.GetRGB(HUD_COLOUR_WHITE, FadeIn(255)));
        CFontNew::SetScale(SCREEN_MULTIPLIER(2.6f), SCREEN_MULTIPLIER(4.8f));

        char* h = TextNew.GetText(MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].tabName).text;
        char* a = TextNew.GetText(MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].actionName).text;
        if (h)
            CFontNew::PrintString(rect.left + SCREEN_COORD(32.0f), rect.top + SCREEN_COORD(19.0f), h);

        if (a) {
            CFontNew::SetScale(SCREEN_MULTIPLIER(0.6f), SCREEN_MULTIPLIER(1.2f));
            CFontNew::PrintString(rect.left + SCREEN_COORD(32.0f), rect.top + SCREEN_COORD(148.0f), a);
        }
    }
    else {
        CRect menuEntry;
        CRGBA menuEntryColor;
        CRGBA menuEntryTextColor;
        float shiftText;

        menuEntry = GetMenuEntryRect();

        // Background rect.
        menuEntry.top += (menuEntry.bottom + GetMenuHorSpacing()) * MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].Entries[0].y;
        static float bb = 0.0f;
        bb = clamp(bb, 0.0f, (menuEntry.bottom + GetMenuHorSpacing()) * VISIBLE_ENTRIES - 1);
        CSprite2d::DrawRect(CRect(menuEntry.left, menuEntry.top, menuEntry.left + menuEntry.right, menuEntry.top + bb), CRGBA(0, 0, 0, FadeIn(180)));
        bb = max((menuEntry.bottom * (GetEntryBackHeight() + 1)) + (GetMenuHorSpacing() * (GetEntryBackHeight())), 0.0f);
        //

        nCurrentEntryItemHover = -1;

        if (GetLastMenuScreenEntry() > VISIBLE_ENTRIES) {
            menuEntry.top -= fMenuEntryScrollOffset;
            CRect r = GetMenuEntryRect();
            r.top += (r.bottom + GetMenuHorSpacing()) * VISIBLE_ENTRIES;
            r.top += (r.bottom + GetMenuHorSpacing()) * MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].Entries[0].y;

            CSprite2d::DrawRect(CRect(r.left, r.top, r.left + r.right, r.top + r.bottom), CRGBA(0, 0, 0, FadeIn(180)));
            CRect s = {};

            const float is = SCREEN_COORD(16.0f);
            s.left = r.left + (r.right * 0.5f);
            s.top = r.top + (r.bottom * 0.5f);

            s.right = s.left + is;
            s.bottom = s.top + is;
            s.left -= is;
            s.top -= is;

            MenuSprites[MENU_SELECTOR]->Draw(s, CRGBA(255, 255, 255, 255));
        }

        static bool doubleCheckHover = false;
        for (int i = 0; i < MAX_MENU_ENTRIES; i++) {
            bool insideMenuBoundaries = true;
            CRect boundaries = GetMenuEntryRect();

            boundaries.top += (boundaries.bottom + GetMenuHorSpacing()) * MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].Entries[0].y;

            if (menuEntry.top < boundaries.top || menuEntry.top > boundaries.top + (boundaries.bottom + GetMenuHorSpacing()) * (VISIBLE_ENTRIES - 1)) {
                insideMenuBoundaries = false;
            }

            char* leftText = NULL;
            char* rightText = NULL;
            char leftTextTmp[64];
            char rightTextTmp[64];

            if (HasToContinueLoop(i))
                continue;

            const int entryType = MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].Entries[i].type;
            switch (entryType) {
            case MENUENTRY_LOADGAME:
                leftText = nSaveSlots[i];
                sprintf(leftTextTmp, "%02d - %s", i + 1, leftText ? leftText : TextNew.GetText("FE_UNK").text);
                leftText = leftTextTmp;
                rightText = nSaveSlotsDate[i];
                break;
            case MENUENTRY_REDEFINEKEY:
                sprintf(leftTextTmp, "CA_%02d", i);
                leftText = TextNew.GetText(leftTextTmp).text;

                if (nCurrentInputType == MENUINPUT_REDEFINEKEY && nCurrentEntryItem == i) {
                    sprintf(rightTextTmp, "?");

                    if (FLASH_ITEM(500, 200))
                        rightText = rightTextTmp;
                }
                else {
                    if (Controls[i].key != rsNULL)
                        rightText = (char*)CPadNew::KeyToString(Controls[i].key);
                    else {
                        sprintf(rightTextTmp, "???");
                        rightText = rightTextTmp;
                    }
                }
                break;
            default:
                leftText = TextNew.GetText(MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].Entries[i].entryName).text;
                break;
            }

            if (insideMenuBoundaries) {
                if (i != 0) {
                    menuEntry.top += (menuEntry.bottom + GetMenuHorSpacing()) * MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].Entries[i].y;
                    bb += (menuEntry.bottom + GetMenuHorSpacing()) * MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].Entries[i].y;
                }

                if (bDrawMouse && CheckHover(menuEntry.left, menuEntry.left + menuEntry.right, menuEntry.top, menuEntry.top + menuEntry.bottom) && !doubleCheckHover) {
                    nCurrentEntryItemHover = i;

                    if (pad->GetLeftMouseJustUp()) {
                        Audio.PlayChunk(CHUNK_MENU_SELECT, 1.0f);

                        if (nCurrentEntryItemHover == nCurrentEntryItem)
                            ProcessEntryStuff(1, 0);
                        else
                            SetInputTypeAndClear(MENUINPUT_ENTRY, nCurrentEntryItemHover);
                    }
                }
            }

            menuEntryColor = { 0, 0, 0, 0 };
            menuEntryTextColor = HudColourNew.GetRGB(HUD_COLOUR_WHITE, FadeIn(255));
            shiftText = 0.0f;

            if (i == nCurrentEntryItemHover) {
                menuEntryColor = { 100, 100, 100, FadeIn(180) };
                menuEntryTextColor = HudColourNew.GetRGB(HUD_COLOUR_WHITE, FadeIn(255));
            }

            if (i == nCurrentEntryItem) {
                if (nCurrentInputType == MENUINPUT_ENTRY) {
                    menuEntryColor = { 255, 255, 255, FadeIn(255) };
                    menuEntryTextColor = HudColourNew.GetRGB(HUD_COLOUR_BLACK, FadeIn(255));
                    shiftText = 16.0f;
                }

                if (!insideMenuBoundaries) {
                    if (nCurrentEntryItem > VISIBLE_ENTRIES - 1) {
                        fMenuEntryScrollOffset = (menuEntry.bottom + GetMenuHorSpacing()) * (nCurrentEntryItem - VISIBLE_ENTRIES + 1);
                    }
                    else {
                        fMenuEntryScrollOffset = (menuEntry.bottom + GetMenuHorSpacing()) * (nCurrentEntryItem);
                    }
                }
            }

            if (insideMenuBoundaries) {
                CSprite2d::DrawRect(CRect(menuEntry.left, menuEntry.top, menuEntry.left + menuEntry.right, menuEntry.top + menuEntry.bottom), menuEntryColor);

                CFontNew::SetBackground(false);
                CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
                CFontNew::SetAlignment(CFontNew::ALIGN_LEFT);
                CFontNew::SetWrapX(SCREEN_COORD(640.0f));
                CFontNew::SetFontStyle(CFontNew::FONT_1);
                CFontNew::SetDropShadow(0.0f);
                CFontNew::SetOutline(0.0f);
                CFontNew::SetDropColor(CRGBA(0, 0, 0, 0));
                CFontNew::SetColor(menuEntryTextColor);
                CFontNew::SetScale(SCREEN_MULTIPLIER(0.6f), SCREEN_MULTIPLIER(1.2f));
                CFontNew::PrintString(menuEntry.left + SCREEN_COORD(12.0f), menuEntry.top + SCREEN_COORD(5.0f), leftText);

                bool arrows = true;
                switch (MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].Entries[i].type) {
                case MENUENTRY_NONE:
                    break;
                case MENUENTRY_SCREENTYPE:
                    rightText = TextNew.GetText(TempSettings.screenType ? "FE_SCN1" : "FE_SCN0").text;
                    break;
                case MENUENTRY_CHANGERES:
                    if (char* mode = VideoModeList[TempSettings.videoMode]) {
                        strcpy(rightTextTmp, mode);
                        rightText = rightTextTmp;
                    }
                    break;
                case MENUENTRY_ASPECTRATIO:
                    if (char* ar[] = { "Auto", "4:3", "16:9" }) {
                        rightText = ar[TempSettings.aspectRatio];
                    }
                    break;
                case MENUENTRY_REFRESHRATE:
                    rightText = "60Hz";
                    break;
                case MENUENTRY_MSAA:
                    switch (TempSettings.antiAliasing) {
                    case 0:
                    case 1:
                        rightText = TextNew.GetText("FE_OFF").text;
                        break;
                    case 2:
                        rightText = "x2";
                        break;
                    case 3:
                        rightText = "x4";
                        break;
                    case 4:
                        rightText = "x8";
                        break;
                    };
                    break;
                case MENUENTRY_FRAMELIMITER:
                    rightText = TextNew.GetText(TempSettings.frameLimiter ? "FE_ON" : "FE_OFF").text;
                    break;
                case MENUENTRY_VISUALQUALITY:
                    switch (TempSettings.visualQuality) {
                    case 0:
                        rightText = TextNew.GetText("FE_LOW").text;
                        break;
                    case 1:
                        rightText = TextNew.GetText("FE_MED").text;
                        break;
                    case 2:
                        rightText = TextNew.GetText("FE_HIG").text;
                        break;
                    case 3:
                        rightText = TextNew.GetText("FE_VHIG").text;
                        break;
                    };
                    break;
                case MENUENTRY_SHOWHUD:
                    rightText = TextNew.GetText(TempSettings.showHUD ? "FE_ON" : "FE_OFF").text;
                    break;
                case MENUENTRY_SHOWRADAR:
                    rightText = TextNew.GetText(TempSettings.showRadar ? "FE_ON" : "FE_OFF").text;
                    break;
                case MENUENTRY_WEAPONTARGET:
                    rightText = TextNew.GetText(TempSettings.weaponTarget ? "FE_COMP" : "FE_SIM").text;
                    break;
                case MENUENTRY_GPSROUTE:
                    rightText = TextNew.GetText(TempSettings.gpsRoute ? "FE_ON" : "FE_OFF").text;
                    break;
                case MENUENTRY_MEASUREMENTSYS:
                    rightText = TextNew.GetText(TempSettings.measurementSys ? "FE_IMP" : "FE_MET").text;
                    break;
                case MENUENTRY_SUBTITLES:
                    rightText = TextNew.GetText(TempSettings.subtitles ? "FE_ON" : "FE_OFF").text;
                    break;
                case MENUENTRY_INVERTMOUSEY:
                    rightText = TextNew.GetText(TempSettings.invertMouseY ? "FE_ON" : "FE_OFF").text;
                    break;
                case MENUENTRY_MOUSESTEER:
                    rightText = TextNew.GetText(TempSettings.mouseSteering ? "FE_ON" : "FE_OFF").text;
                    break;
                case MENUENTRY_MOUSEFLYING:
                    rightText = TextNew.GetText(TempSettings.mouseFlying ? "FE_ON" : "FE_OFF").text;
                    break;
                case MENUENTRY_RADIOSTATION:
                    if (bool radioOff = (Settings.radioStation != RADIO_NONE) && CRadioHud::CanRetuneRadioStation()) {
                        sprintf(rightTextTmp, "RADIO%d", Settings.radioStation);
                        rightText = TextNew.GetText(rightTextTmp).text;
                    }
                    else {
                        rightText = TextNew.GetText("RADOFF").text;
                    }
                    break;
                case MENUENTRY_RADIOAUTOSELECT:
                    rightText = TextNew.GetText(TempSettings.radioAutoSelect ? "FE_ON" : "FE_OFF").text;
                    break;
                case MENUENTRY_RADIOEQ:
                    rightText = TextNew.GetText(TempSettings.radioEQ ? "FE_ON" : "FE_OFF").text;
                    break;
                case MENUENTRY_TRACKSAUTOSCAN:
                    rightText = TextNew.GetText(TempSettings.tracksAutoScan ? "FE_ON" : "FE_OFF").text;
                    break;
                case MENUENTRY_RADIOMODE:
                    switch (TempSettings.radioMode) {
                    default:
                        rightText = TextNew.GetText("FE_OFF").text;
                        break;
                    }
                    break;
                case MENUENTRY_SHOWCONTROLSFOR:
                    rightText = TextNew.GetText(TempSettings.showControlsFor ? "PAD_VH" : "PAD_FT").text;
                    break;
                case MENUENTRY_INVERTPADX1:
                    rightText = TextNew.GetText(TempSettings.invertPadX1 ? "FE_ON" : "FE_OFF").text;
                    break;
                case MENUENTRY_INVERTPADY1:
                    rightText = TextNew.GetText(TempSettings.invertPadY1 ? "FE_ON" : "FE_OFF").text;
                    break;
                case MENUENTRY_INVERTPADX2:
                    rightText = TextNew.GetText(TempSettings.invertPadX2 ? "FE_ON" : "FE_OFF").text;
                    break;
                case MENUENTRY_INVERTPADY2:
                    rightText = TextNew.GetText(TempSettings.invertPadY2 ? "FE_ON" : "FE_OFF").text;
                    break;
                case MENUENTRY_SWAPPADAXIS1:
                    rightText = TextNew.GetText(TempSettings.swapPadAxis1 ? "FE_ON" : "FE_OFF").text;
                    break;
                case MENUENTRY_SWAPPADAXIS2:
                    rightText = TextNew.GetText(TempSettings.swapPadAxis2 ? "FE_ON" : "FE_OFF").text;
                    break;
                case MENUENTRY_LANDINGPAGE:
                    rightText = TextNew.GetText(TempSettings.landingPage ? "FE_ON" : "FE_OFF").text;
                    break;
                case MENUENTRY_LANGUAGE:
                    sprintf(rightTextTmp, "LANG_%d", TempSettings.language);
                    rightText = TextNew.GetText(rightTextTmp).text;
                    break;

                    // Sliders
                case MENUENTRY_BRIGHTNESS:
                    DrawSliderRightAlign(menuEntry.left + menuEntry.right + SCREEN_COORD(-12.0f), menuEntry.top + SCREEN_COORD(14.0f), TempSettings.brightness / 512.0f);
                    break;
                case MENUENTRY_GAMMA:
                    DrawSliderRightAlign(menuEntry.left + menuEntry.right + SCREEN_COORD(-12.0f), menuEntry.top + SCREEN_COORD(14.0f), TempSettings.gamma / 1.0f);
                    break;
                case MENUENTRY_SAFEZONESIZE:
                    DrawSliderRightAlign(menuEntry.left + menuEntry.right + SCREEN_COORD(-12.0f), menuEntry.top + SCREEN_COORD(14.0f), TempSettings.safeZoneSize / 96.0f);
                    break;
                case MENUENTRY_RADIOVOLUME:
                    DrawSliderRightAlign(menuEntry.left + menuEntry.right + SCREEN_COORD(-12.0f), menuEntry.top + SCREEN_COORD(14.0f), TempSettings.radioVolume / 64.0f);
                    break;
                case MENUENTRY_SFXVOLUME:
                    DrawSliderRightAlign(menuEntry.left + menuEntry.right + SCREEN_COORD(-12.0f), menuEntry.top + SCREEN_COORD(14.0f), TempSettings.sfxVolume / 64.0f);
                    break;
                case MENUENTRY_DRAWDISTANCE:
                    DrawSliderRightAlign(menuEntry.left + menuEntry.right + SCREEN_COORD(-12.0f), menuEntry.top + SCREEN_COORD(14.0f), (TempSettings.drawDist - 0.8f) * 1.0f);
                    break;
                case MENUENTRY_MOUSESENSITIVITY:
                    DrawSliderRightAlign(menuEntry.left + menuEntry.right + SCREEN_COORD(-12.0f), menuEntry.top + SCREEN_COORD(14.0f), TempSettings.mouseSensitivity * 200.0f);
                    break;
                default:
                    arrows = false;
                    break;
                }

                if (!arrows)
                    shiftText = 0.0f;

                if (rightText) {
                    CFontNew::SetAlignment(CFontNew::ALIGN_RIGHT);
                    CFontNew::PrintString(menuEntry.left + menuEntry.right + SCREEN_COORD(-12.0f - shiftText), menuEntry.top + SCREEN_COORD(5.0f), rightText);

                    if (shiftText && arrows) {
                        float arrowX = menuEntry.left + menuEntry.right + SCREEN_COORD(-52.0f) - CFontNew::GetStringWidth(rightText, true);
                        float arrowY = menuEntry.top + SCREEN_COORD(6.0f);
                        float arrowScale = SCREEN_COORD(24.0f);
                        MenuSprites[MENU_ARROW_LEFT]->Draw(arrowX, arrowY, arrowScale, arrowScale, CRGBA(0, 0, 0, 255));

                        doubleCheckHover = false;
                        if (CheckHover(arrowX - arrowScale, arrowX + arrowScale, arrowY, arrowY + arrowScale)) {
                            if (pad->GetLeftMouseJustUp()) {
                                Audio.PlayChunk(CHUNK_MENU_SELECT, 1.0f);

                                ProcessEntryStuff(0, -1);
                            }

                            doubleCheckHover = true;
                        }

                        arrowX = menuEntry.left + menuEntry.right + SCREEN_COORD(-28.0f);
                        MenuSprites[MENU_ARROW_RIGHT]->Draw(arrowX, arrowY, arrowScale, arrowScale, CRGBA(0, 0, 0, 255));

                        if (CheckHover(arrowX - arrowScale, arrowX + arrowScale, arrowY, arrowY + arrowScale)) {
                            if (pad->GetLeftMouseJustUp()) {
                                Audio.PlayChunk(CHUNK_MENU_SELECT, 1.0f);

                                ProcessEntryStuff(0, 1);
                            }
                            doubleCheckHover = true;
                        }
                    }
                }
            }
            menuEntry.top += menuEntry.bottom + GetMenuHorSpacing();
        }

        switch (MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].type) {
        case MENUENTRY_PAD:
            DrawTabGamePad();
            break;
        case MENUENTRY_GFX:
            DrawTabMemoryAvailable();
            break;
        case MENUENTRY_POPULATESAVESLOT:
            DrawTabNumSaveGames();
            break;
        case MENUENTRY_KEYBINDING:
            DrawTabKeyBindings();
            break;
        }
    }
}

void CMenuNew::DrawTabMemoryAvailable() {
    CRect menuEntry = GetMenuEntryRect();

    float bb = max(menuEntry.bottom + GetMenuHorSpacing(), 0);
    CSprite2d::DrawRect(CRect(menuEntry.left, menuEntry.top, menuEntry.left + menuEntry.right, menuEntry.top + menuEntry.bottom + bb), CRGBA(0, 0, 0, FadeIn(180)));

    CFontNew::SetBackground(false);
    CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
    CFontNew::SetAlignment(CFontNew::ALIGN_LEFT);
    CFontNew::SetWrapX(SCREEN_COORD(640.0f));
    CFontNew::SetFontStyle(CFontNew::FONT_1);
    CFontNew::SetDropShadow(0.0f);
    CFontNew::SetOutline(0.0f);
    CFontNew::SetDropColor(CRGBA(0, 0, 0, 0));
    CFontNew::SetColor(HudColourNew.GetRGB(HUD_COLOUR_WHITE, FadeIn(255)));
    CFontNew::SetScale(SCREEN_MULTIPLIER(0.6f), SCREEN_MULTIPLIER(1.2f));

    char* str = TextNew.GetText("FE_MEM").text;
    sprintf(gString, "%s: %lu MB / %lu MB", str, nUsedVidMemory, nFreeVidMemory);
    CFontNew::PrintString(menuEntry.left + SCREEN_COORD(12.0f), menuEntry.top + SCREEN_COORD(24.0f), gString);

    float progress = nUsedVidMemory / (nFreeVidMemory / 1.0f);
    DrawSliderRightAlign(menuEntry.left + menuEntry.right + SCREEN_COORD(-12.0f), menuEntry.top + SCREEN_COORD(32.0f), progress);
}

void CMenuNew::DrawPadLine(float x, float y, int w, int h) {
    CRect r;
    CRGBA c = { 255, 255, 255, FadeIn(100) };

    float s = SCREEN_MULTIPLIER(1.0f);

    r = { x, y - s, x + (s * w), y + s };
    CSprite2d::DrawRect(r, c);

    r = { x - s + (s * w), y, x + s + (s * w), y + (s * h) };
    CSprite2d::DrawRect(r, c);
}

void CMenuNew::DrawTabGamePad() {
    CRect rect = GetMenuEntryRect();

    float bb = max(rect.bottom + GetMenuHorSpacing(), 0) * 6;
    CSprite2d::DrawRect(CRect(rect.left, rect.top, rect.left + rect.right, rect.top + rect.bottom + bb), CRGBA(0, 0, 0, FadeIn(180)));

    if (nCurrentInputType == MENUINPUT_BAR) {
        if (CheckHover(rect.left, rect.left + rect.right, rect.top, rect.top + rect.bottom + bb)) {
            CPadNew* pad = CPadNew::GetPad(0);

            if (pad->GetLeftMouseJustUp()) {
                Audio.PlayChunk(CHUNK_MENU_SELECT, 1.0f);

                SetInputTypeAndClear(MENUINPUT_TAB);
            }
        }
    }

    if (!GINPUT) {
        CFontNew::SetBackground(false);
        CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
        CFontNew::SetAlignment(CFontNew::ALIGN_CENTER);
        CFontNew::SetWrapX(SCREEN_COORD(640.0f));
        CFontNew::SetFontStyle(CFontNew::FONT_1);
        CFontNew::SetDropShadow(0.0f);
        CFontNew::SetOutline(0.0f);
        CFontNew::SetDropColor(CRGBA(0, 0, 0, 0));
        CFontNew::SetColor(HudColourNew.GetRGB(HUD_COLOUR_WHITE, FadeIn(255)));
        CFontNew::SetScale(SCREEN_MULTIPLIER(0.6f), SCREEN_MULTIPLIER(1.2f));

        char* str = TextNew.GetText("FE_GINPUT").text;
        CFontNew::PrintString(rect.left + rect.right / 2, rect.top + bb / 2, str);
        return;
    }

    CRect ctrlRect = rect;
    ctrlRect.left = MENU_X(990.0f);
    ctrlRect.top = MENU_Y(212.0f);;
    ctrlRect.right = SCREEN_COORD(383.0f);;
    ctrlRect.bottom = SCREEN_COORD(383.0f);;

    MenuSprites[MENU_XBOX_GAMEPAD]->Draw(CRect(ctrlRect.left, ctrlRect.top, ctrlRect.left + ctrlRect.right, ctrlRect.top + ctrlRect.bottom), CRGBA(255, 255, 255, FadeIn(255)));

    int count = 0;
    char buff[256];
    char* str = NULL;
    CFontNew::SetBackground(false);
    CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
    CFontNew::SetWrapX(SCREEN_COORD(640.0f));
    CFontNew::SetFontStyle(CFontNew::FONT_1);
    CFontNew::SetDropShadow(0.0f);
    CFontNew::SetOutline(0.0f);
    CFontNew::SetDropColor(CRGBA(0, 0, 0, 0));
    CFontNew::SetColor(HudColourNew.GetRGB(HUD_COLOUR_PURE_WHITE, FadeIn(255)));
    CFontNew::SetScale(SCREEN_MULTIPLIER(0.6f), SCREEN_MULTIPLIER(1.2f));

    CFontNew::SetAlignment(CFontNew::ALIGN_CENTER);

    str = TextNew.GetText(TempSettings.showControlsFor ? "PAD_VH" : "PAD_FT").text;
    TextNew.UpperCase(str);
    CFontNew::PrintString(rect.left + rect.right / 2, rect.top, str);

    CFontNew::SetScale(SCREEN_MULTIPLIER(0.5f), SCREEN_MULTIPLIER(1.0f));

    { // Left
        float x = rect.left + SCREEN_COORD(266.0f);
        float y = rect.top + SCREEN_COORD(56.0f);
        float next = 0.0f;

        int w[] = {
            84,
            67,
            137,
            76,
            76,
            124,
            104,
            126,
            142
        };

        int h[] = {
            4,
            4,
            34,
            6,
            0,
             0,
            -10,
            -18,
            -56
        };

        CFontNew::SetAlignment(CFontNew::ALIGN_RIGHT);

        for (int i = 0; i < 9; i++) {
            if (GInputPadSettings[0].ControlsSet == 1)
                sprintf(buff, "PAD_%02d", TempSettings.showControlsFor ? count + 18 : count);
            else if (GInputPadSettings[0].ControlsSet == 2) {
                switch (i) {
                case 0:
                    strcpy(buff, TempSettings.showControlsFor ? "PAD_33" : "PAD_10");
                    break;
                case 1:
                    strcpy(buff, TempSettings.showControlsFor ? "PAD_18" : "PAD_36");
                    break;
                case 2:
                    strcpy(buff, TempSettings.showControlsFor ? "PAD_02" : "PAD_02");
                    break;
                case 3:
                    strcpy(buff, TempSettings.showControlsFor ? "PAD_21" : "PAD_03");
                    break;
                case 4:
                    strcpy(buff, TempSettings.showControlsFor ? "PAD_22" : "PAD_04");
                    break;
                case 5:
                    strcpy(buff, TempSettings.showControlsFor ? "PAD_37" : "PAD_37");
                    break;
                case 6:
                    strcpy(buff, TempSettings.showControlsFor ? (FLASH_ITEM(4000, 2000) ? "PAD_25" : "PAD_38") : "PAD_38");
                    break;
                case 7:
                    strcpy(buff, TempSettings.showControlsFor ? (FLASH_ITEM(4000, 2000) ? "PAD_41" : "PAD_39") : (FLASH_ITEM(4000, 2000) ? "PAD_41" : "PAD_39"));
                    break;
                case 8:
                    strcpy(buff, TempSettings.showControlsFor ? "PAD_40" : "PAD_40");
                    break;
                }               
            }
            str = TextNew.GetText(buff).text;
            TextNew.UpperCase(str);
            DrawPadLine(x, y + next, w[i], h[i]);
            CFontNew::PrintString(x - SCREEN_COORD(8.0f), y + next - SCREEN_COORD(12.0f), str);

            next += SCREEN_COORD(30.0f - i);
            count++;
        }
    }
    { // Right
        float x = rect.left + SCREEN_COORD(610.0f);
        float y = rect.top + SCREEN_COORD(56.0f);
        float next = 0.0f;

        int w[] = {
            -84,
            -67,
            -137,
            -76,
            -50,
            -76,
            -100,
            -126,
            -146
        };

        int h[] = {
            4,
            4,
            34,
             0,
            -6,
            -10,
            -52,
            -24,
            -50
        };

        CFontNew::SetAlignment(CFontNew::ALIGN_LEFT);

        for (int i = 0; i < 9; i++) {
            if (GInputPadSettings[0].ControlsSet == 1)
                sprintf(buff, "PAD_%02d", TempSettings.showControlsFor ? count + 18 : count);
            else if (GInputPadSettings[0].ControlsSet == 2) {
                switch (i) {
                case 0:
                    strcpy(buff, TempSettings.showControlsFor ? "PAD_32" : "PAD_31");
                    break;
                case 1:
                    strcpy(buff, TempSettings.showControlsFor ? "PAD_27" : "PAD_43");
                    break;
                case 2:
                    strcpy(buff, TempSettings.showControlsFor ? "PAD_11" : "PAD_11");
                    break;
                case 3:
                    strcpy(buff, TempSettings.showControlsFor ? "PAD_12" : "PAD_12");
                    break;
                case 4:
                    strcpy(buff, TempSettings.showControlsFor ? "PAD_24" : "PAD_13");
                    break;
                case 5:
                    strcpy(buff, TempSettings.showControlsFor ? "PAD_28" : "PAD_14");
                    break;
                case 6:
                    strcpy(buff, TempSettings.showControlsFor ? "PAD_24" : "PAD_15");
                    break;
                case 7:
                    strcpy(buff, TempSettings.showControlsFor ? "PAD_34" : "PAD_16");
                    break;
                case 8:
                    strcpy(buff, TempSettings.showControlsFor ? "PAD_17" : "PAD_17");
                    break;
                }            
            }            
            str = TextNew.GetText(buff).text;
            TextNew.UpperCase(str);
            DrawPadLine(x, y + next, w[i], h[i]);
            CFontNew::PrintString(x + SCREEN_COORD(8.0f), y + next - SCREEN_COORD(12.0f), str);

            next += SCREEN_COORD(30.0f - i);
            count++;
        }
    }
}

void CMenuNew::DrawTabNumSaveGames() {
    CRect menuEntry = GetMenuEntryRect();

    CSprite2d::DrawRect(CRect(menuEntry.left, menuEntry.top, menuEntry.left + menuEntry.right, menuEntry.top + menuEntry.bottom), CRGBA(0, 0, 0, FadeIn(180)));

    CFontNew::SetBackground(false);
    CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
    CFontNew::SetAlignment(CFontNew::ALIGN_LEFT);
    CFontNew::SetWrapX(SCREEN_COORD(640.0f));
    CFontNew::SetFontStyle(CFontNew::FONT_1);
    CFontNew::SetDropShadow(0.0f);
    CFontNew::SetOutline(0.0f);
    CFontNew::SetDropColor(CRGBA(0, 0, 0, 0));
    CFontNew::SetColor(HudColourNew.GetRGB(HUD_COLOUR_WHITE, FadeIn(105)));
    CFontNew::SetScale(SCREEN_MULTIPLIER(0.6f), SCREEN_MULTIPLIER(1.2f));

    char* str = NULL;
    if (nNumOfSaveGames > 0) {
        sprintf(gString, TextNew.GetText("FE_SAVNUM").text, nNumOfSaveGames);
        str = gString;
    }
    else {
        str = TextNew.GetText("FE_NOSAV").text;
    }
    CFontNew::PrintString(menuEntry.left + SCREEN_COORD(12.0f), menuEntry.top + SCREEN_COORD(5.0f), str);
}

void CMenuNew::DrawTabKeyBindings() {
    CRect menuEntry = GetMenuEntryRect();

    CSprite2d::DrawRect(CRect(menuEntry.left, menuEntry.top, menuEntry.left + menuEntry.right, menuEntry.top + menuEntry.bottom), CRGBA(0, 0, 0, FadeIn(180)));

    CFontNew::SetBackground(false);
    CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
    CFontNew::SetAlignment(CFontNew::ALIGN_LEFT);
    CFontNew::SetWrapX(SCREEN_COORD(640.0f));
    CFontNew::SetFontStyle(CFontNew::FONT_1);
    CFontNew::SetDropShadow(0.0f);
    CFontNew::SetOutline(0.0f);
    CFontNew::SetDropColor(CRGBA(0, 0, 0, 0));
    CFontNew::SetColor(HudColourNew.GetRGB(HUD_COLOUR_WHITE, FadeIn(105)));
    CFontNew::SetScale(SCREEN_MULTIPLIER(0.6f), SCREEN_MULTIPLIER(1.2f));

    char* str = NULL;

    str = TextNew.GetText("FE_ACT").text;
    TextNew.UpperCase(str);
    CFontNew::PrintString(menuEntry.left + SCREEN_COORD(12.0f), menuEntry.top + SCREEN_COORD(5.0f), str);

    CFontNew::SetAlignment(CFontNew::ALIGN_RIGHT);
    str = TextNew.GetText("FE_PRI").text;
    TextNew.UpperCase(str);
    CFontNew::PrintString(menuEntry.left + menuEntry.right - SCREEN_COORD(12.0f), menuEntry.top + SCREEN_COORD(5.0f), str);
}

void CMenuNew::DrawLandingPage() {
    const float previousBaseRes = plugin::screen::GetBaseResolution();
    if ((SCREEN_WIDTH / SCREEN_HEIGHT) < (DEFAULT_WIDTH / DEFAULT_HEIGHT)) {
        plugin::screen::SetBaseResolution(DEFAULT_HEIGHT * (2.0f * SAFE_ZONE_HEIGHT_MULT));
    }

    DrawInfoBox();

    CRect menuEntry;
    CRGBA menuEntryTextColor;
    CPadNew* pad = CPadNew::GetPad(0);

    menuEntry = { HUD_X(532.0f), HUD_BOTTOM(128.0f), HUD_RIGHT(96.0f), SCREEN_COORD(74.0f) };

    CHudNew::DrawSimpleRectGradInverted(CRect(menuEntry.left, menuEntry.top, menuEntry.right, menuEntry.top + menuEntry.bottom), CRGBA(0, 0, 0, FadeIn(150)));

    float shift = SCREEN_COORD(24.0f);
    float spacing = SCREEN_COORD(64.0f);
    float textoffset = SCREEN_COORD(16.5f);

    menuEntry.right -= shift;

    nCurrentTabItemHover = -1;
    for (int i = 0; i < MAX_MENU_ENTRIES; i++) {
        if (MenuScreen[nCurrentScreen].Tab[i].tabName[0] == '\0')
            continue;

        char* leftText = TextNew.GetText(MenuScreen[nCurrentScreen].Tab[i].tabName).text;

        if (i != 0) {
            char* prevText = TextNew.GetText(MenuScreen[nCurrentScreen].Tab[i - 1].tabName).text;
            menuEntry.right -= (CFontNew::GetStringWidth(prevText, true) + spacing);
        }

        float nx = (CFontNew::GetStringWidth(leftText, true) + spacing);
        if (bDrawMouse && CheckHover(menuEntry.right - nx, menuEntry.right, menuEntry.top, menuEntry.top + menuEntry.bottom)) {
            nCurrentTabItemHover = i;
            SetInputTypeAndClear(MENUINPUT_TAB, nCurrentTabItemHover);

            if (pad->GetLeftMouseJustUp()) {
                Audio.PlayChunk(CHUNK_MENU_SELECT, 1.0f);

                ProcessTabStuff();
            }
        }

        menuEntryTextColor = HudColourNew.GetRGB(HUD_COLOUR_GREY, FadeIn(255));

        if (i == nCurrentTabItem) {
            menuEntryTextColor = HudColourNew.GetRGB(HUD_COLOUR_PURE_WHITE, FadeIn(255));
        }

        if (i == nCurrentTabItemHover) {
            menuEntryTextColor = HudColourNew.GetRGB(HUD_COLOUR_PURE_WHITE, FadeIn(255));
        }
        
        CFontNew::SetBackground(false);
        CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
        CFontNew::SetAlignment(CFontNew::ALIGN_RIGHT);
        CFontNew::SetWrapX(SCREEN_COORD(640.0f));
        CFontNew::SetFontStyle(CFontNew::FONT_1);
        CFontNew::SetDropShadow(0.0f);
        CFontNew::SetOutline(0.0f);
        CFontNew::SetDropColor(CRGBA(0, 0, 0, FadeIn(0)));
        CFontNew::SetColor(menuEntryTextColor);
        CFontNew::SetScale(SCREEN_MULTIPLIER(0.92f), SCREEN_MULTIPLIER(1.8f));

        TextNew.UpperCase(leftText);
        CFontNew::PrintString(menuEntry.right, menuEntry.top + textoffset, leftText);
    }

    plugin::screen::SetBaseResolution(previousBaseRes);
}

void CMenuNew::DrawInfoBox() {
    const float previousBaseRes = plugin::screen::GetBaseResolution();
    if ((SCREEN_WIDTH / SCREEN_HEIGHT) < (DEFAULT_WIDTH / DEFAULT_HEIGHT)) {
        plugin::screen::SetBaseResolution(DEFAULT_HEIGHT * (2.0f * SAFE_ZONE_HEIGHT_MULT));
    }

    CRect rect;
    rect = { HUD_RIGHT(96.0f + 1302.0f), HUD_Y(218.0f), SCREEN_COORD(1302.0f), SCREEN_COORD(644.0f) };
    DrawPatternBackground(CRect(rect.left, rect.top, (rect.left + rect.right), (rect.top + rect.bottom)), CRGBA(HudColourNew.GetRGB(HUD_COLOUR_BLACK, FadeIn(150))));
    FrontendSprites[FRONTEND_INFOBOX]->Draw(CRect(rect.left + SCREEN_COORD(870.0f), rect.top, rect.left + rect.right, rect.top + rect.bottom), CRGBA(255, 255, 255, FadeIn(255)));

    CFontNew::SetBackground(false);
    CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
    CFontNew::SetAlignment(CFontNew::ALIGN_LEFT);
    CFontNew::SetWrapX(SCREEN_COORD(800.0f));
    CFontNew::SetFontStyle(CFontNew::FONT_1);
    CFontNew::SetDropShadow(0.0f);
    CFontNew::SetOutline(0.0f);
    CFontNew::SetDropColor(CRGBA(0, 0, 0, FadeIn(0)));
    CFontNew::SetColor(HudColourNew.GetRGB(HUD_COLOUR_WHITE, FadeIn(255)));
    CFontNew::SetScale(SCREEN_MULTIPLIER(2.6f), SCREEN_MULTIPLIER(4.8f));

    char* str = TextNew.GetText("FE_GTACUT").text;
    CFontNew::PrintString(rect.left + SCREEN_COORD(32.0f), rect.top + SCREEN_COORD(19.0f), str);

    char* str1 = TextNew.GetText("FE_GTAINFO").text;
    CFontNew::SetScale(SCREEN_MULTIPLIER(0.6f), SCREEN_MULTIPLIER(1.2f));
    CFontNew::PrintString(rect.left + SCREEN_COORD(32.0f), rect.top + SCREEN_COORD(148.0f), str1);
    plugin::screen::SetBaseResolution(previousBaseRes);
}

void CMenuNew::DrawLoadingPage() {
    DrawInfoBox();

    if (fLoadingPercentage < 100.0f) {
        if (nLoadingTimeInMs < CTimer::m_snTimeInMillisecondsPauseMode) {
            fLoadingPercentage += 1.0f;
            nLoadingTimeInMs = CTimer::m_snTimeInMillisecondsPauseMode + FAKE_LOADING_WAIT_TIME;
        }

        DrawLoadingBar("FE_STORYL");
    }
}

void CMenuNew::DrawLoadingBar(char* str) {
    CRect r;
    r.left = HUD_RIGHT(96.0f);
    r.right = SCREEN_COORD(32.0f);
    r.top = HUD_BOTTOM(90.0f);
    r.bottom = SCREEN_COORD(36.0f);

    static float x = 0.0f;
    static float y = 0.0f;
    static float s = SCREEN_COORD(20.0f);
    CSprite2d::DrawRect(CRect(x, r.top, r.left, r.top + r.bottom), CRGBA(0, 0, 0, 150));
    x = r.left;
    y = r.top + SCREEN_COORD(17.0f);

    CFontNew::SetBackground(false);
    CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
    CFontNew::SetAlignment(CFontNew::ALIGN_RIGHT);
    CFontNew::SetWrapX(SCREEN_COORD(980.0f));
    CFontNew::SetFontStyle(CFontNew::FONT_1);
    CFontNew::SetDropShadow(0.0f);
    CFontNew::SetOutline(0.0f);
    CFontNew::SetDropColor(CRGBA(0, 0, 0, 0));
    CFontNew::SetColor(HudColourNew.GetRGB(HUD_COLOUR_WHITE, 255));
    CFontNew::SetScale(SCREEN_MULTIPLIER(0.6f), SCREEN_MULTIPLIER(1.2f));

    x -= s;
    DrawSpinningWheel(x, y, s, s);
    x -= s + SCREEN_COORD(8.0f);

    if (str) {
        str = TextNew.GetText(str).text;
        CFontNew::PrintString(x, r.top + SCREEN_COORD(4.0f), str);
        x -= CFontNew::GetStringWidth(str, true) + s;
    }
}

void CMenuNew::DrawSliderRightAlign(float x, float y, float progress) {
    x = x - SCREEN_COORD(187.0f);
    float w = SCREEN_COORD(187.0f);
    float h = SCREEN_COORD(9.0f);
    DrawProgressBar(x, y, w, h, progress, HudColourNew.GetRGB(Settings.uiMainColor, FadeIn(255)));

    bool point = false;
    int value = 0;
    CPadNew* pad = CPadNew::GetPad(0);
    if (CheckHover(x, x + (w / 2), y, y + h)) {
        value = -1;

        if (pad->GetLeftMouseJustDown())
            point = true;
    }
    else if (CheckHover(x + (w / 2), x + w, y, y + h)) {
        value = 1;

        if (pad->GetLeftMouseJustDown())
            point = true;
    }

    if (point) {
        CheckSliderMovement(value);
        ApplyChanges();
    }
}

void CMenuNew::DrawTabRadioIcons(float x, float y) {

}

void CMenuNew::DrawSpinningWheel(float x, float y, float w, float h) {
    CVector posn[4];
    static float angle = 0.0f;
    angle -= 0.02f * (M_PI * 1.5f);
    angle = CGeneral::LimitRadianAngle(angle);

    for (int i = 0; i < 4; i++) {
        const float a = i * M_PI_2 + (angle - M_PI / 4.0f);
        posn[i].x = x + (0.0f * cosf(a) + 1.0f * sinf(a)) * w;
        posn[i].y = y - (0.0f * sinf(a) - 1.0f * cosf(a)) * h;
    }

    MiscSprites[MISC_SPINNER]->Draw(posn[2].x, posn[2].y, posn[3].x, posn[3].y,
                                    posn[1].x, posn[1].y, posn[0].x, posn[0].y, CRGBA(255, 255, 255, 255));
}

void CMenuNew::ResetMap() {
    fMapZoom = MENU_MAP_ZOOM_MIN;
    vMapBase = GetMapBaseDefault();
    vTempMapBase = vMapBase;
    bShowMenu = true;
    bDrawMenuMap = false;
    bCleanMapScreenNextFrame = false;
    bShowLegend = true;
    CRadarNew::m_bRemoveBlipsLimit = false;
}

void CMenuNew::SetWaypoint(float x, float y) {
    if (FrontEndMenuManager.m_nTargetBlipIndex) {
        CRadar::ClearBlip(FrontEndMenuManager.m_nTargetBlipIndex);
        FrontEndMenuManager.m_nTargetBlipIndex = 0;
    }
    else {
        CVector2D in;
        CVector2D out;

        in.x = (x + GetMenuMapWholeSize() / 2) - MenuNew.vMapBase.x;
        in.y = MenuNew.vMapBase.y - (y + GetMenuMapWholeSize() / 2);

        in.x /= GetMenuMapWholeSize();
        in.y /= GetMenuMapWholeSize();

        CRadar::TransformRadarPointToRealWorldSpace(out, in);

        CVector pos = { out.x, out.y, 0.0f };

        if (pos.x < -3000.0f)
            pos.x = -3000.0f;

        if (pos.x > 3000.0f)
            pos.x = 3000.0f;

        if (pos.y < -3000.0f)
            pos.y = -3000.0f;

        if (pos.y > 3000.0f)
            pos.y = 3000.0f;

        int i = CRadar::SetCoordBlip(BLIP_COORD, pos, 0, BLIP_DISPLAY_BOTH, 0);
        CRadar::SetBlipSprite(i, RADAR_SPRITE_WAYPOINT);
        FrontEndMenuManager.m_nTargetBlipIndex = i;
    }
}

float CMenuNew::GetMenuMapTileSize() {
    const float tileSize = SCREEN_COORD(48.0f);
    return tileSize;
}

int CMenuNew::GetMenuMapTiles() {
    const int tiles = 12;
    return tiles;
}

float CMenuNew::GetMenuMapWholeSize() {
    const float mapWholeSize = GetMenuMapTileSize() * GetMenuMapTiles();
    return mapWholeSize * fMapZoom;
}

void CMenuNew::PrintBrief() {
    CRect mask = GetMenuScreenRect();
    DrawPatternBackground(CRect(mask.left, mask.top, mask.left + mask.right, mask.top + mask.bottom), HudColourNew.GetRGB(HUD_COLOUR_BLACK, FadeIn(150)));

    if (nCurrentInputType == MENUINPUT_BAR) {
        if (CheckHover(mask.left, mask.left + mask.right, mask.top, mask.top + mask.bottom)) {
            CPadNew* pad = CPadNew::GetPad(0);

            if (pad->GetLeftMouseJustUp()) {
                Audio.PlayChunk(CHUNK_MENU_SELECT, 1.0f);

                SetInputTypeAndClear(MENUINPUT_TAB);
            }
        }
    }

    CFontNew::SetBackground(false);
    CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
    CFontNew::SetAlignment(CFontNew::ALIGN_LEFT);
    CFontNew::SetWrapX(SCREEN_COORD(1920.0f));
    CFontNew::SetFontStyle(CFontNew::FONT_1);
    CFontNew::SetDropShadow(0.0f);
    CFontNew::SetOutline(0.0f);
    CFontNew::SetDropColor(CRGBA(0, 0, 0, 0));
    CFontNew::SetColor(HudColourNew.GetRGB(HUD_COLOUR_WHITE, FadeIn(255)));
    CFontNew::SetScale(SCREEN_MULTIPLIER(2.6f), SCREEN_MULTIPLIER(4.8f));

    bool noBrief = true;
    for (int i = 4; i >= 0; i--) {
        tPreviousBrief& brief = CMessages::PreviousBriefs[i];
        if (brief.m_pText) {
            CMessages::InsertNumberInString(brief.m_pText,
                brief.m_nNumber[0], brief.m_nNumber[1],
                brief.m_nNumber[2], brief.m_nNumber[3],
                brief.m_nNumber[4], brief.m_nNumber[5], gString);
            CMessages::InsertStringInString(gString, brief.m_pString);

            CFontNew::SetTokenToIgnore('N', 'n');
            CFontNew::SetScale(SCREEN_MULTIPLIER(0.6f), SCREEN_MULTIPLIER(1.2f));
            CFontNew::PrintString(mask.left + SCREEN_COORD(8.0f), mask.top + SCREEN_COORD(4.0f), gString);
            mask.top += SCREEN_COORD(24.0f);

            noBrief = false;
        }
    }
    CFontNew::SetTokenToIgnore(NULL, NULL);

    if (noBrief) {
        char* str = TextNew.GetText("FE_BRF").text;
        CFontNew::PrintString(mask.left + SCREEN_COORD(32.0f), mask.top + SCREEN_COORD(19.0f), str);

        char* str1 = TextNew.GetText("FE_BRF1").text;
        CFontNew::SetScale(SCREEN_MULTIPLIER(0.6f), SCREEN_MULTIPLIER(1.2f));
        CFontNew::PrintString(mask.left + SCREEN_COORD(32.0f), mask.top + SCREEN_COORD(148.0f), str1);
    }
}

void CMenuNew::PrintStats() {
    if (SAMP) {
        DrawScreenUnavailableOnline();
        return;
    }

    CRect mask = GetMenuScreenRect();

    if (nCurrentInputType == MENUINPUT_BAR) {
        if (CheckHover(mask.left, mask.left + mask.right, mask.top, mask.top + mask.bottom)) {
            CPadNew* pad = CPadNew::GetPad(0);

            if (pad->GetLeftMouseJustUp()) {
                Audio.PlayChunk(CHUNK_MENU_SELECT, 1.0f);

                SetInputTypeAndClear(MENUINPUT_TAB);
            }
        }
    }

    int lines = CStats::ConstructStatLine(99999, 0);
    CRect menuTab  = GetMenuEntryRect();
    CRGBA menuEntryColor;
    bool switchCol = false;

    CFontNew::SetBackground(false);
    CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
    CFontNew::SetWrapX(SCREEN_COORD(640.0f));
    CFontNew::SetFontStyle(CFontNew::FONT_1);
    CFontNew::SetDropShadow(0.0f);
    CFontNew::SetOutline(0.0f);
    CFontNew::SetDropColor(CRGBA(0, 0, 0, 0));
    CFontNew::SetColor(HudColourNew.GetRGB(HUD_COLOUR_WHITE, FadeIn(255)));
    CFontNew::SetScale(SCREEN_MULTIPLIER(0.6f), SCREEN_MULTIPLIER(1.2f));

    for (int i = 0; i < lines; i++) {
        CStats::ConstructStatLine(i, nCurrentTabItem);

        if (gGxtString[0] != '\0' && (gGxtString2[0] != '\0' || CStats::m_ThisStatIsABarChart)) {
            if (switchCol = switchCol == false)
                menuEntryColor = { 20, 20, 20, FadeIn(180) };
            else
                menuEntryColor = { 0, 0, 0, FadeIn(180) };

            CSprite2d::DrawRect(CRect(menuTab.left, menuTab.top, menuTab.left + menuTab.right, menuTab.top + menuTab.bottom), menuEntryColor);

            CFontNew::SetAlignment(CFontNew::ALIGN_LEFT);
            CFontNew::PrintString(menuTab.left + SCREEN_COORD(12.0f), menuTab.top + SCREEN_COORD(6.0f), gGxtString);
            
            if (CStats::m_ThisStatIsABarChart) {
                float x = (menuTab.left + menuTab.right + SCREEN_COORD(-12.0f)) - SCREEN_COORD(237.0f);
                float y = menuTab.top + SCREEN_COORD(14.0f);
                float w = SCREEN_COORD(237.0f);
                float h = SCREEN_COORD(9.0f);
                float p = CStats::GetStatValue(CStats::m_ThisStatIsABarChart) * 0.001f;

                RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)(rwFILTERNEAREST));
                DrawProgressBarWithSprite(CHudNew::StatsSprites[PLRSTAT_PROGRESS_BAR], x, y, w, h, p, HudColourNew.GetRGB(Settings.uiMainColor, FadeIn(255)));
                RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)(rwFILTERLINEARMIPLINEAR));
            }
            else {
                CFontNew::SetAlignment(CFontNew::ALIGN_RIGHT);
                CFontNew::PrintString(menuTab.left + menuTab.right + SCREEN_COORD(-12.0f), menuTab.top + SCREEN_COORD(6.0f), gGxtString2);
            }
            menuTab.top += menuTab.bottom + GetMenuHorSpacing();
            gGxtString[0] = NULL;
            gGxtString2[0] = NULL;
        }
    }
}

void CMenuNew::DrawScreenUnavailableOnline() {
    CRect mask = GetMenuScreenRect();
    DrawPatternBackground(CRect(mask.left, mask.top, mask.left + mask.right, mask.top + mask.bottom), HudColourNew.GetRGB(HUD_COLOUR_BLACK, FadeIn(150)));

    if (nCurrentInputType == MENUINPUT_BAR) {
        if (CheckHover(mask.left, mask.left + mask.right, mask.top, mask.top + mask.bottom)) {
            CPadNew* pad = CPadNew::GetPad(0);

            if (pad->GetLeftMouseJustUp()) {
                Audio.PlayChunk(CHUNK_MENU_SELECT, 1.0f);

                SetInputTypeAndClear(MENUINPUT_TAB);
            }
        }
    }

    CFontNew::SetBackground(false);
    CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
    CFontNew::SetAlignment(CFontNew::ALIGN_LEFT);
    CFontNew::SetWrapX(SCREEN_COORD(1920.0f));
    CFontNew::SetFontStyle(CFontNew::FONT_1);
    CFontNew::SetDropShadow(0.0f);
    CFontNew::SetOutline(0.0f);
    CFontNew::SetDropColor(CRGBA(0, 0, 0, 0));
    CFontNew::SetColor(HudColourNew.GetRGB(HUD_COLOUR_WHITE, FadeIn(255)));

    CFontNew::SetScale(SCREEN_MULTIPLIER(2.6f), SCREEN_MULTIPLIER(4.8f));
    char* str = TextNew.GetText("FE_NOPAGE").text;
    CFontNew::PrintString(mask.left + SCREEN_COORD(32.0f), mask.top + SCREEN_COORD(19.0f), str);

    char* str1 = TextNew.GetText("FE_NOPAGEMP").text;
    CFontNew::SetScale(SCREEN_MULTIPLIER(0.6f), SCREEN_MULTIPLIER(1.2f));
    CFontNew::PrintString(mask.left + SCREEN_COORD(32.0f), mask.top + SCREEN_COORD(148.0f), str1);
}

CVector2D CMenuNew::GetMapBaseDefault() {
    return CVector2D(GetMenuCenterX(), GetMenuCenterY());
}

void CMenuNew::DrawLegend() {
    if (!bShowLegend)
        return;

    CRect rect;
    rect.left = HUD_RIGHT(96.0f);
    rect.top = HUD_Y(54.0f);
    rect.right = SCREEN_COORD(64.0f);
    rect.bottom = SCREEN_COORD(38.0f);

    CFontNew::SetBackground(false);
    CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
    CFontNew::SetAlignment(CFontNew::ALIGN_RIGHT);
    CFontNew::SetWrapX(SCREEN_COORD(640.0f));
    CFontNew::SetFontStyle(CFontNew::FONT_1);
    CFontNew::SetDropShadow(0.0f);
    CFontNew::SetOutline(0.0f);
    CFontNew::SetDropColor(CRGBA(0, 0, 0, 0));
    CFontNew::SetColor(HudColourNew.GetRGB(HUD_COLOUR_WHITE, 255));
    CFontNew::SetScale(SCREEN_MULTIPLIER(0.6f), SCREEN_MULTIPLIER(1.2f));

    float biggestWidth = 0.0f;
    for (int i = 0; i < CRadarNew::m_nMapLegendBlipCount; i++) {
        int id = CRadarNew::m_MapLegendBlipList[i].id;
        unsigned int c = CRadarNew::m_MapLegendBlipList[i].col;
        int f = CRadarNew::m_MapLegendBlipList[i].friendly;
        char* str = NULL;
        CRGBA col;

        if (id > RADAR_SPRITE_CENTRE) {
            char legendString[64];
            sprintf(legendString, "LG_%02d", id);
            str = TextNew.GetText(legendString).text;
            col = CRadarNew::m_BlipsList[id].color;
        }

        static int level;
        static int time;
        if (time < CTimer::m_snTimeInMillisecondsPauseMode) {
            level++;
            time = 800 + CTimer::m_snTimeInMillisecondsPauseMode;
        }
        
        if (level < RADAR_SPRITE_LEVEL || level > RADAR_SPRITE_HIGHER)
            level = RADAR_SPRITE_LEVEL;

        switch (id) {
        case RADAR_LEVEL_CAR:
            str = TextNew.GetText("LG_VEH").text;
            col = CRadarNew::GetRadarTraceColour(c, false, f);
            id = level;
            break;
        case RADAR_LEVEL_CHAR_FRIENDLY:
            str = TextNew.GetText("LG_CHARF").text;
            col = CRadarNew::GetRadarTraceColour(c, false, f);
            id = level;
            break;
        case RADAR_LEVEL_CHAR_ENEMY:
            str = TextNew.GetText("LG_CHARE").text;
            col = CRadarNew::GetRadarTraceColour(c, false, f);
            id = level;
            break;
        case RADAR_LEVEL_OBJECT:
            str = TextNew.GetText("LG_OBJ").text;
            col = CRadarNew::GetRadarTraceColour(c, false, f);
            id = level;
            break;
        case RADAR_LEVEL_DESTINATION:
            str = TextNew.GetText("LG_DEST").text;
            col = CRadarNew::GetRadarTraceColour(c, false, f);
            id = level;
            break;
        case RADAR_SPRITE_MAP_HERE:
            col = HudColourNew.GetRGBA(MenuNew.Settings.uiMainColor);
            id = RADAR_SPRITE_CENTRE;
            break;
        case RADAR_SPRITE_COP:
        case RADAR_SPRITE_COP_HELI:
            col = CTimer::m_snTimeInMillisecondsPauseMode % 800 < 400 ? HudColourNew.GetRGB(HUD_COLOUR_REDDARK, 255) : HudColourNew.GetRGB(HUD_COLOUR_BLUEDARK, 255);
            break;
        }
        CRadarNew::m_MapLegendBlipList[i].sprite = id;

        if (str) {
            if (!faststrcmp(str, "NONE"))
                continue;

            float s = SCREEN_COORD(26.0f);
            float x = SCREEN_COORD(9.0f);
            float y = SCREEN_COORD(5.0f);

            rect.right = CFontNew::GetStringWidth(str, true) + (s * 2);

            if (biggestWidth < rect.right)
                biggestWidth = rect.right;

            CSprite2d::DrawRect(CRect(rect.left, rect.top, rect.left - rect.right, rect.top + rect.bottom), CRGBA(0, 0, 0, 150));

            CFontNew::PrintString(rect.left - (s * 2) + x, rect.top + y, str);

            col.a = 255;
            CRadarNew::m_BlipsSprites[id]->Draw(CRect(rect.left - s - x, rect.top + y, (rect.left - s - x) + s, rect.top + y + s), col);

            // :(
            rect.top += SCREEN_COORD(41.0f);
            if (rect.top > HUD_BOTTOM(128.0f)) {
                rect.left -= biggestWidth + x;
                rect.top = HUD_Y(54.0f);
            }
        }
    }
}

void CMenuNew::DrawMap() {
    CRect mask = GetMenuScreenRect();

    if (nCurrentInputType == MENUINPUT_TAB) {
        if (bShowMenu) {
            CVector2D in = FindPlayerCentreOfWorld_NoInteriorShift(0);
            CVector2D out;
            CRadarNew::TransformRealWorldPointToRadarSpace(out, in);
            in = out;
            CRadarNew::TransformRadarPointToScreenSpace(out, in);

            vMapBase.x -= out.x - SCREEN_HALF_WIDTH;
            vMapBase.y -= out.y - SCREEN_HALF_HEIGHT;
            vTempMapBase = vMapBase;

            for (int i = 0; i < 24; i++) {
                DoMapZoomInOut(false);
                nMapZoomTime = 0;
            }
        }
        bShowMenu = false;
    }
    else {
        CSprite2d::DrawRect(CRect(mask.left, mask.top, mask.left + mask.right, mask.top + mask.bottom), HudColourNew.GetRGB(HUD_COLOUR_BLACK, FadeIn(150)));
        if (CheckHover(mask.left, mask.left + mask.right, mask.top, mask.top + mask.bottom)) {
            CPadNew* pad = CPadNew::GetPad(0);

            if (pad->GetLeftMouseJustUp()) {
                Audio.PlayChunk(CHUNK_MENU_SELECT, 1.0f);

                SetInputTypeAndClear(MENUINPUT_TAB);
            }
        }
    }

    const float mapHalfSize = GetMenuMapWholeSize() / 2;
    CRect rect;
    CRGBA col = { 255, 255, 255, 255 };

    bDrawMenuMap = true;
    bCleanMapScreenNextFrame = true;
    CRadarNew::m_bRemoveBlipsLimit = true;

    float mapZoom = GetMenuMapTileSize() * fMapZoom;
    rect.left = vMapBase.x;
    rect.top = vMapBase.y;
    rect.right = rect.left + mapZoom;
    rect.bottom = rect.top + mapZoom;

    for (int y = 0; y < GetMenuMapTiles(); y++) {
        for (int x = 0; x < GetMenuMapTiles(); x++) {
            CRadarNew::DrawRadarSectionMap(x, y, CRect((rect.left - mapHalfSize), (rect.top - mapHalfSize), (rect.right - mapHalfSize), (rect.bottom - mapHalfSize)), col);

            rect.left += mapZoom;
            rect.right = rect.left + mapZoom;
        }
        rect.left = vMapBase.x;
        rect.right = rect.left + mapZoom;

        rect.top += mapZoom;
        rect.bottom = rect.top + mapZoom;
    }

    CGPS::DrawPathLine();

    CRadarNew::DrawBlips();

    if (!bShowMenu) {
        DrawMapCrosshair(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
        DrawLegend();
    }
}

void CMenuNew::DrawMapCrosshair(float x, float y) {
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)(rwFILTERLINEARMIPLINEAR));

    float length = SCREEN_COORD(178.0f);
    float width = SCREEN_COORD(3.0f);
    float spacing = SCREEN_COORD(16.0f);

    // Top
    MenuSprites[MENU_MAP_CROSSHAIR_LINE_UP]->Draw(CRect(x - (width / 2), (y - length) - spacing, x + (width / 2), (y - spacing)), CRGBA(255, 255, 255, 255));

    // Right
    MenuSprites[MENU_MAP_CROSSHAIR_LINE_RIGHT]->Draw(CRect(x + spacing, y, x + spacing + length, y + width), CRGBA(255, 255, 255, 255));

    // Bottom
    MenuSprites[MENU_MAP_CROSSHAIR_LINE_UP]->Draw(CRect(x - (width / 2), (y + length) + spacing, x + (width / 2), (y + spacing)), CRGBA(255, 255, 255, 255));

    // Left
    MenuSprites[MENU_MAP_CROSSHAIR_LINE_RIGHT]->Draw(CRect((x - spacing), y, (x - spacing) - length, y + width), CRGBA(255, 255, 255, 255));
}

void CMenuNew::DrawPatternBackground(CRect rect, CRGBA col) {
    CSprite2d::DrawRect(rect, col);

    CRect logo;
    const int w = 64;
    const int h = 70;
    const float spacing = SCREEN_COORD(23.0f);

    logo.left = rect.left + (spacing / 2);
    logo.top = rect.top;
    logo.right = SCREEN_COORD(w);
    logo.bottom = SCREEN_COORD(h);

    const int width = (rect.left + rect.right) / w;
    const int height = 4;
    const int a = clamp(col.a, 0 , 40);
    int alpha = a;

    for (int h = 0; h < height; h++) {
        if (logo.top + logo.bottom > rect.bottom
            || logo.top + logo.bottom < rect.top)
            break;

        for (int w = 0; w < width; w++) {
            if (logo.left + logo.right > rect.right
                || logo.left + logo.right < rect.left)
                break;

            MenuSprites[MENU_ROCKSTARLOGO256]->Draw(CRect(logo.left, logo.top, logo.left + logo.right, logo.top + logo.bottom), CRGBA(150, 150, 150, alpha));
            logo.left += logo.right + spacing;
        }

        logo.left = rect.left + (spacing / 2);
        logo.top += logo.bottom + spacing;
        alpha -= a / height;
        alpha = clamp(alpha, 0, 255);
    }
}

void CMenuNew::DrawBorder(CRect rect, CRGBA col) {
    float scale = SCREEN_COORD(5.0f);
    CRect r;

    r.left = rect.left;
    r.top = rect.top;
    r.right = rect.right;
    r.bottom = scale;
    CSprite2d::DrawRect(CRect(r.left, r.top, r.left + r.right, r.top + r.bottom), col);

    r = rect;
    r.right = scale;
    CSprite2d::DrawRect(CRect(r.left, r.top, r.left + r.right, r.top + r.bottom), col);

    r.left = rect.left + rect.right;
    r.top = rect.top;
    r.right = -scale;
    r.bottom = rect.bottom;
    CSprite2d::DrawRect(CRect(r.left, r.top, r.left + r.right, r.top + r.bottom), col);

    r = rect;
    r.top += rect.bottom;
    r.bottom = -scale;
    CSprite2d::DrawRect(CRect(r.left, r.top, r.left + r.right, r.top + r.bottom), col);
}

void CMenuNew::DrawGallery() {
    CRect mask = GetMenuScreenRect();
    CPadNew* pad = CPadNew::GetPad(0);

    if (nGalleryCount > 0) {
        const float w = GetMenuBarRect().right;
        const float h = SCREEN_COORD(158.0f);

        const int c = GALLERY_COLUMNS;
        const int r = GALLERY_ROWS;

        int tabItem = nCurrentTabItem + (nCurrentGalleryPage * (c * r));
        int tabItemHover = nCurrentTabItem + (nCurrentGalleryPage * (c * r));

        for (int i = 0; i < r; i++) {
            for (int j = 0; j < c; j++) {
                int index = j + c * i;

                index += nCurrentGalleryPage * (c * r);

                mask.right = w;
                mask.bottom = h;

                if (!bShowPictureBigSize) {
                    CSprite2d::DrawRect(CRect(mask.left, mask.top, mask.left + mask.right, mask.top + mask.bottom), CRGBA(0, 0, 0, FadeIn(150)));

                    if (bDrawMouse && CheckHover(mask.left, mask.left + mask.right, mask.top, mask.top + mask.bottom)) {
                        tabItemHover = index;

                        if (pad->GetLeftMouseJustUp()) {
                            Audio.PlayChunk(CHUNK_MENU_SELECT, 1.0f);

                            if (tabItemHover == tabItem)
                                ProcessTabStuff();
                            else
                                SetInputTypeAndClear(MENUINPUT_TAB, tabItemHover);
                        }
                    }
                }

                if (Gallery[index] && Gallery[index]->m_pTexture) {
                    float targetAR = w / h;
                    float imageAR = Gallery[index]->m_pTexture->raster->width / Gallery[index]->m_pTexture->raster->height;
                    float b = 0.0f;
                    float r = 0.0f;
                    float prevTop = mask.top;
                    float prevLeft = mask.left;

                    float targetW = w;
                    float targetH = h;
                    float sizeW = 0.0f;
                    float sizeH = 0.0f;

                    if (targetAR < imageAR) {
                        sizeW = targetW;
                        sizeH = roundf(targetH * targetAR / imageAR);
                    }
                    else {
                        sizeW = roundf(targetH / imageAR * targetAR);
                        sizeH = targetH;
                    }

                    mask.right = sizeW;
                    mask.bottom = sizeH;
                    mask.top += roundf((targetH - sizeH) / 2);
                    mask.left += roundf((targetW - sizeW) / 2);

                    if (bShowPictureBigSize) {
                        if (index == tabItem) {
                            mask = GetMenuScreenRect();
                        }
                        else
                            mask = { 0.0f, 0.0f, 0.0f, 0.0f };
                    }

                    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
                    Gallery[index]->Draw(CRect(mask.left, mask.top, mask.left + mask.right, mask.top + mask.bottom), CRGBA(255, 255, 255, FadeIn(255)));

                    mask.top = prevTop;
                    mask.left = prevLeft;
                    mask.right = w;
                    mask.bottom = h;
                }

                if (!bShowPictureBigSize && index == tabItem) {
                    DrawBorder(CRect(mask.left, mask.top, w, h), CRGBA(255, 255, 255, FadeIn(255)));
                }

                if (!bShowPictureBigSize && index == tabItemHover) {
                    CSprite2d::DrawRect(CRect(mask.left, mask.top, mask.left + w, mask.top + h), CRGBA(255, 255, 255, FadeIn(50)));
                }


                mask.left += mask.right + SCREEN_COORD(3.0f);
            }
            mask.left = GetMenuScreenRect().left;
            mask.top += mask.bottom + SCREEN_COORD(3.0f);
        }

        if (!bShowPictureBigSize) {
            CRect r = GetMenuScreenRect();
            r.top += r.bottom + SCREEN_COORD(3.0f);
            r.bottom = r.top + SCREEN_COORD(41.0f);
            r.right += r.left;

            CSprite2d::DrawRect(r, CRGBA(0, 0, 0, FadeIn(255)));

            CFontNew::SetBackground(false);
            CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
            CFontNew::SetAlignment(CFontNew::ALIGN_LEFT);
            CFontNew::SetWrapX(SCREEN_COORD(640.0f));
            CFontNew::SetFontStyle(CFontNew::FONT_1);
            CFontNew::SetDropShadow(0.0f);
            CFontNew::SetOutline(0.0f);
            CFontNew::SetDropColor(CRGBA(0, 0, 0, 0));
            CFontNew::SetColor(HudColourNew.GetRGB(HUD_COLOUR_WHITE, 255));
            CFontNew::SetScale(SCREEN_MULTIPLIER(0.6f), SCREEN_MULTIPLIER(1.2f));

            char str[16];
            sprintf(str, "%d/%d", nCurrentGalleryPage + 1, MAX_GALLERY_PAGES);
            CFontNew::PrintString(r.left + SCREEN_COORD(8.0f), r.top + SCREEN_COORD(8.0f), str);

            float leftArrow[] = { r.right + SCREEN_COORD(-54.0f), r.top + SCREEN_COORD(4.0f), SCREEN_COORD(32.0f), SCREEN_COORD(32.0f) };
            float rightArrow[] = { r.right + SCREEN_COORD(-34.0f), r.top + SCREEN_COORD(4.0f), SCREEN_COORD(32.0f), SCREEN_COORD(32.0f) };

            MenuSprites[MENU_ARROW_LEFT]->Draw(leftArrow[0], leftArrow[1], leftArrow[2], leftArrow[3], CRGBA(255, 255, 255, 255));
            MenuSprites[MENU_ARROW_RIGHT]->Draw(rightArrow[0], rightArrow[1], rightArrow[2], rightArrow[3], CRGBA(255, 255, 255, 255));

            if (bDrawMouse) {
                if (CheckHover(leftArrow[0], leftArrow[0] + leftArrow[2], leftArrow[1], leftArrow[1] + leftArrow[3])) {
                    if (pad->GetLeftMouseJustUp()) {
                        Audio.PlayChunk(CHUNK_MENU_SELECT, 1.0f);

                        nPreviousGalleryPage = nCurrentGalleryPage;
                        nCurrentGalleryPage--;

                        if (nCurrentGalleryPage < 0)
                            nCurrentGalleryPage = MAX_GALLERY_PAGES - 1;

                        bRequestScreenUpdate = true;
                    }
                }
                else if (CheckHover(rightArrow[0], rightArrow[0] + rightArrow[2], rightArrow[1], rightArrow[1] + rightArrow[3])) {
                    if (pad->GetLeftMouseJustUp()) {
                        Audio.PlayChunk(CHUNK_MENU_SELECT, 1.0f);

                        nPreviousGalleryPage = nCurrentGalleryPage;
                        nCurrentGalleryPage++;

                        if (nCurrentGalleryPage > MAX_GALLERY_PAGES - 1)
                            nCurrentGalleryPage = 0;

                        bRequestScreenUpdate = true;
                    }
                }
            }
        }
    }
    else {
        DrawPatternBackground(CRect(mask.left, mask.top, mask.left + mask.right, mask.top + mask.bottom), HudColourNew.GetRGB(HUD_COLOUR_BLACK, 150));

        if (nCurrentInputType == MENUINPUT_BAR) {
            if (CheckHover(mask.left, mask.left + mask.right, mask.top, mask.top + mask.bottom)) {
                CPadNew* pad = CPadNew::GetPad(0);

                if (pad->GetLeftMouseJustUp()) {
                    Audio.PlayChunk(CHUNK_MENU_SELECT, 1.0f);

                    SetInputTypeAndClear(MENUINPUT_TAB);
                }
            }
        }

        CFontNew::SetBackground(false);
        CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
        CFontNew::SetAlignment(CFontNew::ALIGN_LEFT);
        CFontNew::SetWrapX(SCREEN_COORD(640.0f));
        CFontNew::SetFontStyle(CFontNew::FONT_1);
        CFontNew::SetDropShadow(0.0f);
        CFontNew::SetOutline(0.0f);
        CFontNew::SetDropColor(CRGBA(0, 0, 0, 0));
        CFontNew::SetColor(HudColourNew.GetRGB(HUD_COLOUR_WHITE, 255));
        CFontNew::SetScale(SCREEN_MULTIPLIER(2.6f), SCREEN_MULTIPLIER(4.8f));

        char* str = TextNew.GetText("FE_NOG").text;
        CFontNew::PrintString(mask.left + SCREEN_COORD(32.0f), mask.top + SCREEN_COORD(19.0f), str);

        char* str1 = TextNew.GetText("FE_NOG1").text;
        CFontNew::SetScale(SCREEN_MULTIPLIER(0.6f), SCREEN_MULTIPLIER(1.2f));
        CFontNew::PrintString(mask.left + SCREEN_COORD(32.0f), mask.top + SCREEN_COORD(148.0f), str1);
    }
}

bool CMenuNew::CheckHover(float x1, float x2, float y1, float y2) {
    if (!bDrawMouse)
        return false;

    return (vMousePos.x > x1 && vMousePos.x < x2) && (vMousePos.y > y1 && vMousePos.y < y2);
}

bool CMenuNew::MapCrosshairCheckHover(float x1, float x2, float y1, float y2) {
    if (!bDrawMenuMap && bShowMenu)
        return false;

    float x = SCREEN_HALF_WIDTH;
    float y = SCREEN_HALF_HEIGHT;
    return (x > x1 && x < x2) && (y > y1 && y < y2) || CheckHover(x1, x2, y1, y2);
}

void CMenuNew::PassSettingsToCurrentGame(const CMenuSettings* s) {
    CMenuManager& m = FrontEndMenuManager;

    // Keyboard
    TheCamera.m_fMouseAccelHorzntl = s->mouseSensitivity;
    TheCamera.m_fMouseAccelVertical = s->mouseSensitivity;
    m.bInvertMouseY = !s->invertMouseY;
    CVehicle::m_bEnableMouseSteering = s->mouseSteering;
    CVehicle::m_bEnableMouseFlying = s->mouseFlying;
    m.m_nController = s->controller;
    m.invertPadX1 = s->invertPadX1;
    m.invertPadY1 = s->invertPadY1;
    m.invertPadX2 = s->invertPadX2;
    m.invertPadY2 = s->invertPadY2;
    m.swapPadAxis1 = s->swapPadAxis1;
    m.swapPadAxis2 = s->swapPadAxis2;
    
    {
        TheCamera.m_bUseMouse3rdPerson = s->controller == 0;
    }

    // Audio
    m.m_nSfxVolume = s->sfxVolume;
    m.m_nRadioVolume = s->radioVolume;
    m.m_nRadioStation = s->radioStation;
    m.m_bRadioAutoSelect = s->radioAutoSelect;
    m.m_bRadioEq = s->radioEQ;
    m.m_bTracksAutoScan = s->tracksAutoScan;
    m.m_nRadioMode = s->radioMode;

    {
        AudioEngine.SetMusicMasterVolume(s->radioVolume);
        AudioEngine.SetEffectsMasterVolume(s->sfxVolume);
        Audio.SetChunksMasterVolume(s->sfxVolume);
        Audio.SetVolumeForChunk(CHUNK_TD_LOADING_MUSIC, MenuNew.fLoadingTuneVolume); // 
        AudioEngine.SetBassEnhanceOnOff(s->radioEQ);
        AudioEngine.SetRadioAutoRetuneOnOff(s->radioAutoSelect);
    }

    // Display
    m.m_nBrightness = s->brightness;
    m.m_bShowSubtitles = s->subtitles;
    int prevLang = m.m_nLanguage;
    m.m_nLanguage = s->language;
    m.m_bHudOn = s->showHUD;
    m.m_nRadarMode = !s->showRadar;
    m.m_bSavePhotos = s->savePhotos;

    {
        if (m.m_nLanguage == prevLang) {
            m.m_bLanguageChanged = false;
        }
        else {
            m.m_bLanguageChanged = true;
            m.m_bReinitLanguageSettings = true;
            m.InitialiseChangedLanguageSettings(0);
        }
    }

    // Graphics
    m.m_nAppliedResolution = s->videoMode;
    m.m_nResolution = s->videoMode;
    m.m_bMipMapping = s->mipMapping;
    m.m_nAntiAliasingLevel = s->antiAliasing;
    m.m_nAppliedAntiAliasingLevel = s->antiAliasing;
    m.m_fDrawDistance = s->drawDist;
    m.m_bWidescreenOn = s->widescreen;
    m.m_bFrameLimiterOn = s->frameLimiter;

    {
        CRenderer::ms_lodDistScale = s->drawDist;
        g_fx.SetFxQuality((FxQuality_e)s->visualQuality);
        gamma.SetGamma(s->gamma, 1);
        m.m_bChangeVideoMode = true;
        //MenuNew.ChangeVideoMode(s->currentVideoMode, s->currentAntiAliasing);
    }
}

void CMenuNew::FindOutUsedMemory() {
    RwVideoMode vm;
    RwEngineGetVideoModeInfo(&vm, TempSettings.videoMode);
    int vidPix = (vm.width * vm.height);
    int vidBits = vidPix * 32;
    int vidBytes = vidBits / 8;
    int vidKB = vidBytes / 1024;
    int vidMB = vidKB / 1024;

    nUsedVidMemory = vidMB + 8;

    switch (TempSettings.visualQuality) {
    case 0:
        nUsedVidMemory *= 3;
        break;
    case 1:
        nUsedVidMemory *= 4;
        break;
    case 2:
        nUsedVidMemory *= 5;
        break;
    case 3:
        nUsedVidMemory *= 6;
        break;
    }

    if (TempSettings.mipMapping)
        nUsedVidMemory += 2;

    switch (TempSettings.antiAliasing) {
    case 0:
    case 1:
        break;
    case 2:
        nUsedVidMemory += vidMB + 16;
        break;
    case 3:
        nUsedVidMemory += vidMB + 24;
        break;
    case 4:
        nUsedVidMemory += vidMB + 32;
        break;
    }

    nUsedVidMemory += (float)round((TempSettings.drawDist - 0.8f) * 32.0f);
}

void CMenuNew::ChangeVideoMode(int mode, int msaa) {
    RwD3D9ChangeMultiSamplingLevels(msaa);
    RwD3D9ChangeVideoMode(mode);

    plugin::Call<0x7043D0>(); // CreateCameraSubraster

    int w = Scene.m_pRwCamera->frameBuffer->width;
    int h = Scene.m_pRwCamera->frameBuffer->height;

    RsGlobal.maximumWidth = w;
    RsGlobal.maximumHeight = h;
}

void CMenuNew::ApplyGraphicsChanges() {
    const CMenuSettings& ts = TempSettings;
    CMenuSettings& s = Settings;

    ChangeVideoMode(ts.videoMode, ts.antiAliasing);
    RwTextureSetMipmapping(ts.mipMapping);
    g_fx.SetFxQuality((FxQuality_e)ts.visualQuality);
    CRenderer::ms_lodDistScale = ts.drawDist;

    s = ts;
    s.Save();
    nMenuAlert = MENUALERT_NONE;
}

void CMenuNew::ApplyChanges() {
    const CMenuSettings& ts = TempSettings;
    CMenuSettings& s = Settings;

    switch (MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].Entries[nCurrentEntryItem].type) {
    case MENUENTRY_SCREENTYPE:
    case MENUENTRY_CHANGERES:
    case MENUENTRY_ASPECTRATIO:
    case MENUENTRY_MIPMAPPING:
    case MENUENTRY_REFRESHRATE:
    case MENUENTRY_MSAA:
    case MENUENTRY_FRAMELIMITER:
    case MENUENTRY_DRAWDISTANCE:
    case MENUENTRY_VISUALQUALITY:
        if (ts.screenType != s.screenType
            || ts.videoMode != s.videoMode
            || ts.aspectRatio != s.aspectRatio
            || ts.mipMapping != s.mipMapping
            || ts.antiAliasing != s.antiAliasing
            || ts.drawDist != s.drawDist
            || ts.visualQuality != s.visualQuality
            || ts.widescreen != s.widescreen
            || ts.frameLimiter != s.frameLimiter)
            nMenuAlert = MENUALERT_PENDINGCHANGES;
        else
            nMenuAlert = MENUALERT_NONE;
        break;
    default:
        Settings = TempSettings;
        Settings.Save();
        break;
    }
}

void CMenuNew::RestorePreviousSettings() {
    TempSettings = Settings;
    nMenuAlert = MENUALERT_NONE;
}

void CMenuNew::RestoreDefaults(CMenuSettings* ts, int index) {
    switch (index) {
    case SETTINGS_KEYBOARD:
        ts->mouseSensitivity = 0.0025;
        ts->invertMouseY = false;
        ts->mouseSteering = false;
        ts->mouseFlying = false;
        break;
    case SETTINGS_GAMEPAD:
        ts->controller = 0;
        ts->invertPadX1 = 0;
        ts->invertPadY1 = 0;
        ts->invertPadX2 = 0;
        ts->invertPadY2 = 0;
        ts->swapPadAxis1 = 0;
        ts->swapPadAxis2 = 0;
        break;
    case SETTINGS_AUDIO:
        ts->sfxVolume = 52;
        ts->radioVolume = 52;
        ts->radioStation = 1;
        ts->radioAutoSelect = true;
        ts->radioEQ = true;
        ts->tracksAutoScan = false;
        ts->radioMode = 0;
        break;
    case SETTINGS_DISPLAY:
        ts->brightness = 256;
        ts->gamma = 0.5;
        ts->subtitles = true;
        ts->language = 0;
        ts->showHUD = true;
        ts->showRadar = true;
        ts->savePhotos = true;
        ts->gpsRoute = true;
        ts->safeZoneSize = 32.0;
        ts->measurementSys = 0;
        break;
    case SETTINGS_GRAPHICS:
        //videoMode = 1;
        //currentVideoMode = videoMode;
        ts->aspectRatio = 0;
        ts->mipMapping = true;
        //s->antiAliasing = 1;
        //s->currentAntiAliasing = antiAliasing;
        ts->drawDist = 1.2;
        ts->visualQuality = 0;
        ts->widescreen = false;
        ts->frameLimiter = true;
        break;
    case SETTINGS_SAVINGANDSTARTUP:
        ts->landingPage = true;
        break;
    }
}

void CMenuSettings::Clear() {
    mouseSensitivity = 0.0025;
    invertMouseY = false;
    mouseSteering = false;
    mouseFlying = false;

    controller = 0;
    invertPadX1 = 0;
    invertPadY1 = 0;
    invertPadX2 = 0;
    invertPadY2 = 0;
    swapPadAxis1 = 0;
    swapPadAxis2 = 0;

    sfxVolume = 0;
    radioVolume = 0;
    radioStation = 1;
    radioAutoSelect = 0;
    radioEQ = 0;
    tracksAutoScan = false;
    radioMode = 0;

    brightness = 256;
    gamma = 0.5;
    subtitles = true;
    language = 0;
    showHUD = true;
    showRadar = true;
    savePhotos = true;
    gpsRoute = true;
    safeZoneSize = 32.0;
    measurementSys = 0;

    videoMode = 0;
    aspectRatio = 0;
    mipMapping = true;
    antiAliasing = 1;
    drawDist = 1.2;
    visualQuality = 0;
    widescreen = false;
    frameLimiter = true;

    landingPage = true;
    saveSlot = 0;

    const char* defColour = "HUD_COLOUR_MICHAEL";
    strcpy(uiMainColor, defColour);
}

void CMenuSettings::Load() {
    Clear();
    xml_document doc;
    xml_parse_result file = doc.load_file(PLUGIN_PATH(SettingsFileName));
    if (file) {
        auto settings = doc.child("Settings");
        auto version = settings.child("version").attribute("value");

        if (version.as_double() != MENUSETTINGS_VERSION) {
            printf("XML: Settings file is not compatible.");
        }
        else {
            // Keyboard & Mouse
            if (auto keyboard = settings.child("keyboard")) {
                mouseSensitivity = keyboard.child("MouseSensitivity").attribute("value").as_double();
                invertMouseY = keyboard.child("InvertMouseY").attribute("value").as_bool();
                mouseSteering = keyboard.child("MouseSteering").attribute("value").as_bool();
                mouseFlying = keyboard.child("MouseFlying").attribute("value").as_bool();
            }

            if (auto gamepad = settings.child("gamepad")) {
                controller = gamepad.child("Controller").attribute("value").as_int();
                invertPadX1 = gamepad.child("InvertPadX1").attribute("value").as_bool();
                invertPadY1 = gamepad.child("InvertPadY1").attribute("value").as_bool();
                invertPadX2 = gamepad.child("InvertPadX2").attribute("value").as_bool();
                invertPadY2 = gamepad.child("InvertPadY2").attribute("value").as_bool();
                swapPadAxis1 = gamepad.child("SwapPadAxis1").attribute("value").as_bool();
                swapPadAxis2 = gamepad.child("SwapPadAxis2").attribute("value").as_bool();
            }

            // Audio
            if (auto audio = settings.child("audio")) {
                sfxVolume = (char)audio.child("SfxVolume").attribute("value").as_int();
                radioVolume = (char)audio.child("RadioVolume").attribute("value").as_int();
                radioStation = audio.child("RadioStation").attribute("value").as_int();
                radioAutoSelect = audio.child("RadioAutoSelect").attribute("value").as_bool();
                radioEQ = audio.child("RadioEQ").attribute("value").as_bool();
                tracksAutoScan = audio.child("TracksAutoScan").attribute("value").as_bool();
                radioMode = audio.child("RadioMode").attribute("value").as_int();
            }

            // Display
            if (auto display = settings.child("display")) {
                brightness = display.child("Brightness").attribute("value").as_int();
                gamma = display.child("Gamma").attribute("value").as_double();
                subtitles = display.child("Subtitles").attribute("value").as_bool();
                language = display.child("Language").attribute("value").as_int();
                showHUD = display.child("ShowHUD").attribute("value").as_bool();
                showRadar = display.child("ShowRadar").attribute("value").as_bool();
                weaponTarget = display.child("WeaponTarget").attribute("value").as_int();
                savePhotos = display.child("SavePhotos").attribute("value").as_bool();
                gpsRoute = display.child("GpsRoute").attribute("value").as_bool();
                safeZoneSize = display.child("SafeZoneSize").attribute("value").as_double();
                measurementSys = display.child("MeasurementSys").attribute("value").as_int();
            }

            // Graphics
            if (auto graphics = settings.child("graphics")) {
                videoMode = graphics.child("VideoMode").attribute("value").as_int();
                //aspectRatio = graphics.child("AspectRatio").attribute("value").as_int();
                mipMapping = graphics.child("MipMapping").attribute("value").as_bool();
                antiAliasing = graphics.child("AntiAliasing").attribute("value").as_int();
                drawDist = graphics.child("DrawDist").attribute("value").as_double();
                visualQuality = graphics.child("VisualQuality").attribute("value").as_int();
                widescreen = graphics.child("Widescreen").attribute("value").as_bool();
                frameLimiter = graphics.child("FrameLimiter").attribute("value").as_bool();
            }

            // Saving and Startup
            if (auto startup = settings.child("startup")) {
                landingPage = startup.child("LandingPage").attribute("value").as_bool();
                saveSlot = startup.append_child("SaveSlot").append_attribute("value").as_int();
            }

            // Misc
            if (auto misc = settings.child("misc")) {
                char tmp[32];
                sprintf(tmp, "%s", misc.child("UIMainColor").attribute("value").as_string());
                strcpy(uiMainColor, tmp);
            }
        }
    }
    else {
        for (int i = 0; i < NUM_SETTINGS; i++) {
            MenuNew.RestoreDefaults(this, i);
        }
    }

    MenuNew.PassSettingsToCurrentGame(this);

    CPadNew::LoadSettings();
}

void CMenuSettings::Save() {
    pugi::xml_document doc;

    auto declarationNode = doc.append_child(pugi::node_declaration);
    declarationNode.append_attribute("version") = "1.0";
    declarationNode.append_attribute("encoding") = "UTF-8";

    auto settings = doc.append_child("Settings");
    settings.append_child("version").append_attribute("value").set_value(MENUSETTINGS_VERSION);

    // Keyboard & Mouse
    auto keyboard = settings.append_child("keyboard");
    keyboard.append_child("MouseSensitivity").append_attribute("value").set_value(mouseSensitivity);
    keyboard.append_child("InvertMouseY").append_attribute("value").set_value(invertMouseY);
    keyboard.append_child("MouseSteering").append_attribute("value").set_value(mouseSteering);
    keyboard.append_child("MouseFlying").append_attribute("value").set_value(mouseFlying);

    // Gamepad
    auto gamepad = settings.append_child("gamepad");
    gamepad.append_child("Controller").append_attribute("value").set_value(controller);
    gamepad.append_child("InvertPadX1").append_attribute("value").set_value(invertPadX1);
    gamepad.append_child("InvertPadY1").append_attribute("value").set_value(invertPadY1);
    gamepad.append_child("InvertPadX2").append_attribute("value").set_value(invertPadX2);
    gamepad.append_child("InvertPadY2").append_attribute("value").set_value(invertPadY2);
    gamepad.append_child("SwapPadAxis1").append_attribute("value").set_value(swapPadAxis1);
    gamepad.append_child("SwapPadAxis2").append_attribute("value").set_value(swapPadAxis2);

    // Audio
    auto audio = settings.append_child("audio");
    audio.append_child("SfxVolume").append_attribute("value").set_value(sfxVolume);
    audio.append_child("RadioVolume").append_attribute("value").set_value(radioVolume);
    audio.append_child("RadioStation").append_attribute("value").set_value(radioStation);

    audio.append_child("RadioAutoSelect").append_attribute("value").set_value(radioAutoSelect);
    audio.append_child("RadioEQ").append_attribute("value").set_value(radioEQ);
    audio.append_child("TracksAutoScan").append_attribute("value").set_value(tracksAutoScan);
    audio.append_child("RadioMode").append_attribute("value").set_value(radioMode);

    // Display
    auto display = settings.append_child("display");
    display.append_child("Brightness").append_attribute("value").set_value(brightness);
    display.append_child("Gamma").append_attribute("value").set_value(gamma);
    display.append_child("Subtitles").append_attribute("value").set_value(subtitles);
    display.append_child("Language").append_attribute("value").set_value(language);
    display.append_child("ShowHUD").append_attribute("value").set_value(showHUD);
    display.append_child("ShowRadar").append_attribute("value").set_value(showRadar);
    display.append_child("WeaponTarget").append_attribute("value").set_value(weaponTarget);
    display.append_child("SavePhotos").append_attribute("value").set_value(savePhotos);
    display.append_child("GpsRoute").append_attribute("value").set_value(gpsRoute);
    display.append_child("SafeZoneSize").append_attribute("value").set_value(safeZoneSize);
    display.append_child("MeasurementSystem").append_attribute("value").set_value(measurementSys);

    // Graphics
    auto graphics = settings.append_child("graphics");
    graphics.append_child("VideoMode").append_attribute("value").set_value(videoMode);
    //graphics.append_child("AspectRatio").append_attribute("value").set_value(aspectRatio);
    graphics.append_child("MipMapping").append_attribute("value").set_value(mipMapping);
    graphics.append_child("AntiAliasing").append_attribute("value").set_value(antiAliasing);
    graphics.append_child("DrawDist").append_attribute("value").set_value(drawDist);
    graphics.append_child("VisualQuality").append_attribute("value").set_value(visualQuality);
    graphics.append_child("Widescreen").append_attribute("value").set_value(widescreen);
    graphics.append_child("FrameLimiter").append_attribute("value").set_value(frameLimiter);

    auto startup = settings.append_child("startup");
    startup.append_child("LandingPage").append_attribute("value").set_value(landingPage);
    startup.append_child("SaveSlot").append_attribute("value").set_value(saveSlot);

    auto misc = settings.append_child("misc");
    misc.append_child("UIMainColor").append_attribute("value").set_value(uiMainColor);

    bool file = doc.save_file(PLUGIN_PATH(SettingsFileName));
    if (CreateDirectory(PLUGIN_PATH(UserFilesFolder), NULL) || GetLastError() == ERROR_ALREADY_EXISTS) {
        if (!file) {
            printf("XML: Settings file can't be saved.");
        }
    }

    MenuNew.PassSettingsToCurrentGame(this);

    CPadNew::SaveSettings();
}
