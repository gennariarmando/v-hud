#include "VHud.h"
#include "CFont.h"
#include "CCutsceneMgr.h"
#include "CMenuManager.h"
#include "CReplay.h"
#include "CCamera.h"
#include "CTheScripts.h"
#include "CGameLogic.h"
#include "CText.h"
#include "CRadar.h"
#include "CTimer.h"
#include "CMenuSystem.h"
#include "CEntryExitManager.h"
#include "CWorld.h"
#include "eVehicleClass.h"
#include "CDarkel.h"
#include "CMessages.h"
#include "CUserDisplay.h"
#include "CModelInfo.h"
#include "CGarages.h"
#include "CStats.h"
#include "CKeyGen.h"
#include "CTheZones.h"

#include "PedNew.h"
#include "TextNew.h"
#include "FontNew.h"
#include "PadNew.h"
#include "HudColoursNew.h"
#include "HudNew.h"
#include "WeaponSelector.h"
#include "RadioHud.h"
#include "OverlayLayer.h"
#include "RadarNew.h"
#include "Utility.h"
#include "GPS.h"
#include "MenuNew.h"
#include "MenuPanels.h"
#include "TextureMgr.h"
#include "resource.h"

#include <d3d9types.h>
#include <d3d9caps.h>

using namespace plugin;

CHudNew HudNew;

CHudSetting CHudNew::m_HudList[256];

bool CHudNew::bInitialised = false;
bool CHudNew::bShowMoney;
bool CHudNew::bShowMoneyDifference;
int CHudNew::nMoneyFadeAlpha;
int CHudNew::nMoneyDifferenceFadeAlpha;
int CHudNew::nTimeToShowMoney;
int CHudNew::nTimeToShowMoneyDifference;

bool CHudNew::bShowAmmo;
int CHudNew::nAmmoFadeAlpha;
int CHudNew::nTimeToShowAmmoDifference;

int CHudNew::m_nPreviousMoney;
int CHudNew::m_nDiffMoney;
int CHudNew::nTargettedEntityDeathTime;

char CHudNew::m_LastMissionName[128];
bool CHudNew::m_bShowWastedBusted;
bool CHudNew::m_bShowSuccessFailed;

char CHudNew::m_CurrentLevelName[128];
int CHudNew::m_nLevelNameState;

CSprite2d* CHudNew::WantedSprites[NUM_WANTED_SPRITES];
CSprite2d* CHudNew::CrosshairsSprites[NUM_CROSSHAIRS_SPRITES];
CSprite2d* CHudNew::StatsSprites[NUM_PLRSTATS_SPRITES];
CSprite2d* CHudNew::PlayerPortrait[4][2];
int CHudNew::previousModelIndex[4];

void* simple_mask_fxc;

CHudNew::CHudNew() {
    patch::PutRetn(0x58FAE0); // CHud::Draw
    patch::PutRetn(0x58D490); // CHud::DrawAfterFade
    patch::Set(0x53E4FA, 5); // CAudioEngine::DisplayRadioStationName
    patch::Nop(0x53E202, 17); // CPlayerPed::DrawTriangleForMouseRecruitPed

    // No sniper/rocket borders.
    patch::Nop(0x53E4ED, 5);
    patch::Nop(0x53E49E, 5);
    patch::Nop(0x53E448, 5);
    patch::Nop(0x53E3F9, 5);
    patch::Nop(0x53E398, 5);
}

void CHudNew::Init() {
    if (bInitialised)
        return;

    WantedSprites[WANTED_STAR_1] = new CSprite2d();
    WantedSprites[WANTED_STAR_1]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\wanted"), "star1");
    WantedSprites[WANTED_STAR_2] = new CSprite2d();
    WantedSprites[WANTED_STAR_2]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\wanted"), "star2");

    CrosshairsSprites[CROSSHAIR_DOT] = new CSprite2d();
    CrosshairsSprites[CROSSHAIR_DOT]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\crosshair"), "dot");
    CrosshairsSprites[CROSSHAIR_CROSS] = new CSprite2d();
    CrosshairsSprites[CROSSHAIR_CROSS]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\crosshair"), "cross");
    CrosshairsSprites[CROSSHAIR_ROCKET] = new CSprite2d();
    CrosshairsSprites[CROSSHAIR_ROCKET]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\crosshair"), "rocket");
    CrosshairsSprites[CROSSHAIR_SNIPER] = new CSprite2d();
    CrosshairsSprites[CROSSHAIR_SNIPER]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\crosshair"), "sniper");
    CrosshairsSprites[CROSSHAIR_SNIPERTARGET] = new CSprite2d();
    CrosshairsSprites[CROSSHAIR_SNIPERTARGET]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\crosshair"), "snipertarget");
    CrosshairsSprites[CROSSHAIR_ASSAULT] = new CSprite2d();
    CrosshairsSprites[CROSSHAIR_ASSAULT]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\crosshair\\crosshairs"), "assault");

    CrosshairsSprites[CROSSHAIR_PISTOL] = new CSprite2d();
    CrosshairsSprites[CROSSHAIR_PISTOL]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\crosshair\\crosshairs"), "pistol");
    CrosshairsSprites[CROSSHAIR_SHOTGUN] = new CSprite2d();
    CrosshairsSprites[CROSSHAIR_SHOTGUN]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\crosshair\\crosshairs"), "shotgun");
    CrosshairsSprites[CROSSHAIR_SMG] = new CSprite2d();
    CrosshairsSprites[CROSSHAIR_SMG]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\crosshair\\crosshairs"), "smg");

    StatsSprites[PLRSTAT_PLAYER1_ACTIVE] = new CSprite2d();
    StatsSprites[PLRSTAT_PLAYER1_ACTIVE]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\stats\\wheel_part"), "player1_active");
    StatsSprites[PLRSTAT_PLAYER2_ACTIVE] = new CSprite2d();
    StatsSprites[PLRSTAT_PLAYER2_ACTIVE]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\stats\\wheel_part"), "player2_active");
    StatsSprites[PLRSTAT_PLAYER3_ACTIVE] = new CSprite2d();
    StatsSprites[PLRSTAT_PLAYER3_ACTIVE]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\stats\\wheel_part"), "player3_active");
    StatsSprites[PLRSTAT_PLAYER4_ACTIVE] = new CSprite2d();
    StatsSprites[PLRSTAT_PLAYER4_ACTIVE]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\stats\\wheel_part"), "player4_active");

    StatsSprites[PLRSTAT_PROGRESS_BAR] = new CSprite2d();
    StatsSprites[PLRSTAT_PROGRESS_BAR]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\stats"), "progress_bar");
    
    StatsSprites[PLRSTAT_WHEEL] = new CSprite2d();
    StatsSprites[PLRSTAT_WHEEL]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\stats"), "wheel");

    PlayerPortrait[0][0] = new CSprite2d();
    PlayerPortrait[0][1] = new CSprite2d();

    PlayerPortrait[1][0] = new CSprite2d();
    PlayerPortrait[1][1] = new CSprite2d();

    PlayerPortrait[2][0] = new CSprite2d();
    PlayerPortrait[2][1] = new CSprite2d();

    PlayerPortrait[3][0] = new CSprite2d();
    PlayerPortrait[3][1] = new CSprite2d();

    simple_mask_fxc = CreatePixelShaderFromResource(IDR_SIMPLE_MASK);

    ReadSettingsFromFile();
    ReInit();
    bInitialised = true;
}

void CHudNew::ReInit() {
    bShowMoney = false;
    bShowMoneyDifference = false;
    nMoneyFadeAlpha = 0;
    nMoneyDifferenceFadeAlpha = 0;
    nTimeToShowMoneyDifference = 0;

    bShowAmmo = false;
    nAmmoFadeAlpha = 0;
    nTimeToShowAmmoDifference = 0;

    if (CWorld::Players[CWorld::PlayerInFocus].m_pPed)
        m_nPreviousMoney = CWorld::Players[CWorld::PlayerInFocus].m_nMoney;
    else
        m_nPreviousMoney = 0;

    m_nDiffMoney = 0;
    nTargettedEntityDeathTime = 0;

    m_bShowWastedBusted = false;
    m_bShowSuccessFailed = false;

    for (int i = 0; i < 4; i++) {
        previousModelIndex[i] = MODEL_NULL;
    }
}

void CHudNew::Shutdown() {
    if (!bInitialised)
        return;

    for (int i = 0; i < NUM_WANTED_SPRITES; i++) {
        WantedSprites[i]->Delete();
        delete WantedSprites[i];
    }

    for (int i = 0; i < NUM_CROSSHAIRS_SPRITES; i++) {
        CrosshairsSprites[i]->Delete();
        delete CrosshairsSprites[i];
    }

    for (int i = 0; i < NUM_PLRSTATS_SPRITES; i++) {
        StatsSprites[i]->Delete();
        delete StatsSprites[i];
    }

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 2; j++) {
            if (PlayerPortrait[i][j]) {
                PlayerPortrait[i][j]->Delete();
                delete PlayerPortrait[i][j];
            }
        }
    }
    bInitialised = false;
}

void CHudNew::ReadSettingsFromFile() {
    std::ifstream file(PLUGIN_PATH("VHud\\data\\hud.dat"));

    if (file.is_open()) {
        int id = 0;
        for (std::string line; getline(file, line);) {
            char name[64];
            char link[64];
            float x, y, w, h;
            int a;

            if (!line[0] || line[0] == '\t' || line[0] == ' ' || line[0] == '#' || line[0] == '[')
                continue;

            m_HudList[id] = CHudSetting();

            if (line[0] == ';') {
                id++;
                continue;
            }

            sscanf(line.c_str(), "%s %f %f %f %f %s %d", &name, &x, &y, &w, &h, &link, &a);

            CRGBA col = HudColourNew.GetRGB(link, static_cast<unsigned char>(a));

            strcpy(m_HudList[id].name, name);
            m_HudList[id].x = x;
            m_HudList[id].y = y;
            m_HudList[id].w = w;
            m_HudList[id].h = h;
            m_HudList[id].col = col;
            printf("%s at id: %d \n", name, id);
            id++;
        }
        file.close();
    }
}

CHudSetting CHudNew::GetSetting(int setting) {
    return m_HudList[setting];
}

