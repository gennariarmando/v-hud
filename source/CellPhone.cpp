#include "VHud.h"
#include "CellPhone.h"
#include "HudNew.h"
#include "Utility.h"
#include "TextureMgr.h"
#include "MenuNew.h"
#include "HudColoursNew.h"
#include "FontNew.h"
#include "TextNew.h"
#include "WeaponSelector.h"

#include "CTimer.h"
#include "CClock.h"
#include "CWorld.h"

CCellPhone CellPhone;

const char* CellPhoneFileNames[] = {
    "cellphone_border",
    "cellphone_logo",
    "cellphone_mask",
    "cellphone_display",
    "cellphone_header_bar",
    "cellphone_soft_key_bg",
    "cellphone_touch_back",
    "cellphone_touch_home",
    "cellphone_touch_search",
};

const char* CellPhoneIconsFileNames[] = {
    "cellphone_icon_back",
    "cellphone_icon_browser",
    "cellphone_icon_contacts",
    "cellphone_icon_mail",
    "cellphone_icon_notifications",
    "cellphone_icon_quicksave",
    "cellphone_icon_settings",
    "cellphone_icon_snapmatic",
    "cellphone_icon_texts",
    "cellphone_icon_selected",
};

const char* CellPhoneWallpaperFileNames[] = {
    "cellphone_wallpaper_badgerdefault",
    "cellphone_wallpaper_ifruitdefault",
    "cellphone_wallpaper_orangetriangles",
    "cellphone_wallpaper_purpleglow",
    "cellphone_wallpaper_purpletartan",
};

const char* CellPhoneSharedFileNames[] = {
    "cellphone_shared_back",
    "cellphone_shared_battery",
    "cellphone_shared_phone",
    "cellphone_shared_plus",
    "cellphone_shared_signal",
    "cellphone_shared_sign",
};

CCellPhone::CCellPhone() {

}

void CCellPhone::Init() {
    if (bInitialised)
        return;

    for (int i = 0; i < NUM_CELLPHONE_SPRITES; i++) {
        CellPhoneSprites[i] = new CSprite2d();
        CellPhoneSprites[i]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\cellphone\\ifruit"), CellPhoneFileNames[i]);
    }

    for (int i = 0; i < NUM_ICONS_SPRITES; i++) {
        IconsSprites[i] = new CSprite2d();
        IconsSprites[i]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\cellphone\\ifruit\\icons"), CellPhoneIconsFileNames[i]);
    }

    for (int i = 0; i < NUM_WALLPAPER_SPRITES; i++) {
        WallpaperSprites[i] = new CSprite2d();
        WallpaperSprites[i]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\cellphone\\ifruit\\wallpaper"), CellPhoneWallpaperFileNames[i]);
    }

    for (int i = 0; i < NUM_SHARED_SPRITES; i++) {
        SharedSprites[i] = new CSprite2d();
        SharedSprites[i]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\cellphone\\shared"), CellPhoneSharedFileNames[i]);
    }

    bActive = false;
    bShowOrHideAnimation = false;
    nCurrentItem = 0;
    nPreviousItem = 0;
    nTimeLastTimePhoneShown = false;

    BuildAppScreen();

    bInitialised = true;
}

void CCellPhone::Shutdown() {
    if (!bInitialised)
        return;

    for (int i = 0; i < NUM_CELLPHONE_SPRITES; i++) {
        if (CellPhoneSprites[i]) {
            CellPhoneSprites[i]->Delete();
            delete CellPhoneSprites[i];
        }
    }

    for (int i = 0; i < NUM_ICONS_SPRITES; i++) {
        if (IconsSprites[i]) {
            IconsSprites[i]->Delete();
            delete IconsSprites[i];
        }
    }

    for (int i = 0; i < NUM_WALLPAPER_SPRITES; i++) {
        if (WallpaperSprites[i]) {
            WallpaperSprites[i]->Delete();
            delete WallpaperSprites[i];
        }
    }

    for (int i = 0; i < NUM_SHARED_SPRITES; i++) {
        if (SharedSprites[i]) {
            SharedSprites[i]->Delete();
            delete SharedSprites[i];
        }
    }

    bInitialised = false;
}

