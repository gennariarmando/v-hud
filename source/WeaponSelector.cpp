#include "VHud.h"
#include "WeaponSelector.h"
#include "Utility.h"
#include "TextureMgr.h"
#include "HudNew.h"
#include "HudColoursNew.h"
#include "PadNew.h"
#include "MenuNew.h"
#include "OverlayLayer.h"
#include "FontNew.h"
#include "TextNew.h"
#include "CellPhone.h"
#include "Audio.h"
#include "MenuPanels.h"

#include "CGeneral.h"
#include "CWorld.h"
#include "CTimer.h"
#include "CPedIntelligence.h"
#include "CDarkel.h"
#include "CPad.h"
#include "CCamera.h"
#include "CWeaponInfo.h"
#include "CSprite2d.h"
#include "CAERadioTrackManager.h"
#include "CWeapon.h"
#include "CHud.h"

using namespace plugin;

CWeaponSelector weaponSelector;

bool CWeaponSelector::bInitialised;
CSprite2d* CWeaponSelector::WheelSprite[NUM_WHEEL_SPRITES];
CSprite2d* CWeaponSelector::ExtraSprite[NUM_WEXTRA_SPRITES];
CVector2D CWeaponSelector::vMousePos;
bool CWeaponSelector::bShowWeaponWheel;
bool CWeaponSelector::bWeaponWheelJustClosed;
int CWeaponSelector::nWeaponWheelOpenTime;
float CWeaponSelector::fPreviousTimeScale;
int CWeaponSelector::nActiveSlot;
int CWeaponSelector::nActiveWeapon[8];
int CWeaponSelector::nSelectedSlot;
int CWeaponSelector::nSelectedWeapon[8];
CWeaponWheel* CWeaponSelector::WeaponWheel[8][MAX_WEAPONS_ON_WHEEL];
bool CWeaponSelector::bSlowCycle;
int CWeaponSelector::nTimeSinceClosed;
float CWeaponSelector::nPrevCamHor;
float CWeaponSelector::nPrevCamVer;
int CWeaponSelector::nNumSelectedWeaponAvailableInSlot[8];
int CWeaponSelector::nNumWeaponsAvailableInSlot[8];
int CWeaponSelector::nArrayOfAvailableWeapons[8][MAX_WEAPONS_ON_WHEEL];
CWeaponStat* CWeaponSelector::WeaponStat;
float CWeaponSelector::fStatMaxRate;
float CWeaponSelector::fStatsProgress[4];
float CWeaponSelector::fStatsDiff[4];
int CWeaponSelector::nWeaponExtraFadeAlpha;
CWeaponCrosshair CWeaponSelector::nCrosshairs[MAX_WEAPONS_ON_WHEEL];

static LateStaticInit InstallHooks([]() {
    // Disable weapon cycle CPlayerPed::ProcessPlayerWeaponSwitch
    patch::Set<BYTE>(0x60D8C6, 0x90);
    patch::Set<BYTE>(0x60D8C7, 0xE9);
    patch::Set<BYTE>(0x60DA85, 0x90);
    patch::Set<BYTE>(0x60DA86, 0xE9);
});

char* WheelFileNames[] = {
    "wheel_part_active",
    "wheel_part_grad",
    "wheel_part_inactive",
    "wheel_part_solid",
    "wheel_arrow_left",
    "wheel_arrow_right",
    "wheel_extra",
};

char* WExtraFileNames[] = {
    "parachute",
};

CWeaponWheel* CWeaponSelector::GetActiveWeapon() {
    if (nActiveSlot != -1) {
        if (nActiveWeapon[nActiveSlot] != -1)
            return WeaponWheel[nActiveSlot][nActiveWeapon[nActiveSlot]];
    }

    return NULL;
}

void CWeaponSelector::Init() {
    if (bInitialised)
        return;

    Clear();

    ReadSlotFromFile();
    ReadWeaponRatesFromFile();

    for (int i = 0; i < NUM_WHEEL_SPRITES; i++) {
        WheelSprite[i] = new CSprite2d();
        WheelSprite[i]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\weapon"), WheelFileNames[i]);
    }

    for (int i = 0; i < NUM_WEXTRA_SPRITES; i++) {
        ExtraSprite[i] = new CSprite2d();
        ExtraSprite[i]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\weapon\\extra"), WExtraFileNames[i]);
    }

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 128; j++) {
            if (WeaponWheel[i][j] && strncmp(WeaponWheel[i][j]->tex, "\0", 1)) {
                WeaponWheel[i][j]->sprite->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\weapon\\weapons"), WeaponWheel[i][j]->tex);
            }
        }
    }

    bInitialised = true;
}

void CWeaponSelector::Clear() {
    bShowWeaponWheel = false;
    nWeaponWheelOpenTime = 0;
    fPreviousTimeScale = 1.0f;

    ClearWheel();

    bSlowCycle = false;
    nTimeSinceClosed = CTimer::m_snTimeInMilliseconds;

    nPrevCamHor = TheCamera.m_fMouseAccelHorzntl;
    nPrevCamVer = TheCamera.m_fMouseAccelVertical;

    CTimer::ms_fTimeScale = 1.0f;
}

void CWeaponSelector::ReInit() {
    Clear();
}

void CWeaponSelector::Shutdown() {
    if (!bInitialised)
        return;

    Clear();

    for (int i = 0; i < NUM_WHEEL_SPRITES; i++) {
        if (WheelSprite[i]) {
            WheelSprite[i]->Delete();
            delete WheelSprite[i];
        }
    }

    for (int i = 0; i < NUM_WEXTRA_SPRITES; i++) {
        if (ExtraSprite[i]) {
            ExtraSprite[i]->Delete();
            delete ExtraSprite[i];
        }
    }

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 128; j++) {
            if (WeaponWheel[i][j] && strncmp(WeaponWheel[i][j]->tex, "\0", 1)) {
                WeaponWheel[i][j]->sprite->Delete();
                delete WeaponWheel[i][j];
            }
        }
    }

    if (WeaponStat)
        delete[] WeaponStat;

    bInitialised = false;
}