CHudSetting CHudNew::GetSetting(char* name) {
    CHudSetting result = CHudSetting();

    for (int i = 0; i < 256; i++) {
        if (name[4] == m_HudList[i].name[4] && faststrcmp(name, m_HudList[i].name, 5) == 0)
            result = GetSetting(i);
    }

    return result;
}

bool HelpTripSkipShown;
bool ShowTripSkipMessage;

void CHudNew::Draw() {
    if (CTimer::m_UserPause || CTimer::m_CodePause)
        return;

    if (CReplay::Mode != 1 && !CWeapon::ms_bTakePhoto) {
        RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);
        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
        RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*)rwTEXTUREADDRESSCLAMP);
        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)FALSE);
        RwRenderStateSet(rwRENDERSTATESHADEMODE, (void*)rwSHADEMODEFLAT);
        RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)(rwFILTERLINEARMIPLINEAR));
        RwRenderStateSet(rwRENDERSTATETEXTUREPERSPECTIVE, (void*)FALSE);

        CWeaponSelector::ProcessWeaponSelector();
        CWeaponSelector::DrawWheel();

        if (MenuNew.Settings.showRadar && !CHud::bScriptDontDisplayRadar && !TheCamera.m_bWideScreenOn) {
            DrawRadar();
        }

        if (!TheCamera.m_bWideScreenOn) {
            DrawCrosshairs();

            if (MenuNew.Settings.showHUD) {
                if (CTheScripts::bDisplayHud) {
                    DrawWanted();
                    DrawPlayerInfo();
                    CWeaponSelector::DrawWheel();
                }

                if (!CHud::bDrawingVitalStats) {
                    if (!CHud::bScriptDontDisplayVehicleName)
                        DrawVehicleName();

                    if (!CHud::bScriptDontDisplayAreaName) {
                        DrawZoneName();
                        DrawLevelName();
                    }
                }

                DrawMissionTimers();
                DrawHelpText();

                CRadioHud::Draw();
            }
            DrawStats();
        }

        ShowTripSkipMessage = false;
        if (!CGameLogic::SkipCanBeActivated()) {
            HelpTripSkipShown = false;
        }
        else {
            DrawTripSkip();

            if (!HelpTripSkipShown) {
                CHud::SetHelpMessage(TheText.Get("SKIP_1"), 1, 0, 0);
                HelpTripSkipShown = true;
            }
        }

        if (CTheScripts::bDrawSubtitlesBeforeFade)
            DrawSubtitles();

        DrawOddJobMessage(1);
        DrawSuccessFailedMessage();
        DrawWastedBustedText();

        if (CHud::m_bDraw3dMarkers && !TheCamera.m_bWideScreenOn)
            CRadar::Draw3dMarkers();

        if (CMenuSystem::num_menus_in_use)
            CMenuPanels::Process(-99);

        DrawScriptText(0);
    }
}

bool CHudNew::IsAimingWeapon() {
    CPlayerPed* playa = FindPlayerPed(0);
    eCamMode mode = TheCamera.m_aCams[TheCamera.m_nActiveCam].m_nMode;

    return mode == MODE_AIMWEAPON
        || mode == MODE_AIMWEAPON_ATTACHED
        || mode == MODE_ROCKETLAUNCHER
        || mode == MODE_ROCKETLAUNCHER_HS
        || mode == MODE_ROCKETLAUNCHER_RUNABOUT_HS
        || mode == MODE_SNIPER
        || mode == MODE_SNIPER_RUNABOUT
        || mode == MODE_CAMERA
        || mode == MODE_SYPHON;
}

void CHudNew::DrawCrosshairs() {
    float x = SCREEN_WIDTH * CCamera::m_f3rdPersonCHairMultX;
    float y = SCREEN_HEIGHT * CCamera::m_f3rdPersonCHairMultY;
    int modelId = CWeaponInfo::GetWeaponInfo(CWorld::Players[CWorld::PlayerInFocus].m_pPed->m_aWeapons[CWorld::Players[CWorld::PlayerInFocus].m_pPed->m_nActiveWeaponSlot].m_nType, 1)->m_nModelId1;
    float radius = CWorld::Players[CWorld::PlayerInFocus].m_pPed->GetWeaponRadiusOnScreen();
    CRect rect;
    CRGBA col;
    CPlayerPed* playa = FindPlayerPed(-1);
    char* crosshairName = CWeaponSelector::nCrosshairs[playa->m_aWeapons[playa->m_nActiveWeaponSlot].m_nType].name;

    if (!crosshairName)
        return;

    if (playa && playa->m_pIntelligence && playa->m_pIntelligence->GetTaskUseGun()) {
        bool ik = (playa->m_pIntelligence->GetTaskUseGun()->m_pWeaponInfo->m_nFlags.bAimWithArm && !playa->m_pIntelligence->GetTaskUseGun()->m_ArmIKInUse);

        if (ik && !playa->m_nPedFlags.bIsDucking)
            return;

        if (!playa->m_pPlayerData->m_bHaveTargetSelected) {
            if (CTheScripts::bDrawCrossHair || !TheCamera.m_bTransitionState) {
                if (1) {
                    if (!faststrcmp(crosshairName, "sniper")) {
                        COverlayLayer::SetEffect(EFFECT_LENS_DISTORTION);

                        static int shoot = playa->m_aWeapons[playa->m_nActiveWeaponSlot].m_nTotalAmmo;
                        static int time = 0;

                        rect.left = (SCREEN_WIDTH / 2) - SCREEN_COORD(960.0f);
                        rect.right = (SCREEN_WIDTH / 2) + SCREEN_COORD(960.0f);
                        rect.top = (SCREEN_HEIGHT / 2) - SCREEN_COORD(960.0f);
                        rect.bottom = (SCREEN_HEIGHT / 2) + SCREEN_COORD(960.0f);

                        CSprite2d::DrawRect(CRect(0.0f, 0.0f, rect.left, SCREEN_HEIGHT), CRGBA(0, 0, 0, 255));
                        CSprite2d::DrawRect(CRect(rect.right, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT), CRGBA(0, 0, 0, 255));

                        CrosshairsSprites[CROSSHAIR_SNIPER]->Draw(rect, CRGBA(255, 255, 255, 255));

                        col = HudColourNew.GetRGB(HUD_COLOUR_GREEN, 255);
                        rect.left = (SCREEN_WIDTH / 2) - SCREEN_COORD(96.0f);
                        rect.right = (SCREEN_WIDTH / 2) + SCREEN_COORD(96.0f);
                        rect.top = (SCREEN_HEIGHT / 2) - SCREEN_COORD(96.0f);
                        rect.bottom = (SCREEN_HEIGHT / 2) + SCREEN_COORD(96.0f);

                        if (playa->m_aWeapons[playa->m_nActiveWeaponSlot].m_nTotalAmmo != shoot) {
                            shoot = playa->m_aWeapons[playa->m_nActiveWeaponSlot].m_nTotalAmmo;
                            time = playa->m_aWeapons[playa->m_nActiveWeaponSlot].m_nTimeForNextShot;
                        }

                        if (time > CTimer::m_snTimeInMilliseconds)
                            col = HudColourNew.GetRGB(HUD_COLOUR_RED, 255);

                        CrosshairsSprites[CROSSHAIR_SNIPERTARGET]->Draw(rect, col);
                    }
                    else if (!faststrcmp(crosshairName, "rocket")) {
                        rect.left = (SCREEN_WIDTH / 2) - SCREEN_COORD(96.0f);
                        rect.right = (SCREEN_WIDTH / 2) + SCREEN_COORD(96.0f);
                        rect.top = (SCREEN_HEIGHT / 2) - SCREEN_COORD(96.0f);
                        rect.bottom = (SCREEN_HEIGHT / 2) + SCREEN_COORD(96.0f);
                        CrosshairsSprites[CROSSHAIR_ROCKET]->Draw(rect, CRGBA(255, 255, 255, 255));
                    }
                    else if (faststrcmp(crosshairName, "none")) {
                        int alpha = 255;
                        static int dotAlpha;
                        static int alphaTime = 0;

                        if (playa->m_pPlayerTargettedPed && playa->m_pPlayerTargettedPed->m_fHealth <= 0.0f) {
                            nTargettedEntityDeathTime = CTimer::m_snTimeInMilliseconds + 200;
                            playa->m_pPlayerTargettedPed = NULL;
                        }

                        if (playa->m_pPlayerTargettedPed)
                            alphaTime = CTimer::m_snTimeInMilliseconds + 50;

                        if (alphaTime > CTimer::m_snTimeInMilliseconds) {
                            alpha = 100;
                            playa->m_pPlayerTargettedPed = NULL;
                        }

                        dotAlpha = (int)interpF(dotAlpha, alpha, 0.2f * CTimer::ms_fTimeStep);

                        float w = GET_SETTING(HUD_CROSSHAIR_DOT).w;
                        float h = GET_SETTING(HUD_CROSSHAIR_DOT).w;
                        CRGBA col = GET_SETTING(HUD_CROSSHAIR_DOT).col;
                        col.a = dotAlpha;
                        CrosshairsSprites[CROSSHAIR_DOT]->Draw(CRect(x - SCREEN_COORD(w), y - SCREEN_COORD(h), x + SCREEN_COORD(w), y + SCREEN_COORD(h)), col);

                        if (nTargettedEntityDeathTime > CTimer::m_snTimeInMilliseconds) {
                            w = GET_SETTING(HUD_CROSSHAIR_CROSS).w;
                            h = GET_SETTING(HUD_CROSSHAIR_CROSS).w;
                            CrosshairsSprites[CROSSHAIR_CROSS]->Draw(CRect(x - SCREEN_COORD(w), y - SCREEN_COORD(h), x + SCREEN_COORD(w), y + SCREEN_COORD(h)), GET_SETTING(HUD_CROSSHAIR_CROSS).col);
                        }
                    }
                }
            }
        }
    }
}

void CHudNew::DrawPlayerInfo() {
    DrawMoneyCounter();
    DrawAmmo();
}