void CCellPhone::AddNewApp(int type, char* name) {
    int c = 0;
    int r = 0;
    for (int i = 0; i < MAX_APPS_ON_SCREEN; i++) {
        if (c > NUM_APPS_COLUMNS - 1) {
            c = 0;
            r++;
        }

        if (Apps[i].name[0] == '\0') {
            Apps[i].add(type, name, c, r);
            break;
        }

        c++;
    }
}

void CCellPhone::BuildAppScreen() {
    AddNewApp(CELLPHONE_APP_MAIL, "APP_MAIL");
    AddNewApp(CELLPHONE_APP_TEXTS, "APP_TEXT");
    AddNewApp(CELLPHONE_APP_NOTIFICATIONS, "APP_NOT");
    AddNewApp(CELLPHONE_APP_QUICKSAVE, "APP_SAV");
    AddNewApp(CELLPHONE_APP_CONTACTS, "APP_CON");
    AddNewApp(CELLPHONE_APP_SETTINGS, "APP_SET");
    AddNewApp(CELLPHONE_APP_CAMERA, "APP_CAM");
    AddNewApp(CELLPHONE_APP_BROWSER, "APP_BROW");
}

int CCellPhone::GetLastAppOnScreen() {
    int result = 0;
    for (int i = 0; i < MAX_APPS_ON_SCREEN; i++) {
        if (Apps[i].name[0] != '\0')
            result++;
    }

    return result;
}

void CCellPhone::ShowHidePhone(bool on, bool force) {
    CPlayerPed* playa = CWorld::Players[0].m_pPed;

    if (on) {
        if (bActive)
            return;

        bShowOrHideAnimation = true;
        bActive = true;
        nCurrentItem = 4;
        nPreviousItem = 4;
        bResetAnimation = true;

        playa->m_nSavedWeapon = playa->m_aWeapons[playa->m_nActiveWeaponSlot].m_nType;
        playa->RemoveWeaponAnims(playa->m_nActiveWeaponSlot, -1000.0f);
        playa->MakeChangesForNewWeapon(WEAPON_UNARMED);
    }
    else {
        if (force) {
            bActive = false;
            bResetAnimation = true;
        }

        bShowOrHideAnimation = false;

        playa->RemoveWeaponAnims(playa->m_nActiveWeaponSlot, -1000.0f);
        playa->MakeChangesForNewWeapon(playa->m_nSavedWeapon);
    }

    nTimeLastTimePhoneShown = CTimer::m_snTimeInMillisecondsPauseMode + 500;
}

void CCellPhone::Process() {
    CPadNew* pad = CPadNew::GetPad(0);
    
    if (bRequestPhoneClose) {
        bRequestPhoneClose = false;
        if (bActive)
            ShowHidePhone(false);
        return;
    }

    if (nTimeLastTimePhoneShown < CTimer::m_snTimeInMillisecondsPauseMode) {
        if (bActive && pad->GetPhoneHideJustDown()) {
            ShowHidePhone(false);
        }
        else if (pad->GetPhoneShowJustDown()) {
            ShowHidePhone(true);
        }

        nTimeLastTimePhoneShown = 0;
    }

    if (!bActive)
        return;

    bool Up = pad->GetPhoneUpJustDown();
    bool Down = pad->GetPhoneDownJustDown();
    bool Enter = pad->GetPhoneEnterJustDown();

    if (Up) {
        while (true) {
            nPreviousItem = nCurrentItem;
            nCurrentItem--;

            if (nCurrentItem < 0) {
                nCurrentItem = GetLastAppOnScreen();
            }

            if (Apps[nCurrentItem].name[0] != '\0')
                break;
        }
    }
    else if (Down) {
        while (true) {
            nPreviousItem = nCurrentItem;
            nCurrentItem++;

            if (nCurrentItem > GetLastAppOnScreen()) {
                nCurrentItem = 0;
            }

            if (Apps[nCurrentItem].name[0] != '\0')
                break;
        }
    }
    else if (Enter) {
        ProcessPhoneApp();
    }
}

