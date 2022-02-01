#pragma once
#define MENUSETTINGS_VERSION 1.0
#define MENU_SCREEN_CHANGE_WAIT_TIME 250

#define MENU_MAP_ZOOM_MIN (1.1f)
#define MENU_MAP_ZOOM_MAX (32.0f)

#define FAKE_LOADING_WAIT_TIME 25

#include "PadNew.h"
#include "CSprite2d.h"

typedef void (*funcCall)();

enum eMouseType : char {
    MOUSE_ARROW,
    MOUSE_FUCKU,
    MOUSE_GRAB,
    MOUSE_HAND,
};

enum eMenuSprites {
    MENU_MOUSE_ARROW,
    MENU_MOUSE_FUCKU,
    MENU_MOUSE_GRAB,
    MENU_MOUSE_HAND, 
    MENU_ROCKSTARLOGO256,
    MENU_XBOX_GAMEPAD,
    MENU_ARROW_LEFT,
    MENU_ARROW_RIGHT,
    MENU_BOUGHT,
    MENU_BOUGHT_B,
    MENU_CAR,
    MENU_CAR_B,
    MENU_CLOTHES,
    MENU_CLOTHES_B,
    MENU_WEAPONS,
    MENU_WEAPONS_B,
    MENU_MAP_CROSSHAIR_LINE_RIGHT,
    MENU_MAP_CROSSHAIR_LINE_UP,
    MENU_SELECTOR,
    NUM_MENU_SPRITES,
};

enum eMiscSprites {
    MISC_RECTGRAD,
    MISC_RECTGRADCENTERED,
    MISC_SKIPICON,
    MISC_SPINNER,
    NUM_MISC_SPRITES,
};

enum eFrontendSprites {
    FRONTEND_BACK,
    FRONTEND_FRONT,
    FRONTEND_INFOBOX,
    FRONTEND_GTALOGO,
    NUM_FRONTEND_SPRITES
};

enum eMenuBarItems {
    MENUBAR_MAP,
    MENUBAR_BRIEF,
    MENUBAR_STATS,
    MENUBAR_SETTINGS,
    MENUBAR_GAME,
    MENUBAR_GALLERY,
};

enum eMenuScreens {
    MENUSCREEN_NONE,
    MENUSCREEN_MAP,
    MENUSCREEN_BRIEF,
    MENUSCREEN_STATS,
    MENUSCREEN_SETTINGS,
    MENUSCREEN_GAME,
    MENUSCREEN_GALLERY,
    MENUSCREEN_LANDING,
    MENUSCREEN_SAVE,
    MENUSCREEN_KEYBIND,
    MENUSCREEN_LOADING,
    NUM_MENUSCREENS
};

enum eMenuEntries {
    MENUENTRY_NONE = 0,
    MENUENTRY_CHANGETAB,
    MENUENTRY_SCREENTYPE,
    MENUENTRY_CHANGERES,
    MENUENTRY_ASPECTRATIO,
    MENUENTRY_MIPMAPPING,
    MENUENTRY_REFRESHRATE,
    MENUENTRY_MSAA,
    MENUENTRY_FRAMELIMITER,
    MENUENTRY_DRAWDISTANCE,
    MENUENTRY_VISUALQUALITY,
    MENUENTRY_SFXVOLUME,
    MENUENTRY_RADIOVOLUME,
    MENUENTRY_RADIOSTATION,
    MENUENTRY_RADIOAUTOSELECT,
    MENUENTRY_RADIOEQ,
    MENUENTRY_TRACKSAUTOSCAN,
    MENUENTRY_RADIOMODE,
    MENUENTRY_MOUSESENSITIVITY,
    MENUENTRY_INVERTMOUSEY,
    MENUENTRY_MOUSESTEER,
    MENUENTRY_MOUSEFLYING,
    MENUENTRY_SHOWCONTROLSFOR,
    MENUENTRY_INVERTPADX1,
    MENUENTRY_INVERTPADY1,
    MENUENTRY_INVERTPADX2,
    MENUENTRY_INVERTPADY2,
    MENUENTRY_SWAPPADAXIS1,
    MENUENTRY_SWAPPADAXIS2,
    MENUENTRY_BRIGHTNESS,
    MENUENTRY_GAMMA,
    MENUENTRY_SHOWHUD,
    MENUENTRY_SHOWRADAR,
    MENUENTRY_WEAPONTARGET,
    MENUENTRY_SIMPLERETICULESIZE,
    MENUENTRY_GPSROUTE,
    MENUENTRY_SAFEZONESIZE,
    MENUENTRY_MEASUREMENTSYS,
    MENUENTRY_SUBTITLES,
    MENUENTRY_RESTOREDEFAULTS,
    MENUENTRY_LANDINGPAGE,
    MENUENTRY_LOADGAME,
    MENUENTRY_NUMOFSAVEGAMES,
    MENUENTRY_ACTION,
    MENUENTRY_PAD,
    MENUENTRY_GFX,
    MENUENTRY_POPULATESAVESLOT,
    MENUENTRY_SAVEGAME,
    MENUENTRY_STORYMODE,
    MENUENTRY_SETTINGS,
    MENUENTRY_QUIT,
    MENUENTRY_MAP,
    MENUENTRY_STAT,
    MENUENTRY_SHOWPICTURE,
    MENUENTRY_REDEFINEKEY,
    MENUENTRY_KEYBINDING,
    MENUENTRY_LANGUAGE,
};

