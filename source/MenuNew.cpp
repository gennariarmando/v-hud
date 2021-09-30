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
};

char* MiscSpritesFileNames[] = {
    "rect_grad",
    "rect_grad_centered",
    "skip_icon",
    "spinner",
};

char* LandingPageSpritesFileNames[] = {
    "landing_back_0",
    "landing_front_0",
};

char* SettingsFileName = "VHud\\ufiles\\settings.xml";

inline CRect GetMenuBarRect() { return CRect(MENU_X(311.0f), MENU_Y(181.0f), SCREEN_COORD(214.0f), SCREEN_COORD(38.0f)); }
inline CRect GetMenuTabRect() { return CRect(MENU_X(311.0f), MENU_Y(240.0f), SCREEN_COORD(432.0f), SCREEN_COORD(38.0f)); }
inline CRect GetMenuEntryRect() { return CRect(MENU_X(746.0f), MENU_Y(240.0f), SCREEN_COORD(864.0f), SCREEN_COORD(38.0f)); }
inline CRect GetMenuScreenRect() { return CRect(MENU_X(311.0f), MENU_Y(181.0f + 39.0f + 20.0f), SCREEN_COORD((214.0f + 3.0f) * 6.0f), SCREEN_COORD(645.0f)); }
inline float GetMenuHorSpacing() { return SCREEN_COORD(3.0f); };

int VMenuState = 0;

int& gGameState = *(int*)0xC8D4C0;

CMenuNew::CMenuNew() {
    //patch::Nop(0x53E7A0, 10);
    //patch::Nop(0x53BF3F, 10);

    patch::Set<BYTE>(0x53E797, 0xEB);
    patch::Nop(0x53EB85, 2);
    patch::Nop(0x53E826, 2);
    patch::Nop(0x748CE1, 10);

    // Lock gGameState to 6
    //patch::Set<BYTE>(0xC8D4C0, 5);
    //patch::Nop(0x747483, 6);
    //patch::Nop(0x748C90, 10);
    //patch::Nop(0x748C23, 5);
    //patch::Nop(0x748C2B, 5);

    //patch::Nop(0x53E9F1, 5); // No mouse centering

    //auto doPCTitleFadeOut = []() {
    //    CLoadingScreen::DoPCTitleFadeOut();
    //};
    //patch::RedirectCall(0x748C9A, (void(__cdecl*)())doPCTitleFadeOut);

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

    for (int i = 0; i < NUM_LANDING_SPRITES; i++) {
        LandingSprites[i] = new CSprite2d();
        LandingSprites[i]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\landing_page"), LandingPageSpritesFileNames[i]);    
    }

    for (int i = 0; i < NUM_SCREENSHOTS_SPRITES; i++) {
        char str[32];
        sprintf(str, "info_screen_%d", i);
        InfoScreensSprites[i] = new CSprite2d();
        InfoScreensSprites[i]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\landing_page\\info_screens"), str);
    }

    BuildMenuBar();
    BuildMenuScreen();

    MenuNew.Settings.Load();
    TempSettings = Settings;

    bLandingPage = Settings.landingPage;

    if (bLandingPage)
        SetLandingPageBehaviour();

    bInitialised = true;
}

void CMenuNew::Shutdown() {
    if (!bInitialised)
        return;

    for (int i = 0; i < NUM_MENU_SPRITES; i++) {
        MenuSprites[i]->Delete();
        delete MenuSprites[i];
    }

    for (int i = 0; i < NUM_MISC_SPRITES; i++) {
        MiscSprites[i]->Delete();
        delete MiscSprites[i];
    }

    for (int i = 0; i < NUM_LANDING_SPRITES; i++) {
        LandingSprites[i]->Delete();
        delete LandingSprites[i];
    }

    for (int i = 0; i < NUM_SCREENSHOTS_SPRITES; i++) {
        InfoScreensSprites[i]->Delete();
        delete InfoScreensSprites[i];
    }

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

    bShowMenuBar = true;
    bShowMenu = true;
    bNoTransparentBackg = false;
    bLandingPage = false;
    bInvertInput = false;

    nNumOfSaveGames = 0;

    for (int i = 0; i < 9; i++) {
        strcpy(nSaveSlots[i], "\0");
    }
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
    AddNewScreen("FE_STA");

    // MENUSCREEN_SETTINGS
    if (auto settings = AddNewScreen("FE_SET")) {
        if (auto pad = AddNewTab(settings, MENUTAB_NONE, "FE_PAD", NULL)) {
            AddNewEntry(pad, MENUENTRY_INVERTPADX1, "FE_ILSX", 0, 7);
            AddNewEntry(pad, MENUENTRY_INVERTPADY1, "FE_ILSY", 0, 0);
            AddNewEntry(pad, MENUENTRY_INVERTPADX2, "FE_ILA", 0, 0);
            AddNewEntry(pad, MENUENTRY_INVERTPADY2, "FE_IRSX", 0, 0);
            AddNewEntry(pad, MENUENTRY_SWAPPADAXIS1, "FE_IRSY", 0, 0);
            AddNewEntry(pad, MENUENTRY_SWAPPADAXIS2, "FE_IRA", 0, 0);
            AddNewEntry(pad, MENUENTRY_RESTOREDEFAULTS, "FE_RDEF", 0, 0);
        }

        if (auto key = AddNewTab(settings, MENUTAB_NONE, "FE_KEY", NULL)) {
            AddNewEntry(key, MENUENTRY_MOUSESENSITIVITY, "FE_MVEL", 0, 0);
            AddNewEntry(key, MENUENTRY_INVERTMOUSEY, "FE_MINY", 0, 0);
            AddNewEntry(key, MENUENTRY_MOUSESTEER, "FE_MSTE", 0, 0);
            AddNewEntry(key, MENUENTRY_MOUSEFLYING, "FE_MFLY", 0, 0);
            AddNewEntry(key, MENUENTRY_RESTOREDEFAULTS, "FE_RDEF", 0, 0);
        }

        if (auto kbin = AddNewTab(settings, MENUTAB_ACTION, "FE_KBIN", "FE_KBIN1")) {

        }

        if (auto audio = AddNewTab(settings, MENUTAB_NONE, "FE_AUD", NULL)) {
            AddNewEntry(audio, MENUENTRY_SFXVOLUME, "FE_SFXV", 0, 0);
            AddNewEntry(audio, MENUENTRY_RADIOVOLUME, "FE_RADV", 0, 0);
            AddNewEntry(audio, MENUENTRY_RADIOSTATION, "FE_RSTA", 0, 0);
            AddNewEntry(audio, MENUENTRY_RADIOAUTOSELECT, "FE_RAUT", 0, 0);
            AddNewEntry(audio, MENUENTRY_RADIOEQ, "FE_RAEQ", 0, 0);
            AddNewEntry(audio, MENUENTRY_TRACKSAUTOSCAN, "FE_TAUT", 0, 0);
            AddNewEntry(audio, MENUENTRY_RADIOMODE, "FE_RMOD", 0, 0);
            AddNewEntry(audio, MENUENTRY_RESTOREDEFAULTS, "FE_RDEF", 0, 0);
        }

        if (auto display = AddNewTab(settings, MENUTAB_NONE, "FE_DIS", NULL)) {
            AddNewEntry(display, MENUENTRY_SHOWRADAR, "FE_RAD", 0, 0);
            AddNewEntry(display, MENUENTRY_SHOWHUD, "FE_HUD", 0, 0);
            AddNewEntry(display, MENUENTRY_GPSROUTE, "FE_GPS", 0, 0);
            AddNewEntry(display, MENUENTRY_BRIGHTNESS, "FE_BRI", 0, 0);
            AddNewEntry(display, MENUENTRY_GAMMA, "FE_GMA", 0, 0);
            AddNewEntry(display, MENUENTRY_SAFEZONESIZE, "FE_SAFE", 0, 0);
            AddNewEntry(display, MENUENTRY_SUBTITLES, "FE_SUB", 0, 0);
            AddNewEntry(display, MENUENTRY_MEASUREMENTSYS, "FE_MSYS", 0, 0);
            AddNewEntry(display, MENUENTRY_RESTOREDEFAULTS, "FE_RDEF", 0, 0);
        }

        if (auto graphics = AddNewTab(settings, MENUTAB_NONE, "FE_GFX", NULL)) {
            AddNewEntry(graphics, MENUENTRY_SCREENTYPE, "FE_STP", 0, 2);
            AddNewEntry(graphics, MENUENTRY_CHANGERES, "FE_RES", 0, 0);
            //AddNewEntry(graphics, MENUENTRY_ASPECTRATIO, "FE_AR", 0, 0);
            AddNewEntry(graphics, MENUENTRY_FRAMELIMITER, "FE_FRL", 0, 0);

            AddNewEntry(graphics, MENUENTRY_MSAA, "FE_MSA", 0, 1);
            AddNewEntry(graphics, MENUENTRY_DRAWDISTANCE, "FE_DD", 0, 0);
            AddNewEntry(graphics, MENUENTRY_VISUALQUALITY, "FE_FXQ", 0, 0);
            AddNewEntry(graphics, MENUENTRY_RESTOREDEFAULTS, "FE_RDEF", 0, 0);
        }

        if (auto startup = AddNewTab(settings, MENUTAB_NONE, "FE_SAS", NULL)) {
            AddNewEntry(startup, MENUENTRY_LANDINGPAGE, "FE_LAND", 0, 0);
        }
    }

    // MENUSCREEN_GAME
    if (auto game = AddNewScreen("FE_GAM")) {
        if (auto loadGame = AddNewTab(game, MENUTAB_POPULATESAVESLOT, "FE_LGAM", NULL)) {
            AddNewEntry(loadGame, MENUENTRY_LOADGAME, "FE_NOSAV", 0, 1);
            AddNewEntry(loadGame, MENUENTRY_LOADGAME, "FE_NOSAV", 0, 0);
            AddNewEntry(loadGame, MENUENTRY_LOADGAME, "FE_NOSAV", 0, 0);
            AddNewEntry(loadGame, MENUENTRY_LOADGAME, "FE_NOSAV", 0, 0);
            AddNewEntry(loadGame, MENUENTRY_LOADGAME, "FE_NOSAV", 0, 0);
            AddNewEntry(loadGame, MENUENTRY_LOADGAME, "FE_NOSAV", 0, 0);
            AddNewEntry(loadGame, MENUENTRY_LOADGAME, "FE_NOSAV", 0, 0);
            AddNewEntry(loadGame, MENUENTRY_LOADGAME, "FE_NOSAV", 0, 0);
        }

        if (auto newGame = AddNewTab(game, MENUTAB_ACTION, "FE_NGAM", "FE_NGAM1")) {

        }

        if (auto exitGame = AddNewTab(game, MENUTAB_ACTION, "FE_EXIT", "FE_EXIT1")) {

        }
    }

    // MENUSCREEN_GALLERY
    AddNewScreen("FE_GAL");

    if (auto landing = AddNewScreen("FE_LAND")) {
        AddNewTab(landing, MENUTAB_STORYMODE, "FE_STORY", NULL);
        AddNewTab(landing, MENUTAB_SETTINGS, "FE_SET", NULL);
        AddNewTab(landing, MENUTAB_QUIT, "FE_QUIT", NULL);
    }
}