void CHudNew::DrawMoneyCounter() {
    char str[16];
    CPlayerInfo playa = CWorld::Players[0];

    if (bShowMoney)
        nMoneyFadeAlpha = (int)interpF(nMoneyFadeAlpha, 255, 0.2f * CTimer::ms_fTimeStep);
    else
        nMoneyFadeAlpha = (int)interpF(nMoneyFadeAlpha, 0, 0.2f * CTimer::ms_fTimeStep);

    if (bShowMoneyDifference)
        nMoneyDifferenceFadeAlpha = (int)interpF(nMoneyDifferenceFadeAlpha, 255, 0.2f * CTimer::ms_fTimeStep);
    else
        nMoneyDifferenceFadeAlpha = (int)interpF(nMoneyDifferenceFadeAlpha, 0, 0.2f * CTimer::ms_fTimeStep);

    static float heightLerp = 0.0f;
    heightLerp = interpF(heightLerp, GetShiftOffsetForMoney(), 0.4f * CTimer::ms_fTimeStep);

    CFontNew::SetBackground(false);
    CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
    CFontNew::SetAlignment(CFontNew::ALIGN_RIGHT);
    CFontNew::SetWrapX(SCREEN_COORD(640.0f));
    CFontNew::SetFontStyle(CFontNew::FONT_3);
    CFontNew::SetDropShadow(0.0f);
    CFontNew::SetOutline(SCREEN_COORD(2.0f));
    CFontNew::SetDropColor(CRGBA(0, 0, 0, 255));
    CFontNew::SetColor(GET_SETTING(HUD_CASH).col);
    CFontNew::Details.color.a = nMoneyFadeAlpha;
    CFontNew::Details.dropColor.a = nMoneyFadeAlpha;

    CFontNew::SetScale(SCREEN_MULTIPLIER(GET_SETTING(HUD_CASH).w), SCREEN_MULTIPLIER(GET_SETTING(HUD_CASH).h));

    sprintf_s(str, "$%d", playa.m_nDisplayMoney);
    CFontNew::PrintString(HUD_RIGHT(GET_SETTING(HUD_CASH).x), HUD_Y(GET_SETTING(HUD_CASH).y) + heightLerp, str);

    if (m_nPreviousMoney != playa.m_nMoney) {
        bShowMoney = true;
        bShowMoneyDifference = true;
        nTimeToShowMoney = CTimer::m_snTimeInMilliseconds + 3000;
        nTimeToShowMoneyDifference = CTimer::m_snTimeInMilliseconds + 1500;
        m_nDiffMoney = (playa.m_nMoney - m_nPreviousMoney);
    }

    if (m_nDiffMoney > 0) {
        sprintf_s(str, "+$%d", ABS(m_nDiffMoney));
        CFontNew::SetColor(HudColourNew.GetRGB("HUD_COLOUR_GREEN", 255));
    }
    else if (m_nDiffMoney < 0) {
        sprintf_s(str, "-$%d", ABS(m_nDiffMoney));
        CFontNew::SetColor(HudColourNew.GetRGB("HUD_COLOUR_RED", 255));
    }

    CFontNew::Details.color.a = nMoneyDifferenceFadeAlpha;
    CFontNew::Details.dropColor.a = nMoneyDifferenceFadeAlpha;

    CFontNew::SetScale(SCREEN_MULTIPLIER(GET_SETTING(HUD_CASH).w * 0.86f), SCREEN_MULTIPLIER(GET_SETTING(HUD_CASH).h * 0.86f));
    CFontNew::PrintString(HUD_RIGHT(GET_SETTING(HUD_CASH).x), HUD_Y(GET_SETTING(HUD_CASH).y + 2.0f) + CFontNew::GetHeightScale(SCREEN_MULTIPLIER(GET_SETTING(HUD_CASH).h)) + heightLerp, str);

    if (nTimeToShowMoneyDifference < CTimer::m_snTimeInMilliseconds || m_nDiffMoney == 0)
        bShowMoneyDifference = false;

    if (nTimeToShowMoney < CTimer::m_snTimeInMilliseconds)
        bShowMoney = false;

    m_nPreviousMoney = playa.m_nMoney;
}

void CHudNew::DrawAmmo() {
    CPed* playa = FindPlayerPed(0);
    int slot = playa->m_nActiveWeaponSlot;
    int weaponType = playa->m_aWeapons[slot].m_nType;
    int totalAmmo = playa->m_aWeapons[slot].m_nTotalAmmo;
    int ammoInClip = playa->m_aWeapons[slot].m_nAmmoInClip;
    int maxAmmoInClip = CWeaponInfo::GetWeaponInfo(playa->m_aWeapons[slot].m_nType, playa->GetWeaponSkill())->m_nAmmoClip;
    int ammo, clip;
    char str_ammo[16], str_clip[16];

    if (maxAmmoInClip <= 1 || maxAmmoInClip >= 1000) {
        sprintf(str_ammo, "%d", totalAmmo);
        strcpy(str_clip, "");
    }
    else {
        if (weaponType == WEAPON_FTHROWER) {
            unsigned int total = 9999;
            if ((totalAmmo - ammoInClip) / 10 <= 9999)
                total = (totalAmmo - ammoInClip) / 10;
            sprintf(str_ammo, "%d", total);
            sprintf(str_clip, "%d", ammoInClip / 10);
        }
        else {
            unsigned int total = totalAmmo - ammoInClip;
            if (totalAmmo - ammoInClip > 9999) {
                strcpy(str_ammo, "-");
                strcpy(str_clip, "-");
            }
            else {
                sprintf(str_ammo, "%d", total);
                sprintf(str_clip, "%d", ammoInClip);
            }
        }
    }

    if (bShowAmmo)
        nAmmoFadeAlpha = (int)interpF(nAmmoFadeAlpha, 255, 0.2f * CTimer::ms_fTimeStep);
    else
        nAmmoFadeAlpha = (int)interpF(nAmmoFadeAlpha, 0, 0.2f * CTimer::ms_fTimeStep);

    static int previousTotalAmmo;
    static int previousAmmoInClip;
    if (previousTotalAmmo != (totalAmmo - ammoInClip) || previousAmmoInClip != ammoInClip) {
        bShowAmmo = true;
        nTimeToShowAmmoDifference = CTimer::m_snTimeInMilliseconds + 8000;
    }

    CFontNew::SetBackground(false);
    CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
    CFontNew::SetWrapX(SCREEN_WIDTH);
    CFontNew::SetClipX(SCREEN_WIDTH);
    CFontNew::SetFontStyle(CFontNew::FONT_3);
    CFontNew::SetAlignment(CFontNew::ALIGN_RIGHT);
    CFontNew::SetDropShadow(0.0f);
    CFontNew::SetOutline(SCREEN_COORD(2.0f));
    CFontNew::SetScale(SCREEN_MULTIPLIER(GET_SETTING(HUD_AMMO).w), SCREEN_MULTIPLIER(GET_SETTING(HUD_AMMO).h));

    CFontNew::SetDropColor(CRGBA(0, 0, 0, nAmmoFadeAlpha));

    static float heightLerp = 0.0f;
    heightLerp = interpF(heightLerp, GetShiftOffsetForAmmo(), 0.4f * CTimer::ms_fTimeStep);

    if (CDarkel::FrenzyOnGoing()
        || weaponType == WEAPON_UNARMED
        || weaponType == WEAPON_DETONATOR
        || weaponType >= WEAPON_DILDO1 && weaponType < WEAPON_GRENADE
        || weaponType == WEAPON_PARACHUTE
        || CWeaponInfo::GetWeaponInfo((eWeaponType)weaponType, 1)->m_nWeaponFire == 5
        || CWeaponInfo::GetWeaponInfo((eWeaponType)weaponType, 1)->m_nSlot <= 1) {
        nTimeToShowAmmoDifference = 0;
        bShowAmmo = false;
    }
    else {
        CFontNew::SetColor(HudColourNew.GetRGB("HUD_COLOUR_WHITE", nAmmoFadeAlpha));
        CFontNew::PrintString(HUD_RIGHT(GET_SETTING(HUD_AMMO).x + 6.0f) - CFontNew::GetStringWidth(str_clip, false), HUD_Y(GET_SETTING(HUD_AMMO).y) + heightLerp, str_ammo);

        CFontNew::SetColor(HudColourNew.GetRGB("HUD_COLOUR_GREY", nAmmoFadeAlpha));
        CFontNew::PrintString(HUD_RIGHT(GET_SETTING(HUD_AMMO).x), HUD_Y(GET_SETTING(HUD_AMMO).y) + heightLerp, str_clip);
    }

    if (nTimeToShowAmmoDifference < CTimer::m_snTimeInMilliseconds)
        bShowAmmo = false;

    previousTotalAmmo = (totalAmmo - ammoInClip);
    previousAmmoInClip = ammoInClip;
}

void CHudNew::DrawSimpleRect(CRect const& rect, CRGBA const& col) {
    unsigned int savedShade;
    unsigned int savedAlpha;
    unsigned int savedFilter;
    RwRenderStateGet(rwRENDERSTATESHADEMODE, &savedShade);
    RwRenderStateSet(rwRENDERSTATESHADEMODE, (void*)rwSHADEMODEFLAT);
    RwRenderStateGet(rwRENDERSTATEVERTEXALPHAENABLE, &savedAlpha);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
    RwRenderStateGet(rwRENDERSTATETEXTUREFILTER, &savedFilter);
    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERNEAREST);

    CSprite2d::DrawRect(rect, col);

    RwRenderStateSet(rwRENDERSTATESHADEMODE, (void*)savedShade);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)savedAlpha);
    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)savedFilter);
}

void CHudNew::DrawSimpleRectGrad(CRect const& rect, CRGBA const& col) {
    unsigned int savedShade;
    unsigned int savedAlpha;
    unsigned int savedFilter;
    RwRenderStateGet(rwRENDERSTATESHADEMODE, &savedShade);
    RwRenderStateSet(rwRENDERSTATESHADEMODE, (void*)rwSHADEMODEFLAT);
    RwRenderStateGet(rwRENDERSTATEVERTEXALPHAENABLE, &savedAlpha);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
    RwRenderStateGet(rwRENDERSTATETEXTUREFILTER, &savedFilter);
    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERNEAREST);

    CSprite2d::SetVertices(rect.left, rect.top, rect.right, rect.top,
                          rect.left, rect.bottom, rect.right, rect.bottom, col, col, col, col);
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(MenuNew.MiscSprites[MISC_RECTGRAD]->m_pTexture));
    RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, CSprite2d::maVertices, 4);

    RwRenderStateSet(rwRENDERSTATESHADEMODE, (void*)savedShade);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)savedAlpha);
    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)savedFilter);
}