enum eMenuMessages {
    MENUMESSAGE_NONE,
    MENUMESSAGE_NEW_GAME,
    MENUMESSAGE_LOAD_GAME,
    MENUMESSAGE_SAVE_GAME,
    MENUMESSAGE_EXIT_GAME,
    MENUMESSAGE_DELETE_GAME,
    MENUMESSAGE_LOSE_CHANGES_ASK,
    MENUMESSAGE_RESTOREDEFAULTKEYS,
    MENUMESSAGE_SETKEYTONULL,
};

enum {
    MAX_MENU_BAR_ITEMS = 12,
    MAX_MENU_SCREENS = 12,
    MAX_MENU_TABS = 128,
    MAX_MENU_ENTRIES = 128,
    MAX_HELP_TEXT = 8,
    VISIBLE_ENTRIES = 16,
};

enum {
    MENU_HOVER_NONE = -1,
};

enum {
    MENUINPUT_BAR = 1,
    MENUINPUT_TAB, 
    MENUINPUT_ENTRY,
    MENUINPUT_MESSAGE,
    MENUINPUT_GALLERYPIC,
    MENUINPUT_REDEFINEKEY,
};

enum eSettingsIndex {
    SETTINGS_GAMEPAD,
    SETTINGS_KEYBOARD,
    SETTINGS_AUDIO,
    SETTINGS_DISPLAY,
    SETTINGS_GRAPHICS,
    SETTINGS_SAVINGANDSTARTUP,
    SETTINGS_REDEFINEKEY,
    NUM_SETTINGS,
};

enum eMenuAlerts {
    MENUALERT_NONE,
    MENUALERT_PENDINGCHANGES,
};

enum {
    GALLERY_COLUMNS = 6,
    GALLERY_ROWS = 4,
    MAX_GALLERY_PAGES = 4,
    MAX_GALLERY_PICTURES = ((GALLERY_COLUMNS * GALLERY_ROWS) * MAX_GALLERY_PAGES),
};

/*
-MENUBAR:
     _________________   _________________
    |       MAP       | |      BRIEF      |

-SCREEN:
   -MENUTAB:          -MENUENTRY:

    DISPLAY  |         Resolution: 1920x1080x32
    GRAPHICS |         Aspect Ratio: Auto
    AUDIO    |
*/

class CMenuBar {
public:
    char barName[16];
    int targetScreen;

public:
    inline CMenuBar* AddItem(char* name, int screen) {
        strcpy(barName, name);
        targetScreen = screen;
        return this;
    }
};

class CMenuEntry {
public:
    int type;
    char entryName[16];
    int x;
    int y;

public:
    inline CMenuEntry* AddEntry(int _type, char* _entryName, int _x, int _y) {
        type = _type;
        strcpy(entryName, _entryName);
        x = _x;
        y = _y;
        return this;
    }

    inline void RemoveEntry() {
        type = MENUENTRY_NONE;
        for (int i = 0; i < 16; i++)
            entryName[i] = '\0';
        x = 0;
        y = 0;
    }
};

class CMenuTab {
public:
    int type;
    char tabName[16];
    char actionName[16];
    CMenuEntry Entries[MAX_MENU_ENTRIES]; 
    bool full;

public:
    inline void AddTab(int _type, char* _tabName, char* _actionName, bool _full) {
        type = _type;
        strcpy(tabName, _tabName);
        if (_actionName)
            strcpy(actionName, _actionName);
        full = _full;
    }
};

class CMenuScreen {
public:
    char screenName[16];
    CMenuTab Tab[MAX_MENU_TABS];

public:
    inline void AddScreen(char* _screenName) {
        strcpy(screenName, _screenName);
    }
};