void CCellPhone::ProcessPhoneApp() {
    CPed* playa = FindPlayerPed(-1);

    switch (Apps[nCurrentItem].type) {
    case CELLPHONE_APP_QUICKSAVE:
        if (playa->m_nPedFlags.bInVehicle) {
            CHud::SetHelpMessage(TextNew.GetText("QSAV_ERR").text, true, false, true);
        }
        else {
            MenuNew.SetSavePageBehaviour(false);
        }
        ShowHidePhone(false, true);
        break;
    }
}

void CCellPhone::DrawPhone(float x, float y) {
    CVector2D pos;
    CVector2D scale;
    CRGBA logoCol = { 91, 94, 110, 255 };

    pos.x = x;
    pos.y = y;
    scale.x = SCREEN_COORD(282.0f);
    scale.y = SCREEN_COORD(484.0f);
    CellPhoneSprites[CELLPHONE_MASK]->Draw(CRect(pos.x, pos.y, pos.x + scale.x, pos.y + scale.y), CRGBA(0, 0, 0, 255));

    pos.x = x;
    pos.y = y + SCREEN_COORD(2.0f);
    scale.x = SCREEN_COORD(282.0f);
    scale.y = SCREEN_COORD(482.0f);
    CellPhoneSprites[CELLPHONE_BORDER]->Draw(CRect(pos.x, pos.y, pos.x + scale.x, pos.y + scale.y), CRGBA(41, 65, 114, 255));

    pos.x += SCREEN_COORD(112.0f);
    pos.y += SCREEN_COORD(16.0f);
    scale.x = SCREEN_COORD(54.0f);
    scale.y = SCREEN_COORD(30.0f);
    CellPhoneSprites[CELLPHONE_LOGO]->Draw(CRect(pos.x, pos.y, pos.x + scale.x, pos.y + scale.y), logoCol);

    float dx = x + SCREEN_COORD(23.0f);
    float dy = y + SCREEN_COORD(58.0f);
    float dw = SCREEN_COORD(236.0f);
    float dh = SCREEN_COORD(340.0f);
    WallpaperSprites[CELLPHONE_WALLPAPER_IFRUITDEFAULT]->Draw(CRect(dx, dy, dx + dw, dy + dh), CRGBA(255, 255, 255, 255));

    // Status bar
    float bw = dw;
    float bh = SCREEN_COORD(22.0f);
    float sx = dx;
    float sy = dy + SCREEN_COORD(4.0f);
    CSprite2d::DrawRect(CRect(dx, dy, dx + bw, dy + bh), CRGBA(5, 5, 5, 255));

    sx += SCREEN_COORD(3.0f);
    SharedSprites[CELLPHONE_SHARED_SIGNAL]->Draw(CRect(sx, sy, sx + SCREEN_COORD(16.0f), sy + SCREEN_COORD(15.0f)), CRGBA(255, 255, 255, 255));
    sx += SCREEN_COORD(20.0f);
    SharedSprites[CELLPHONE_SHARED_SIGN]->Draw(CRect(sx, sy, sx + SCREEN_COORD(15.0f), sy + SCREEN_COORD(15.0f)), CRGBA(255, 255, 255, 255));
    sx += SCREEN_COORD(20.0f);

    CFontNew::SetBackground(false);
    CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
    CFontNew::SetWrapX(SCREEN_WIDTH);
    CFontNew::SetClipX(3);
    CFontNew::SetFontStyle(CFontNew::FONT_1);
    CFontNew::SetAlignment(CFontNew::ALIGN_LEFT);
    CFontNew::SetColor(HudColourNew.GetRGB(HUD_COLOUR_WHITE, 255));
    CFontNew::SetDropColor(CRGBA(0, 0, 0, 255));
    CFontNew::SetDropShadow(0.0f);
    CFontNew::SetOutline(0.0f);
    CFontNew::SetScale(SCREEN_MULTIPLIER(0.44f), SCREEN_MULTIPLIER(1.0f));

    char daytmp[32];
    char* day;
    sprintf(daytmp, "DAY_%d", CClock::CurrentDay);
    day = TextNew.GetText(daytmp).text;
    CFontNew::PrintString(sx, dy, day);
    CFontNew::SetClipX(SCREEN_WIDTH);
    sx = dx + (dw / 2);

    char time[16];
    sprintf(time, "%02d:%02d", CClock::ms_nGameClockHours, CClock::ms_nGameClockMinutes);
    CFontNew::SetAlignment(CFontNew::ALIGN_CENTER);
    CFontNew::PrintString(sx, dy, time);
    sx = dx + (dw - SCREEN_COORD(30.0f));
    sy += SCREEN_COORD(2.0f);

    SharedSprites[CELLPHONE_SHARED_BATTERY]->Draw(CRect(sx, sy, sx + SCREEN_COORD(26.0f), sy + SCREEN_COORD(12.0f)), CRGBA(255, 255, 255, 255));
    //

    // Top line
    CRGBA col = CRGBA(111, 141, 194, 255);
    float tlx = dx;
    float tly = dy + bh;
    float tlw = dw;
    float tlh = SCREEN_COORD(40.0f);
    CellPhoneSprites[CELLPHONE_HEADER_BAR]->Draw(CRect(tlx, tly, tlx + tlw, tly + tlh), col);

    const float headerX = tlx + (tlw / 2);
    const float headerY = tly + SCREEN_COORD(2.0f);
    CFontNew::SetAlignment(CFontNew::ALIGN_CENTER);
    CFontNew::SetScale(SCREEN_MULTIPLIER(0.72f), SCREEN_MULTIPLIER(1.62f));
    CFontNew::PrintString(headerX, headerY, TextNew.GetText(Apps[nCurrentItem].name).text);

    const float iconSize = 24.0f;
    const float spacing = SCREEN_COORD(iconSize / 2);

    tlx += SCREEN_COORD(iconSize * 2);
    tly += tlh + SCREEN_COORD(iconSize * 2);

    for (int i = 0; i < MAX_APPS_ON_SCREEN; i++) {
        if (Apps[i].name[0] == '\0')
            continue;

        int icon = -1;
        switch (Apps[i].type) {
        case CELLPHONE_APP_MAIL:
            icon = CELLPHONE_ICON_MAIL;
            break;
        case CELLPHONE_APP_BROWSER:
            icon = CELLPHONE_ICON_BROWSER;
            break;
        case CELLPHONE_APP_CONTACTS:
            icon = CELLPHONE_ICON_CONTACTS;
            break;
        case CELLPHONE_APP_SETTINGS:
            icon = CELLPHONE_ICON_SETTINGS;
            break;
        case CELLPHONE_APP_TEXTS:
            icon = CELLPHONE_ICON_TEXTS;
            break;
        case CELLPHONE_APP_NOTIFICATIONS:
            icon = CELLPHONE_ICON_NOTIFICATIONS;
            break;
        case CELLPHONE_APP_QUICKSAVE:
            icon = CELLPHONE_ICON_QUICKSAVE;
            break;
        case CELLPHONE_APP_CAMERA:
            icon = CELLPHONE_ICON_SNAPMATIC;
            break;
        }

        if (icon != -1) {
            float is = iconSize * 1.2f;
            float ix = tlx + ((SCREEN_COORD(is) * 2) + spacing) * (Apps[i].c);
            float iy = tly + ((SCREEN_COORD(is) * 2) + spacing) * (Apps[i].r);

            if (nCurrentItem == i) {
                is = iconSize * 1.55f;
                IconsSprites[CELLPHONE_ICON_SELECTED]->Draw(CRect(ix - SCREEN_COORD(is), iy - SCREEN_COORD(is), ix + SCREEN_COORD(is), iy + SCREEN_COORD(is)), col);
                is = iconSize * 1.4f;
            }

            IconsSprites[icon]->Draw(CRect(ix - SCREEN_COORD(is), iy - SCREEN_COORD(is), ix + SCREEN_COORD(is), iy + SCREEN_COORD(is)), CRGBA(255, 255, 255, 255));
        }
    }

    // Bottom line
    float blx = dx;
    float bly = dy + dh - SCREEN_COORD(40.0f);
    float blw = dw;
    float blh = SCREEN_COORD(40.0f);
    CellPhoneSprites[CELLPHONE_SOFT_KEY_BG_GRAD]->Draw(CRect(blx, bly, blx + blw, bly + blh), CRGBA(255, 255, 255, 255));

    CRect line;
    line.left = blx + (blw / 2) + SCREEN_COORD(38.0f);
    line.top = bly;
    line.right = line.left + SCREEN_COORD(1.0f);
    line.bottom = line.top + blh;
    CSprite2d::DrawRect(CRect(line), CRGBA(100, 100, 100, 100));

    line.left = blx + (blw / 2) - SCREEN_COORD(38.0f);
    line.right = line.left + SCREEN_COORD(1.0f);
    CSprite2d::DrawRect(CRect(line), CRGBA(100, 100, 100, 100));

    float softKeyCenterX = blx + (blw / 2);
    float softKeyCenterY = bly + (blh / 2);
    DrawSoftKey(softKeyCenterX, softKeyCenterY, CELLPHONE_SHARED_PLUS, 1, HudColourNew.GetRGB(HUD_COLOUR_GREEN, 255));
    DrawSoftKey(softKeyCenterX, softKeyCenterY, CELLPHONE_SHARED_BACK, 2, HudColourNew.GetRGB(HUD_COLOUR_RED, 255));

    softKeyCenterY += SCREEN_COORD(42.0f);
    DrawTouchButton(softKeyCenterX, softKeyCenterY, CELLPHONE_TOUCH_SEARCH, 0, logoCol);
    DrawTouchButton(softKeyCenterX, softKeyCenterY, CELLPHONE_TOUCH_HOME, 1, logoCol);
    DrawTouchButton(softKeyCenterX, softKeyCenterY, CELLPHONE_TOUCH_BACK, 2, logoCol);
}