void CHudNew::DrawSimpleRectGradInverted(CRect const& rect, CRGBA const& col) {
    unsigned int savedShade;
    unsigned int savedAlpha;
    unsigned int savedFilter;
    RwRenderStateGet(rwRENDERSTATESHADEMODE, &savedShade);
    RwRenderStateSet(rwRENDERSTATESHADEMODE, (void*)rwSHADEMODEFLAT);
    RwRenderStateGet(rwRENDERSTATEVERTEXALPHAENABLE, &savedAlpha);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
    RwRenderStateGet(rwRENDERSTATETEXTUREFILTER, &savedFilter);
    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERNEAREST);

    CSprite2d::SetVertices(rect.right, rect.top, rect.left, rect.top,
        rect.right, rect.bottom, rect.left, rect.bottom, col, col, col, col);
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(MenuNew.MiscSprites[MISC_RECTGRAD]->m_pTexture));
    RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, CSprite2d::maVertices, 4);

    RwRenderStateSet(rwRENDERSTATESHADEMODE, (void*)savedShade);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)savedAlpha);
    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)savedFilter);
}

void CHudNew::DrawSimpleRectGradCentered(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, CRGBA col) {
    unsigned int savedShade;
    unsigned int savedAlpha;
    unsigned int savedFilter;
    RwRenderStateGet(rwRENDERSTATESHADEMODE, &savedShade);
    RwRenderStateSet(rwRENDERSTATESHADEMODE, (void*)rwSHADEMODEFLAT);
    RwRenderStateGet(rwRENDERSTATEVERTEXALPHAENABLE, &savedAlpha);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
    RwRenderStateGet(rwRENDERSTATETEXTUREFILTER, &savedFilter);
    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERNEAREST);

    CSprite2d::SetVertices(x1, y1, x2, y2, x3, y3, x4, y4, col, col, col, col);
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(MenuNew.MiscSprites[MISC_RECTGRADCENTERED]->m_pTexture));
    RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, CSprite2d::maVertices, 4);

    RwRenderStateSet(rwRENDERSTATESHADEMODE, (void*)savedShade);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)savedAlpha);
    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)savedFilter);
}

void CHudNew::CheckPlayerPortrait(int id) {
    CPed* playa = FindPlayerPed(id);
    char name[64];
    char col[64];
    char mask[64];

    if (id == CWorld::PlayerInFocus)
        sprintf(col, "%s%d_color", "colors\\player", id + 1);
    else
        sprintf(col, "%s", "colors\\default_color");

    sprintf(mask, "%s%d_mask", "masks\\player", id + 1);

    if (playa) {
        if (previousModelIndex[id] != playa->m_nModelIndex) {
            if (PlayerPortrait[id][0]) {
                PlayerPortrait[id][0]->Delete();
            }
            if (PlayerPortrait[id][1]) {
                PlayerPortrait[id][1]->Delete();
            }

            previousModelIndex[id] = playa->m_nModelIndex;
        }

        if (playa->m_nModelIndex < MODEL_CUTOBJ14 && faststrcmp(PedNameList[playa->m_nModelIndex], "unknown")) {
            if (PlayerPortrait[id][0] && PlayerPortrait[id][0]->m_pTexture == NULL) {
                sprintf(name, "%s%s", "portraits\\", PedNameList[playa->m_nModelIndex]);
                PlayerPortrait[id][0]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\stats"), name, mask, col);
            }
            if (PlayerPortrait[id][1] && PlayerPortrait[id][1]->m_pTexture == NULL) {
                sprintf(name, "%s%s", "portraits\\", PedNameList[playa->m_nModelIndex]);
                PlayerPortrait[id][1]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\stats"), name);
            }
        }
    }

    if (PlayerPortrait[id][0] && PlayerPortrait[id][0]->m_pTexture == NULL)
        PlayerPortrait[id][0]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\stats"), "portraits\\unknown", mask, col);

    if (PlayerPortrait[id][1] && PlayerPortrait[id][1]->m_pTexture == NULL)
        PlayerPortrait[id][1]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\stats"), "portraits\\unknown");
}

void CHudNew::DrawPlayerPortrait(int id, float x, float y, float w, float h) {
    float _x = x + 11.5f;
    float _y = y - 33.25f;
    float _w = w * 0.66f;
    float _h = h * 0.66f;

    CheckPlayerPortrait(id);

    float scale = 0.353f;
    if (PlayerPortrait[id] && PlayerPortrait[id][0]->m_pTexture) {
        switch (id) {
            case 0:
                PlayerPortrait[id][0]->Draw(HUD_RIGHT(_x + (24.0f * scale)), HUD_BOTTOM(_y), SCREEN_COORD(_w), SCREEN_COORD(_h), CRGBA(255, 255, 255, id == CWorld::PlayerInFocus ? 235 : 100));
                break;
            case 1:
                PlayerPortrait[id][0]->Draw(HUD_RIGHT(_x - (_w * scale)), HUD_BOTTOM(_y + (_h * scale)), SCREEN_COORD(_w), SCREEN_COORD(_h), CRGBA(255, 255, 255, id == CWorld::PlayerInFocus ? 235 : 100));
                break;
            case 2:
                PlayerPortrait[id][0]->Draw(HUD_RIGHT(_x - (24.0f * scale) - ((_w * scale) * 2.0f)), HUD_BOTTOM(_y), SCREEN_COORD(_w), SCREEN_COORD(_h), CRGBA(255, 255, 255, id == CWorld::PlayerInFocus ? 235 : 100));
                break;
            case 3:
                PlayerPortrait[id][0]->Draw(HUD_RIGHT(_x - (_w * scale)), HUD_BOTTOM(_y - (_h * scale)), SCREEN_COORD(_w), SCREEN_COORD(_h), CRGBA(255, 255, 255, id == CWorld::PlayerInFocus ? 235 : 100));
                break;
        }
    }

    if (id == CWorld::PlayerInFocus) {
        switch (id) {
            case 0:
                StatsSprites[PLRSTAT_PLAYER1_ACTIVE]->Draw(HUD_RIGHT(x), HUD_BOTTOM(y), SCREEN_COORD(64.0f * 0.75f), SCREEN_COORD(h), HudColourNew.GetRGB(MenuNew.Settings.uiMainColor, 255));
                break;
            case 1:
                StatsSprites[PLRSTAT_PLAYER2_ACTIVE]->Draw(HUD_RIGHT(x), HUD_BOTTOM(y), SCREEN_COORD(w), SCREEN_COORD(64.0f * 0.75f), HudColourNew.GetRGB(MenuNew.Settings.uiMainColor, 255));
                break;
            case 2:
                StatsSprites[PLRSTAT_PLAYER3_ACTIVE]->Draw(HUD_RIGHT(x - w + (64.0f * 0.75f)), HUD_BOTTOM(y), SCREEN_COORD(64.0f * 0.75f), SCREEN_COORD(h), HudColourNew.GetRGB(MenuNew.Settings.uiMainColor, 255));
                break;
            case 3:
                StatsSprites[PLRSTAT_PLAYER4_ACTIVE]->Draw(HUD_RIGHT(x), HUD_BOTTOM(y - h + (64.0f * 0.75f)), SCREEN_COORD(w), SCREEN_COORD(64.0f * 0.75f), HudColourNew.GetRGB(MenuNew.Settings.uiMainColor, 255));
                break;
        }
    }
}

void CHudNew::DrawStats() {
    CPed* playa = FindPlayerPed(-1);

    static int prevTimeScale = CTimer::ms_fTimeScale;
    static bool bJustClosed = false;
    if (CPadNew::GetPad(0)->GetShowPlayerInfo(500) && !IsAimingWeapon()) {
        CHud::bDrawingVitalStats = true;
    }
    else {
        if (CHud::bDrawingVitalStats)
            bJustClosed = true;

        CHud::bDrawingVitalStats = false;
    }

    if (CHud::bDrawingVitalStats) {
        COverlayLayer::SetEffect(EFFECT_BLUR_COLOR);
        bShowMoney = true;
        nTimeToShowMoney = CTimer::m_snTimeInMilliseconds + 50;
        prevTimeScale = CTimer::ms_fTimeScale;
        CTimer::ms_fTimeScale = 0.25;

        if (!playa)
            return;

        float x = GET_SETTING(HUD_VITAL_STATS).x;
        float y = GET_SETTING(HUD_VITAL_STATS).y;
        float w = GET_SETTING(HUD_VITAL_STATS).w;
        float h = GET_SETTING(HUD_VITAL_STATS).h;
        CRGBA col = GET_SETTING(HUD_VITAL_STATS).col;

        DrawSimpleRect(CRect(HUD_RIGHT(x), HUD_BOTTOM(y), HUD_RIGHT(x) + SCREEN_COORD(w), HUD_BOTTOM(y) + SCREEN_COORD(h)), col);

        CFontNew::SetBackground(false);
        CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
        CFontNew::SetWrapX(SCREEN_WIDTH);
        CFontNew::SetClipX(SCREEN_WIDTH);
        CFontNew::SetFontStyle(CFontNew::FONT_4);
        CFontNew::SetAlignment(CFontNew::ALIGN_LEFT);
        CFontNew::SetDropColor(CRGBA(0, 0, 0, 255));
        CFontNew::SetColor(HudColourNew.GetRGB(HUD_COLOUR_WHITE, 255));
        CFontNew::SetDropShadow(0.0f);
        CFontNew::SetOutline(0.0f);
        CFontNew::SetScale(SCREEN_MULTIPLIER(0.52f), SCREEN_MULTIPLIER(1.24f));

        char* str[] = {
            "STAT068", // Respect
            "STAT022", // Stamina
            "CURWSKL", // Weapon skill
            "STAT023", // Muscle
            "STAT021", // Fat
            "STAT025", // Sex appeal
            "STAT081", // Gambling
            "STAT225", // Lung capacity
        };

        int wepType = playa->m_aWeapons[playa->m_nActiveWeaponSlot].m_nType;
        if (wepType == WEAPON_TEC9) {
            wepType = WEAPON_MICRO_UZI;
        }

        int wep_stat_id = wepType < WEAPON_PISTOL || wepType > WEAPON_TEC9 ? -1 : wepType <= WEAPON_M4 ? wepType - WEAPON_PISTOL + STAT_PISTOL_SKILL : STAT_MACHINE_PISTOL_SKILL;

        unsigned short stat[] = {
            64,
            22,
            wep_stat_id,
            23,
            21,
            25,
            81,
            225,
        };

        float spacing = 0.0f;
        for (int i = 0; i < 8; i++) {
            unsigned int savedFilter;
            RwRenderStateGet(rwRENDERSTATETEXTUREFILTER, (void*)&savedFilter);
            RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERNEAREST);
            DrawProgressBarWithSprite(StatsSprites[PLRSTAT_PROGRESS_BAR], HUD_RIGHT((x - 12.0f)), HUD_BOTTOM(spacing + (y - 34.0f)), SCREEN_COORD(164.0f), SCREEN_COORD(8.0f), CStats::GetStatValue(stat[i]) / 1000, HudColourNew.GetRGB(MenuNew.Settings.uiMainColor, 255));
            RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)savedFilter);

            CFontNew::PrintString(HUD_RIGHT(x - 12.0f), HUD_BOTTOM(spacing + ((y - 34.0f) + 28.0f)), TheText.Get(str[i]));
            spacing -= (34.0f + 8.0f);
        }

        // Player
        x = GetSetting(HUD_PLAYER_WHEEL).x;
        y = GetSetting(HUD_PLAYER_WHEEL).y;
        w = GetSetting(HUD_PLAYER_WHEEL).w;
        h = GetSetting(HUD_PLAYER_WHEEL).h;
        col = GetSetting(HUD_PLAYER_WHEEL).col;

        StatsSprites[PLRSTAT_WHEEL]->Draw(HUD_RIGHT(x), HUD_BOTTOM(y), SCREEN_COORD(w), SCREEN_COORD(h), col);

        for (int i = 0; i < 4; i++)
            DrawPlayerPortrait(i, x, y, w, h);
    }
    else {
        if (bJustClosed) {
            CTimer::ms_fTimeScale = prevTimeScale;
            bJustClosed = false;
        }
    }
}