void CWeaponSelector::ReadSlotFromFile() {
    std::ifstream file(PLUGIN_PATH("VHud\\data\\weapon_selector.dat"));

    int slot = 0;
    if (file.is_open()) {
        for (std::string line; getline(file, line);) {
            if (!line[0] || line[0] == '#')
                continue;

            char str[64];
            int num_weapons = 0;
            sscanf(line.c_str(), "%s", &str);

            if (faststrcmp(str, "SLOT") == 0) {
                getline(file, line); // num_weapons
                sscanf(line.c_str(), "%s %d", &str, &num_weapons);

                for (int i = 0; i < num_weapons; i++) {
                    int id = 0;
                    char name[32];
                    char tex[32];
                    char crosshair[32];

                    getline(file, line); // WEAPON

                    getline(file, line); // id
                    sscanf(line.c_str(), "%s %d", &str, &id);

                    getline(file, line); // name
                    sscanf(line.c_str(), "%s %s", &str, &name);

                    getline(file, line); // texture
                    sscanf(line.c_str(), "%s %s", &str, &tex);

                    getline(file, line); // crosshair
                    sscanf(line.c_str(), "%s %s", &str, &crosshair);

                    WeaponWheel[slot][i] = new CWeaponWheel();
                    CWeaponWheel* w = WeaponWheel[slot][i];
                    w->id = id;
                    strcpy(w->name, name);
                    strcpy(w->tex, tex);
                    nCrosshairs[w->id] = CWeaponCrosshair();
                    strcpy(nCrosshairs[w->id].name, crosshair);
                    printf("slot:%d, id:%d, name:%s, tex:%s\n", slot, id, name, tex);
                    getline(file, line); // END
                }

                getline(file, line); // END
                slot++;
            }
        }

        file.close();
    }
}

void CWeaponSelector::ReadWeaponRatesFromFile() {
    std::ifstream file(PLUGIN_PATH("VHud\\data\\weapon_rates.dat"));

    int id = 0;
    if (file.is_open()) {
        for (std::string line; getline(file, line);) {
            if (!line[0] || line[0] == '#')
                continue;

            char str[64];
            float s1, s2, s3;
            sscanf(line.c_str(), "%s %f %f %f", &str, &s1, &s2, &s3);

            if (!strcmp(str, "NUM_WEAPONS")) {
                WeaponStat = new CWeaponStat[(int)s1];
            }
            else if (!strcmp(str, "MAX_RATE")) {
                fStatMaxRate = s1;
            }
            else {
                WeaponStat[id].id = id;
                strcpy(WeaponStat[id].name, str);
                WeaponStat[id].skill1 = s1;
                WeaponStat[id].skill2 = s2;
                WeaponStat[id].skill3 = s3;

                id++;
            }
        }

        file.close();
    }
}

void CWeaponSelector::RegisterSounds() {
    ;;
}

bool CWeaponSelector::IsAbleToSwitchWeapon() {
    CPed* playa = CWorld::Players[0].m_pPed;
    eCamMode mode = TheCamera.m_aCams[TheCamera.m_nActiveCam].m_nMode;

    return
        playa
        && !playa->m_nPedFlags.bInVehicle
        && !playa->m_nPedFlags.bDontRender
        && playa->m_nPedState != PEDSTATE_MAKE_PHONECALL
        && playa->m_nPedState != PEDSTATE_ANSWER_MOBILE
        && playa->m_nPedState != PEDSTATE_PAUSE
        && playa->IsPedShootable()
        && playa->m_nPedState != PEDSTATE_FACE_PHONE
        && !playa->m_nPedFlags.bIsInTheAir
        && !playa->m_nPedFlags.bFiringWeapon
        && !playa->m_nPedFlags.bIsAimingGun
        && !playa->m_pPlayerData->m_bInVehicleDontAllowWeaponChange
        && !playa->m_pPlayerData->m_bHaveTargetSelected
        && !CDarkel::FrenzyOnGoing()
        && !playa->m_pAttachedTo
        && !playa->m_nPhysicalFlags.bAttachedToEntity
        && !playa->m_pIntelligence->GetTaskJetPack()
        && !playa->m_pIntelligence->GetTaskClimb()
        && !playa->m_pIntelligence->GetTaskInAir()
        && !playa->m_pTargetedObject
        && !playa->m_pPlayerData->m_bDontAllowWeaponChange
        && !playa->m_pIntelligence->GetUsingParachute()
        && !playa->m_pIntelligence->IsInACarOrEnteringOne()
        && !CHud::bDrawingVitalStats
        && !CMenuPanels::bActive
        && !CHudNew::m_bShowWastedBusted
        && !CHudNew::m_bShowSuccessFailed
        && !playa->m_nPedFlags.bTestForShotInVehicle
        && !playa->m_nPedFlags.bUsingMobilePhone
        && !CPadNew::GetPad(0)->bDisablePlayerCycleWeapon
        && playa->IsPedInControl()
        && mode != MODE_AIMWEAPON
        && mode != MODE_AIMWEAPON_ATTACHED
        && mode != MODE_ROCKETLAUNCHER
        && mode != MODE_ROCKETLAUNCHER_HS
        && mode != MODE_ROCKETLAUNCHER_RUNABOUT_HS
        && mode != MODE_SNIPER
        && mode != MODE_SNIPER_RUNABOUT
        && mode != MODE_CAMERA
        && mode != MODE_SYPHON
        && mode != MODE_1STPERSON
        && mode != MODE_AIMWEAPON_FROMCAR
        && mode != MODE_AIMWEAPON_ATTACHED
        && mode != MODE_TWOPLAYER_IN_CAR_AND_SHOOTING;
}