void CMenuNew::DrawBackground() {
    CSprite2d::DrawRect(CRect(-5.0f, -5.0f, SCREEN_WIDTH + 5.0f, SCREEN_HEIGHT + 5.0f), CRGBA(0, 0, 0, 255));
    MenuNew.LandingSprites[LANDING_BACK_0]->Draw(CRect(MENU_X(0.0f), MENU_Y(0.0f), MENU_RIGHT(0.0f), MENU_BOTTOM(0.0f)), CRGBA(255, 255, 255, 255));
    MenuNew.LandingSprites[LANDING_FRONT_0]->Draw(CRect(MENU_X(0.0f), MENU_Y(-3.0f), MENU_X(764.0f), MENU_BOTTOM(-3.0f)), CRGBA(255, 255, 255, 255));
}

void CMenuNew::SetLandingPageBehaviour() {
    bShowMenu = false;
    bShowMenuBar = false;
    bInvertInput = true;
    bNoTransparentBackg = true;
    OpenMenuScreen(MENUSCREEN_LANDING);
    SetInputTypeAndClear(MENUINPUT_TAB, 0);
}

void CMenuNew::SetDefaultPageBehaviour() {
    bShowMenu = true;
    bShowMenuBar = false;
    bInvertInput = false;
    bNoTransparentBackg = true;
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

CMenuTab* CMenuNew::AddNewTab(CMenuScreen* s, int type, char* tabName, char* actionName) {
    for (int j = 0; j < MAX_MENU_TABS; j++) {
        CMenuTab* t = &s->Tab[j];

        if (t->tabName[0] != '\0')
            continue;

        t->AddTab(type, tabName, actionName);
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

void CMenuNew::SetInputTypeAndClear(int input, int n = 0) {
    if (IsLoading() || AudioEngine.IsRadioRetuneInProgress())
        return;

    if (input > 0) {
        nPreviousInputType = nCurrentInputType;
        nCurrentInputType = input;

        if (n != -1) {
            switch (input) {
            case MENUINPUT_BAR:
                nPreviousBarItem = nCurrentBarItem;
                nCurrentBarItem = n;
                nCurrentBarItemHover = MENU_HOVER_NONE;
                nPreviousBarItemHover = MENU_HOVER_NONE;
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
    for (int i = 0; i < MAX_MENU_TABS; i++) {
        if (MenuScreen[nCurrentScreen].Tab[i].tabName[0] != '\0')
            result++;
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

        if (str[0] != '\0')
            return true;
    }
    else {
        if (MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].Entries[i].entryName[0] != '\0')
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
        CTimer::StartUserPause();
        AudioEngine.Service();

        pad->Clear(0, 1);
        //pad->ClearKeyBoardHistory();
        pad->ClearMouseHistory();

        AudioEngine.StopRadio(NULL, 0);
    }
    else {
        CTimer::EndUserPause();

        pad->Clear(1, 1);
        //pad->ClearKeyBoardHistory();
        pad->ClearMouseHistory();

        Clear();
        SetInputTypeAndClear(MENUINPUT_BAR);
        bRequestScreenUpdate = true;
    }

    nOpenCloseWaitTime = GetTimeInMillisecondsRight() + MENU_OPEN_CLOSE_WAIT_TIME;
    bMenuActive = on;
}

void CMenuNew::OpenMenuScreen(int screen) {
    CTimer::StartUserPause();
    nOpenCloseWaitTime = 0;
    bMenuActive = true;

    nPreviousScreen = nCurrentScreen;
    nCurrentScreen = screen;
}

void CMenuNew::CenterCursor() {
    POINT p;
    p.x = SCREEN_WIDTH / 2;
    p.y = SCREEN_HEIGHT / 2;
    if (ClientToScreen(RsGlobal.ps->window, &p)) {
        SetCursorPos(p.x, p.y);

        RsGlobal.ps->lastMousePos.x = p.x;
        RsGlobal.ps->lastMousePos.y = p.y;
    }
}

void CMenuNew::Process() {
    float x = CPadNew::GetMouseInput(256.0f).x;
    float y = CPadNew::GetMouseInput(256.0f).y;

    vOldMousePos.x = vMousePos.x;
    vOldMousePos.y = vMousePos.y;

    if (x < 0.01f || x > 0.01f)
        vMousePos.x += x;

    if (y < 0.01f || y > 0.01f)
        vMousePos.y += y;

    if (vMousePos.x < 0)
        vMousePos.x = 0;
    if (vMousePos.x > SCREEN_WIDTH)
        vMousePos.x = SCREEN_WIDTH;
    if (vMousePos.y < 0)
        vMousePos.y = 0;
    if (vMousePos.y > SCREEN_HEIGHT)
        vMousePos.y = SCREEN_HEIGHT;

    CenterCursor();

    // Input
    CPadNew* pad = CPadNew::GetPad(0);

    if (pad->CheckForMouseInput())
        bDrawMouse = true;
    else if (pad->CheckForControllerInput())
        bDrawMouse = false;

    bool Up = pad->GetMenuUpJustDown();
    bool Down = pad->GetMenuDownJustDown();
    bool Left = pad->GetMenuLeftJustDown();
    bool Right = pad->GetMenuRightJustDown();
    bool Enter = pad->GetMenuEnterJustDown() || (pad->GetLeftMouseJustDown() && nCurrentEntryItemHover == nCurrentEntryItem);
    bool Back = pad->GetMenuBackJustDown() || (pad->GetRightMouseJustDown() && nCurrentInputType != MENUINPUT_BAR);
    bool Space = pad->GetMenuSpaceJustDown();

    if (bInvertInput) {
        Up = Right;
        Down = Left;
    }

    if (bMenuActive) {
        if (MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].type == MENUTAB_POPULATESAVESLOT) {
            if (!bSaveSlotsPopulated) {
                PcSaveHelper.PopulateSlotInfo();
                nNumOfSaveGames = GetNumOfSaveGames();
                bSaveSlotsPopulated = true;
            }
        }
        else {
            nNumOfSaveGames = 0;
            for (int i = 0; i < 9; i++) {
                strcpy(nSaveSlots[i], "\0");
            }
            bSaveSlotsPopulated = false;
        }

        switch (nCurrentInputType) {
        case MENUINPUT_BAR:
            if (bShowMenuBar) {
                if (Left) {
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
                    SetInputTypeAndClear(MENUINPUT_TAB, nCurrentTabItem);
                }
                else if (Back) {
                    OpenCloseMenu(false, false);
                }
            }
            break;
        case MENUINPUT_TAB:
            if (!IsLoading()) {
                if (Up) {
                    nPreviousTabItem = nCurrentTabItem;
                    nCurrentTabItem--;

                    if (nCurrentTabItem < 0)
                        nCurrentTabItem = GetLastMenuScreenTab() - 1;

                    SetInputTypeAndClear(MENUINPUT_ENTRY, 0);
                    SetInputTypeAndClear(MENUINPUT_TAB, nCurrentTabItem);
                }
                else if (Down) {
                    nPreviousTabItem = nCurrentTabItem;
                    nCurrentTabItem++;

                    if (nCurrentTabItem > GetLastMenuScreenTab() - 1)
                        nCurrentTabItem = 0;

                    SetInputTypeAndClear(MENUINPUT_ENTRY, 0);
                    SetInputTypeAndClear(MENUINPUT_TAB, nCurrentTabItem);
                }
                else if (Enter) {
                    ProcessTabStuff();
                }
                else if (Back) {
                    if (bLandingPage) {
                        if (nCurrentScreen != MENUSCREEN_LANDING) {
                            bShowMenu = false;
                            bShowMenuBar = false;
                            bInvertInput = true;
                            OpenMenuScreen(MENUSCREEN_LANDING);
                            SetInputTypeAndClear(MENUINPUT_TAB, nPreviousTabItem);
                        }
                    }
                    else {
                        SetInputTypeAndClear(MENUINPUT_BAR, nCurrentBarItem);
                    }
                }
            }
            break;
        case MENUINPUT_ENTRY:
            if (!IsLoading()) {
                if (Up) {
                    while (true) {
                        nPreviousEntryItem = nCurrentEntryItem;
                        nCurrentEntryItem--;

                        if (HasToContinueLoopInverse(nCurrentEntryItem))
                            break;

                        if (nCurrentEntryItem < GetFirstMenuScreenEntry())
                            nCurrentEntryItem = GetLastMenuScreenEntry() + 1;
                    }
                }
                else if (Down) {
                    while (true) {
                        nPreviousEntryItem = nCurrentEntryItem;
                        nCurrentEntryItem++;

                        if (HasToContinueLoopInverse(nCurrentEntryItem))
                            break;

                        if (nCurrentEntryItem > GetLastMenuScreenEntry() + 1)
                            nCurrentEntryItem = GetFirstMenuScreenEntry() - 1;                    
                    }
                }
                else if (Back) {
                    if (nMenuAlert == MENUALERT_NONE) {
                        SetInputTypeAndClear(MENUINPUT_ENTRY, 0);
                        SetInputTypeAndClear(MENUINPUT_TAB, nCurrentTabItem);
                    }
                    else {
                        ProcessAlertStuff();
                    }
                }

                ProcessEntryStuff(Enter, Left ? -1 : Right ? 1 : 0);

                if (Space) {
                    if (nMenuAlert == MENUALERT_PENDINGCHANGES)
                        ApplyGraphicsChanges();
                }
            }
            break;
        case MENUINPUT_MESSAGE:
            if (!IsLoading()) {
                ProcessMessagesStuff(Enter, Back, Space, Left ? -1 : Right ? 1 : 0);
            }
            break;
        };

        switch (MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].Entries[nCurrentEntryItem].type) {
        case MENUENTRY_RADIOSTATION:
            StartRadio();
            break;
        default:
            StopRadio();
            break;
        }

        FindOutUsedMemory();

        if (bShowMenuBar) {
            if (bRequestScreenUpdate) {
                if (nCurrentScreen != MenuBar[nCurrentBarItem].targetScreen) {
                    fScreenAlpha = 0;
                    nLoadingTime = GetTimeInMillisecondsRight() + MENU_SCREEN_CHANGE_WAIT_TIME;
                }
                bRequestScreenUpdate = false;
            }

            if (!IsLoading()) {
                if (nCurrentScreen != MenuBar[nCurrentBarItem].targetScreen) {
                    nPreviousScreen = nCurrentScreen;
                    nCurrentScreen = MenuBar[nCurrentBarItem].targetScreen;
                    nLoadingTime = 0;
                }

                fScreenAlpha += 0.02f * 255.0f;
                fScreenAlpha = clamp(fScreenAlpha, 0, 255);
            }
        }
        else {
            fScreenAlpha = 255;
        }
    }
    else {
        if (pad->GetOpenCloseMenuJustDown()) {
            OpenCloseMenu(true, false);
        }
    }
}

unsigned int CMenuNew::GetTimeInMillisecondsRight() {
    return CTimer::m_UserPause ? CTimer::m_snTimeInMillisecondsPauseMode : CTimer::m_CodePause ? CTimer::m_snTimeInMillisecondsNonClipped : CTimer::m_snTimeInMilliseconds;
}

unsigned char CMenuNew::FadeIn(unsigned char alpha) {
    if (nCurrentInputType == MENUINPUT_BAR) {
        return (unsigned char)clamp(fScreenAlpha, 0, alpha / 3);
    }
    else
        return (unsigned char)clamp(fScreenAlpha, 0, alpha);
}

void CMenuNew::ProcessTabStuff() {
    switch (MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].type) {
    case MENUTAB_ACTION:
        if (!faststrcmp(MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].tabName, "FE_NGAM")) {
            SetMenuMessage(MENUMESSAGE_NEW_GAME);
        }
        else if (!faststrcmp(MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].tabName, "FE_EXIT")) {
            SetMenuMessage(MENUMESSAGE_EXIT_GAME);
        }
        break;
    case MENUTAB_STORYMODE:
        DoSettingsBeforeStartingAGame(false);
        break;
    case MENUTAB_SETTINGS:
        SetInputTypeAndClear(MENUINPUT_TAB, 0);
        OpenMenuScreen(MENUSCREEN_SETTINGS);
        SetDefaultPageBehaviour();
        break;
    case MENUTAB_QUIT:
        SetMenuMessage(MENUMESSAGE_EXIT_GAME);
        break;
    default:
        if (GetLastMenuScreenEntry() - 1 != -1)
            SetInputTypeAndClear(MENUINPUT_ENTRY, nPreviousEntryItem);
        break;
    }
}