float CHudNew::GetShiftOffsetForMoney() {
    float y = 0.0f;

    if (FindPlayerWanted(-1)->m_nWantedLevel > 0)
        y += SCREEN_COORD(2.0f) + SCREEN_COORD(GET_SETTING(HUD_WANTED_STARS).h);

    return y;
}

float CHudNew::GetShiftOffsetForAmmo() {
    float y = 0.0f;

    if (FindPlayerWanted(-1)->m_nWantedLevel > 0)
        y += SCREEN_COORD(2.0f) + SCREEN_COORD(GET_SETTING(HUD_WANTED_STARS).h);

    if (CHudNew::IsMoneyCounterDisplaying())
        y += SCREEN_COORD(2.0f) + CFontNew::GetHeightScale(SCREEN_MULTIPLIER(GET_SETTING(HUD_CASH).h));

    if (CHudNew::IsMoneyDifferenceDisplaying())
        y += SCREEN_COORD(2.0f) + CFontNew::GetHeightScale(SCREEN_MULTIPLIER(GET_SETTING(HUD_CASH).h));

    return y;
}

void CHudNew::DrawWanted() {
    if (FindPlayerWanted(-1)->m_nWantedLevel < 1)
        return;

    float x = GET_SETTING(HUD_WANTED_STARS).x;
    float y = GET_SETTING(HUD_WANTED_STARS).y;
    float w = GET_SETTING(HUD_WANTED_STARS).w;
    float h = GET_SETTING(HUD_WANTED_STARS).h;
    CRGBA c = GET_SETTING(HUD_WANTED_STARS).col;

    float spacing = 0.0f;
    for (int i = 0; i < FindPlayerWanted(-1)->MaximumWantedLevel; i++) {
        if (FindPlayerWanted(-1)->m_nWantedLevel > i
            && (CTimer::m_snTimeInMilliseconds > FindPlayerWanted(-1)->m_nLastTimeWantedLevelChanged + 2000
                || CTimer::m_FrameCounter & 8) && (CRadarNew::m_bCopPursuit || CTimer::m_FrameCounter & 8))
            WantedSprites[WANTED_STAR_2]->Draw(HUD_RIGHT(x + w + spacing), HUD_Y(y), SCREEN_COORD(w), SCREEN_COORD(h), c);
        else
            WantedSprites[WANTED_STAR_1]->Draw(HUD_RIGHT(x + w + spacing), HUD_Y(y), SCREEN_COORD(w), SCREEN_COORD(h), CRGBA(0, 0, 0, 100));
        spacing += w - 2.0f;
    }
}

void CHudNew::DrawVehicleName() {
    if (CHud::m_pVehicleName) {
        int state = CHud::m_VehicleState;
        char* last = CHud::m_pLastVehicleName;
        float alpha = 0.0f;

        if (CHud::m_pVehicleName != CHud::m_pLastVehicleName) {
            if (CHud::m_VehicleState) {
                if (CHud::m_VehicleState > 0 && CHud::m_VehicleState <= 4) {
                    state = 4;
                    CHud::m_VehicleState = 4;
                    CHud::m_VehicleNameTimer = 0;
                }
            }
            else {
                state = 2;
                CHud::m_VehicleState = 2;
                CHud::m_VehicleNameTimer = 0;
                CHud::m_VehicleFadeTimer = 0;
                CHud::m_pVehicleNameToPrint = CHud::m_pVehicleName;
                if (CHud::m_ZoneState == 1 || CHud::m_ZoneState == 2)
                    CHud::m_ZoneState = 3;
            }
            last = CHud::m_pVehicleName;
            CHud::m_pLastVehicleName = CHud::m_pVehicleName;
        }

        if (state) {
            switch (state) {
            case 1:
                if (CHud::m_VehicleNameTimer > 6000) {
                    CHud::m_VehicleFadeTimer = 1000;
                    CHud::m_VehicleState = 3;
                }

                alpha = 255.0f;
                CHud::m_VehicleFadeTimer = 1000;
                break;
            case 2:
                CHud::m_VehicleFadeTimer += CTimer::ms_fTimeStep * 0.02f * 1000.0f;
                if (CHud::m_VehicleFadeTimer > 1000) {
                    CHud::m_VehicleState = 1;
                    CHud::m_VehicleFadeTimer = 1000;
                }
                alpha = CHud::m_VehicleFadeTimer * 0.001f * 255.0f;
                break;
            case 3:
                CHud::m_VehicleFadeTimer += CTimer::ms_fTimeStep * 0.02f * -1000.0f;
                if (CHud::m_VehicleFadeTimer < 0) {
                    CHud::m_VehicleState = 0;
                    CHud::m_VehicleFadeTimer = 0;
                }
                alpha = CHud::m_VehicleFadeTimer * 0.001f * 255.0f;
                break;
            case 4:
                CHud::m_VehicleFadeTimer += CTimer::ms_fTimeStep * 0.02f * -1000.0f;
                if (CHud::m_VehicleFadeTimer < 0) {
                    CHud::m_VehicleFadeTimer = 0;
                    CHud::m_VehicleState = 2;
                    CHud::m_pVehicleNameToPrint = last;
                }
                alpha = CHud::m_VehicleFadeTimer * 0.001f * 255.0f;
                break;
            default:
                break;

            }

            CHud::m_VehicleNameTimer += CTimer::ms_fTimeStep * 0.02f * 1000.0f;

            char* textClass[] = {
                "NORMAL",
                "POORFAM",
                "RICHFAM",
                "EXECUTI",
                "WORKER",
                "BIG",
                "SERVICE",
                "MOPED",
                "MOTORBI",
                "LEISURE",
                "WORKERB",
                "BICYCLE",
                "IGNORE",
            };

            char vehicleName[128];
            if (FindPlayerVehicle(0, true)) {
                unsigned char vehClass = ((CVehicleModelInfo*)(CModelInfo::GetModelInfo(FindPlayerVehicle(0, true)->m_nModelIndex)))->m_nVehicleClass;
                if (vehClass == -1 || vehClass > 12)
                    vehClass = 12;

                sprintf(vehicleName, "%s, %s", CHud::m_pVehicleNameToPrint, CTextNew::GetText(textClass[vehClass]).text);
            }
            else
                strcpy(vehicleName, CHud::m_pVehicleNameToPrint);

            CFontNew::SetBackground(false);
            CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
            CFontNew::SetAlignment(CFontNew::ALIGN_RIGHT);
            CFontNew::SetWrapX(SCREEN_COORD(640.0f));
            CFontNew::SetFontStyle(CFontNew::FONT_2);
            CFontNew::SetDropShadow(SCREEN_COORD(2.0f));
            CFontNew::SetDropColor(CRGBA(0, 0, 0, alpha));

            CRGBA col = GET_SETTING(HUD_VEHICLE_NAME).col;
            CFontNew::SetColor(CRGBA(col.r, col.g, col.b, alpha));
            CFontNew::SetScale(SCREEN_MULTIPLIER(GET_SETTING(HUD_VEHICLE_NAME).w), SCREEN_MULTIPLIER(GET_SETTING(HUD_VEHICLE_NAME).h));
            CFontNew::PrintString(HUD_RIGHT(GET_SETTING(HUD_VEHICLE_NAME).x), HUD_BOTTOM(GET_SETTING(HUD_VEHICLE_NAME).y), vehicleName);
        }
    }
    else {
        CHud::m_pLastVehicleName = NULL;
        CHud::m_VehicleState = 0;
        CHud::m_VehicleFadeTimer = 0;
        CHud::m_VehicleNameTimer = 0;
    }
}

void CHudNew::DrawMissionTimers() {

}

void CHudNew::DrawRadar() {
    if (CEntryExitManager::ms_exitEnterState != 1
        && CEntryExitManager::ms_exitEnterState != 2
        && (CHud::m_ItemToFlash != 8 || CTimer::m_FrameCounter & 8)) {
        RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*)rwTEXTUREADDRESSCLAMP);
        RwRenderStateSet(rwRENDERSTATETEXTUREPERSPECTIVE, (void*)TRUE);
        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
        RwRenderStateSet(rwRENDERSTATESHADEMODE, (void*)rwSHADEMODEFLAT);
        RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERLINEARMIPLINEAR);
        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
        RwRenderStateSet(rwRENDERSTATEBORDERCOLOR, (void*)RWRGBALONG(0, 0, 0, 255));
        RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);
        RwRenderStateSet(rwRENDERSTATEFOGTYPE, (void*)rwFOGTYPELINEAR);
        RwRenderStateSet(rwRENDERSTATECULLMODE, (void*)rwCULLMODECULLNONE);

        CRadarNew::DrawMap();

        CRadarNew::DrawRadarRectangle();
        CRadarNew::DrawBlips();

        CGPS::DrawDistanceFromWaypoint();

        RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);
        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);
        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
    }
}