void CWeaponSelector::ProcessWeaponSelector() {
    UpdateCursorPos();

    if (IsAbleToSwitchWeapon()) {
        if (nTimeSinceClosed < CTimer::m_snTimeInMilliseconds) {
            if (!bShowWeaponWheel && CPadNew::GetPad(0)->GetShowWeaponWheel(350)) {
                OpenWeaponWheel(true);
            }
            else if (bShowWeaponWheel && bSlowCycle && !CPadNew::GetPad(0)->GetShowWeaponWheel()) {
                CloseWeaponWheel(true);
            }
            else {
                if (!bSlowCycle) {
                    if (!CellPhone.bActive) {
                        if (!HAS_PAD_IN_HANDS(0) && CPadNew::GetPad(0)->GetWeaponWheelCycleRight()) {
                            OpenWeaponWheelQuickSwitch("RIGHT");
                        }
                        else if (!HAS_PAD_IN_HANDS(0) && CPadNew::GetPad(0)->GetWeaponWheelCycleLeft()) {
                            OpenWeaponWheelQuickSwitch("LEFT");
                        }
                    }
                }
            }

            if (bShowWeaponWheel && bSlowCycle && CPadNew::GetPad(0)->GetShowWeaponWheel()) {
                if (CPadNew::GetPad(0)->GetWeaponWheelCycleRight()) {
                    SwitchWeaponFromSlot("RIGHT");
                }
                else if (CPadNew::GetPad(0)->GetWeaponWheelCycleLeft()) {
                    SwitchWeaponFromSlot("LEFT");
                }
            }

            float x[] = {
                -64.0f,
               -251.0f,
               -315.0f,
               -251.0f,
                -64.0f,
               123.0f,
               187.0f,
               123.0f,
            };
            float y[] = {
                187.0f,
                90.0f,
               -69.0f,
               -239.0f,
               -303.0f,
               -239.0f,
               -69.0f,
                90.0f
            };

            if (bSlowCycle) {
                CVector2D centre = { SCREEN_COORD_CENTER_X + GET_SETTING(HUD_WEAPON_WHEEL).x, SCREEN_COORD_CENTER_Y + (GET_SETTING(HUD_WEAPON_WHEEL).y) };

                CVector2D pos = LimitMousePosition(vMousePos);
                float a = atan2f(pos.y - centre.y, pos.x - centre.x) * (180.0f / M_PI);

                a = ConstrainAngle(-a);

                float deg = 45.0f;
                float dist = (vMousePos - centre).Magnitude();
                float radius = 45.0f;

                int previousSlot = nSelectedSlot;

                if (dist > radius) {
                    if ((a > 340.0f && a < 360.0f) || (a > 0.0f && a < 25.0f)) {
                        nSelectedSlot = 6;
                    }
                    if (a > 25.0f && a < 25.0f + deg) {
                        nSelectedSlot = 5;
                    }
                    if (a > 25.0f + deg && a < 25.0f + deg * 2) {
                        nSelectedSlot = 4;
                    }
                    if (a > 25.0f + deg * 2 && a < 25.0f + deg * 3) {
                        nSelectedSlot = 3;
                    }
                    if (a > 25.0f + deg * 3 && a < 25.0f + deg * 4) {
                        nSelectedSlot = 2;
                    }
                    if (a > 25.0f + deg * 4 && a < 25.0f + deg * 5) {
                        nSelectedSlot = 1;
                    }
                    if (a > 25.0f + deg * 5 && a < 25.0f + deg * 6) {
                        nSelectedSlot = 0;
                    }
                    if (a >= 25.0f + deg * 6 && a <= 25.0f + deg * 7) {
                        nSelectedSlot = 7;
                    }
                }

                // Debug line                 
                CSprite2d::Draw2DPolygon(vMousePos.x, vMousePos.y, GET_SETTING(HUD_WEAPON_WHEEL_MOUSE_LINE).w + vMousePos.x, GET_SETTING(HUD_WEAPON_WHEEL_MOUSE_LINE).w + vMousePos.y, centre.x, centre.y, GET_SETTING(HUD_WEAPON_WHEEL_MOUSE_LINE).w + centre.x, GET_SETTING(HUD_WEAPON_WHEEL_MOUSE_LINE).w + centre.y, GET_SETTING(HUD_WEAPON_WHEEL_MOUSE_LINE).col);
            
                if (previousSlot != nSelectedSlot) {
                    Audio.PlayChunk(CHUNK_WHEEL_MOVE, 1.0f);
                }
            }
        }
    }
    else
        CloseWeaponWheel(false);

    if (bWeaponWheelJustClosed) {
        ClearWheel();
        bWeaponWheelJustClosed = false;
    }
}

void CWeaponSelector::CenterCursor() {
    CVector2D centre = { SCREEN_COORD_CENTER_X + GET_SETTING(HUD_WEAPON_WHEEL).x, SCREEN_COORD_CENTER_Y + (GET_SETTING(HUD_WEAPON_WHEEL).y) };

    vMousePos.x = centre.x;
    vMousePos.y = centre.y;
}

void CWeaponSelector::UpdateCursorPos() {
    float x = CPadNew::GetMouseInput(256.0f).x;
    float y = CPadNew::GetMouseInput(256.0f).y;

    if (HAS_PAD_IN_HANDS(0)) {
        x = CPadNew::GetPad(0)->NewState.RightStickX * 1.0f;
        y = CPadNew::GetPad(0)->NewState.RightStickY * 1.0f;
    }

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
}

CVector2D CWeaponSelector::LimitMousePosition(CVector2D& pos) {
    float radius = 50.0f;

    CVector2D centerPosition = { SCREEN_COORD_CENTER_X + GET_SETTING(HUD_WEAPON_WHEEL).x, SCREEN_COORD_CENTER_Y + (GET_SETTING(HUD_WEAPON_WHEEL).y) };
    float dist = (pos - centerPosition).Magnitude();

    if (dist > radius) {
        CVector2D fromOriginToObject = pos - centerPosition;
        fromOriginToObject *= radius / dist;
        pos.x = centerPosition.x + fromOriginToObject.x;
        pos.y = centerPosition.y + fromOriginToObject.y;
    }

    return pos;
}

void CWeaponSelector::ResetCameraMovement() {
    TheCamera.m_fMouseAccelHorzntl = nPrevCamHor;
    TheCamera.m_fMouseAccelVertical = nPrevCamVer;
    nPrevCamHor = 0.0f;
    nPrevCamVer = 0.0f;
}

void CWeaponSelector::DisableCameraMovement() {
    nPrevCamHor = TheCamera.m_fMouseAccelHorzntl;
    nPrevCamVer = TheCamera.m_fMouseAccelVertical;

    TheCamera.m_fMouseAccelHorzntl = 0.0f;
    TheCamera.m_fMouseAccelVertical = 0.0f;
}

void CWeaponSelector::SwitchWeaponFromSlot(const char* dir) {
    if (bShowWeaponWheel) {
        if (!strcmp(dir, "LEFT")) {
            nSelectedWeapon[nSelectedSlot] = GetPreviousWeaponInSlot(nSelectedSlot);
            Audio.PlayChunk(CHUNK_WHEEL_MOVE, 1.0f);
        }
        else if (!strcmp(dir, "RIGHT")) {
            nSelectedWeapon[nSelectedSlot] = GetNextWeaponInSlot(nSelectedSlot);
            Audio.PlayChunk(CHUNK_WHEEL_MOVE, 1.0f);
        }
    }
}