class CMenuSettings {
public:
    // Keyboard & Mouse
    double mouseSensitivity;
    bool invertMouseY;
    bool mouseSteering;
    bool mouseFlying;

    // Gamepad
    int showControlsFor;
    int controller;
    bool invertPadX1;
    bool invertPadY1;
    bool invertPadX2;
    bool invertPadY2;
    bool swapPadAxis1;
    bool swapPadAxis2;

    // Audio
    char sfxVolume;
    char radioVolume;
    char radioStation;
    bool radioAutoSelect;
    bool radioEQ;
    bool tracksAutoScan;
    char radioMode;

    // Display
    int brightness;
    double gamma;
    bool subtitles;
    int language;
    bool showHUD;
    bool showRadar;
    int weaponTarget;
    double simpleReticuleSize;
    bool savePhotos;
    bool gpsRoute;
    double safeZoneSize;
    int measurementSys;

    // Graphics
    int screenType;
    int videoMode;
    int screenWidth;
    int screenHeight;
    int aspectRatio;
    bool mipMapping;
    int antiAliasing;
    double drawDist;
    int visualQuality;
    bool widescreen;
    bool frameLimiter;

    // Saving and Startup
    bool landingPage;
    int saveSlot;

    // Misc
    char uiMainColor[32];

public:
    void Clear();
    void Load();
    void Save();
};

class CMenuNew {
public:
    CSprite2d* MenuSprites[NUM_MENU_SPRITES];
    CSprite2d* MiscSprites[NUM_MISC_SPRITES];
    CSprite2d* FrontendSprites[NUM_FRONTEND_SPRITES];

    CMenuSettings TempSettings;
    CMenuSettings Settings;
    CMenuBar MenuBar[MAX_MENU_BAR_ITEMS];
    CMenuScreen MenuScreen[MAX_MENU_SCREENS];

    int NumVideoModes;
    char** VideoModeList;

public:
    bool bInitialised;
    char nMouseType;
    bool bDrawMouse;
    CVector2D vMousePos;
    CVector2D vOldMousePos;

    bool bMenuActive;

    bool bShowMenu;

    int nPreviousInputType;
    int nCurrentInputType;

    int nCurrentBarItem;
    int nPreviousBarItem;

    int nCurrentBarItemHover;
    int nPreviousBarItemHover;

    int tabItemBeforeScreenChange;
    int nCurrentTabItem;
    int nPreviousTabItem;
    int nCurrentTabItemHover;
    int nPreviousTabItemHover;

    int nCurrentEntryItem;
    int nPreviousEntryItem;
    int nCurrentEntryItemHover;
    int nPreviousEntryItemHover;

    int nCurrentScreen;
    int nPreviousScreen;

    float fMenuEntryScrollOffset;

    bool bRequestScreenUpdate;
    float fScreenAlpha;
    unsigned int nLoadingTime;

    unsigned int nUsedVidMemory;
    unsigned int nFreeVidMemory;

    bool bRadioEnabled;

    int nCurrentMessage;
    int nPreviousMessage;

    int nMenuAlert;

    bool bShowMenuExtraText;
    bool bShowMenuBar;
    bool bNoTransparentBackg;
    bool bStylizedBackground;

    bool bLandingPage;
    bool bStartOrLoadGame;
    bool bInvertInput;

    int nNumOfSaveGames;
    bool bSaveSlotsPopulated;

    char nSaveSlots[9][64];
    char nSaveSlotsDate[9][32];

    int nControlsHelperCount;

    struct {
        char* text;
        bool hasFunc;
        funcCall func;
    } nControlsHelper[MAX_HELP_TEXT];

    bool bSavePage;

    bool bRequestMenuClose;

    bool bCleanMapScreenNextFrame;
    bool bDrawMenuMap;
    CVector2D vMapBase;
    CVector2D vTempMapBase;
    float fMapZoom;

    int nMapZoomTime;
    bool bShowLegend;

    int nTimeForSafeZonesToShow;

    bool bPreviouslyInGallery;
    bool bScanGallery;
    int nGalleryCount;
    CSprite2d* Gallery[MAX_GALLERY_PICTURES];
    bool bShowPictureBigSize;
    int nPreviousGalleryPage;
    int nCurrentGalleryPage;

    float fLoadingTuneVolume;
    bool bLoadingTuneStarted;

    unsigned short PreviousPlayerControls;
    unsigned short CurrentPlayerControls;

    int nTimePassedSinceLastKeyBind;