void CHudNew::DrawTripSkip() {
    ShowTripSkipMessage = true;
    SetHelpMessage("FEC_TSK", false, false, false);
}

void CHudNew::DrawScriptText(bool priority) {
    CHud::DrawScriptText(priority);
}

void CHudNew::DrawSubtitles() {
    if (CHud::m_Message[0]) {
        CFontNew::SetBackground(false);
        CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
        CFontNew::SetAlignment(CFontNew::ALIGN_CENTER);
        CFontNew::SetWrapX(SCREEN_COORD(940.0f));
        CFontNew::SetFontStyle(CFontNew::FONT_1);
        CFontNew::SetDropShadow(SCREEN_COORD(2.0f));
        CFontNew::SetOutline(0.0f);
        CFontNew::SetDropColor(CRGBA(0, 0, 0, 255));
        CFontNew::SetColor(GET_SETTING(HUD_SUBTITLES).col);
        CFontNew::SetScale(SCREEN_MULTIPLIER(GET_SETTING(HUD_SUBTITLES).w), SCREEN_MULTIPLIER(GET_SETTING(HUD_SUBTITLES).h));

        CFontNew::PrintStringFromBottom(SCREEN_COORD_CENTER_LEFT(GET_SETTING(HUD_SUBTITLES).x), HUD_BOTTOM(GET_SETTING(HUD_SUBTITLES).y), CHud::m_Message);
    }
}

void CHudNew::DrawHelpText() {
    if (CHud::m_pHelpMessage[0]) {
        if (!CMessages::StringCompare(CHud::m_pHelpMessage, CHud::m_pLastHelpMessage, 400)) {
            switch (CHud::m_nHelpMessageState) {
            case 0:
                CHud::m_nHelpMessageState = 2;
                CHud::m_nHelpMessageTimer = 0;
                CHud::m_nHelpMessageFadeTimer = 0;
                CMessages::StringCopy(CHud::m_pHelpMessageToPrint, CHud::m_pHelpMessage, 400);

                CHud::m_fHelpMessageTime = CMessages::GetStringLength(CHud::m_pHelpMessage) / 20.0f + 3.0f;
                break;
            case 1:
            case 2:
            case 3:
            case 4:
                CHud::m_nHelpMessageState = 4;
                CHud::m_nHelpMessageTimer = 5;
                break;
            default:
                break;
            }
            CMessages::StringCopy(CHud::m_pLastHelpMessage, CHud::m_pHelpMessage, 400);
        }

        float alpha = 255.0f;

        if (CHud::m_nHelpMessageState) {
            switch (CHud::m_nHelpMessageState) {
            case 1:
                alpha = 255.0;
                CHud::m_nHelpMessageFadeTimer = 600;
                if (!CHud::m_bHelpMessagePermanent) {
                    if (CHud::m_nHelpMessageTimer > CHud::m_fHelpMessageTime * 1000.0f || CHud::m_bHelpMessageQuick && CHud::m_nHelpMessageTimer > 3000) {
                        CHud::m_nHelpMessageState = 3;
                        CHud::m_nHelpMessageFadeTimer = 600;
                    }
                }
                break;
            case 2:
                if (!TheCamera.m_bWideScreenOn) {
                    CHud::m_nHelpMessageFadeTimer += 2 * (CTimer::ms_fTimeStep / 50.0f * 1000.0f);
                    if (CHud::m_nHelpMessageFadeTimer > 0) {
                        CHud::m_nHelpMessageFadeTimer = 0;
                        CHud::m_nHelpMessageState = 1;
                    }
                    alpha = CHud::m_nHelpMessageFadeTimer / 1000.0f * 255.0f;
                }
                break;
            case 3:
                CHud::m_nHelpMessageFadeTimer -= 2 * (CTimer::ms_fTimeStep / 50.0f * 1000.0f);
                if (CHud::m_nHelpMessageFadeTimer < 0 || TheCamera.m_bWideScreenOn) {
                    CHud::m_nHelpMessageFadeTimer = 0;
                    CHud::m_nHelpMessageState = 0;
                }
                alpha = CHud::m_nHelpMessageFadeTimer / 1000.0f * 255.0f;
                break;
            case 4:
                CHud::m_nHelpMessageFadeTimer -= 2 * (CTimer::ms_fTimeStep / 50.0f * 1000.0f);
                if (CHud::m_nHelpMessageFadeTimer < 0) {
                    CHud::m_nHelpMessageFadeTimer = 0;
                    CHud::m_nHelpMessageState = 2;
                    CMessages::StringCopy(CHud::m_pHelpMessageToPrint, CHud::m_pLastHelpMessage, 400);
                }
                alpha = CHud::m_nHelpMessageFadeTimer / 1000.0f * 255.0f;
                break;
            default:
                break;
            }

            if (!CCutsceneMgr::ms_running) {
                CHud::m_nHelpMessageTimer += CTimer::ms_fTimeStep / 50.0f * 1000.0f;

                CFontNew::SetFontStyle(CFontNew::FONT_1);
                CFontNew::SetScale(SCREEN_MULTIPLIER(0.6f), SCREEN_MULTIPLIER(1.2f));
                if (CFontNew::GetStringWidth(CHud::m_pHelpMessageToPrint, true) <= SCREEN_COORD(256.0f)) {//(!CHud::m_nHelpMessageStatId) {
                    PrintSmallHelpText(alpha);
                }
                else {
                    PrintBigHelpText(alpha);
                }
            }
        }
    }
    else {
        CHud::m_nHelpMessageState = 0;
    }
}

void CHudNew::PrintSmallHelpText(int alpha) {
    CRGBA c = GET_SETTING(HUD_HELP_BOX_SMALL_BORDER).col;

    CFontNew::SetBackground(false);
    CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
    CFontNew::SetBackgroundBorder(CRect(0.0f, 0.0f, 0.0f, 0.0f));
    CFontNew::SetAlignment(CFontNew::ALIGN_LEFT);
    CFontNew::SetFontStyle(CFontNew::FONT_1);
    CFontNew::SetDropShadow(0.0f);
    CFontNew::SetOutline(0.0f);
    CFontNew::SetDropColor(CRGBA(0, 0, 0, 0));
    CFontNew::SetWrapX(SCREEN_COORD(254.0f));

    char string[256];
    if (CHud::m_nHelpMessageStatId) {
        if (CHud::m_nHelpMessageStatId >= 10) {
            if (CHud::m_nHelpMessageStatId >= 100)
                sprintf(string, "STAT%d", CHud::m_nHelpMessageStatId);
            else
                sprintf(string, "STAT0%d", CHud::m_nHelpMessageStatId);
        }
        else
            sprintf(string, "STAT00%d", CHud::m_nHelpMessageStatId);

        sprintf(string, "%s %c", TheText.Get(string), CHud::m_pHelpMessageToPrint[0]);

        CRect r;
        r.left = HUD_X(96.0f);
        r.top = HUD_BOTTOM(324.0f);
        r.right = r.left + SCREEN_COORD(270.0f);
        r.bottom = r.top + SCREEN_COORD(64.0f);

        DrawSimpleRectGrad(r, c);

        float progress = 0.0f;
        if (CHud::m_nHelpMessageStatId == 336)
            progress = plugin::CallMethodAndReturn<unsigned int, 0x5F6AA0>(0xC09928 + FindPlayerPed(-1)->m_pPlayerData->m_nPlayerGroup * 0x2D4);
        else
            progress = CStats::GetStatValue(CHud::m_nHelpMessageStatId);

        char percentage[16];
        sprintf(percentage, "%d/100", (int)progress / 10);

        c = GET_SETTING(HUD_HELP_BOX_SMALL_TEXT).col;
        CFontNew::SetColor(CRGBA(c.r, c.g, c.b, clamp(alpha, 0, c.a)));
        CFontNew::SetScale(SCREEN_MULTIPLIER(0.6f), SCREEN_MULTIPLIER(1.2f));
        CFontNew::PrintString(HUD_X(96.0f + 50.0f), HUD_BOTTOM(324.0f), string);
        CFontNew::PrintString(HUD_X(96.0f + 50.0f), HUD_BOTTOM(304.0f), percentage);

        CheckPlayerPortrait(CWorld::PlayerInFocus);
        r.left = HUD_X(96.0f + 4.0f);
        r.top = HUD_BOTTOM(324.0f - 4.0f);
        r.right = r.left + SCREEN_COORD(38.0f);
        r.bottom = r.top + SCREEN_COORD(38.0f);
        DrawSimpleRect(r, HudColourNew.GetRGB(HUD_COLOUR_BLACK, 200));
        PlayerPortrait[CWorld::PlayerInFocus][1]->Draw(r, CRGBA(255, 255, 255, 255));
        unsigned int savedFilter;
        RwRenderStateGet(rwRENDERSTATETEXTUREFILTER, (void*)&savedFilter);
        RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERNEAREST);
        DrawProgressBarWithSprite(StatsSprites[PLRSTAT_PROGRESS_BAR], HUD_X(96.0f + 4.0f), HUD_BOTTOM(300.0f - 23.0f), SCREEN_COORD(262.0f), SCREEN_COORD(13.0f), progress / 1000, HudColourNew.GetRGB(MenuNew.Settings.uiMainColor, 255));
        RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)savedFilter);
    }
    else if (ShowTripSkipMessage) {
        CRect r;
        r.left = HUD_X(96.0f);
        r.top = HUD_BOTTOM(324.0f);
        r.right = r.left + SCREEN_COORD(270.0f);
        r.bottom = r.top + SCREEN_COORD(64.0f);

        DrawSimpleRectGrad(r, c);

        r.left = HUD_X(104.0f);
        r.top = HUD_BOTTOM(324.0f);
        r.right = r.left + SCREEN_COORD(64.0f);
        r.bottom = r.top + SCREEN_COORD(64.0f);
        MenuNew.MiscSprites[MISC_SKIPICON]->Draw(r, CRGBA(255, 255, 255, 255));

        c = GET_SETTING(HUD_HELP_BOX_SMALL_TEXT).col;
        CFontNew::SetColor(CRGBA(c.r, c.g, c.b, clamp(alpha, 0, c.a)));
        CFontNew::SetScale(SCREEN_MULTIPLIER(GET_SETTING(HUD_HELP_BOX_SMALL_TEXT).w), SCREEN_MULTIPLIER(GET_SETTING(HUD_HELP_BOX_SMALL_TEXT).h));
        CFontNew::PrintString(HUD_X(96.0f + 50.0f), HUD_BOTTOM(324.0f), "TRIP SKIP");
    }
    else {
        CFontNew::SetBackground(true);
        CFontNew::SetGradBackground(true);
        CFontNew::SetBackgroundColor(CRGBA(c.r, c.g, c.b, clamp(alpha, 0, c.a)));
        CHudSetting hb = GET_SETTING(HUD_HELP_BOX_SMALL_BORDER);
        CFontNew::SetBackgroundBorder(CRect(SCREEN_COORD(hb.x), SCREEN_COORD(hb.y), SCREEN_COORD(hb.w), SCREEN_COORD(hb.h)));

        c = GET_SETTING(HUD_HELP_BOX_SMALL_TEXT).col;
        CFontNew::SetColor(CRGBA(c.r, c.g, c.b, clamp(alpha, 0, c.a)));
        CFontNew::SetScale(SCREEN_MULTIPLIER(GET_SETTING(HUD_HELP_BOX_SMALL_TEXT).w), SCREEN_MULTIPLIER(GET_SETTING(HUD_HELP_BOX_SMALL_TEXT).h));
        CFontNew::PrintStringFromBottom(HUD_X(GET_SETTING(HUD_HELP_BOX_SMALL_TEXT).x), HUD_BOTTOM(GET_SETTING(HUD_HELP_BOX_SMALL_TEXT).y), CHud::m_pHelpMessageToPrint);
        CFontNew::SetGradBackground(false);
    }
}