void CMenuNew::DoSettingsBeforeStartingAGame(bool load) {
    if (load) {
        if (CGenericGameStorage::CheckSlotDataValid(nCurrentEntryItem, false)) {
            FrontEndMenuManager.m_bDontDrawFrontEnd = true;
            CGame::bMissionPackGame = false;
            FrontEndMenuManager.m_bLoadingData = true;
            FrontEndMenuManager.field_1B3C = false;
        }
        else {
            // Error
            return;
        }
    }

    OpenCloseMenu(false, true);
    FrontEndMenuManager.DoSettingsBeforeStartingAGame();
    Clear();
}

void CMenuNew::ProcessMessagesStuff(int enter, int esc, int space, int input) {
    if (!enter && !esc && !space && !input)
        return;

    switch (nCurrentMessage) {
    case MENUMESSAGE_NEW_GAME:
        if (enter) {
            DoSettingsBeforeStartingAGame(false);
        }
        else if (esc) {
            UnSetMenuMessage();
        }
        break;
    case MENUMESSAGE_LOAD_GAME:
        if (enter) {
            DoSettingsBeforeStartingAGame(true);
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

void CMenuNew::ProcessEntryStuff(int enter, int input) {
    if (!enter && !input)
        return;

    switch (MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].Entries[nCurrentEntryItem].type) {
    case MENUENTRY_NONE:
        break;
    case MENUENTRY_LOADGAME:
        if (enter) {
            SetMenuMessage(MENUMESSAGE_LOAD_GAME);
        }
        break;
    case MENUENTRY_SCREENTYPE:
        break;
    case MENUENTRY_CHANGERES:
        if (char** modes = _psGetVideoModeList()) {
            if (input < 0) {
                while (true) {
                    TempSettings.videoMode--;
                    if (TempSettings.videoMode < 0)
                        TempSettings.videoMode = RwEngineGetNumVideoModes();

                    if (modes[TempSettings.videoMode])
                        break;
                }
                ApplyChanges();
            }
            else if (input > 0) {
                while (true) {
                    TempSettings.videoMode++;
                    if (TempSettings.videoMode > RwEngineGetNumVideoModes())
                        TempSettings.videoMode = 0;

                    if (modes[TempSettings.videoMode])
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
        AudioEngine.RetuneRadio(TempSettings.radioStation);
        ApplyChanges();
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
                RestoreDefaults(SETTINGS_GAMEPAD);
            else if (!faststrcmp(MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].tabName, "FE_KEY"))
                RestoreDefaults(SETTINGS_KEYBOARD);
            else if (!faststrcmp(MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].tabName, "FE_AUD"))
                RestoreDefaults(SETTINGS_AUDIO);
            else if (!faststrcmp(MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].tabName, "FE_DIS"))
                RestoreDefaults(SETTINGS_DISPLAY);
            else if (!faststrcmp(MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].tabName, "FE_GFX"))
                RestoreDefaults(SETTINGS_GRAPHICS);
            ApplyChanges();
        }
        break;
    case MENUENTRY_LANDINGPAGE:
        TempSettings.landingPage = TempSettings.landingPage == false;
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

void CMenuNew::StartRadio() {
    if (bRadioEnabled)
        return;

    if (!AECutsceneTrackManager.GetCutsceneTrackStatus()) {
        AERadioTrackManager.StartRadio(Settings.radioStation, 0, 0, 0);
        bRadioEnabled = true;
    }
}

void CMenuNew::StopRadio() {
    if (!bRadioEnabled)
        return;

    AERadioTrackManager.StopRadio(NULL, 0);
    bRadioEnabled = false;
}

void CMenuNew::CheckSliderMovement(double value) {
    CMenuSettings& ts = MenuNew.TempSettings;

    if (value == 0)
        return;

    value = clamp(value, -1, 1);
    value /= 10.0;
    switch (MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].Entries[nCurrentEntryItem].type) {
    case MENUENTRY_DRAWDISTANCE:
        ts.drawDist += float(value * 1.8f);
        ts.drawDist = clamp(ts.drawDist, 0.8f, 1.8f);
        break;
    case MENUENTRY_BRIGHTNESS:
        ts.brightness += int(value * 512);
        ts.brightness = clamp(ts.brightness, 0, 512);
        break;
    case MENUENTRY_GAMMA:
        ts.gamma += float(value * 1.0f);
        ts.gamma = clamp(ts.gamma, 0, 1.0f);
        gamma.SetGamma(ts.gamma, 1);
        break;
    case MENUENTRY_SAFEZONESIZE:
        ts.safeZoneSize += float(value * 32.0f);
        ts.safeZoneSize = clamp(ts.safeZoneSize, 0, 32.0f);
        break;
    case MENUENTRY_SFXVOLUME:
        ts.sfxVolume += char(value * 100);
        ts.sfxVolume = clamp(ts.sfxVolume, 0, 100);
        AudioEngine.SetEffectsMasterVolume(ts.sfxVolume);
        break;
    case MENUENTRY_RADIOVOLUME:
        ts.radioVolume += char(value * 100);
        ts.radioVolume = clamp(ts.radioVolume, 0, 100);
        AudioEngine.SetMusicMasterVolume(ts.radioVolume);
        break;
    case MENUENTRY_MOUSESENSITIVITY:
        ts.mouseSensitivity += float(value * (1.0f / 200.0f));
        ts.mouseSensitivity = clamp(ts.mouseSensitivity, 1.0f/3200.0f, 1.0f/200.0f);
        TheCamera.m_fMouseAccelHorzntl = ts.mouseSensitivity;
        TheCamera.m_fMouseAccelVertical = ts.mouseSensitivity;
        break;
    }
}

void CMenuNew::DrawPauseMenuExtraText() {
    CPlayerInfo playa = CWorld::Players[CWorld::PlayerInFocus];

    // Game name
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

    char* gameName = CTextNew::GetText("FE_GTA").text;
    CFontNew::PrintString(MENU_X(311.0f), MENU_Y(111.0f), gameName);

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
        CTextNew::UpperCase(playerName);
        CFontNew::PrintString(MENU_RIGHT(311.0f + 69.0f + 8.0f), MENU_Y(92.0f), playerName);

        char daytmp[32];
        char* day;
        sprintf(daytmp, "DAY_%d", CClock::CurrentDay);
        day = CTextNew::GetText(daytmp).text;
        CTextNew::UpperCase(day);
        CFontNew::PrintString(MENU_RIGHT(311.0f + 69.0f + 58.0f), MENU_Y(92.0f + 23.0), day);

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

    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)(rwFILTERMIPLINEAR));

    // Set
    if (bNoTransparentBackg) {
        DrawBackground();
    }
    else {
        COverlayLayer::SetEffect(EFFECT_BLUR_COLOR);
    }

    if (bShowMenu) {
        if (bShowMenuBar) {
            DrawPauseMenuExtraText();

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

            nCurrentBarItemHover = MENU_HOVER_NONE;
            for (int i = 0; i < MAX_MENU_BAR_ITEMS; i++) {
                if (MenuBar[i].barName[0] == '\0')
                    continue;

                if (bDrawMouse && CheckHover(menuBar.left, menuBar.left + menuBar.right, menuBar.top, menuBar.top + menuBar.bottom)) {
                    nCurrentBarItemHover = i;

                    CPadNew* pad = CPadNew::GetPad(0);

                    if (pad->GetLeftMouseJustDown()) {
                        SetInputTypeAndClear(MENUINPUT_ENTRY);
                        SetInputTypeAndClear(MENUINPUT_BAR, nCurrentBarItemHover);
                        SetInputTypeAndClear(MENUINPUT_TAB);
                        bRequestScreenUpdate = true;
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

                char* str = CTextNew::GetText(MenuBar[i].barName).text;
                CTextNew::UpperCase(str);
                CFontNew::PrintString(menuBar.left + (menuBar.right * 0.5f), menuBar.top + SCREEN_COORD(6.0f), str);

                menuBar.left += menuBar.right + spacing;
                menuBarSelected.left += menuBarSelected.right + spacing;
            }
        }
        else {
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

            char* str = CTextNew::GetText(MenuScreen[nCurrentScreen].screenName).text;
            CTextNew::UpperCase(str);
            CFontNew::PrintString(bar.left + (bar.right * 0.5f), bar.top + SCREEN_COORD(6.0f), str);
        }
    }

    // Screens
    if (IsLoading()) {
        const float mult = 1.0f;
        const float scale = 32.0f * mult;
        DrawSpinningWheel(SCREEN_COORD_CENTER_LEFT(0.0f), SCREEN_COORD_CENTER_DOWN(0.0f), SCREEN_COORD(scale), SCREEN_COORD(scale));
    }
    else {
        switch (nCurrentScreen) {
        case MENUSCREEN_MAP:
            DrawMap();
            break;
        case MENUSCREEN_GALLERY:
            DrawGallery();
            break;
        case MENUSCREEN_LANDING:
            DrawLandingPage();
            break;
        default:
            DrawDefault();
            break;
        }
    }

    if (nCurrentMessage != MENUMESSAGE_NONE) {
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
        case MENUMESSAGE_EXIT_GAME:
            header = "FE_EXITW";
            msg = "FE_EXITW1";
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
        CFontNew::SetWrapX(SCREEN_COORD(980.0f));
        CFontNew::SetFontStyle(CFontNew::FONT_3);
        CFontNew::SetDropShadow(0.0f);
        CFontNew::SetOutline(0.0f);
        CFontNew::SetDropColor(CRGBA(0, 0, 0, 0));
        CFontNew::SetColor(HudColourNew.GetRGB(HUD_COLOUR_YELLOW, 255));
        CFontNew::SetScale(SCREEN_MULTIPLIER(3.2f), SCREEN_MULTIPLIER(6.3f));

        if (header) {
            CFontNew::PrintString(SCREEN_COORD_CENTER_X, SCREEN_COORD_CENTER_Y + SCREEN_COORD(-116.0f), CTextNew::GetText(header).text);
        }

        if (msg) {
            CFontNew::SetFontStyle(CFontNew::FONT_1);
            CFontNew::SetColor(HudColourNew.GetRGB(HUD_COLOUR_WHITE, 255));
            CFontNew::SetScale(SCREEN_MULTIPLIER(0.86f), SCREEN_MULTIPLIER(1.80f));

            char* str = CTextNew::GetText(msg).text;
            int n = CFontNew::PrintString(SCREEN_COORD_CENTER_X, SCREEN_COORD_CENTER_Y + SCREEN_COORD(36.0f), str);

            CRect r;
            r.left = SCREEN_COORD_CENTER_RIGHT(-64.0f) - clamp(CFontNew::GetStringWidth(str, true), 0, CFontNew::Details.wrapX) / 2;
            r.top = SCREEN_COORD(568.0f);
            r.right = SCREEN_COORD_CENTER_RIGHT(64.0f) + clamp(CFontNew::GetStringWidth(str, true), 0, CFontNew::Details.wrapX) / 2;
            r.bottom = SCREEN_COORD(3.0f);
            CSprite2d::DrawRect(CRect(r.left, r.top, r.right, r.top + r.bottom), CRGBA(HudColourNew.GetRGB(HUD_COLOUR_WHITE, 255)));

            r.top += SCREEN_COORD(22.0f);
            r.top += CFontNew::GetHeightScale(CFontNew::Details.scale.y) * (n);
            CSprite2d::DrawRect(CRect(r.left, r.top, r.right, r.top + r.bottom), CRGBA(HudColourNew.GetRGB(HUD_COLOUR_WHITE, 255)));
        }
    }

    if (bDrawMouse) {
        if (nMouseType <= MOUSE_HAND) {
            MenuSprites[nMouseType]->Draw(vMousePos.x, vMousePos.y, SCREEN_COORD(30.0f), SCREEN_COORD(32.0f), CRGBA(255, 255, 255, 255));
        }
    }
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
    if (MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].type != MENUTAB_POPULATESAVESLOT)
        return 0;

    int result = 0;
    for (int i = 0; i < 9; i++) {
        char* str = CGenericGameStorage::GetNameOfSavedGame(i);

        strcpy(nSaveSlots[i], str);

        if (str[0] != '\0')
            result++;
    }

    return result;
}