    bool bLoadingPage;
    float fLoadingPercentage;
    int nLoadingTimeInMs;
    bool bLoad;
    int nSlot;

public:
    void Init();
    void Shutdown();
    void Clear();
    void BuildMenuBar();
    void BuildMenuScreen();

    void DrawBackground();

    void SetLandingPageBehaviour();
    void SetLoadingPageBehaviour();
    void PlayLoadingTune();
    void StopLoadingTune();
    void DoFadeTune();
    void SetSavePageBehaviour(bool background);
    void SetDefaultPageBehaviour();

    void AddNewBarItem(char* name, int screen);
    CMenuScreen* AddNewScreen(char* name);
    CMenuTab* AddNewTab(CMenuScreen* s, int type, char* tabName, char* actionName, bool full);
    CMenuEntry* AddNewEntry(CMenuTab* t, int type, char* entryName, int x, int y);

    CMenuScreen* GetMenuScreen(char* name);
    CMenuTab* GetMenuTab(CMenuScreen* s, char* name);
    CMenuEntry* GetMenuEntry(CMenuTab* t, char* name);
    CMenuEntry* GetMenuEntry(CMenuTab* t, int i);

    void SetInputTypeAndClear(int input, int n = 0);
    int GetLastMenuBarItem();
    int GetLastMenuScreenTab();
    int GetFirstMenuScreenEntry();
    int GetLastMenuScreenEntry();
    bool HasToContinueLoop(int i);
    bool HasToContinueLoopInverse(int i);
    int GetEntryBackHeight();
    void OpenCloseMenu(bool on);
    void OpenMenuScreen(int screen);
    void CenterCursor();
    void DoMapZoomInOut(bool out);
    void RemoveUnusedControllerSettings();
    void ProcessGoThrough(int input);
    void ProcessGoBack(int input);
    void Process();
    void ScanGalleryPictures(bool force);
    unsigned char FadeIn(unsigned char alpha);
    void ProcessTabStuff();
    void DoSettingsBeforeStartingAGame(bool load, int slot = -1);
    void DoStuffBeforeSavingAGame(int slot);
    bool ProcessMenuToGameSwitch(bool force);
    void ProcessMessagesStuff(int enter, int esc, int space, int input);
    void ProcessAlertStuff();
    int ResToIndex(int w, int h);
    char** GetVideoModeList();
    void ProcessEntryStuff(int enter, int input);
    void RetuneRadio(char id);
    void StartRadio();
    void StopRadio();
    void CheckSliderMovement(double value);
    void DrawPauseMenuExtraText();
    void Draw();
    void DrawControlsHelper();
    void AppendHelpText(const char* text, funcCall func);
    void SetMenuMessage(int type);
    void UnSetMenuMessage();
    bool IsLoading();
    int GetNumOfSaveGames();
    void DrawSafeZoneAngle(float x, float y, int w, int h);
    void DrawDefault();
    void DrawTabMemoryAvailable();
    void DrawPadLine(float x, float y, int w, int h);
    void DrawTabGamePad();
    void DrawTabNumSaveGames();
    void DrawTabKeyBindings();
    void DrawLandingPage();
    void DrawInfoBox();
    void DrawLoadingPage();
    void DrawLoadingBar(char* str);
    void DrawSliderRightAlign(float x, float y, float progress);
    void DrawTabRadioIcons(float x, float y);
    void DrawSpinningWheel(float x, float y, float w, float h);
    void ResetMap();
    void SetWaypoint(float x, float y);
    float GetMenuMapTileSize();
    int GetMenuMapTiles();
    float GetMenuMapWholeSize();
    void PrintBrief();
    void PrintStats();
    void DrawScreenUnavailableOnline();
    CVector2D GetMapBaseDefault();
    void DrawLegend();
    void DrawZone();
    void DrawMap();
    void DrawMapCrosshair(float x, float y);
    void DrawPatternBackground(CRect rect, CRGBA col);
    void DrawBorder(CRect rect, CRGBA col);
    void DrawGallery();
    bool CheckHover(float x1, float x2, float y1, float y2);
    bool MapCrosshairCheckHover(float x1, float x2, float y1, float y2);

public:
    static void PassSettingsToCurrentGame(const CMenuSettings* s);
    void FindOutUsedMemory();
    void ChangeVideoMode(int mode, int msaa);
    void ChangeVideoModeWindowed(int mode, int msaa);
    void ProcessFullscreenToggle();
    void ApplyGraphicsChanges();
    void ApplyChanges();
    void RestorePreviousSettings();
    void RestoreDefaults(CMenuSettings* ts, int index);
};

extern CMenuNew MenuNew;
extern char* gGxtString;
extern char* gGxtString2;