void CHudNew::PrintBigHelpText(int alpha) {
    CRGBA c = GET_SETTING(HUD_HELP_BOX_BORDER).col;
    CFontNew::SetBackground(true);
    CFontNew::SetBackgroundColor(CRGBA(c.r, c.g, c.b, clamp(alpha, 0, c.a)));
    CHudSetting hb = GET_SETTING(HUD_HELP_BOX_BORDER);
    CFontNew::SetBackgroundBorder(CRect(SCREEN_COORD(hb.x), SCREEN_COORD(hb.y), SCREEN_COORD(hb.w), SCREEN_COORD(hb.h)));
    CFontNew::SetAlignment(CFontNew::ALIGN_LEFT);
    CFontNew::SetFontStyle(CFontNew::FONT_1);
    CFontNew::SetDropShadow(0.0f);
    CFontNew::SetOutline(0.0f);
    CFontNew::SetDropColor(CRGBA(0, 0, 0, 0));
    CFontNew::SetWrapX(SCREEN_COORD(421.0f));

    c = GET_SETTING(HUD_HELP_BOX_TEXT).col;
    CFontNew::SetColor(CRGBA(c.r, c.g, c.b, clamp(alpha, 0, c.a)));

    CFontNew::SetScale(SCREEN_MULTIPLIER(GET_SETTING(HUD_HELP_BOX_TEXT).w), SCREEN_MULTIPLIER(GET_SETTING(HUD_HELP_BOX_TEXT).h));
    CFontNew::PrintString(HUD_X(GET_SETTING(HUD_HELP_BOX_TEXT).x), HUD_Y(GET_SETTING(HUD_HELP_BOX_TEXT).y), CHud::m_pHelpMessageToPrint);
}

void CHudNew::DrawOddJobMessage(bool priority) {

}

void CHudNew::DrawSuccessFailedMessage() {
    char* mainText = NULL;
    char* bottomText = NULL;
    static float offset = 0.0f;
    static int time = -1;
    CRGBA col = HudColourNew.GetRGB(HUD_COLOUR_YELLOW, 150);

    SetHUDSafeZone(false);

    m_bShowSuccessFailed = true;
    if (CHud::m_BigMessage[0][0] && !strcmp(CHud::m_BigMessage[0], TheText.Get("M_PASS"))) {
        mainText = CTextNew::GetText("M_PASS").text;
        bottomText = m_LastMissionName;
        col = HudColourNew.GetRGB(HUD_COLOUR_YELLOW, 150);
    }
    else if (CHud::m_BigMessage[0][0] && !strcmp(CHud::m_BigMessage[0], TheText.Get("M_PASSR"))) {
        mainText = CTextNew::GetText("M_PASS").text;
        bottomText = m_LastMissionName;
        col = HudColourNew.GetRGB(HUD_COLOUR_YELLOW, 150);
    }
    else if (CHud::m_BigMessage[0][0] && !strcmp(CHud::m_BigMessage[0], TheText.Get("M_PASSS"))) {
        mainText = CTextNew::GetText("M_PASS").text;
        bottomText = m_LastMissionName;
        col = HudColourNew.GetRGB(HUD_COLOUR_YELLOW, 150);
    }
    else if (CHud::m_BigMessage[0][0] && !strcmp(CHud::m_BigMessage[0], TheText.Get("M_FAIL"))) {
        mainText = CTextNew::GetText("M_FAIL").text;

        switch (FindPlayerPed(-1)->m_nPedState) {
            case PEDSTATE_DEAD:
                bottomText = TheText.Get("DEAD");
                break;
            case PEDSTATE_ARRESTED:
                bottomText = TheText.Get("BUSTED");
                break;
            default:
                bottomText = "";
                break;
        }

        col = HudColourNew.GetRGB(HUD_COLOUR_RED, 150);
    }
    else {
        offset = 0.0f;
        time = -1;
        m_bShowSuccessFailed = false;
    }
    
    if (m_bShowSuccessFailed) {
        if (time == -1)
            time = CTimer::m_snTimeInMilliseconds + 1500;

        if (time < CTimer::m_snTimeInMilliseconds) {
            time = 0;
            if (offset > -256.0f)
                offset -= CTimer::ms_fTimeStep * 0.02f * 512.0f;
        }

        CFontNew::SetBackground(false);
        CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
        CFontNew::SetAlignment(CFontNew::ALIGN_CENTER);
        CFontNew::SetWrapX(SCREEN_COORD(1920.0f));
        CFontNew::SetFontStyle(CFontNew::FONT_3);
        CFontNew::SetDropShadow(0.0f);
        CFontNew::SetOutline(0.0f);
        CFontNew::SetDropColor(CRGBA(0, 0, 0, 0));

        CFontNew::SetColor(col);
        CFontNew::SetScale(SCREEN_MULTIPLIER(GET_SETTING(HUD_BIG_MESSAGE).w), SCREEN_MULTIPLIER(GET_SETTING(HUD_BIG_MESSAGE).h));

        float left = HUD_X(0.0f);
        float right = HUD_RIGHT(0.0f);
        float top1 = HUD_Y(offset) + SCREEN_COORD_CENTER_Y - HUD_Y(101.0f);
        float bottom1 = HUD_Y(offset) + SCREEN_COORD_CENTER_Y + HUD_Y(72.0f);
        float top2 = HUD_Y(offset) + SCREEN_COORD_CENTER_Y - HUD_Y(143.0f);
        float bottom2 = HUD_Y(offset) + SCREEN_COORD_CENTER_Y + HUD_Y(117.0f);

        DrawSimpleRectGradCentered(left, top1, right, top2, left, bottom1, right, bottom2, CRGBA(0, 0, 0, 150));

        CFontNew::PrintString(SCREEN_COORD_CENTER_LEFT(GET_SETTING(HUD_BIG_MESSAGE).x), SCREEN_COORD_CENTER_DOWN(offset + GET_SETTING(HUD_BIG_MESSAGE).y), mainText);

        CFontNew::SetDropShadow(0.0f);
        CFontNew::SetOutline(SCREEN_COORD(1.0f));
        CFontNew::SetFontStyle(CFontNew::FONT_1);
        CFontNew::SetDropColor(CRGBA(0, 0, 0, 255));
        CFontNew::SetColor(HudColourNew.GetRGB(HUD_COLOUR_WHITE, 255));
        CFontNew::SetScale(SCREEN_MULTIPLIER(0.96f), SCREEN_MULTIPLIER(1.8f));
        CFontNew::PrintString(SCREEN_COORD_CENTER_LEFT(GET_SETTING(HUD_BIG_MESSAGE).x), SCREEN_COORD_CENTER_DOWN(offset + GET_SETTING(HUD_BIG_MESSAGE).y + 114.0f), bottomText);
    }

    SetHUDSafeZone(true);
}