void CMenuNew::DrawDefault() {
    {
        CRect menuTab;
        CRGBA menuTabColor;
        CRGBA menuTabTextColor;

        menuTab = GetMenuTabRect();
        nCurrentTabItemHover = -1;
        for (int i = 0; i < MAX_MENU_TABS; i++) {
            if (MenuScreen[nCurrentScreen].Tab[i].tabName[0] == '\0')
                continue;

            if (bDrawMouse && CheckHover(menuTab.left, menuTab.left + menuTab.right, menuTab.top, menuTab.top + menuTab.bottom)) {
                nCurrentTabItemHover = i;

                CPadNew* pad = CPadNew::GetPad(0);

                if (pad->GetLeftMouseJustDown()) {
                    SetInputTypeAndClear(MENUINPUT_ENTRY);
                    SetInputTypeAndClear(MENUINPUT_TAB, nCurrentTabItemHover);
                    bRequestScreenUpdate = true;
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

            char* str = CTextNew::GetText(MenuScreen[nCurrentScreen].Tab[i].tabName).text;
            CFontNew::PrintString(menuTab.left + SCREEN_COORD(12.0f), menuTab.top + SCREEN_COORD(6.0f), str);

            menuTab.top += menuTab.bottom + GetMenuHorSpacing();
        }
    }

    if (MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].type == MENUTAB_ACTION) {
        CRect rect = GetMenuEntryRect();
        rect.bottom = GetMenuScreenRect().bottom;
        DrawPatternBackground(CRect(rect.left, rect.top, rect.left + rect.right, rect.top + rect.bottom), CRGBA(0, 0, 0, FadeIn(180)));

        CFontNew::SetBackground(false);
        CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
        CFontNew::SetAlignment(CFontNew::ALIGN_LEFT);
        CFontNew::SetWrapX(SCREEN_COORD(640.0f));
        CFontNew::SetFontStyle(CFontNew::FONT_1);
        CFontNew::SetDropShadow(0.0f);
        CFontNew::SetOutline(0.0f);
        CFontNew::SetDropColor(CRGBA(0, 0, 0, 0));
        CFontNew::SetColor(HudColourNew.GetRGB(HUD_COLOUR_WHITE, FadeIn(255)));
        CFontNew::SetScale(SCREEN_MULTIPLIER(2.6f), SCREEN_MULTIPLIER(4.8f));

        char* h = CTextNew::GetText(MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].tabName).text;
        char* a = CTextNew::GetText(MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].actionName).text;
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
        CSprite2d::DrawRect(CRect(menuEntry.left, menuEntry.top, menuEntry.left + menuEntry.right, menuEntry.top + bb), CRGBA(0, 0, 0, FadeIn(180)));
        bb = max((menuEntry.bottom * (GetEntryBackHeight() + 1)) + (GetMenuHorSpacing() * (GetEntryBackHeight())), 0);
        //

        sprintf(gString, "currentEntry: %d", nCurrentEntryItem);
        CFontNew::PrintString(0.0f, 0.0f, gString);

        nCurrentEntryItemHover = -1;
        for (int i = 0; i < MAX_MENU_ENTRIES; i++) {
            char* leftText = NULL;
            char* rightText = NULL;
            char leftTextTmp[64];
            char rightTextTmp[64];

            if (HasToContinueLoop(i))
                continue; 

            if (MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].Entries[i].type == MENUENTRY_LOADGAME) {
                leftText = nSaveSlots[i];
                sprintf(leftTextTmp, "%02d - %s", i + 1, leftText ? leftText : CTextNew::GetText("FE_UNK").text);
                leftText = leftTextTmp;
            }
            else {
                leftText = CTextNew::GetText(MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].Entries[i].entryName).text;
            }

            if (i != 0) {
                menuEntry.top += (menuEntry.bottom + GetMenuHorSpacing()) * MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].Entries[i].y;
                bb += (menuEntry.bottom + GetMenuHorSpacing()) * MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].Entries[i].y;
            }

            if (bDrawMouse && CheckHover(menuEntry.left, menuEntry.left + menuEntry.right, menuEntry.top, menuEntry.top + menuEntry.bottom)) {
                nCurrentEntryItemHover = i;

                CPadNew* pad = CPadNew::GetPad(0);

                if (pad->GetLeftMouseJustDown()) {
                    SetInputTypeAndClear(MENUINPUT_ENTRY, nCurrentEntryItemHover);
                    bRequestScreenUpdate = true;
                }
            }

            menuEntryColor = { 0, 0, 0, 0 };
            menuEntryTextColor = HudColourNew.GetRGB(HUD_COLOUR_WHITE, FadeIn(255));
            shiftText = 0.0f;

            if (i == nCurrentEntryItemHover) {
                menuEntryColor = { 100, 100, 100, FadeIn(180) };
                menuEntryTextColor = HudColourNew.GetRGB(HUD_COLOUR_WHITE, FadeIn(255));
            }

            if (nCurrentInputType == MENUINPUT_ENTRY) {
                if (i == nCurrentEntryItem) {
                    menuEntryColor = { 255, 255, 255, FadeIn(255) };
                    menuEntryTextColor = HudColourNew.GetRGB(HUD_COLOUR_BLACK, FadeIn(255));
                    shiftText = 16.0f;
                }
            }

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

            switch (MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].Entries[i].type) {
            case MENUENTRY_NONE:
                break;
            case MENUENTRY_SCREENTYPE:
                rightText = CTextNew::GetText(TempSettings.screenType ? "FE_SCN1" : "FE_SCN0").text;
                break;
            case MENUENTRY_CHANGERES:
                if (char* mode = _psGetVideoModeList()[TempSettings.videoMode]) {
                    strcpy(rightTextTmp, mode);
                    rightText = rightTextTmp;
                }
                else
                    rightText = CTextNew::GetText("FE_UNK").text;
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
                    rightText = CTextNew::GetText("FE_OFF").text;
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
                rightText = CTextNew::GetText(TempSettings.frameLimiter ? "FE_ON" : "FE_OFF").text;
                break;
            case MENUENTRY_VISUALQUALITY:
                switch (TempSettings.visualQuality) {
                case 0:
                    rightText = CTextNew::GetText("FE_LOW").text;
                    break;
                case 1:
                    rightText = CTextNew::GetText("FE_MED").text;
                    break;
                case 2:
                    rightText = CTextNew::GetText("FE_HIG").text;
                    break;
                case 3:
                    rightText = CTextNew::GetText("FE_VHIG").text;
                    break;
                };
                break;
            case MENUENTRY_SHOWHUD:
                rightText = CTextNew::GetText(Settings.showHUD ? "FE_ON" : "FE_OFF").text;
                break;
            case MENUENTRY_SHOWRADAR:
                rightText = CTextNew::GetText(Settings.showRadar ? "FE_ON" : "FE_OFF").text;
                break;
            case MENUENTRY_GPSROUTE:
                rightText = CTextNew::GetText(Settings.gpsRoute ? "FE_ON" : "FE_OFF").text;
                break;
            case MENUENTRY_MEASUREMENTSYS:
                rightText = CTextNew::GetText(Settings.measurementSys ? "FE_IMP" : "FE_MET").text;
                break;
            case MENUENTRY_SUBTITLES:
                rightText = CTextNew::GetText(Settings.subtitles ? "FE_ON" : "FE_OFF").text;
                break;
            case MENUENTRY_INVERTMOUSEY:
                rightText = CTextNew::GetText(Settings.invertMouseY ? "FE_ON" : "FE_OFF").text;
                break;
            case MENUENTRY_MOUSESTEER:
                rightText = CTextNew::GetText(Settings.mouseSteering ? "FE_ON" : "FE_OFF").text;
                break;
            case MENUENTRY_MOUSEFLYING:
                rightText = CTextNew::GetText(Settings.mouseFlying ? "FE_ON" : "FE_OFF").text;
                break;
            case MENUENTRY_RADIOSTATION:
                if (bool radioOff = (Settings.radioStation != RADIO_NONE)) {
                    sprintf(rightTextTmp, "RADIO%d", Settings.radioStation);
                    rightText = CTextNew::GetText(rightTextTmp).text;
                }
                else {
                    rightText = CTextNew::GetText("RADOFF").text;
                }
                break;
            case MENUENTRY_RADIOAUTOSELECT:
                rightText = CTextNew::GetText(Settings.radioAutoSelect ? "FE_ON" : "FE_OFF").text;
                break;
            case MENUENTRY_RADIOEQ:
                rightText = CTextNew::GetText(Settings.radioEQ ? "FE_ON" : "FE_OFF").text;
                break;
            case MENUENTRY_TRACKSAUTOSCAN:
                rightText = CTextNew::GetText(Settings.tracksAutoScan ? "FE_ON" : "FE_OFF").text;
                break;
            case MENUENTRY_RADIOMODE:
                switch (Settings.radioMode) {
                default:
                    rightText = CTextNew::GetText("FE_OFF").text;
                    break;
                }
                break;
            case MENUENTRY_INVERTPADX1:
                rightText = CTextNew::GetText(Settings.invertPadX1 ? "FE_ON" : "FE_OFF").text;
                break;
            case MENUENTRY_INVERTPADY1:
                rightText = CTextNew::GetText(Settings.invertPadY1 ? "FE_ON" : "FE_OFF").text;
                break;
            case MENUENTRY_INVERTPADX2:
                rightText = CTextNew::GetText(Settings.invertPadX2 ? "FE_ON" : "FE_OFF").text;
                break;
            case MENUENTRY_INVERTPADY2:
                rightText = CTextNew::GetText(Settings.invertPadY2 ? "FE_ON" : "FE_OFF").text;
                break;
            case MENUENTRY_SWAPPADAXIS1:
                rightText = CTextNew::GetText(Settings.swapPadAxis1 ? "FE_ON" : "FE_OFF").text;
                break;
            case MENUENTRY_SWAPPADAXIS2:
                rightText = CTextNew::GetText(Settings.swapPadAxis2 ? "FE_ON" : "FE_OFF").text;
                break;
            case MENUENTRY_LANDINGPAGE:
                rightText = CTextNew::GetText(Settings.landingPage ? "FE_ON" : "FE_OFF").text;
                break;

                // Sliders
            case MENUENTRY_BRIGHTNESS:
                DrawSliderRightAlign(menuEntry.left + menuEntry.right + SCREEN_COORD(-12.0f), menuEntry.top + SCREEN_COORD(14.0f), TempSettings.brightness / 512.0f);
                break;
            case MENUENTRY_GAMMA:
                DrawSliderRightAlign(menuEntry.left + menuEntry.right + SCREEN_COORD(-12.0f), menuEntry.top + SCREEN_COORD(14.0f), TempSettings.gamma / 1.0f);
                break;
            case MENUENTRY_SAFEZONESIZE:
                DrawSliderRightAlign(menuEntry.left + menuEntry.right + SCREEN_COORD(-12.0f), menuEntry.top + SCREEN_COORD(14.0f), TempSettings.safeZoneSize / 32.0f);
                break;
            case MENUENTRY_RADIOVOLUME:
                DrawSliderRightAlign(menuEntry.left + menuEntry.right + SCREEN_COORD(-12.0f), menuEntry.top + SCREEN_COORD(14.0f), TempSettings.radioVolume / 100.0f);
                break;
            case MENUENTRY_SFXVOLUME:
                DrawSliderRightAlign(menuEntry.left + menuEntry.right + SCREEN_COORD(-12.0f), menuEntry.top + SCREEN_COORD(14.0f), TempSettings.sfxVolume / 100.0f);
                break;
            case MENUENTRY_DRAWDISTANCE:
                DrawSliderRightAlign(menuEntry.left + menuEntry.right + SCREEN_COORD(-12.0f), menuEntry.top + SCREEN_COORD(14.0f), (TempSettings.drawDist - 0.8f) * 1.0f);
                break;
            case MENUENTRY_MOUSESENSITIVITY:
                DrawSliderRightAlign(menuEntry.left + menuEntry.right + SCREEN_COORD(-12.0f), menuEntry.top + SCREEN_COORD(14.0f), TempSettings.mouseSensitivity * 200.0f);
                break;
            }

            if (rightText) {
                CFontNew::SetAlignment(CFontNew::ALIGN_RIGHT);
                CFontNew::PrintString(menuEntry.left + menuEntry.right + SCREEN_COORD(-12.0f - shiftText), menuEntry.top + SCREEN_COORD(5.0f), rightText);
                
                if (shiftText) {
                    float arrowX = menuEntry.left + menuEntry.right + SCREEN_COORD(-52.0f) - CFontNew::GetStringWidth(rightText, true);
                    float arrowY = menuEntry.top + SCREEN_COORD(6.0f);
                    float arrowScale = SCREEN_COORD(24.0f);
                    MenuSprites[MENU_ARROW_LEFT]->Draw(arrowX, arrowY, arrowScale, arrowScale, CRGBA(0, 0, 0, 255));

                    if (true) {
                        if (CheckHover(arrowX - arrowScale, arrowX + arrowScale, arrowY, arrowY + arrowScale)) {
                            if (CPadNew::GetPad(0)->GetLeftMouseJustDown()) {
                                ProcessEntryStuff(0, -1);

                            }
                        }

                        arrowX = menuEntry.left + menuEntry.right + SCREEN_COORD(-28.0f);
                        MenuSprites[MENU_ARROW_RIGHT]->Draw(arrowX, arrowY, arrowScale, arrowScale, CRGBA(0, 0, 0, 255));

                        if (CheckHover(arrowX - arrowScale, arrowX + arrowScale, arrowY, arrowY + arrowScale)) {
                            if (CPadNew::GetPad(0)->GetLeftMouseJustDown()) {
                                ProcessEntryStuff(0, 1);
                            }
                        }
                    }
                }
            }

            menuEntry.top += menuEntry.bottom + GetMenuHorSpacing();
        }
    }

    if (!faststrcmp(MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].tabName, "FE_GFX")) {
        DrawTabMemoryAvailable();
    }
    if (!faststrcmp(MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].tabName, "FE_PAD")) {
        DrawTabGamePad();
    }
    if (MenuScreen[nCurrentScreen].Tab[nCurrentTabItem].type == MENUTAB_POPULATESAVESLOT) {
        DrawTabNumSaveGames();
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

    char* str = CTextNew::GetText("FE_MEM").text;
    sprintf(gString, "%s: %lu MB / %lu MB", str, nUsedVidMemory, nFreeVidMemory);
    CFontNew::PrintString(menuEntry.left + SCREEN_COORD(12.0f), menuEntry.top + SCREEN_COORD(24.0f), gString);

    float progress = nUsedVidMemory / (nFreeVidMemory / 1.0f);
    DrawSliderRightAlign(menuEntry.left + menuEntry.right + SCREEN_COORD(-12.0f), menuEntry.top + SCREEN_COORD(32.0f), progress);
}