int CWeaponSelector::GetPreviousWeaponInSlot(int slot) {
    int i = nSelectedWeapon[slot] - 1;

    if (i < 0)
        i = 127;

    for (i; i > -1; i--) {
        if (nArrayOfAvailableWeapons[slot][i] != -1) {
            if (nNumSelectedWeaponAvailableInSlot[slot] < 2)
                nNumSelectedWeaponAvailableInSlot[slot] = nNumWeaponsAvailableInSlot[slot];
            else
                nNumSelectedWeaponAvailableInSlot[slot]--;

            return i;
        }
    }

    nNumSelectedWeaponAvailableInSlot[slot] = nNumWeaponsAvailableInSlot[slot];
    return GetLastWeaponAvailable(slot);
}

int CWeaponSelector::GetNextWeaponInSlot(int slot) {
    int i = nSelectedWeapon[slot] + 1;

    if (i > 127)
        i = 0;

    for (i; i < 128; i++) {
        if (nArrayOfAvailableWeapons[slot][i] != -1) {
            if (nNumSelectedWeaponAvailableInSlot[slot] > nNumWeaponsAvailableInSlot[slot])
                nNumSelectedWeaponAvailableInSlot[slot] = 1;
            else
                nNumSelectedWeaponAvailableInSlot[slot]++;

            return i;
        }
    }

    nNumSelectedWeaponAvailableInSlot[slot] = 1;
    return GetFirstWeaponAvailable(slot);
}

int CWeaponSelector::GetFirstWeaponAvailable(int slot) {
    for (int i = 0; i < 128; i++) {
        if (nArrayOfAvailableWeapons[slot][i] != -1) {
            return i;
        }
    }

    return 0;
}

int CWeaponSelector::GetLastWeaponAvailable(int slot) {
    for (int i = 127; i > 0; i--) {
        if (nArrayOfAvailableWeapons[slot][i] != -1) {
            return i;
        }
    }

    return 127;
}

void CWeaponSelector::PopulateSlot(int slot) {
    CPed* playa = FindPlayerPed(0);

    for (int j = 0; j < 128; j++) {
        CWeaponWheel* w = WeaponWheel[slot][j];

        if (w) {
            if (playa->DoWeHaveWeaponAvailable((eWeaponType)w->id) && (playa->m_aWeapons[playa->GetWeaponSlot((eWeaponType)w->id)].HasWeaponAmmoToBeUsed() || playa->m_aWeapons[playa->GetWeaponSlot((eWeaponType)w->id)].m_nType == WEAPON_UNARMED)) {
                nArrayOfAvailableWeapons[slot][j] = j;
                nNumWeaponsAvailableInSlot[slot]++;

                if (playa->m_aWeapons[playa->m_nActiveWeaponSlot].m_nType == (eWeaponType)w->id) {
                    nSelectedSlot = slot;
                    nSelectedWeapon[slot] = j;

                    nActiveSlot = slot;
                    nActiveWeapon[slot] = j;
                    nNumSelectedWeaponAvailableInSlot[slot] = nNumWeaponsAvailableInSlot[slot];
                }

                continue;
            }
        }

        nArrayOfAvailableWeapons[slot][j] = -1;
    }
}

void CWeaponSelector::OpenWeaponWheelQuickSwitch(const char* dir) {
    if (bShowWeaponWheel) {
        if (!strcmp(dir, "LEFT")) {
            while (1) {
                nSelectedSlot--;

                if (nSelectedSlot < 0)
                    nSelectedSlot = 7;

                if (nNumWeaponsAvailableInSlot[nSelectedSlot] != 0)
                    break;
            };
            Audio.PlayChunk(CHUNK_WHEEL_MOVE, 1.0f);
        }
        else if (!strcmp(dir, "RIGHT")) {
            while (1) {
                nSelectedSlot++;

                if (nSelectedSlot > 7)
                    nSelectedSlot = 0;

                if (nNumWeaponsAvailableInSlot[nSelectedSlot] != 0)
                    break;
            };

            Audio.PlayChunk(CHUNK_WHEEL_MOVE, 1.0f);
        }
    }

    bSlowCycle = false;
    OpenWeaponWheel(false);
}

void CWeaponSelector::OpenWeaponWheel(bool slow) {
    CPed* playa = FindPlayerPed(0);

    if (!bShowWeaponWheel) {
        fPreviousTimeScale = CTimer::ms_fTimeScale;

        for (int i = 0; i < 8; i++)
            PopulateSlot(i);

        for (int i = 0; i < 8; i++) {
            if (nSelectedWeapon[i] == -1)
                nSelectedWeapon[i] = GetFirstWeaponAvailable(i);
        }

        bSlowCycle = slow;

        if (bSlowCycle) {
            DisableCameraMovement();

            if (!VHud::bSAMP)
                CTimer::ms_fTimeScale = 0.25f;

            Audio.PlayChunk(CHUNK_WHEEL_OPEN_CLOSE, 1.0f);

            Audio.SetLoop(true);
            Audio.PlayChunk(CHUNK_WHEEL_BACKGROUND, 1.0f);
            Audio.SetLoop(false);
        }
    }

    nWeaponWheelOpenTime = CTimer::m_snTimeInMilliseconds + (500 * CTimer::ms_fTimeScale);
    bShowWeaponWheel = true;
    CenterCursor();
}