void CHudNew::DrawZoneName() {
    if (CHud::m_pZoneName) {
        int state = CHud::m_ZoneState;
        char* last = CHud::m_pLastZoneName;
        float alpha = 0.0f;

        if (CHud::m_pZoneName != CHud::m_pLastZoneName) {
            switch (CHud::m_ZoneState) {
            case 0:
                if (!CTheScripts::bPlayerIsOffTheMap && CTheScripts::bDisplayHud ||
                    CEntryExitManager::ms_exitEnterState == 1 ||
                    CEntryExitManager::ms_exitEnterState == 2) {
                    state = 2;
                    CHud::m_ZoneState = 2;
                    CHud::m_ZoneNameTimer = 0;
                    CHud::m_ZoneFadeTimer = 0;
                    CHud::m_ZoneToPrint = CHud::m_pZoneName;

                    if (CHud::m_VehicleState == 1 || CHud::m_VehicleState == 2)
                        CHud::m_VehicleState = 3;
                }
                break;
            case 1:
            case 2:
            case 3:
                state = 4;
                CHud::m_ZoneState = 4;
            case 4:
                CHud::m_ZoneNameTimer = 0;
                break;
            default:
                break;
            }
            last = CHud::m_pZoneName;
            CHud::m_pLastZoneName = CHud::m_pZoneName;
        }

        alpha = 255.0f;

        if (state) {
            switch (state) {
            case 1:
                if (CHud::m_ZoneNameTimer > 6000) {
                    CHud::m_ZoneFadeTimer = 1000;
                    CHud::m_ZoneState = 3;
                }

                alpha = 255.0f;
                CHud::m_ZoneFadeTimer = 1000;
                break;
            case 2:
                CHud::m_ZoneFadeTimer += CTimer::ms_fTimeStep * 0.02f * 1000.0f;
                if (CHud::m_ZoneFadeTimer > 1000) {
                    CHud::m_ZoneState = 1;
                    CHud::m_ZoneFadeTimer = 1000;
                }
                alpha = CHud::m_ZoneFadeTimer * 0.001f * 255.0f;
                break;
            case 3:
                CHud::m_ZoneFadeTimer += CTimer::ms_fTimeStep * 0.02f * -1000.0f;
                if (CHud::m_ZoneFadeTimer < 0) {
                    CHud::m_ZoneState = 0;
                    CHud::m_ZoneFadeTimer = 0;
                }
                alpha = CHud::m_ZoneFadeTimer * 0.001f * 255.0f;
                break;
            case 4:
                CHud::m_ZoneFadeTimer += CTimer::ms_fTimeStep * 0.02f * -1000.0f;
                if (CHud::m_ZoneFadeTimer < 0) {
                    CHud::m_ZoneFadeTimer = 0;
                    CHud::m_ZoneState = 2;
                    CHud::m_ZoneToPrint = last;
                }
                alpha = CHud::m_ZoneFadeTimer * 0.001f * 255.0f;
                break;
            default:
                break;

            }
            CHud::m_ZoneNameTimer += CTimer::ms_fTimeStep * 0.02f * 1000.0f;

            CFontNew::SetBackground(false);
            CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
            CFontNew::SetAlignment(CFontNew::ALIGN_RIGHT);
            CFontNew::SetWrapX(SCREEN_COORD(640.0f));
            CFontNew::SetFontStyle(CFontNew::FONT_2);
            CFontNew::SetDropShadow(SCREEN_COORD(2.0f));
            CFontNew::SetDropColor(CRGBA(0, 0, 0, alpha));

            CRGBA col = GET_SETTING(HUD_ZONE_NAME).col;
            CFontNew::SetColor(CRGBA(col.r, col.g, col.b, alpha));
            CFontNew::SetScale(SCREEN_MULTIPLIER(GET_SETTING(HUD_ZONE_NAME).w), SCREEN_MULTIPLIER(GET_SETTING(HUD_ZONE_NAME).h));
            CFontNew::PrintString(HUD_RIGHT(GET_SETTING(HUD_ZONE_NAME).x), HUD_BOTTOM(GET_SETTING(HUD_ZONE_NAME).y), CHud::m_ZoneToPrint);
        }
    }
    else {
        CHud::m_pLastZoneName = NULL;
        CHud::m_ZoneState = 0;
        CHud::m_ZoneFadeTimer = 0;
        CHud::m_ZoneNameTimer = 0;
    }
}

void CHudNew::DrawLevelName() {
    static bool showText = false;
    static float alpha = 0.0f;
    static int time = 0;
    static int previousLevel = -1;

    if (previousLevel != CTheZones::m_CurrLevel || m_nLevelNameState == 1) {
        sprintf(m_CurrentLevelName, "LEVEL%d", CTheZones::m_CurrLevel);

        time = CTimer::m_snTimeInMilliseconds + (m_nLevelNameState ? 500 : 6000);
        showText = true;
        previousLevel = CTheZones::m_CurrLevel;
    }

    if (showText) {
        if (time < CTimer::m_snTimeInMilliseconds) {
            alpha -= CTimer::ms_fTimeStep * 0.02f * 255.0f;

            if (alpha <= 0.0f)
                showText = false;
        }
        else {
            if (m_nLevelNameState) {
                alpha = 255;
                m_nLevelNameState = 0;
            }
            else
                alpha += CTimer::ms_fTimeStep * 0.02f * 255.0f;
        }

        alpha = clamp(alpha, 0, 255);

        CFontNew::SetBackground(false);
        CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
        CFontNew::SetAlignment(CFontNew::ALIGN_RIGHT);
        CFontNew::SetWrapX(SCREEN_COORD(640.0f));
        CFontNew::SetFontStyle(CFontNew::FONT_2);
        CFontNew::SetDropShadow(SCREEN_COORD(2.0f));
        CFontNew::SetDropColor(CRGBA(0, 0, 0, alpha));

        CRGBA col = GET_SETTING(HUD_LEVEL_NAME).col;
        CFontNew::SetColor(CRGBA(col.r, col.g, col.b, alpha));
        CFontNew::SetScale(SCREEN_MULTIPLIER(GET_SETTING(HUD_LEVEL_NAME).w), SCREEN_MULTIPLIER(GET_SETTING(HUD_LEVEL_NAME).h));
        CFontNew::PrintString(HUD_RIGHT(GET_SETTING(HUD_LEVEL_NAME).x), HUD_BOTTOM(GET_SETTING(HUD_LEVEL_NAME).y), CTextNew::GetText(m_CurrentLevelName).text);
    }
}

void CHudNew::DrawAfterFade() {
    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)(rwFILTERLINEARMIPLINEAR));
    RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*)rwTEXTUREADDRESSCLAMP);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);

    if (!CTimer::m_UserPause && CReplay::Mode != 1 && !CWeapon::ms_bTakePhoto) {
        DrawMissionTitle();
    }
}

void CHudNew::DrawWastedBustedText() {
    if (m_bShowSuccessFailed)
        return;

    SetHUDSafeZone(false);

    char* str = NULL;
    static eHudSettings i;
    static int time = -1;
    switch (FindPlayerPed(-1)->m_nPedState) {
        case PEDSTATE_DEAD:
        case PEDSTATE_DIE:
            str = TheText.Get("DEAD");
            i = HUD_WASTED_TEXT;
            COverlayLayer::SetEffect(EFFECT_BLACK_N_WHITE);
            if (time == -1)
                time = CTimer::m_snTimeInMilliseconds + 2000;
            break;
        case PEDSTATE_ARRESTED:
            str = TheText.Get("BUSTED");
            i = HUD_BUSTED_TEXT;
            COverlayLayer::SetEffect(EFFECT_BLACK_N_WHITE);
            if (time == -1)
                time = CTimer::m_snTimeInMilliseconds + 2000;
            break;
    }

    if (time != -1 && time < CTimer::m_snTimeInMilliseconds) {
        m_bShowWastedBusted = true;
        time = -1;
    }

    if (m_bShowWastedBusted && str) {
        CFontNew::SetBackground(false);
        CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
        CFontNew::SetAlignment(CFontNew::ALIGN_CENTER);
        CFontNew::SetWrapX(SCREEN_COORD(640.0f));
        CFontNew::SetFontStyle(CFontNew::FONT_3);
        CFontNew::SetDropShadow(0.0f);
        CFontNew::SetOutline(0.0f);
        CFontNew::SetDropColor(CRGBA(0, 0, 0, 0));

        CFontNew::SetColor(GET_SETTING(i).col);
        CFontNew::SetScale(SCREEN_MULTIPLIER(GET_SETTING(i).w), SCREEN_MULTIPLIER(GET_SETTING(i).h));

        float left = HUD_X(0.0f);
        float right = HUD_RIGHT(0.0f);
        float top1 = SCREEN_COORD_CENTER_Y - HUD_Y(101.0f);
        float bottom1 = SCREEN_COORD_CENTER_Y + HUD_Y(72.0f);
        float top2 = SCREEN_COORD_CENTER_Y - HUD_Y(143.0f);
        float bottom2 = SCREEN_COORD_CENTER_Y + HUD_Y(117.0f);

        DrawSimpleRectGradCentered(left, top1, right, top2, left, bottom1, right, bottom2, CRGBA(0, 0, 0, 150));

        CFontNew::PrintString(SCREEN_COORD_CENTER_LEFT(GET_SETTING(i).x), SCREEN_COORD_CENTER_DOWN(GET_SETTING(i).y), str);
    }

    SetHUDSafeZone(true);
}

void CHudNew::DrawMissionTitle() {
    static bool showText = false;
    static float alpha = 0.0f;
    static int time = -1;

    if (CHud::m_BigMessage[1][0]) {
        if (!showText) {
            strcpy(m_LastMissionName, CHud::m_BigMessage[1]);

            if (time == -1)
                time = CTimer::m_snTimeInMilliseconds + 4000;

            CHud::m_BigMessage[1][0] = NULL;
            showText = true;
        }
    }

    if (showText) {
        if (time < CTimer::m_snTimeInMilliseconds) {
            alpha -= CTimer::ms_fTimeStep * 0.02f * 255.0f;

            if (alpha <= 0.0f)
                showText = false;
        }
        else {
            alpha += CTimer::ms_fTimeStep * 0.02f * 255.0f;
        }

        alpha = clamp(alpha, 0, 255);

        CFontNew::SetBackground(false);
        CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
        CFontNew::SetAlignment(CFontNew::ALIGN_RIGHT);
        CFontNew::SetWrapX(SCREEN_COORD(640.0f));
        CFontNew::SetFontStyle(CFontNew::FONT_2);
        CFontNew::SetDropShadow(SCREEN_COORD(2.0f));
        CFontNew::SetDropColor(CRGBA(0, 0, 0, alpha));

        CRGBA col = HudColourNew.GetRGB(MenuNew.Settings.uiMainColor, 255);
        CFontNew::SetColor(CRGBA(col.r, col.g, col.b, alpha));
        CFontNew::SetScale(SCREEN_MULTIPLIER(GET_SETTING(HUD_MISSION_TITLE).w), SCREEN_MULTIPLIER(GET_SETTING(HUD_MISSION_TITLE).h));
        CFontNew::PrintString(HUD_RIGHT(GET_SETTING(HUD_MISSION_TITLE).x), HUD_BOTTOM(GET_SETTING(HUD_MISSION_TITLE).y), m_LastMissionName);
    } 
    else {
        time = -1;
        alpha = 0.0f;
    }
}

bool CHudNew::IsMoneyCounterDisplaying() {
    return bShowMoney && nMoneyFadeAlpha > 0;
}

bool CHudNew::IsMoneyDifferenceDisplaying() {
    return bShowMoneyDifference && nMoneyDifferenceFadeAlpha > 0;
}

bool CHudNew::IsAmmoCounterDisplaying() {
    return bShowAmmo && nAmmoFadeAlpha > 0;
}