void CCellPhone::DrawSoftKey(float x, float y, int type, int slot, CRGBA col) {
    CRect softKey;
    const float w = SCREEN_COORD(14.0f);
    const float h = SCREEN_COORD(16.0f);

    switch (slot) {
    case 0:
        x += SCREEN_COORD(-80.0f);
        break;
    case 1:
        break;
    case 2:
        x += SCREEN_COORD(80.0f);
        break;
    }

    softKey.left = x - w;
    softKey.top = y - h;
    softKey.right = x + w;
    softKey.bottom = y + h;

    SharedSprites[type]->Draw(softKey, col);
}

void CCellPhone::DrawTouchButton(float x, float y, int type, int slot, CRGBA col) {
    CRect softKey;
    const float softKeyScale = SCREEN_COORD(10.0f);

    switch (slot) {
    case 0:
        x += SCREEN_COORD(-80.0f);
        break;
    case 1:
        break;
    case 2:
        x += SCREEN_COORD(80.0f);
        break;
    }

    softKey.left = x - softKeyScale;
    softKey.top = y - softKeyScale;
    softKey.right = x + softKeyScale;
    softKey.bottom = y + softKeyScale;

    CellPhoneSprites[type]->Draw(softKey, col);
}

void CCellPhone::Draw() {
    if (!bActive)
        return;

    static float offset = 0.0f;
    const float up = 462.0f;
    const float down = 0.0f;

    if (bResetAnimation) {
        offset = down;
        bResetAnimation = false;
    }

    if (bShowOrHideAnimation) {
        offset = interpF(offset, up, 0.4f * CTimer::ms_fTimeStep);
    }
    else {
        if (isNearlyEqualF(offset, down, 0.01f)) {
            bActive = false;
        }
        else {
            offset = interpF(offset, down, 0.4f * CTimer::ms_fTimeStep);
        }
    }

    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
    DrawPhone(HUD_RIGHT(382.0f), HUD_BOTTOM(offset));
}