void CWeaponSelector::CloseWeaponWheel(bool switchon) {
    CPlayerPed* playa = CWorld::Players[0].m_pPed;

    CTimer::ms_fTimeScale = fPreviousTimeScale;
    fPreviousTimeScale = 1.0f;

    if (switchon) {
        CWeaponWheel* w = WeaponWheel[nSelectedSlot][nSelectedWeapon[nSelectedSlot]];

        int weap = nArrayOfAvailableWeapons[nSelectedSlot][nSelectedWeapon[nSelectedSlot]];
        if (nSelectedWeapon[nSelectedSlot] != -1 && weap != -1) {
            playa->m_pPlayerData->m_nChosenWeapon = playa->GetWeaponSlot((eWeaponType)w->id);

            if (playa->m_pPlayerData->m_nChosenWeapon != playa->m_nActiveWeaponSlot) {
                playa->RemoveWeaponAnims(playa->m_nActiveWeaponSlot, -1000.0f);
                playa->MakeChangesForNewWeapon(playa->m_aWeapons[playa->m_pPlayerData->m_nChosenWeapon].m_nType);
                nActiveSlot = nSelectedSlot;
                nActiveWeapon[nSelectedSlot] = nSelectedWeapon[nSelectedSlot];
            }
            else {
                nSelectedSlot = nActiveSlot;
                nSelectedWeapon[nActiveSlot] = nActiveWeapon[nActiveSlot];
            }
        }
    }

    //ClearWheel();

    if (bSlowCycle) {
        ResetCameraMovement();
        Audio.PlayChunk(CHUNK_WHEEL_OPEN_CLOSE, 1.0f);
        Audio.StopChunk(CHUNK_WHEEL_BACKGROUND);
    }

    nTimeSinceClosed = 250 + CTimer::m_snTimeInMilliseconds;
    nWeaponWheelOpenTime = 0;
    bSlowCycle = false;
    bShowWeaponWheel = false;
    bWeaponWheelJustClosed = true;
}

void CWeaponSelector::ClearWheel() {
    nSelectedSlot = 0;

    for (int i = 0; i < 8; i++)
        nSelectedWeapon[i] = -1;

    nActiveSlot = 0;
    for (int i = 0; i < 8; i++)
        nActiveWeapon[i] = -1;

    for (int i = 0; i < 8; i++) {
        nNumWeaponsAvailableInSlot[i] = 0;
        nNumSelectedWeaponAvailableInSlot[i] = 1;

        for (int j = 0; j < 128; j++) {
            nArrayOfAvailableWeapons[i][j] = -1;
        }
    }

    fStatsProgress[0] = 0.0f;
    fStatsProgress[1] = 0.0f;
    fStatsProgress[2] = 0.0f;
    fStatsProgress[3] = 0.0f;
    fStatsDiff[0] = 0.0f;
    fStatsDiff[1] = 0.0f;
    fStatsDiff[2] = 0.0f;
    fStatsDiff[3] = 0.0f;

    nWeaponExtraFadeAlpha = 0;
}

void CWeaponSelector::DrawWheelPart(char id, float x, float y, int n, CRGBA const& col) {
    unsigned int savedShade;
    unsigned int savedAlpha;
    unsigned int savedFilter;
    RwRenderStateGet(rwRENDERSTATESHADEMODE, &savedShade);
    RwRenderStateSet(rwRENDERSTATESHADEMODE, (void*)rwSHADEMODEGOURAUD);
    RwRenderStateGet(rwRENDERSTATEVERTEXALPHAENABLE, &savedAlpha);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
    RwRenderStateGet(rwRENDERSTATETEXTUREFILTER, &savedFilter);
    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERMIPLINEAR);

    float r = M_PI_4 * n;
    float angle = r + M_PI;
    float w = 256.0f * 1.16f;
    float h = 128.0f * 1.16f;
    float offset = 239.0f;
    y -= offset;

    CVector2D posn[4];
    posn[0].x = SCREEN_COORD(x) - SCREEN_COORD(w * 0.5f);
    posn[0].y = SCREEN_COORD(y) + SCREEN_COORD(h * 0.5f);
    posn[1].x = SCREEN_COORD(x) + SCREEN_COORD(w * 0.5f);
    posn[1].y = SCREEN_COORD(y) + SCREEN_COORD(h * 0.5f);
    posn[2].x = SCREEN_COORD(x) - SCREEN_COORD(w * 0.5f);
    posn[2].y = SCREEN_COORD(y) - SCREEN_COORD(h * 0.5f);
    posn[3].x = SCREEN_COORD(x) + SCREEN_COORD(w * 0.5f);
    posn[3].y = SCREEN_COORD(y) - SCREEN_COORD(h * 0.5f);
    RotateVertices(posn, SCREEN_COORD(x), SCREEN_COORD(y + offset), -angle);

    WheelSprite[id]->Draw(
        SCREEN_COORD_CENTER_X + posn[0].x, SCREEN_COORD_CENTER_Y + posn[0].y,
        SCREEN_COORD_CENTER_X + posn[1].x, SCREEN_COORD_CENTER_Y + posn[1].y,
        SCREEN_COORD_CENTER_X + posn[2].x, SCREEN_COORD_CENTER_Y + posn[2].y,
        SCREEN_COORD_CENTER_X + posn[3].x, SCREEN_COORD_CENTER_Y + posn[3].y, col);

    RwRenderStateSet(rwRENDERSTATESHADEMODE, (void*)savedShade);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)savedAlpha);
    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)savedFilter);
}

