#pragma once
#include "CSprite2d.h"

enum eCellPhoneSprites {
    CELLPHONE_BORDER,
    CELLPHONE_LOGO,
    CELLPHONE_MASK,
    CELLPHONE_DISPLAY,
    CELLPHONE_HEADER_BAR,
    CELLPHONE_SOFT_KEY_BG_GRAD,
    CELLPHONE_TOUCH_BACK,
    CELLPHONE_TOUCH_HOME,
    CELLPHONE_TOUCH_SEARCH,
    NUM_CELLPHONE_SPRITES,
};

enum eCellPhoneIconsSprites {
    CELLPHONE_ICON_BACK,
    CELLPHONE_ICON_BROWSER,
    CELLPHONE_ICON_CONTACTS,
    CELLPHONE_ICON_MAIL,
    CELLPHONE_ICON_NOTIFICATIONS,
    CELLPHONE_ICON_QUICKSAVE,
    CELLPHONE_ICON_SETTINGS,
    CELLPHONE_ICON_SNAPMATIC,
    CELLPHONE_ICON_TEXTS,
    CELLPHONE_ICON_SELECTED,
    NUM_ICONS_SPRITES,
};

enum eCellPhoneBackgroundSprites {
    CELLPHONE_WALLPAPER_BADGERDEFAULT,
    CELLPHONE_WALLPAPER_IFRUITDEFAULT,
    CELLPHONE_WALLPAPER_ORANGETRIANGLES,
    CELLPHONE_WALLPAPER_PURPLEGLOW,
    CELLPHONE_WALLPAPER_PURPLETARTAN,
    NUM_WALLPAPER_SPRITES,
};

enum eCellPhoneSharedSprites {
    CELLPHONE_SHARED_BACK,
    CELLPHONE_SHARED_BATTERY,
    CELLPHONE_SHARED_PHONE,
    CELLPHONE_SHARED_PLUS,
    CELLPHONE_SHARED_SIGNAL,
    CELLPHONE_SHARED_SIGN,
    NUM_SHARED_SPRITES,
};

enum {
    NUM_APPS_COLUMNS = 3,
    NUM_APPS_ROWS = 3,
    MAX_APPS_ON_SCREEN = (NUM_APPS_COLUMNS * NUM_APPS_ROWS),
};

enum eApplications {
    CELLPHONE_APP_MAIL,
    CELLPHONE_APP_BROWSER,
    CELLPHONE_APP_CONTACTS,
    CELLPHONE_APP_SETTINGS,
    CELLPHONE_APP_TEXTS,
    CELLPHONE_APP_NOTIFICATIONS,
    CELLPHONE_APP_QUICKSAVE,
    CELLPHONE_APP_CAMERA,
};

class CApps {
public:
    int type;
    char name[16];
    int c;
    int r;

public:
    void add(int _type, char* _name, int _c, int _r) {
        type = _type;
        strcpy(name, _name);
        c = _c;
        r = _r;
    }
};

class CCellPhone {
public:
    CSprite2d* CellPhoneSprites[NUM_CELLPHONE_SPRITES];
    CSprite2d* IconsSprites[NUM_ICONS_SPRITES];
    CSprite2d* WallpaperSprites[NUM_WALLPAPER_SPRITES];
    CSprite2d* SharedSprites[NUM_SHARED_SPRITES];
    CApps Apps[MAX_APPS_ON_SCREEN];

private:
    bool bInitialised;
    bool bResetAnimation;
    bool bShowOrHideAnimation;

public:
    bool bActive;
    bool bRequestPhoneClose;
    int nCurrentItem;
    int nPreviousItem;
    int nTimeLastTimePhoneShown;

public:
    void Init();
    void Shutdown();
    void AddNewApp(int type, char* name);
    void BuildAppScreen();
    int GetLastAppOnScreen();
    void ShowHidePhone(bool on, bool force = false);
    void Process();
    void ProcessPhoneApp();
    void DrawPhone(float x, float y);
    void DrawSoftKey(float x, float y, int type, int slot, CRGBA col);
    void DrawTouchButton(float x, float y, int type, int slot, CRGBA col);
    void Draw();
};

extern CCellPhone CellPhone;