void CMenuNew::DrawTabGamePad() {
    CRect rect = GetMenuEntryRect();

    float bb = max(rect.bottom + GetMenuHorSpacing(), 0) * 6;
    CSprite2d::DrawRect(CRect(rect.left, rect.top, rect.left + rect.right, rect.top + rect.bottom + bb), CRGBA(0, 0, 0, FadeIn(180)));


    CRect ctrlRect = rect;
    ctrlRect.left = MENU_X(990.0f);
    ctrlRect.top = MENU_Y(212.0f);;
    ctrlRect.right = SCREEN_COORD(383.0f);;
    ctrlRect.bottom = SCREEN_COORD(383.0f);;

    MenuSprites[MENU_XBOX_GAMEPAD]->Draw(CRect(ctrlRect.left, ctrlRect.top, ctrlRect.left + ctrlRect.right, ctrlRect.top + ctrlRect.bottom), CRGBA(255, 255, 255, FadeIn(255)));
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

    char* str = CTextNew::GetText("FE_SAVNUM").text;
    sprintf(gString, CTextNew::GetText("FE_SAVNUM").text, nNumOfSaveGames);
    CFontNew::PrintString(menuEntry.left + SCREEN_COORD(12.0f), menuEntry.top + SCREEN_COORD(5.0f), gString);
}