void CWeaponSelector::DrawWheel() {
    CPed* playa = FindPlayerPed(0);

    if (bShowWeaponWheel) {
        if (bSlowCycle) 
            COverlayLayer::SetEffect(EFFECT_BLUR_COLOR);

        static int previousWeaponSelectedShit;
        static int previousSlotSelectedShit;

        if (previousWeaponSelectedShit != nSelectedWeapon[nSelectedSlot] || previousSlotSelectedShit != nSelectedSlot)
            nWeaponExtraFadeAlpha = 0;
        else
            nWeaponExtraFadeAlpha = (int)interpF(nWeaponExtraFadeAlpha, 255, 0.2f * CTimer::ms_fTimeStep);

        if (!bSlowCycle && nWeaponWheelOpenTime < CTimer::m_snTimeInMilliseconds)
            CloseWeaponWheel(true);

        float x = GET_SETTING(HUD_WEAPON_WHEEL).x;
        float y = GET_SETTING(HUD_WEAPON_WHEEL).y;
        float w = GET_SETTING(HUD_WEAPON_WHEEL).w;
        float h = GET_SETTING(HUD_WEAPON_WHEEL).h;
        CRGBA col = CRGBA(255, 255, 255, 255);

        for (int i = 0; i < 8; i++) {
            if (i == nSelectedSlot)
                DrawWheelPart(WHEEL_PART_GRAD, x, y, i, HudColourNew.GetRGB(VHud::Settings.UIMainColor, 200));
            else
                DrawWheelPart(WHEEL_PART_SOLID, x, y, i, CRGBA(0, 0, 0, 100));

            if (i == nActiveSlot && nSelectedWeapon[i] == nActiveWeapon[i])
                DrawWheelPart(WHEEL_PART_ACTIVE, x, y, i, HudColourNew.GetRGB(VHud::Settings.UIMainColor, 255));
            else
                DrawWheelPart(WHEEL_PART_INACTIVE, x, y, i, CRGBA(0, 0, 0, 150));

            CWeaponWheel* wep = WeaponWheel[i][nSelectedWeapon[i]];
            int weap = nArrayOfAvailableWeapons[i][nSelectedWeapon[i]];
            if (nSelectedWeapon[i] != -1 && weap != -1) {
                float wx = x;
                float wy = y;
                float ax = x;
                float ay = y;

                switch (i) {
                case 0:
                    wx += -148.5f;
                    wy += 166.0f;
                    ax += 2.0f;
                    ay += 262.0f;
                    break;
                case 1:
                    wx += -328.0f;
                    wy += 90.0f;
                    ax += -148.0f;
                    ay += 200.0f;
                    break;
                case 2:
                    wx += -393.0f;
                    wy += -84.0f;
                    ax += -238.0f;
                    ay += 46.0f;
                    break;
                case 3:
                    wx += -328.0f;
                    wy += -246.0f;
                    ax += -194.0f;
                    ay += -134.0f;
                    break;
                case 4:
                    wx += -144.5f;
                    wy += -330.0f;
                    ax += 2.0f;
                    ay += -224.0f;
                    break;
                case 5:
                    wx += 31.0f;
                    wy += -246.0f;
                    ax += 194.0f;
                    ay += -134.0f;
                    break;
                case 6:
                    wx += 96.0f;
                    wy += -84.0f;
                    ax += 238.0f;
                    ay += 46.0f;
                    break;
                case 7:
                    wx += 27.0f;
                    wy += 90.0f;
                    ax += 148.0f;
                    ay += 200.0f;
                    break;
                }

                wep->sprite->Draw(SCREEN_COORD_CENTER_X + SCREEN_COORD(wx), SCREEN_COORD_CENTER_Y + SCREEN_COORD(wy), SCREEN_COORD(297.0f), SCREEN_COORD(148.5f), CRGBA(255, 255, 255, 255));

                int slot = playa->GetWeaponSlot((eWeaponType)wep->id);
                int weaponType = playa->m_aWeapons[slot].m_nType;
                int totalAmmo = playa->m_aWeapons[slot].m_nTotalAmmo;
                int ammoInClip = playa->m_aWeapons[slot].m_nAmmoInClip;
                int maxAmmoInClip = CWeaponInfo::GetWeaponInfo(playa->m_aWeapons[slot].m_nType, playa->GetWeaponSkill((eWeaponType)wep->id))->m_nAmmoClip;
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
                        sprintf(str_clip, " / %d", ammoInClip / 10);
                    }
                    else {
                        unsigned int total = totalAmmo - ammoInClip;
                        if (totalAmmo - ammoInClip > 9999) {
                            strcpy(str_ammo, "-");
                            strcpy(str_clip, " / -");
                        }
                        else {
                            sprintf(str_ammo, "%d", total);
                            sprintf(str_clip, " / %d", ammoInClip);
                        }
                    }
                }

                if (weaponType == WEAPON_UNARMED
                    || weaponType == WEAPON_DETONATOR
                    || weaponType >= WEAPON_DILDO1 && weaponType < WEAPON_GRENADE
                    || weaponType == WEAPON_PARACHUTE
                    || CWeaponInfo::GetWeaponInfo((eWeaponType)weaponType, 1)->m_nWeaponFire == 5
                    || CWeaponInfo::GetWeaponInfo((eWeaponType)weaponType, 1)->m_nSlot <= 1) {
                }
                else {
                    CFontNew::SetBackground(false);
                    CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
                    CFontNew::SetWrapX(SCREEN_WIDTH);
                    CFontNew::SetClipX(SCREEN_WIDTH);
                    CFontNew::SetFontStyle(CFontNew::FONT_4);
                    CFontNew::SetAlignment(CFontNew::ALIGN_CENTER);
                    CFontNew::SetDropColor(CRGBA(0, 0, 0, 255));
                    CFontNew::SetDropShadow(0.0f);
                    CFontNew::SetOutline(SCREEN_MULTIPLIER(1.5f));
                    CFontNew::SetScale(SCREEN_MULTIPLIER(0.58f), SCREEN_MULTIPLIER(1.30f));

                    if (str_ammo && str_clip) {
                        CFontNew::SetColor(HudColourNew.GetRGB(HUD_COLOUR_WHITE, 255));
                        CFontNew::PrintString(SCREEN_COORD_CENTER_X + SCREEN_COORD(ax) - (CFontNew::GetStringWidth(str_clip, true) * 0.5f), SCREEN_COORD_CENTER_Y + SCREEN_COORD(ay), str_ammo);

                        CFontNew::SetColor(HudColourNew.GetRGB(HUD_COLOUR_GREY, 255));
                        CFontNew::PrintString(SCREEN_COORD_CENTER_X + SCREEN_COORD(ax) + (CFontNew::GetStringWidth(str_ammo, true) * 0.5f), SCREEN_COORD_CENTER_Y + SCREEN_COORD(ay), str_clip);
                    }
                }
            }
        }

        // *Extra* Parachute
        bool hasPara = playa->DoWeHaveWeaponAvailable(WEAPON_PARACHUTE);
        if (hasPara) {
            WheelSprite[WHEEL_EXTRA]->Draw(SCREEN_COORD_CENTER_X + SCREEN_COORD(x + 202.0f), SCREEN_COORD_CENTER_Y + SCREEN_COORD(y + 202.0f), SCREEN_COORD(100.0f), SCREEN_COORD(100.0f), CRGBA(0, 0, 0, 50));
            ExtraSprite[WEXTRA_PARA]->Draw(SCREEN_COORD_CENTER_X + SCREEN_COORD(x + 198.0f), SCREEN_COORD_CENTER_Y + SCREEN_COORD(y + 198.0f), SCREEN_COORD(106.0f), SCREEN_COORD(106.0f), CRGBA(255, 255, 255, 255));
        }

        // Weapon name
        int weap = nArrayOfAvailableWeapons[nSelectedSlot][nSelectedWeapon[nSelectedSlot]];
        if (nSelectedWeapon[nSelectedSlot] != -1 && weap != -1 && nActiveWeapon[nActiveSlot] != -1) {
            CWeaponWheel* selected_wep = WeaponWheel[nSelectedSlot][nSelectedWeapon[nSelectedSlot]];
            CWeaponWheel* active_wep = WeaponWheel[nActiveSlot][nActiveWeapon[nActiveSlot]];

            if (active_wep && selected_wep)
                DrawStats(active_wep->id, selected_wep->id);

            CFontNew::SetBackground(false);
            CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
            CFontNew::SetWrapX(SCREEN_WIDTH);
            CFontNew::SetClipX(SCREEN_WIDTH);
            CFontNew::SetFontStyle(CFontNew::FONT_4);
            CFontNew::SetAlignment(CFontNew::ALIGN_CENTER);
            CFontNew::SetDropColor(CRGBA(0, 0, 0, 255));
            CFontNew::SetColor(HudColourNew.GetRGB(HUD_COLOUR_WHITE, 255));
            CFontNew::SetDropShadow(0.0f);
            CFontNew::SetOutline(SCREEN_MULTIPLIER(1.5f));
            CFontNew::SetScale(SCREEN_MULTIPLIER(0.58f), SCREEN_MULTIPLIER(1.28f));

            if (selected_wep && nSelectedWeapon[nSelectedSlot] != -1) {
                CFontNew::SetScale(SCREEN_MULTIPLIER(0.64f), SCREEN_MULTIPLIER(1.42f));
                CFontNew::PrintString(SCREEN_COORD_CENTER_X + SCREEN_COORD(x + 2.0f), SCREEN_COORD_CENTER_Y + SCREEN_COORD(y - 120.0f), TextNew.GetText(selected_wep->name).text);
            }

            if (nNumWeaponsAvailableInSlot[nSelectedSlot] > 1) {
                int max = nNumWeaponsAvailableInSlot[nSelectedSlot];
                int curr = nNumSelectedWeaponAvailableInSlot[nSelectedSlot];

                sprintf(gString, "%d    /    %d", curr, max);
                CFontNew::PrintString(SCREEN_COORD_CENTER_X + SCREEN_COORD(x + 2.0f), SCREEN_COORD_CENTER_Y + SCREEN_COORD(y - 82.0f), gString);

                DrawSpriteWithBorder(WheelSprite[WHEEL_ARROW_LEFT], SCREEN_COORD_CENTER_X + SCREEN_COORD((x + 2.0f) - (46.0f / 2) - 32.0f) - (CFontNew::GetStringWidth(gString, true) * 0.5f), SCREEN_COORD_CENTER_Y + SCREEN_COORD(y - 89.0f), SCREEN_COORD(46.0f), SCREEN_COORD(46.0f), SCREEN_COORD(1.5f), HudColourNew.GetRGB(VHud::Settings.UIMainColor, 255));
                DrawSpriteWithBorder(WheelSprite[WHEEL_ARROW_RIGHT], SCREEN_COORD_CENTER_X + SCREEN_COORD((x + 2.0f) - (46.0f / 2) + 32.0f) + (CFontNew::GetStringWidth(gString, true) * 0.5f), SCREEN_COORD_CENTER_Y + SCREEN_COORD(y - 89.0f), SCREEN_COORD(46.0f), SCREEN_COORD(46.0f), SCREEN_COORD(1.5f), HudColourNew.GetRGB(VHud::Settings.UIMainColor, 255));
            }

            float offset = 0.0f;
            CWeaponInfo* selectedInfo = CWeaponInfo::GetWeaponInfo((eWeaponType)(selected_wep->id), playa->GetWeaponSkill((eWeaponType)selected_wep->id));


            CFontNew::Details.color.a = FadeIn(255);
            CFontNew::Details.dropColor.a = FadeIn(255);
            if (selectedInfo->m_nFlags.bTwinPistol) {
                CFontNew::SetScale(SCREEN_MULTIPLIER(0.58f), SCREEN_MULTIPLIER(1.28f));

                CFontNew::PrintString(SCREEN_COORD_CENTER_X + SCREEN_COORD(x + 2.0f), SCREEN_COORD_CENTER_Y + SCREEN_COORD(y - 47.0f), TextNew.GetText("DOUBLEW").text);
                offset += 30.0f;
            }
            int slot = playa->GetWeaponSlot((eWeaponType)selected_wep->id);
            int ammo = playa->m_aWeapons[slot].m_nTotalAmmo - playa->m_aWeapons[slot].m_nAmmoInClip;
            if (ammo > 9999) {
                CFontNew::SetScale(SCREEN_MULTIPLIER(0.58f), SCREEN_MULTIPLIER(1.28f));
                CFontNew::PrintString(SCREEN_COORD_CENTER_X + SCREEN_COORD(x + 2.0f), SCREEN_COORD_CENTER_Y + SCREEN_COORD(y - 47.0f + offset), TextNew.GetText("UNLIMIT").text);
            }
        }

        previousWeaponSelectedShit = nSelectedWeapon[nSelectedSlot];
        previousSlotSelectedShit = nSelectedSlot;
    }
}

int CWeaponSelector::FadeIn(int a) {
    return min(nWeaponExtraFadeAlpha, a);
}

float CWeaponSelector::GetShiftOffsetForStatsBox() {
    float y = SCREEN_COORD(8.0f);

    if (FindPlayerWanted(-1)->m_nWantedLevel > 0)
        y += SCREEN_COORD(8.0f) + SCREEN_COORD(GET_SETTING(HUD_WANTED_STARS).h);

    if (CHudNew::IsMoneyCounterDisplaying())
        y += SCREEN_COORD(8.0f) + CFontNew::GetHeightScale(SCREEN_MULTIPLIER(GET_SETTING(HUD_CASH).h));

    if (CHudNew::IsMoneyDifferenceDisplaying())
        y += SCREEN_COORD(8.0f) + CFontNew::GetHeightScale(SCREEN_MULTIPLIER(GET_SETTING(HUD_CASH).h));

    if (CHudNew::IsAmmoCounterDisplaying())
        y += SCREEN_COORD(8.0f) + CFontNew::GetHeightScale(SCREEN_MULTIPLIER(GET_SETTING(HUD_AMMO).h));

    return y;
}

void CWeaponSelector::DrawStats(int active_id, int selected_id) {
    CPed* playa = FindPlayerPed(-1);

    if (!playa || nSelectedSlot == 0)
        return;

    float x = GET_SETTING(HUD_WEAPON_STATS_BOX).x;
    float y = GET_SETTING(HUD_WEAPON_STATS_BOX).y;
    float w = GET_SETTING(HUD_WEAPON_STATS_BOX).w;
    float h = GET_SETTING(HUD_WEAPON_STATS_BOX).h;
    CRGBA col = GET_SETTING(HUD_WEAPON_STATS_BOX).col;

    static float heightLerp = 0.0f;
    heightLerp = interpF(heightLerp, GetShiftOffsetForStatsBox(), 0.8f * CTimer::ms_fTimeStep);
    CHudNew::DrawSimpleRect(CRect(HUD_RIGHT(x), HUD_Y(y) + heightLerp, HUD_RIGHT(x) + SCREEN_COORD(w), HUD_Y(y) + heightLerp + SCREEN_COORD(h)), col);

    x = x - GET_SETTING(HUD_WEAPON_STATS_BAR).x;
    y = y + GET_SETTING(HUD_WEAPON_STATS_BAR).y;
    w = GET_SETTING(HUD_WEAPON_STATS_BAR).w;
    h = GET_SETTING(HUD_WEAPON_STATS_BAR).h;

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

    CWeaponInfo* selectedInfo = CWeaponInfo::GetWeaponInfo((eWeaponType)(selected_id), playa->GetWeaponSkill((eWeaponType)selected_id));
    float selectedDamage = ((float)(selectedInfo->m_nDamage)) / 150.0f;
    float selectedfireRate = 0.0f;
    float selectedAccuracy = selectedInfo->m_fAccuracy / 2.0f;
    float selectedRange = selectedInfo->m_fTargetRange / 100.0f;

    CWeaponInfo* activeInfo = CWeaponInfo::GetWeaponInfo((eWeaponType)(active_id), playa->GetWeaponSkill((eWeaponType)active_id));
    float activeDamage = ((float)(activeInfo->m_nDamage)) / 150.0f;
    float activefireRate = 0.0f;
    float activeAccuracy = activeInfo->m_fAccuracy / 2.0f;
    float activeRange = activeInfo->m_fTargetRange / 100.0f;

    switch (playa->GetWeaponSkill((eWeaponType)selected_id)) {
    case 0:
        selectedfireRate = WeaponStat[selected_id].skill1 / fStatMaxRate;
        break;
    case 1:
        selectedfireRate = WeaponStat[selected_id].skill2 / fStatMaxRate;
        break;
    case 2:
        selectedfireRate = WeaponStat[selected_id].skill3 / fStatMaxRate;
        break;
    }

    switch (playa->GetWeaponSkill((eWeaponType)active_id)) {
    case 0:
        activefireRate = WeaponStat[active_id].skill1 / fStatMaxRate;
        break;
    case 1:
        activefireRate = WeaponStat[active_id].skill2 / fStatMaxRate;
        break;
    case 2:
        activefireRate = WeaponStat[active_id].skill3 / fStatMaxRate;
        break;
    }

    float selected[] = {
        selectedDamage,
        selectedfireRate,
        selectedAccuracy,
        selectedRange,
    };

    float active[] = {
        activeDamage,
        activefireRate,
        activeAccuracy,
        activeRange,
    };

    char* statName[] = {
        "DAMAGE",
        "FIRERAT",
        "ACCURAC",
        "RANGE",
    };

    float spacing = 0.0f;
    for (int i = 0; i < 4; i++) {
        CRGBA col;

        fStatsProgress[i] = interpF(fStatsProgress[i], (active[i]), 0.4f * CTimer::ms_fTimeStep);
        fStatsDiff[i] = interpF(fStatsDiff[i], (selected[i] - active[i]), 0.4f * CTimer::ms_fTimeStep);

        if (nActiveSlot == 0) {
            fStatsProgress[i] = interpF(fStatsProgress[i], (selected[i]), 0.4f * CTimer::ms_fTimeStep);
            fStatsDiff[i] = 0.0f;
        }

        if (fStatsDiff[i] > 0.0f)
            col = HudColourNew.GetRGB(HUD_COLOUR_BLUE, 255);
        else
            col = HudColourNew.GetRGB(HUD_COLOUR_RED, 255);

        DrawProgressBarWithProgressDifference(HUD_RIGHT(x), HUD_Y(spacing + y) + heightLerp, SCREEN_COORD(w), SCREEN_COORD(h), fStatsProgress[i], GET_SETTING(HUD_WEAPON_STATS_BAR).col, fStatsDiff[i], col);
        CFontNew::PrintString(HUD_RIGHT(x), HUD_Y(spacing + (y - 28.0f)) + heightLerp, TextNew.GetText(statName[i]).text);
        spacing += 42.0f;
    }

    //CFontNew::SetAlignment(CFontNew::ALIGN_LEFT);
    //CFontNew::PrintString(0, 0, (char*)plugin::Format("active id:%d selected id:%d", active_id, selected_id).c_str());
    //CFontNew::PrintString(0, 40, (char*)plugin::Format("active dmg:%f selected dmg:%f", activeDamage, selectedDamage).c_str());
    //CFontNew::PrintString(0, 80, (char*)plugin::Format("active fireRate:%f selected dmg:%f", activefireRate, selectedfireRate).c_str());
    //CFontNew::PrintString(0, 120, (char*)plugin::Format("active accuracy:%f selected accuracy:%f", activeAccuracy, selectedAccuracy).c_str());
    //CFontNew::PrintString(0, 160, (char*)plugin::Format("active range:%f selected range:%f", activeRange, selectedRange).c_str());
}

CWeaponWheel::CWeaponWheel() {
    sprite = new CSprite2d();
    id = -1;
    strcpy(name, "NONE");
    strcpy(tex, "NONE");
}

CWeaponWheel::~CWeaponWheel() {
    delete sprite;
}

CWeaponStat::CWeaponStat() {
    id = -1;
    strcpy(name, "NONE");
    skill1 = 0.0f;
    skill2 = 0.0f;
    skill3 = 0.0f;
}

CWeaponCrosshair::CWeaponCrosshair() {
    strcpy(name, "");
}