void CMenuNew::DrawLandingPage() {
    CRect rect;
    rect = { MENU_X(522.0f), MENU_Y(218.0f), SCREEN_COORD(1302.0f), SCREEN_COORD(644.0f) };
    DrawPatternBackground(CRect(rect.left, rect.top, rect.left + rect.right, rect.top + rect.bottom), CRGBA(HudColourNew.GetRGB(HUD_COLOUR_BLACK, 150)));
    InfoScreensSprites[INFOSCREEN_0]->Draw(CRect(rect.left + SCREEN_COORD(870.0f), rect.top, rect.left + rect.right, rect.top + rect.bottom), CRGBA(255, 255, 255, 255));

    CFontNew::SetBackground(false);
    CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
    CFontNew::SetAlignment(CFontNew::ALIGN_LEFT);
    CFontNew::SetWrapX(SCREEN_COORD(800.0f));
    CFontNew::SetFontStyle(CFontNew::FONT_1);
    CFontNew::SetDropShadow(0.0f);
    CFontNew::SetOutline(0.0f);
    CFontNew::SetDropColor(CRGBA(0, 0, 0, 0));
    CFontNew::SetColor(HudColourNew.GetRGB(HUD_COLOUR_WHITE, 255));
    CFontNew::SetScale(SCREEN_MULTIPLIER(2.6f), SCREEN_MULTIPLIER(4.8f));

    char* str = CTextNew::GetText("FE_GTACUT").text;
    CFontNew::PrintString(rect.left + SCREEN_COORD(32.0f), rect.top + SCREEN_COORD(19.0f), str);

    char* str1 = CTextNew::GetText("FE_GTAINFO").text;
    CFontNew::SetScale(SCREEN_MULTIPLIER(0.6f), SCREEN_MULTIPLIER(1.2f));
    CFontNew::PrintString(rect.left + SCREEN_COORD(32.0f), rect.top + SCREEN_COORD(148.0f), str1);

    CRect menuEntry;
    CRGBA menuEntryTextColor;

    menuEntry = { MENU_X(532.0f), SCREEN_COORD_BOTTOM(128.0f), MENU_RIGHT(96.0f), SCREEN_COORD(74.0f) };

    CHudNew::DrawSimpleRectGradInverted(CRect(menuEntry.left, menuEntry.top, menuEntry.right, menuEntry.top + menuEntry.bottom), CRGBA(0, 0, 0, 150));

    float shift = SCREEN_COORD(24.0f);
    float spacing = SCREEN_COORD(64.0f);
    float textoffset = SCREEN_COORD(16.5f);

    menuEntry.right -= shift;

    nCurrentTabItemHover = -1;
    for (int i = 0; i < MAX_MENU_ENTRIES; i++) {
        if (MenuScreen[nCurrentScreen].Tab[i].tabName[0] == '\0')
            continue;

        char* leftText = CTextNew::GetText(MenuScreen[nCurrentScreen].Tab[i].tabName).text;

        if (i != 0) {
            char* prevText = CTextNew::GetText(MenuScreen[nCurrentScreen].Tab[i - 1].tabName).text;
            menuEntry.right -= (CFontNew::GetStringWidth(prevText, true) + spacing);
        }

        float nx = (CFontNew::GetStringWidth(leftText, true) + spacing);
        if (bDrawMouse && CheckHover(menuEntry.right - nx, menuEntry.right, menuEntry.top, menuEntry.top + menuEntry.bottom)) {
            nCurrentTabItemHover = i;

            CPadNew* pad = CPadNew::GetPad(0);

            if (pad->GetLeftMouseJustDown()) {
                nPreviousTabItem = nCurrentTabItem;
                nCurrentTabItem = nCurrentTabItemHover;
                ProcessTabStuff();
            }
        }

        menuEntryTextColor = HudColourNew.GetRGB(HUD_COLOUR_GREYLIGHT, FadeIn(255));

        if (i == nCurrentTabItem) {
            menuEntryTextColor = HudColourNew.GetRGB(HUD_COLOUR_WHITE, FadeIn(255));
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
        CFontNew::SetDropColor(CRGBA(0, 0, 0, 0));
        CFontNew::SetColor(menuEntryTextColor);
        CFontNew::SetScale(SCREEN_MULTIPLIER(0.82f), SCREEN_MULTIPLIER(1.6f));

        CTextNew::UpperCase(leftText);
        CFontNew::PrintString(menuEntry.right, menuEntry.top + textoffset, leftText);
        CFontNew::PrintString(menuEntry.right, menuEntry.top + textoffset, leftText);
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

void CMenuNew::DrawMap() {
    CRect mask;
    mask.left = MENU_X(311.0f);
    mask.top = MENU_Y(181.0f + 39.0f + 20.0f);
    mask.right = SCREEN_COORD((214.0f + 3.0f) * 6.0f);
    mask.bottom = SCREEN_COORD(645.0f);

    CSprite2d::DrawRect(CRect(mask.left, mask.top, mask.left + mask.right, mask.top + mask.bottom), HudColourNew.GetRGB(HUD_COLOUR_BLACK, FadeIn(150)));
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

void CMenuNew::DrawGallery() {
    CRect mask;
    mask.left = MENU_X(311.0f);
    mask.top = MENU_Y(181.0f + 39.0f + 20.0f);
    mask.right = SCREEN_COORD(1302.0f);
    mask.bottom = SCREEN_COORD(645.0f);

    DrawPatternBackground(CRect(mask.left, mask.top, mask.left + mask.right, mask.top + mask.bottom), HudColourNew.GetRGB(HUD_COLOUR_BLACK, 150));

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

    char* str = CTextNew::GetText("FE_NOG").text;
    CFontNew::PrintString(mask.left + SCREEN_COORD(32.0f), mask.top + SCREEN_COORD(19.0f), str);

    char* str1 = CTextNew::GetText("FE_NOG1").text;
    CFontNew::SetScale(SCREEN_MULTIPLIER(0.6f), SCREEN_MULTIPLIER(1.2f));
    CFontNew::PrintString(mask.left + SCREEN_COORD(32.0f), mask.top + SCREEN_COORD(148.0f), str1);

}

bool CMenuNew::CheckHover(int x1, int x2, int y1, int y2) {
    return (vMousePos.x > x1 && vMousePos.x < x2) && (vMousePos.y > y1 && vMousePos.y < y2);
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
    m.m_bMapLegend = s->mapLegend;

    {
        if (m.m_nLanguage == prevLang) {
            m.m_bLanguageChanged = false;
        }
        else {
            m.m_bLanguageChanged = true;
            TheText.Load();
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
    int vidBits = vidPix * vm.depth;
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

    CTextureMgr::ReloadTextures();
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

void CMenuNew::RestoreDefaults(int index) {
    CMenuSettings& ts = TempSettings;

    switch (index) {
    case SETTINGS_KEYBOARD:
        ts.mouseSensitivity = 1.0f / 400.0f;
        ts.invertMouseY = false;
        ts.mouseSteering = false;
        ts.mouseFlying = false;
        break;
    case SETTINGS_GAMEPAD:
        ts.controller = 0;
        ts.invertPadX1 = 0;
        ts.invertPadY1 = 0;
        ts.invertPadX2 = 0;
        ts.invertPadY2 = 0;
        ts.swapPadAxis1 = 0;
        ts.swapPadAxis2 = 0;
        break;
    case SETTINGS_AUDIO:
        ts.sfxVolume = 90;
        ts.radioVolume = 90;
        ts.radioStation = 1;
        ts.radioAutoSelect = 0;
        ts.radioEQ = 0;
        ts.tracksAutoScan = false;
        ts.radioMode = 0;
        break;
    case SETTINGS_DISPLAY:
        ts.brightness = 256;
        ts.gamma = 0.5f;
        ts.subtitles = true;
        ts.language = 0;
        ts.showHUD = true;
        ts.showRadar = true;
        ts.savePhotos = true;
        ts.mapLegend = true;
        ts.gpsRoute = true;
        ts.safeZoneSize = 0.0f;
        ts.measurementSys = 0;
        break;
    case SETTINGS_GRAPHICS:
        //videoMode = 1;
        //currentVideoMode = videoMode;
        ts.aspectRatio = 0;
        ts.mipMapping = true;
        //s.antiAliasing = 1;
        //s.currentAntiAliasing = antiAliasing;
        ts.drawDist = 1.2f;
        ts.visualQuality = 0;
        ts.widescreen = false;
        ts.frameLimiter = true;
        break;
    case SETTINGS_SAVINGANDSTARTUP:
        ts.landingPage = true;
        break;
    }
}

void CMenuSettings::Clear() {
    mouseSensitivity = 1.0f / 400.0f;
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

    sfxVolume = 90;
    radioVolume = 90;
    radioStation = 1;
    radioAutoSelect = 0;
    radioEQ = 0;
    tracksAutoScan = false;
    radioMode = 0;

    brightness = 256;
    gamma = 0.5f;
    subtitles = true;
    language = 0;
    showHUD = true;
    showRadar = true;
    savePhotos = true;
    mapLegend = true;
    gpsRoute = true;
    safeZoneSize = 0.0f;
    measurementSys = 0;

    videoMode = 1;
    aspectRatio = 0;
    mipMapping = true;
    antiAliasing = 1;
    drawDist = 1.2f;
    visualQuality = 0;
    widescreen = false;
    frameLimiter = true;

    landingPage = true;

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
                mouseSensitivity = keyboard.child("MouseSensitivity").attribute("value").as_float();
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
                sfxVolume = audio.child("SfxVolume").attribute("value").as_int();
                radioVolume = audio.child("RadioVolume").attribute("value").as_int();
                radioStation = audio.child("RadioStation").attribute("value").as_int();
                radioAutoSelect = audio.child("RadioAutoSelect").attribute("value").as_bool();
                radioEQ = audio.child("RadioEQ").attribute("value").as_bool();
                tracksAutoScan = audio.child("TracksAutoScan").attribute("value").as_bool();
                radioMode = audio.child("RadioMode").attribute("value").as_int();
            }

            // Display
            if (auto display = settings.child("display")) {
                brightness = display.child("Brightness").attribute("value").as_int();
                gamma = display.child("Gamma").attribute("value").as_float();
                subtitles = display.child("Subtitles").attribute("value").as_bool();
                language = display.child("Language").attribute("value").as_int();
                showHUD = display.child("ShowHUD").attribute("value").as_bool();
                showRadar = display.child("ShowRadar").attribute("value").as_bool();
                savePhotos = display.child("SavePhotos").attribute("value").as_bool();
                mapLegend = display.child("MapLegend").attribute("value").as_bool();
                gpsRoute = display.child("GpsRoute").attribute("value").as_bool();
                safeZoneSize = display.child("SafeZoneSize").attribute("value").as_float();
                measurementSys = display.child("MeasurementSys").attribute("value").as_int();
            }

            // Graphics
            if (auto graphics = settings.child("graphics")) {
                videoMode = graphics.child("VideoMode").attribute("value").as_int();
                //aspectRatio = graphics.child("AspectRatio").attribute("value").as_int();
                mipMapping = graphics.child("MipMapping").attribute("value").as_bool();
                antiAliasing = graphics.child("AntiAliasing").attribute("value").as_int();
                drawDist = graphics.child("DrawDist").attribute("value").as_float();
                visualQuality = graphics.child("VisualQuality").attribute("value").as_int();
                widescreen = graphics.child("Widescreen").attribute("value").as_bool();
                frameLimiter = graphics.child("FrameLimiter").attribute("value").as_bool();
            }

            // Saving and Startup
            if (auto startup = settings.child("startup")) {
                landingPage = startup.child("LandingPage").attribute("value").as_bool();
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
            MenuNew.RestoreDefaults(i);
        }
    }

    MenuNew.PassSettingsToCurrentGame(this);
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
    display.append_child("SavePhotos").append_attribute("value").set_value(savePhotos);
    display.append_child("MapLegend").append_attribute("value").set_value(mapLegend);
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

    auto misc = settings.append_child("misc");
    misc.append_child("UIMainColor").append_attribute("value").set_value(uiMainColor);

    bool file = doc.save_file(PLUGIN_PATH(SettingsFileName));
    if (!file) {
        printf("XML: Settings file can't be saved.");
    }

    MenuNew.PassSettingsToCurrentGame(this);
}
