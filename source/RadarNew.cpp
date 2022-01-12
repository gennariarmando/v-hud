#include "VHud.h"
#include "RadarNew.h"
#include "HudColoursNew.h"
#include "Utility.h"
#include "GPS.h"
#include "TextureMgr.h"
#include "HudNew.h"
#include "OverlayLayer.h"
#include "PadNew.h"
#include "resource.h"
#include "MenuNew.h"
#include "CellPhone.h"

#include "CCamera.h"
#include "CRadar.h"
#include "CMenuManager.h"
#include "CStreaming.h"
#include "CTimer.h"
#include "CScene.h"
#include "CPedIntelligence.h"
#include "CHud.h"
#include "CWorld.h"
#include "CStats.h"
#include "CTheScripts.h"
#include "CEntryExit.h"
#include "CEntryExitManager.h"
#include "C3dMarker.h"
#include "C3dMarkers.h"
#include "CGeneral.h"
#include "CPickups.h"
#include "eModelID.h"

#include <d3d9.h>
#include <d3d9types.h>
#include <d3d9caps.h>

using namespace plugin;

CRadarNew RadarNew;

CSprite2d* CRadarNew::m_RadarSprites[NUM_RADAR_SPRITES];
CSprite2d* CRadarNew::m_BlipsSprites[NUM_BLIPS_SPRITES];
CSprite2d* CRadarNew::m_MiniMapSprites[12 * 12];
CSprite2d* CRadarNew::m_PickupsSprites[NUM_PICKUPS_BLIPS_SPRITES];
CRadarAnim CRadarNew::Anim;
CVector2D CRadarNew::m_vRadarMapQuality;

CBlip CRadarNew::m_BlipsList[NUM_BLIPS_SPRITES];
int CRadarNew::m_BlipsCount;
bool CRadarNew::m_bInitialised;
RwCamera* CRadarNew::m_pCamera;
RwFrame* CRadarNew::m_pFrame;
RwRaster* CRadarNew::m_pFrameBuffer1;
RwRaster* CRadarNew::m_pFrameBuffer2;
RwRaster* CRadarNew::m_pFrameBuffer3;
int CRadarNew::m_nMapLegendBlipCount;
CRadarLegend CRadarNew::m_MapLegendBlipList[175];
unsigned int CRadarNew::m_nActualTraceIdToPass;
bool CRadarNew::m_bCopPursuit;
bool CRadarNew::m_b3dRadar;
int CRadarNew::m_nRadarRangeExtendTime;
bool CRadarNew::m_bRemoveBlipsLimit;

bool bShowWeaponPickupsOnRadar = false;

const float World = 6000.0f;

void* radar_gps_alpha_mask_fxc;
void* multi_alpha_mask_fxc;

const char* RadarSpriteNames[] = {
     "radar_rect",
     "radar_rect_2p",
     "radar_plane_mask",
     "radar_plane",
     "radar_damage",
     "radar_mask",
};

const char* PickupsBlipsFileNames[]{
    "pickup_unknown",
    "pickup_weapon_armour",
    "pickup_weapon_assault_rifle",
    "pickup_weapon_bat",
    "pickup_weapon_down",
    "pickup_weapon_grenadelauncher",
    "pickup_weapon_grenades",
    "pickup_weapon_health",
    "pickup_weapon_knife",
    "pickup_weapon_molotov",
    "pickup_weapon_pipebomb",
    "pickup_weapon_pistol",
    "pickup_weapon_poolcue",
    "pickup_weapon_rocket",
    "pickup_weapon_shotgun",
    "pickup_weapon_smg",
    "pickup_weapon_sniper",
    "pickup_weapon_stickybomb",
    "pickup_weapon_up",
};

CRadarNew::CRadarNew() {
    patch::RedirectJump(0x583480, TransformRadarPointToScreenSpace);
    patch::RedirectJump(0x583530, TransformRealWorldPointToRadarSpace);
    patch::RedirectJump(0x5832F0, LimitRadarPoint);
    patch::RedirectJump(0x585FF0, (void(__cdecl *)(unsigned short, float, float, unsigned char))DrawRadarSprite);
    patch::RedirectCall(0x58563E, TransformRadarPoint); // Gang overlay
    patch::RedirectCall(0x586408, TransformRadarPoint);
    //patch::RedirectJump(0x583350, LimitToMap);
    patch::RedirectJump(0x5835A0, TransformRadarPointToRealWorldSpace); 

    patch::RedirectJump(0x584070, ShowRadarTraceWithHeight);
    // patch::RedirectJump(0x5859F0, AddBlipToLegendList);
    patch::PutRetn(0x5859F0);

    patch::RedirectJump(0x585040, ClipRadarPoly);
    patch::RedirectJump(0x583670, CalculateCachedSinCos);

    patch::RedirectJump(0x584770, GetRadarTraceColour);
}

void CRadarNew::Init() {
    if (m_bInitialised)
        return;

    ReadBlipsFromFile();

    for (int i = 0; i < NUM_RADAR_SPRITES; i++) {
        m_RadarSprites[i] = new CSprite2d();
        m_RadarSprites[i]->m_pTexture = CTextureMgr::LoadDDSTextureCB(PLUGIN_PATH("VHud\\radar"), RadarSpriteNames[i]);
    }

    for (int i = 0; i < m_BlipsCount; i++) {
        m_BlipsSprites[i] = new CSprite2d();
        m_BlipsSprites[i]->m_pTexture = CTextureMgr::LoadDDSTextureCB(PLUGIN_PATH("VHud\\blips"), m_BlipsList[i].texName);
    }

    for (int i = 0; i < 12 * 12; i++) {
        char name[32];
        sprintf(name, "radar_%02d", i + 1);
        m_MiniMapSprites[i] = new CSprite2d();
        m_MiniMapSprites[i]->m_pTexture = CTextureMgr::LoadDDSTextureCB(PLUGIN_PATH("VHud\\map"), name);
    }

    if (m_MiniMapSprites[0] && m_MiniMapSprites[0]->m_pTexture) {
        m_vRadarMapQuality.x = m_MiniMapSprites[0]->m_pTexture->raster->width;
        m_vRadarMapQuality.y = m_MiniMapSprites[0]->m_pTexture->raster->height;
    }

    for (int i = 0; i < NUM_PICKUPS_BLIPS_SPRITES; i++) {
        m_PickupsSprites[i] = new CSprite2d();
        m_PickupsSprites[i]->m_pTexture = CTextureMgr::LoadDDSTextureCB(PLUGIN_PATH("VHud\\pickups"), PickupsBlipsFileNames[i]);
    }

    CreateCamera();

    radar_gps_alpha_mask_fxc = CreatePixelShaderFromResource(IDR_RADAR_GPS_ALPHA_MASK);
    multi_alpha_mask_fxc = CreatePixelShaderFromResource(IDR_MULTI_ALPHA_MASK);

    m_bInitialised = true;
}

void CRadarNew::Shutdown() {
    if (!m_bInitialised)
        return;

    for (int i = 0; i < NUM_RADAR_SPRITES; i++) {
        if (m_RadarSprites[i]) {
            m_RadarSprites[i]->Delete();
            delete m_RadarSprites[i];
        }
    }

    for (int i = 0; i < NUM_BLIPS_SPRITES; i++) {
        if (m_BlipsSprites[i]) {
            m_BlipsSprites[i]->Delete();
            delete m_BlipsSprites[i];
        }
    }

    for (int i = 0; i < 12 * 12; i++) {
        if (m_MiniMapSprites[i]) {
            m_MiniMapSprites[i]->Delete();
            delete m_MiniMapSprites[i];
        }
    }

    for (int i = 0; i < NUM_PICKUPS_BLIPS_SPRITES; i++) {
        if (m_PickupsSprites[i]) {
            m_PickupsSprites[i]->Delete();
            delete m_PickupsSprites[i];
        }
    }

    DestroyCamera();

    m_bInitialised = false;
}

void CRadarNew::Clear() {
    m_bCopPursuit = false;
    m_b3dRadar = false;
    m_nRadarRangeExtendTime = 0;
    m_bRemoveBlipsLimit = false;
}

void CRadarNew::ReadBlipsFromFile() {
    std::ifstream file(PLUGIN_PATH("VHud\\data\\blips.dat"));

    m_BlipsCount = 0;
    if (file.is_open()) {
        for (std::string line; getline(file, line);) {
            char name[64];
            char col[64];

            if (!line[0] || line[0] == '#' || line[0] == '[')
                continue;

            sscanf(line.c_str(), "%s %s", &name, &col);

            strcpy(m_BlipsList[m_BlipsCount].texName, name);

            if (!faststrcmp(col, "-"))
                m_BlipsList[m_BlipsCount].color = HudColourNew.GetRGBA(MenuNew.Settings.uiMainColor);
            else
                m_BlipsList[m_BlipsCount].color = HudColourNew.GetRGBA(col);
            m_BlipsCount++;
        }

        file.close();
    }
}

void CRadarNew::CreateCamera() {
    m_pCamera = RwCameraCreate();
    m_pFrame = RwFrameCreate();
    m_pFrameBuffer1 = RwRasterCreate(m_vRadarMapQuality.x, m_vRadarMapQuality.y, 0, rwRASTERTYPECAMERATEXTURE);
    m_pFrameBuffer2 = RwRasterCreate(m_vRadarMapQuality.x, m_vRadarMapQuality.y, 0, rwRASTERTYPECAMERATEXTURE);
    m_pFrameBuffer3 = RwRasterCreate(m_vRadarMapQuality.x, m_vRadarMapQuality.y, 0, rwRASTERTYPECAMERATEXTURE);
    rwObjectHasFrameSetFrame(m_pCamera, m_pFrame);
    RwCameraSetProjection(m_pCamera, rwPARALLEL);
    RwCameraSetNearClipPlane(m_pCamera, 0.05f);
    RwCameraSetFarClipPlane(m_pCamera, 1.0f);
}

void CRadarNew::DestroyCamera() {
    if (m_pFrameBuffer1)
        RwRasterDestroy(m_pFrameBuffer1);

    if (m_pFrameBuffer2)
        RwRasterDestroy(m_pFrameBuffer2);

    if (m_pFrameBuffer3)
        RwRasterDestroy(m_pFrameBuffer3);

    if (m_pFrame)
        RwFrameDestroy(m_pFrame);

    if (m_pCamera) {
        m_pCamera->frameBuffer = NULL;
        m_pCamera->zBuffer = NULL;
        //RwCameraDestroy(m_pCamera);
        m_pCamera = NULL;
    }
}

unsigned int CRadarNew::GetRadarTraceColour(unsigned int c, bool bright, bool friendly) {
    CRGBA color;
    switch (c) {
    case TRACE_COLOUR_RED:
        color = HudColourNew.GetRGB(HUD_COLOUR_RED, 255);
        break;
    case TRACE_COLOUR_GREEN:
        color = HudColourNew.GetRGB(HUD_COLOUR_GREEN, 255);
        break;
    case TRACE_COLOUR_BLUE:
        color = HudColourNew.GetRGB(HUD_COLOUR_BLUEDARK, 255);
        break;
    case TRACE_COLOUR_WHITE:
        color = HudColourNew.GetRGB(HUD_COLOUR_WHITE, 255);
        break;
    case TRACE_COLOUR_YELLOW:
        color = HudColourNew.GetRGB(HUD_COLOUR_YELLOW, 255);
        break;
    case TRACE_COLOUR_PURPLE:
        color = HudColourNew.GetRGB(HUD_COLOUR_PURPLE, 255);
        break;
    case TRACE_COLOUR_CYAN:
        color = HudColourNew.GetRGB(HUD_COLOUR_BLUELIGHT, 255);
        break;
    case TRACE_COLOUR_THREAT:
        if (friendly)
            color = HudColourNew.GetRGB(HUD_COLOUR_BLUE, 255);
        else
            color = HudColourNew.GetRGB(HUD_COLOUR_RED, 255);
        break;
    case TRACE_COLOUR_DESTINATION:
    default:
        color = HudColourNew.GetRGB(HUD_COLOUR_YELLOW, 255);
        break;
    }

    return color.ToInt();
}

void CRadarNew::DrawBlips() {
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)(FALSE));
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)(FALSE));
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)(TRUE));
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)(rwBLENDSRCALPHA));
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)(rwBLENDINVSRCALPHA));
    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)(FALSE));

    m_nActualTraceIdToPass = -1;
    m_nMapLegendBlipCount = 0;
    AddBlipToLegendList(false, RADAR_SPRITE_MAP_HERE);

    CRadarNew::DrawRadarCop();

    tRadarTrace* trace = CRadar::ms_RadarTrace;
    for (int i = 0; i < 175; i++) {
        if (!trace[i].m_bTrackingBlip)
            continue;

        switch (trace[i].m_nBlipType) {
        case BLIP_COORD:
        case BLIP_CONTACTPOINT:
            if (LOBYTE(trace[i].m_nBlipSprite) == RADAR_SPRITE_WAYPOINT || DisplayThisBlip(HIBYTE(trace[i].m_nBlipSprite), i) || LOBYTE(trace[i].m_nBlipSprite) != RADAR_SPRITE_NONE)
                DrawCoordBlip(i, trace[i].m_nBlipSprite != RADAR_SPRITE_NONE);
            break;
        case BLIP_CAR:
        case BLIP_CHAR:
        case BLIP_OBJECT:
        case BLIP_PICKUP:
            if (DisplayThisBlip(HIBYTE(trace[i].m_nBlipSprite), i) || LOBYTE(trace[i].m_nBlipSprite) != RADAR_SPRITE_NONE)
                DrawEntityBlip(i, trace[i].m_nBlipSprite != RADAR_SPRITE_NONE);
            break;
        case BLIP_SPOTLIGHT:
        case BLIP_AIRSTRIP:
            if (!CTheScripts::bPlayerIsOffTheMap && !MenuNew.bDrawMenuMap)
                DrawEntityBlip(i, 1);
            break;
        }
    }

    if (bShowWeaponPickupsOnRadar)
        DrawPickupBlips();

    if (MenuNew.bDrawMenuMap) {
        CVector2D in = FindPlayerCentreOfWorld_NoInteriorShift(0);
        CVector2D out;
        TransformRealWorldPointToRadarSpace(out, in);
        in = out;
        TransformRadarPointToScreenSpace(out, in);

        float angle = FindPlayerHeading(0) - M_PI;

        if (FLASH_ITEM(1200, 200))
            DrawRotatingRadarSprite(m_BlipsSprites[RADAR_SPRITE_CENTRE], out.x, out.y, angle, SCREEN_COORD(GET_SETTING(HUD_RADAR_BLIPS_SIZE).w + 1.0f), SCREEN_COORD(GET_SETTING(HUD_RADAR_BLIPS_SIZE).h + 1.0f), HudColourNew.GetRGB(MenuNew.Settings.uiMainColor, 255));
    }
    else {
        CVector2D in, out;

        // Draw radar north
        CVector2D vec2d;
        vec2d.x = CRadar::vec2DRadarOrigin.x;
        vec2d.y = (M_PI * 2) * CRadar::m_radarRange + CRadar::vec2DRadarOrigin.y;
        TransformRealWorldPointToRadarSpace(in, vec2d);
        LimitRadarPoint(in);
        TransformRadarPointToScreenSpace(out, in);
        DrawRadarSprite(RADAR_SPRITE_NORTH, out.x, out.y, 255);

        // Draw radar centre.
        CVector2D centreWorld = FindPlayerCentreOfWorld_NoInteriorShift(-1);
        TransformRealWorldPointToRadarSpace(in, centreWorld);
        LimitRadarPoint(in);
        TransformRadarPointToScreenSpace(out, in);
        
        float angle = FindPlayerHeading(0) - (CRadar::m_fRadarOrientation + M_PI);
        DrawRotatingRadarSprite(m_BlipsSprites[RADAR_SPRITE_CENTRE], out.x, out.y, angle, SCREEN_COORD(GET_SETTING(HUD_RADAR_BLIPS_SIZE).w + 1.0f), SCREEN_COORD(GET_SETTING(HUD_RADAR_BLIPS_SIZE).h + 1.0f), CRGBA(255, 255, 255, 255));
    }
}

void CRadarNew::DrawEntityBlip(int id, bool sprite) {
    m_nActualTraceIdToPass = id;
    CRadar::DrawEntityBlip(id, sprite);
}

void CRadarNew::DrawCoordBlip(int id, bool sprite) {
    m_nActualTraceIdToPass = id;
    CRadar::DrawCoordBlip(id, sprite);
}

void CRadarNew::DrawPickupBlips() {
    if (MenuNew.bDrawMenuMap)
        return;

    for (int i = 0; i < MAX_NUM_PICKUPS; i++) {
        CPickup p = CPickups::aPickUps[i];
        int s = -1;

        if (p.m_nReferenceIndex && p.m_nModelIndex && !p.m_nFlags.bDisabled && p.IsVisible()) {
            switch (p.m_nModelIndex) {
            case MODEL_GUN_DILDO1:
            case MODEL_GUN_DILDO2:
            case MODEL_GUN_VIBE1:
            case MODEL_GUN_VIBE2:
                s = PICKUP_WEAPON_PIPEBOMB;
                break;
            case MODEL_FLOWERA:
            case MODEL_GUN_CANE:
            case MODEL_GUN_BOXWEE:
            case MODEL_GUN_BOXBIG:
            case MODEL_CELLPHONE:
            case MODEL_BRASSKNUCKLE:
            case MODEL_GOLFCLUB:
            case MODEL_NITESTICK:
            case MODEL_TEARGAS:
            case MODEL_SHOVEL:
            case MODEL_POOLCUE:
            case MODEL_KATANA:
            case MODEL_CHNSAW:
            case MODEL_SATCHEL:
                s = PICKUP_WEAPON_STICKYBOMB;
                break;
            case MODEL_BOMB:
            case MODEL_SPRAYCAN:
            case MODEL_FIRE_EX:
            case MODEL_CAMERA:
            case MODEL_NVGOGGLES:
            case MODEL_IRGOGGLES:
            case MODEL_JETPACK:
            case MODEL_GUN_PARA:
            case MODEL_ADRENALINE:
            case MODEL_MINIGUN:
                s = PICKUP_UNKNOWN;
                break;
            case MODEL_KNIFECUR:
                s = PICKUP_WEAPON_KNIFE;
                break;
            case MODEL_BAT:
                s = PICKUP_WEAPON_BAT;
                break;
            case MODEL_GRENADE:
                s = PICKUP_WEAPON_GRENADES;
                break;
            case MODEL_MOLOTOV:
                s = PICKUP_WEAPON_MOLOTOV;
                break;
            case MODEL_ROCKETLA:
            case MODEL_HEATSEEK:
                s = PICKUP_WEAPON_ROCKET;
                break;
            case MODEL_COLT45:
            case MODEL_SILENCED:
            case MODEL_DESERT_EAGLE:
                s = PICKUP_WEAPON_PISTOL;
                break;
            case MODEL_CHROMEGUN:
            case MODEL_SAWNOFF:
            case MODEL_SHOTGSPA:
            case MODEL_CUNTGUN:
                s = PICKUP_WEAPON_SHOTGUN;
                break;
            case MODEL_MICRO_UZI:
            case MODEL_MP5LNG:
            case MODEL_TEC9:
                s = PICKUP_WEAPON_SMG;
                break;
            case MODEL_FLARE:
                break;
            case MODEL_AK47:
            case MODEL_M4:
                s = PICKUP_WEAPON_ASSAULT_RIFLE;
                break;
            case MODEL_SNIPER:
            case MODEL_FLAME:
                s = PICKUP_WEAPON_SNIPER;
                break;
            case MODEL_ARMOUR:
            case MODEL_BODYARMOUR:
                s = PICKUP_WEAPON_ARMOUR;
                break;
            case MODEL_HEALTH:
                s = PICKUP_WEAPON_HEALTH;
                break;
            default:
                continue;
            }

            if (s != -1) {
                unsigned int savedFilter;
                RwRenderStateGet(rwRENDERSTATETEXTUREFILTER, &savedFilter);
                RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERNEAREST);

                AddAnyBlipNoLegend(m_PickupsSprites[s], p.GetPosn(), SCREEN_COORD(32.0f * 0.7f), SCREEN_COORD(16.0f * 0.7f), M_PI, false,
                    HudColourNew.GetRGB(HUD_COLOUR_WHITE, 255), false);

                RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)savedFilter);
            }
        }
    }
}

void CRadarNew::SetBlipSprite(int i, unsigned short icon) {
    int index = CRadar::GetActualBlipArrayIndex(i);
    if (index != -1) {
        CRadar::ms_RadarTrace[index].m_nBlipSprite = icon;
    }
}

int CRadarNew::CalculateBlipAlpha(float dist) {
    if (dist > 0.5f)
        return 255;
    else
        return (0.5f - dist) * 255;
}

void CRadarNew::DrawRadarCop() {
    if (FindPlayerWanted(0)->m_nWantedLevel < 1)
        return;

    for (int i = CPools::ms_pPedPool->m_nSize; i; i--) {
        CPed* ped = CPools::ms_pPedPool->GetAt(i - 1);

        if (!ped || ped->IsPlayer() || ped->m_fHealth <= 0.0f)
            continue;

        // Is ped
        if (ped->m_nModelIndex == MODEL_LAPD1
            || ped->m_nModelIndex == MODEL_SFPD1
            || ped->m_nModelIndex == MODEL_LVPD1
            || ped->m_nModelIndex == MODEL_CSHER
            || ped->m_nModelIndex == MODEL_LAPDM1
            || ped->m_nModelIndex == MODEL_SWAT
            || ped->m_nModelIndex == MODEL_FBI
            || ped->m_nModelIndex == MODEL_ARMY
            || ped->m_nModelIndex == MODEL_DSHER) {

            CRGBA color = CTimer::m_snTimeInMillisecondsPauseMode % 800 < 400 ? HudColourNew.GetRGB(HUD_COLOUR_REDDARK, 255) : HudColourNew.GetRGB(HUD_COLOUR_BLUEDARK, 255);
            AddAnyBlip(RADAR_SPRITE_COP, *(CEntity*)ped, SCREEN_COORD(GET_SETTING(HUD_RADAR_BLIPS_COP_SIZE).w), SCREEN_COORD(GET_SETTING(HUD_RADAR_BLIPS_COP_SIZE).h), 0.0f, !m_bCopPursuit, color, false);
        }
    }

    for (int i = CPools::ms_pVehiclePool->m_nSize; i; i--) {
        CVehicle* veh = CPools::ms_pVehiclePool->GetAt(i - 1);

        if (!veh || (veh && veh->m_fHealth <= 0.0f) || veh == FindPlayerVehicle(-1, 0))
            continue;

        // Is heli
        if (veh->m_nModelIndex == MODEL_POLMAV) {
            static float angle = 0.0f;
            angle += 0.02f * (M_PI * 1.5f);
            angle = CGeneral::LimitRadianAngle(angle);

            CRGBA color = CTimer::m_snTimeInMillisecondsPauseMode % 800 < 400 ? HudColourNew.GetRGB(HUD_COLOUR_REDDARK, 255) : HudColourNew.GetRGB(HUD_COLOUR_BLUEDARK, 255);
            AddAnyBlip(RADAR_SPRITE_COP_HELI, *(CEntity*)veh, SCREEN_COORD(GET_SETTING(HUD_RADAR_BLIPS_COP_HELI_SIZE).w), SCREEN_COORD(GET_SETTING(HUD_RADAR_BLIPS_COP_HELI_SIZE).h), angle, !m_bCopPursuit, color, false);
        }
    }
}

void CRadarNew::TransformRadarPointToRealWorldSpace(CVector2D& out, CVector2D& in) {
    if (MenuNew.bDrawMenuMap) {
        float w = (World / 2) / World;
        out.x = (in.x - w) * World;
        out.y = (w + in.y) * World;
    }
    else {
        float s = -CRadar::cachedSin;
        float c = CRadar::cachedCos;

        out.x = s * in.y + c * in.x;
        out.y = c * in.y - s * in.x;

        out.x = out.x * CRadar::m_radarRange + CRadar::vec2DRadarOrigin.x;
        out.y = out.y * CRadar::m_radarRange + CRadar::vec2DRadarOrigin.y;
    }
}

void CRadarNew::TransformRealWorldPointToRadarSpace(CVector2D& out, CVector2D& in) {
    if (MenuNew.bDrawMenuMap) {
        out.x = (in.x + World / 2) / World;
        out.y = ((World / 2) - in.y) / World;
    }
    else {
        float s = CRadar::cachedSin;
        float c = CRadar::cachedCos;
        float x = (in.x - CRadar::vec2DRadarOrigin.x) * (1.0f / CRadar::m_radarRange);
        float y = (in.y - CRadar::vec2DRadarOrigin.y) * (1.0f / CRadar::m_radarRange);

        out.x = s * y + c * x;
        out.y = c * y - s * x;
    }
}

void CRadarNew::TransformRadarPointToScreenSpace(CVector2D &out, CVector2D &in) {
    __asm push edx

    if (MenuNew.bDrawMenuMap) {
        float x = in.x;
        float y = in.y;
        x *= MenuNew.GetMenuMapWholeSize();
        y *= MenuNew.GetMenuMapWholeSize();

        x += MenuNew.vMapBase.x - MenuNew.GetMenuMapWholeSize() / 2;
        y += MenuNew.vMapBase.y - MenuNew.GetMenuMapWholeSize() / 2;

        out.x = x;
        out.y = y;
    }
    else {
        out.x = ((GET_SETTING(HUD_RADAR).h / GET_SETTING(HUD_RADAR).w * in.x) * SCREEN_COORD(GET_SETTING(HUD_RADAR).w) + HUD_X(GET_SETTING(HUD_RADAR).x));  
        out.y = ((HUD_BOTTOM(GET_SETTING(HUD_RADAR).y)) - SCREEN_COORD(GET_SETTING(HUD_RADAR).h) * in.y);
    }

    __asm pop edx
}

void CRadarNew::TransformRadarPoint(CVector2D& out, CVector2D& in) {
    __asm push edx

    if (MenuNew.bDrawMenuMap) {
        float x = in.x;
        float y = in.y;
        x *= MenuNew.GetMenuMapWholeSize();
        y *= MenuNew.GetMenuMapWholeSize();

        x += MenuNew.vMapBase.x - MenuNew.GetMenuMapWholeSize() / 2;
        y += MenuNew.vMapBase.y - MenuNew.GetMenuMapWholeSize() / 2;

        out.x = x;
        out.y = y;
    }
    else {
        out.x = ((((GET_SETTING(HUD_RADAR).h / GET_SETTING(HUD_RADAR).w) * in.x) * (m_vRadarMapQuality.x * 0.5f)) + (m_vRadarMapQuality.x * 0.5f));
        out.y = ((m_vRadarMapQuality.y * 0.5f) - (m_vRadarMapQuality.y * 0.5f) * in.y);
    }
    __asm pop edx
}

bool CRadarNew::IsPointTouchingRect(CVector2D& point, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
    float v11;
    float v12;
    float v13;
    float v14;
    float v15;
    float v16;
    float v17;
    float v18;
    float v19;
    float v20;
    float v21;
    float v22;

    if (x2 == x1 && y2 == y1)
        return false;

    if (x4 == x3 && y4 == y3 || x3 == x1 && y3 == y1 || x3 == x2 && y3 == y2 || x4 == x1 && y4 == y1 || x4 == x2 && y4 == y2)
        return false;
    
    v16 = x2 - x1;
    v17 = y2 - y1;
    v18 = x3 - x1;
    v20 = y3 - y1;
    v21 = x4 - x1;
    v22 = y4 - y1;
    v11 = sqrtf(v16 * v16 + v17 * v17);
    v12 = v20 * (v16 / v11) - v18 * (v17 / v11);
    v19 = v17 / v11 * v20 + v16 / v11 * v18;
    v14 = v22 * (v16 / v11) - v21 * (v17 / v11);

    if (v12 < 0.0f && v14 < 0.0f || v12 >= 0.0f && v14 >= 0.0f)
        return false;

    v13 = v17 / v11 * v22 + v16 / v11 * v21;
    v15 = v13 + (v19 - v13) * v14 / (v14 - v12);

    if (v15 < 0.0f)
        return false;

    if (v15 > v11)
        return false;

    point.x = v16 / v11 * v15 + x1;
    point.y = v15 * (v17 / v11) + y1;
    return true;
}

void CRadarNew::LimitPoint(float &x1, float &y1, float x2, float y2, float x3, float y3, float x4, float y4) {
    double v8;
    double v9;
    double v10;
    int i;
    double v12;
    double v13;
    double v14;
    double v15;
    double v16;
    double v17; 
    float v19;
    float v20;
    float v21;
    float v22;
    CVector2D in;

    v8 = y3;
    v9 = x3;
    v10 = y2;
    i = 0;
    v12 = x2;
    v13 = 0.0;
    while (true) {
        switch (i) {
        case 0:
            v14 = v13;
            v19 = v12;
            v20 = v10;
            v22 = v10;
            v21 = v9;
            break;
        case 1:
            v19 = v12;
            v21 = v12;
            v20 = v10;
            v15 = v8;
            v14 = v13;
            v22 = v15;
            break;
        case 2:
            v19 = v12;
            v16 = v8;
            v14 = v13;
            v20 = v16;
            v22 = v16;
            v21 = v9;
            break;
        case 3:
            v19 = v9;
            v21 = v9;
            v20 = v10;
            v17 = v8;
            v14 = v13;
            v22 = v17;
            break;
        default:
            v14 = v13;
            break;
        }

        if (IsPointTouchingRect(in, v19, v20, v21, v22, v14, v14, x4, y4))
            break;

        if (++i >= 4) {
            x1 = x4;
            y1 = y4;
            return;
        }
        v10 = y2;
        v12 = x2;
        v8 = y3;
        v13 = 0.0f;
        v9 = x3;
    }
    x1 = in.x;
    y1 = in.y;
}

float CRadarNew::LimitRadarPoint(CVector2D& point) {
    float v1;
    float v4;
    float* v6;
    float* v7;
    float v8;
    float v9;
    float v10;
    float v12;
    float v13;
    float v14;
    float v15;
    float v16;
    float v17;
    bool v18;
    bool v19;
    float v20;
    float v21;
    float v22;
    float v23;
    float v24;
    float v25;
    float v26;
    float v27;
    bool v28;
    bool v29;
    float v30;
    float v31;
    float v32;
    float v33;
    float v34;
    float v35;
    float v36;
    float v37;
    float v38;
    float v39;
    float v40;
    float v41;
    float v42;
    CPed* playa = FindPlayerPed(0);

    if (m_bRemoveBlipsLimit)
        return 0.0f;

    if (MenuNew.bDrawMenuMap) {
        return point.Magnitude();
    }

    float bx = GET_SETTING(HUD_RADAR_BLIPS_BORDER_SIZE).w;
    float by = GET_SETTING(HUD_RADAR_BLIPS_BORDER_SIZE).h;

    v40 = GET_SETTING(HUD_RADAR).w / GET_SETTING(HUD_RADAR).h;
    v4 = v40 - 1.0f;
    v37 = -1.0f - v4;
    v36 = v4 + 1.0f;
    v38 = 1.0f;
    v39 = -1.0f;

    if (Is3dRadar()) {
        v6 = &point.x;
        v7 = &point.y;

        if (point.x <= v37 || point.x >= v36 || point.y <= -1.0 || *v7 >= 1.0f) {
            LimitPoint(point.x, point.y, v37, 1.0f, v36, -1.0f, point.x, point.y);
        }

        v8 = 0.5f - *v7 * 0.5f;
        v9 = v8 * 2.25f;
        v10 = sinh(v9);
        v12 = v10 / 4.691f;
        v13 = v12;
        v41 = 1.0f - (v12 + v12);
        *v7 = v41;
        v42 = point.x * 1.5f * v13 + point.x;
        v14 = v42;
        point.x = v42;

        v15 = 1.0f - (GET_SETTING(HUD_RADAR).w - bx * v40) / GET_SETTING(HUD_RADAR).w;
        v37 = v15 + v37;
        v36 = v36 - v15;
        v16 = 1.0f - (GET_SETTING(HUD_RADAR).h - by) / GET_SETTING(HUD_RADAR).h;
        v39 = v16 - 1.0f;
        v17 = v14;
        v38 = 1.0f - v16;

        if (v37 >= v17) {
            v20 = v39;
        }
        else {
            v18 = v36 < v17;
            v19 = v36 == v17;
            v20 = v39;
            if (!v18 && !v19) {
                v21 = v38;
                if (v41 > v39 && v41 < v21)
                    return 0.99f;
                goto LABEL_21;
            }
        }
        v21 = v38;
    LABEL_21:
        v35 = *v7;
        v34 = point.x;
        v22 = v20;
        v23 = v37;
        v33 = v22;
        v24 = v36;
        goto LABEL_30;
    }

    v25 = 1.0f - (GET_SETTING(HUD_RADAR).w - bx * v40) / GET_SETTING(HUD_RADAR).w;
    v37 = v25 + v37;
    v36 = v36 - v25;
    v26 = 1.0f - (GET_SETTING(HUD_RADAR).h - by) / GET_SETTING(HUD_RADAR).h;
    v39 = v26 - 1.0f;
    v38 = 1.0f - v26;

    v6 = &point.x;
    v27 = point.x;
    v28 = v37 < v27;
    v29 = v37 == v27;
    v23 = v37;

    if ((v28 || v29) && point.x <= v36 && point.y >= v39 && point.y <= v38)
        return 0.99f;

    v7 = &point.y;
    v35 = point.y;
    v34 = point.x;
    v33 = v39;
    v24 = v36;
    v21 = v38;
LABEL_30:
    v30 = v24;
    v31 = v21;
    v32 = v23;

    LimitPoint(*v6, *v7, v32, v31, v30, v33, v34, v35);

    return 1.1f;
}

void CRadarNew::DrawRadarSprite(unsigned short id, float x, float y, unsigned char alpha) {
    float w = GET_SETTING(HUD_RADAR_BLIPS_SIZE).w;
    float h = GET_SETTING(HUD_RADAR_BLIPS_SIZE).h;

    if (MenuNew.bDrawMenuMap) {
        if (MenuNew.MapCrosshairCheckHover(x - SCREEN_COORD(w), x + SCREEN_COORD(w), y - SCREEN_COORD(h), y + SCREEN_COORD(h)) && MenuNew.nCurrentInputType == MENUINPUT_TAB) {
            w *= RADAR_MENU_BLIP_HOVER_MULT;
            h *= RADAR_MENU_BLIP_HOVER_MULT;
        }
        else {
            w *= RADAR_MENU_BLIP_MULT;
            h *= RADAR_MENU_BLIP_MULT;
        }
    }

    if (DisplayThisBlip(id, -99)) {
        m_BlipsSprites[id]->Draw(CRect(x - SCREEN_COORD(w), y - SCREEN_COORD(h), x + SCREEN_COORD(w), y + SCREEN_COORD(h)), CRGBA(m_BlipsList[id].color.r, m_BlipsList[id].color.g, m_BlipsList[id].color.b, alpha));
        AddBlipToLegendList(false, id);
    }
}

bool CRadarNew::DisplayThisBlip(int spriteId, char priority) {
    return true;
}

void CRadarNew::LimitToMap(float* x, float* y) {

}

void CRadarNew::AddAnyBlipNoLegend(CSprite2d* sprite, CVector posn, float width, float height, float angle, bool vcone, CRGBA const& col, bool limit) {
    float x = 0.0f;
    float y = 0.0f;
    float w = width;
    float h = height;

    static CVector2D in;
    static CVector2D out;
    in = CVector2D(0.0f, 0.0f);
    out = CVector2D(0.0f, 0.0f);
    TransformRealWorldPointToRadarSpace(in, CVector2D(posn.x, posn.y));
    float dist = LimitRadarPoint(in);
    TransformRadarPointToScreenSpace(out, in);

    if (!MenuNew.bDrawMenuMap) {
        if (!limit)
            if (dist > 1.0f)
                return;
    }

    x = out.x;
    y = out.y;

    DrawRotatingRadarSprite(sprite, x, y, angle, w, h, col);
}

void CRadarNew::AddAnyBlip(unsigned short id, CVector posn, float width, float height, float angle, bool vcone, CRGBA const& col, bool limit) {
    float x = 0.0f;
    float y = 0.0f;
    float w = width;
    float h = height;

    static CVector2D in;
    static CVector2D out;
    in = CVector2D(0.0f, 0.0f);
    out = CVector2D(0.0f, 0.0f);
    TransformRealWorldPointToRadarSpace(in, CVector2D(posn.x, posn.y));
    float dist = LimitRadarPoint(in);
    TransformRadarPointToScreenSpace(out, in);

    if (!MenuNew.bDrawMenuMap) {
        if (!limit)
            if (dist > 1.0f)
                return;
    }

    x = out.x;
    y = out.y;

    if (DisplayThisBlip(id, -99)) {
        DrawRotatingRadarSprite(m_BlipsSprites[id], x, y, angle, w, h, col);
        AddBlipToLegendList(false, id);
    }
}


void CRadarNew::AddAnyBlip(unsigned short id, CEntity e, float width, float height, float angle, bool vcone, CRGBA const& col, bool limit) {
    float x = 0.0f;
    float y = 0.0f;
    float w = width;
    float h = height;

    if (MenuNew.bDrawMenuMap) {
        if (MenuNew.MapCrosshairCheckHover(x - SCREEN_COORD(w), x + SCREEN_COORD(w), y - SCREEN_COORD(h), y + SCREEN_COORD(h)) && MenuNew.nCurrentInputType == MENUINPUT_TAB) {
            w *= RADAR_MENU_BLIP_HOVER_MULT;
            h *= RADAR_MENU_BLIP_HOVER_MULT;
        }
        else {
            w *= RADAR_MENU_BLIP_MULT;
            h *= RADAR_MENU_BLIP_MULT;
        }
    }

    static CVector2D in;
    static CVector2D out;
    in = CVector2D(0.0f, 0.0f);
    out = CVector2D(0.0f, 0.0f);
    TransformRealWorldPointToRadarSpace(in, CVector2D(e.GetPosition().x, e.GetPosition().y));
    float dist = LimitRadarPoint(in);
    TransformRadarPointToScreenSpace(out, in);

    if (!MenuNew.bDrawMenuMap) {
        if (!limit)
            if (dist > 1.0f)
                return;
    }

    x = out.x;
    y = out.y;

    if (DisplayThisBlip(id, -99)) {
        if (vcone) {
            float a1 = e.GetHeading();
            float a2 = TheCamera.GetHeading();
            float angle = a1 - (M_PI + a2);
            DrawRotatingRadarSprite(m_BlipsSprites[RADAR_SPRITE_VCONE], x, y, angle, SCREEN_COORD(96.0f), SCREEN_COORD(96.0f), HudColourNew.GetRGB(HUD_COLOUR_BLUEDARK, 60));
        }

        DrawRotatingRadarSprite(m_BlipsSprites[id], x, y, angle, w, h, col);
        AddBlipToLegendList(false, id);
    }
}

void CRadarNew::DrawRadarRectangle() {
    unsigned int savedShade;
    unsigned int savedAlpha;
    unsigned int savedFilter;
    RwRenderStateGet(rwRENDERSTATESHADEMODE, &savedShade);
    RwRenderStateSet(rwRENDERSTATESHADEMODE, (void*)rwSHADEMODEFLAT);
    RwRenderStateGet(rwRENDERSTATEVERTEXALPHAENABLE, &savedAlpha);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
    RwRenderStateGet(rwRENDERSTATETEXTUREFILTER, &savedFilter);
    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERNEAREST);

    CPlayerInfo playa = CWorld::Players[0];
    CPlayerInfo playa2 = CWorld::Players[1];

    // Draw radar rectangle.
    CRect rect;
    CRGBA col = GET_SETTING(HUD_RADAR_FOREGROUND).col;
    CSprite2d* sprite = NULL;

    if (InTwoPlayersMode() && playa2.m_pPed)
        sprite = m_RadarSprites[RADAR_RECT_2P];
    else
        sprite = m_RadarSprites[RADAR_RECT];

    rect.bottom = HUD_BOTTOM(GET_SETTING(HUD_RADAR_FOREGROUND).h);
    rect.right = HUD_X(GET_SETTING(HUD_RADAR_FOREGROUND).w);
    rect.top = HUD_BOTTOM(GET_SETTING(HUD_RADAR_FOREGROUND).y);
    rect.left = HUD_X(GET_SETTING(HUD_RADAR_FOREGROUND).x);

    if (sprite && sprite->m_pTexture != NULL)
        sprite->Draw(rect, col);

    col = GET_SETTING(HUD_RADAR_BACKGROUND).col;
    rect.bottom = HUD_BOTTOM(GET_SETTING(HUD_RADAR_BACKGROUND).h);
    rect.right = HUD_X(GET_SETTING(HUD_RADAR_BACKGROUND).w);
    rect.top = HUD_BOTTOM(GET_SETTING(HUD_RADAR_BACKGROUND).y);
    rect.left = HUD_X(GET_SETTING(HUD_RADAR_BACKGROUND).x);
    
    CSprite2d::DrawRect(rect, col);

    if (InTwoPlayersMode() && playa2.m_pPed) {
        col = GET_SETTING(HUD_RADAR_BACKGROUND_P2).col;
        rect.bottom = HUD_BOTTOM(GET_SETTING(HUD_RADAR_BACKGROUND_P2).h);
        rect.right = HUD_X(GET_SETTING(HUD_RADAR_BACKGROUND_P2).w);
        rect.top = HUD_BOTTOM(GET_SETTING(HUD_RADAR_BACKGROUND_P2).y);
        rect.left = HUD_X(GET_SETTING(HUD_RADAR_BACKGROUND_P2).x);

        CSprite2d::DrawRect(rect, col);
    }

    // Bars
    if (playa.m_pPed) {
        float progress = 0.0f;

        // Health bar
        col = GET_SETTING(HUD_HEALTH_BAR).col;
        if (CHud::m_ItemToFlash != 4 || CTimer::m_FrameCounter & 8) {
            progress = playa.m_pPed->m_fHealth / static_cast<float>(playa.m_nMaxHealth);
            DrawProgressBar(HUD_X(GET_SETTING(HUD_HEALTH_BAR).x), HUD_BOTTOM(GET_SETTING(HUD_HEALTH_BAR).y), SCREEN_COORD(GET_SETTING(HUD_HEALTH_BAR).w), SCREEN_COORD(GET_SETTING(HUD_HEALTH_BAR).h), progress,
                (playa.m_pPed->m_fHealth <= 20.0f && CTimer::m_FrameCounter & 8) ? HudColourNew.GetRGB(HUD_COLOUR_RED, col.a) : col);
        }

        // Armour bar
        col = GET_SETTING(HUD_ARMOUR_BAR).col;
        if (CHud::m_ItemToFlash != 3 || CTimer::m_FrameCounter & 8) {
            progress = playa.m_pPed->m_fArmour / static_cast<float>(playa.m_nMaxArmour);
            DrawProgressBar(HUD_X(GET_SETTING(HUD_ARMOUR_BAR).x), HUD_BOTTOM(GET_SETTING(HUD_ARMOUR_BAR).y), SCREEN_COORD(playa.m_pPed->m_nPhysicalFlags.bSubmergedInWater ? GET_SETTING(HUD_ARMOUR_BAR_B).w : GET_SETTING(HUD_ARMOUR_BAR).w), SCREEN_COORD(GET_SETTING(HUD_ARMOUR_BAR).h), progress,
                col);
        }

        // Breath bar
        col = GET_SETTING(HUD_BREATH_BAR).col;
        if (CHud::m_ItemToFlash != 10 || CTimer::m_FrameCounter & 8) {
            if (playa.m_pPed->m_nPhysicalFlags.bSubmergedInWater) {
                progress = playa.m_pPed->m_pPlayerData->m_fBreath / CStats::GetFatAndMuscleModifier(STAT_MOD_AIR_IN_LUNG);
                DrawProgressBar(HUD_X(GET_SETTING(HUD_BREATH_BAR).x), HUD_BOTTOM(GET_SETTING(HUD_BREATH_BAR).y), SCREEN_COORD(GET_SETTING(HUD_BREATH_BAR).w), SCREEN_COORD(GET_SETTING(HUD_BREATH_BAR).h), progress,
                    col);
            }
        }

        // Second player
        if (InTwoPlayersMode() && playa2.m_pPed) {
            col = GET_SETTING(HUD_HEALTH_BAR).col;

            // Health bar
            if (CHud::m_ItemToFlash != 4 || CTimer::m_FrameCounter & 8) {
                progress = playa2.m_pPed->m_fHealth / static_cast<float>(playa2.m_nMaxHealth);
                DrawProgressBar(HUD_X(GET_SETTING(HUD_HEALTH_BAR_P2).x), HUD_BOTTOM(GET_SETTING(HUD_HEALTH_BAR_P2).y), SCREEN_COORD(GET_SETTING(HUD_HEALTH_BAR_P2).w), SCREEN_COORD(GET_SETTING(HUD_HEALTH_BAR_P2).h), progress,
                    (playa2.m_pPed->m_fHealth <= 20.0f && CTimer::m_FrameCounter & 8) ? HudColourNew.GetRGB(HUD_COLOUR_RED, col.a) : col);
            }

            // Armour bar
            col = GET_SETTING(HUD_ARMOUR_BAR_P2).col;
            if (CHud::m_ItemToFlash != 3 || CTimer::m_FrameCounter & 8) {
                progress = playa2.m_pPed->m_fArmour / static_cast<float>(playa2.m_nMaxArmour);
                DrawProgressBar(HUD_X(GET_SETTING(HUD_ARMOUR_BAR_P2).x), HUD_BOTTOM(GET_SETTING(HUD_ARMOUR_BAR_P2).y), SCREEN_COORD(playa2.m_pPed->m_nPhysicalFlags.bSubmergedInWater ? GET_SETTING(HUD_ARMOUR_BAR_B_P2).w : GET_SETTING(HUD_ARMOUR_BAR_P2).w), SCREEN_COORD(GET_SETTING(HUD_ARMOUR_BAR_P2).h), progress,
                    col);
            }

            // Breath bar
            col = GET_SETTING(HUD_BREATH_BAR_P2).col;
            if (CHud::m_ItemToFlash != 10 || CTimer::m_FrameCounter & 8) {
                if (playa2.m_pPed->m_nPhysicalFlags.bSubmergedInWater) {
                    progress = playa2.m_pPed->m_pPlayerData->m_fBreath / CStats::GetFatAndMuscleModifier(STAT_MOD_AIR_IN_LUNG);
                    DrawProgressBar(HUD_X(GET_SETTING(HUD_BREATH_BAR_P2).x), HUD_BOTTOM(GET_SETTING(HUD_BREATH_BAR_P2).y), SCREEN_COORD(GET_SETTING(HUD_BREATH_BAR_P2).w), SCREEN_COORD(GET_SETTING(HUD_BREATH_BAR_P2).h), progress, 
                        col);
                }
            }
        }
    }

    RwRenderStateSet(rwRENDERSTATESHADEMODE, (void*)savedShade);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)savedAlpha);
    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)savedFilter);
}

void CRadarNew::DrawRotatingRadarSprite(CSprite2d* sprite, float x, float y, float angle, float width, float height, CRGBA color) {
    CVector posn[4];

    if (MenuNew.bDrawMenuMap) {
        if (MenuNew.MapCrosshairCheckHover(x - SCREEN_COORD(width), x + SCREEN_COORD(width), y - SCREEN_COORD(height), y + SCREEN_COORD(height)) && MenuNew.nCurrentInputType == MENUINPUT_TAB) {
            width *= RADAR_MENU_BLIP_HOVER_MULT;
            height *= RADAR_MENU_BLIP_HOVER_MULT;
        }
        else {
            width *= RADAR_MENU_BLIP_MULT;
            height *= RADAR_MENU_BLIP_MULT;
        }
    }

    for (int i = 0; i < 4; i++) {
        const float a = i * M_PI_2 + (angle - M_PI / 4.0f);
        posn[i].x = x + (0.0f * cosf(a) + 1.0f * sinf(a)) * width;
        posn[i].y = y - (0.0f * sinf(a) - 1.0f * cosf(a)) * height;
    }
    sprite->Draw(posn[2].x, posn[2].y, posn[3].x, posn[3].y, 
                posn[1].x, posn[1].y, posn[0].x, posn[0].y, color);
}

void CRadarNew::CalculateCachedSinCos() {
    float s = 0.0f;
    float c = 0.0f;

    CVector v;
    if (MenuNew.bDrawMenuMap) {
        s = 0.0f;
        c = 1.0f;
    }
    else if (TheCamera.GetLookDirection() == 3) {
        if (TheCamera.m_matrix) {
            v = TheCamera.GetMatrix()->up;
            s = c = atan2(-v.x, v.y);
        }
        else
            s = c = TheCamera.GetHeading();
    }
    else {

        if (TheCamera.m_aCams[TheCamera.m_nActiveCam].m_nMode == MODE_1STPERSON) {
            v = TheCamera.m_aCams[TheCamera.m_nActiveCam].m_pCamTargetEntity->GetMatrix()->up;
            v.Normalise();
        }
        else
            v = TheCamera.m_aCams[TheCamera.m_nActiveCam].m_pCamTargetEntity->GetPosition() - TheCamera.m_aCams[TheCamera.m_nActiveCam].m_vecSourceBeforeLookBehind;

        s = c = atan2(-v.x, v.y);
    }

    CRadar::cachedSin = sin(s);
    CRadar::cachedCos = cos(c);
    CRadar::m_fRadarOrientation = s;
}

void CRadarNew::DrawMap() {
    ScanCopPursuit();

    int x = floor((CRadar::vec2DRadarOrigin.x + 3000.0f) * 0.002f);
    int y = ceil((11.0f - CRadar::vec2DRadarOrigin.y + 3000.0f) * 0.002f);

    CRadar::SetupRadarRect(x, y);

    CalculateCachedSinCos();

    float radarRange = 80.0f;
    float radarShift = 25.0f;
    CPed* playa = FindPlayerPed(-1);

    if (playa->m_nAreaCode)
        radarRange = 30.0f;

    if (m_b3dRadar) {
        radarRange += 50.0f;
    }

    if (CPadNew::GetPad(0)->GetExtendRadarRange() && !CellPhone.bActive && !CHud::bDrawingVitalStats)
        m_nRadarRangeExtendTime = CTimer::m_snTimeInMilliseconds + 3000;

    if (m_nRadarRangeExtendTime > CTimer::m_snTimeInMilliseconds) {
        radarRange += 100.0f;
        CHud::m_VehicleState = 1;
        CHud::m_ZoneState = 1;
        CHudNew::m_nLevelNameState = 1; 
    }

    CRadar::m_radarRange = interpF(CRadar::m_radarRange, radarRange, 0.1f * CTimer::ms_fTimeStep);

    static bool previousState = m_b3dRadar;

    if (previousState != m_b3dRadar) {
        CRadar::m_radarRange = radarRange;
        previousState = m_b3dRadar;
    }

    if (IsPlayerInVehicle()) 
        m_b3dRadar = true;
    else
        m_b3dRadar = false;

    if (CRadar::m_radarRange > 400.0f)
        CRadar::m_radarRange = 400.0f;

    radarShift = Is3dRadar() ? radarShift * (1.5f) : radarShift;
    radarShift *= CRadar::m_radarRange / 50.0f;

    CVector centreOfWorld = FindPlayerCentreOfWorld_NoInteriorShift(0);
    float a = CRadar::m_fRadarOrientation - M_PI_2;
    float c = cosf(a);
    float s = sinf(a);

    CRadar::vec2DRadarOrigin.x = centreOfWorld.x - c * (radarShift);
    CRadar::vec2DRadarOrigin.y = centreOfWorld.y - s * (radarShift);

    DrawRadarMap(x, y);
}

void CRadarNew::DrawRadarSectionMap(int x, int y, CRect const& rect, CRGBA const& col) {
    int index = x + 12 * y;
    CSprite2d* sprite = m_MiniMapSprites[index];

    bool inBounds = (x >= 0 && x <= 11) && (y >= 0 && y <= 11);

    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERMIPLINEAR);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
    RwRenderStateSet(rwRENDERSTATETEXTUREPERSPECTIVE, (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*)rwTEXTUREADDRESSCLAMP);

    if (inBounds) {
        if (sprite && sprite->m_pTexture) {
            RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(sprite->m_pTexture));
            CSprite2d::SetVertices(rect, col, col, col, col);
            RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, CSprite2d::maVertices, 4);
        }
    }
}

void CRadarNew::DrawRadarSection(int x, int y) {
    CVector2D worldPoly[8];
    CVector2D radarCorners[4];
    CVector2D texCoords[8];
    CVector2D screenPoly[8];
    bool inBounds = (x >= 0 && x <= 11) && (y >= 0 && y <= 11);
    int index = x + 12 * y;

    //index = clamp(index, 0, 12 * 12);
    CSprite2d* sprite = m_MiniMapSprites[index];

    GetTextureCorners(x, y, worldPoly);

    for (int i = 0; i < 4; i++) {
        TransformRealWorldPointToRadarSpace(radarCorners[i], worldPoly[i]);
        CRadar::TransformRadarPointToRealWorldSpace(worldPoly[i], radarCorners[i]);
        CRadar::TransformRealWorldToTexCoordSpace(texCoords[i], worldPoly[i], x, y);
        TransformRadarPoint(screenPoly[i], radarCorners[i]);
    }

    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERNEAREST);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);

    CRGBA col = GET_SETTING(HUD_RADAR).col;
    CRGBA wcol = { 56, 73, 80, col.a };

    if (FindPlayerWanted(-1)->m_nWantedLevel > 0 && m_bCopPursuit) {
        col = CTimer::m_snTimeInMilliseconds % (1000) < 500 ? HudColourNew.GetRGB(HUD_COLOUR_REDLIGHT, col.a) : HudColourNew.GetRGB(HUD_COLOUR_BLUE, col.a);
        wcol = col;
    }

    if (!CTheScripts::bPlayerIsOffTheMap) {
        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);
        CSprite2d::SetVertices(4, (float*)screenPoly, (float*)texCoords, wcol);
        RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, CSprite2d::maVertices, 4);

        if (inBounds) {
            if (sprite && sprite->m_pTexture) {
                RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(sprite->m_pTexture));
                CSprite2d::SetVertices(4, (float*)screenPoly, (float*)texCoords, col);
                RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, CSprite2d::maVertices, 4);
            }
        }

        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
        RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERLINEARMIPLINEAR);
    }
}

void CRadarNew::ScanCopPursuit() {
    m_bCopPursuit = false;

    if (FindPlayerPed(-1) && FindPlayerWanted(-1)->m_nWantedLevel > 0) {
        for (int i = CPools::ms_pPedPool->m_nSize; i; i--) {
            CPed* ped = CPools::ms_pPedPool->GetAt(i - 1);

            if (!ped || ped && ped->m_fHealth <= 0.0f)
                continue;

            if (ped->m_nModelIndex == MODEL_LAPD1
                || ped->m_nModelIndex == MODEL_SFPD1
                || ped->m_nModelIndex == MODEL_LVPD1
                || ped->m_nModelIndex == MODEL_CSHER
                || ped->m_nModelIndex == MODEL_LAPDM1
                || ped->m_nModelIndex == MODEL_SWAT
                || ped->m_nModelIndex == MODEL_FBI
                || ped->m_nModelIndex == MODEL_ARMY
                || ped->m_nModelIndex == MODEL_DSHER) {

                if ((ped->GetPosition() - FindPlayerCoors(-1)).Magnitude() < 200.0f && ped->OurPedCanSeeThisEntity(FindPlayerPed(-1), true)) {
                    m_bCopPursuit = true;
                }
                else
                    ped->ClearAll();
            }
        }
    }
}

void CRadarNew::DrawRadarMap(int x, int y) {
    CPed* playa = FindPlayerPed(-1);

    RwCameraEndUpdate(Scene.m_pRwCamera);

    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, reinterpret_cast<void*>(FALSE));
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, reinterpret_cast<void*>(FALSE));

    CRGBA col;
    CRect rect;

    rect.bottom = HUD_BOTTOM(GET_SETTING(HUD_RADAR_RECT).h);
    rect.right = HUD_X(GET_SETTING(HUD_RADAR_RECT).w);
    rect.top = HUD_BOTTOM(GET_SETTING(HUD_RADAR_RECT).y);
    rect.left = HUD_X(GET_SETTING(HUD_RADAR_RECT).x);

    if (FindPlayerWanted(-1)->m_nWantedLevel > 0 && m_bCopPursuit)
        col = HudColourNew.GetRGB(HUD_COLOUR_BLACK, 50);
    else
        col = GET_SETTING(HUD_RADAR_RECT).col;
    CHudNew::DrawSimpleRect(rect, col);

    m_pCamera->frameBuffer = m_pFrameBuffer1;

    col = { 0, 0, 0, 0 };
    RwCameraClear(m_pCamera, (RwRGBA*)&col, rwCAMERACLEARIMAGE);

    RwCameraBeginUpdate(m_pCamera);

    if (playa && !playa->m_nAreaCode) {
        DrawRadarSection(x - 1, y - 1);
        DrawRadarSection(x, y - 1);
        DrawRadarSection(x + 1, y - 1);
        DrawRadarSection(x - 1, y);
        DrawRadarSection(x, y);
        DrawRadarSection(x + 1, y);
        DrawRadarSection(x - 1, y + 1);
        DrawRadarSection(x, y + 1);
        DrawRadarSection(x + 1, y + 1);
        DrawRadarSection(x - 2, y - 2);
        DrawRadarSection(x - 2, y - 1);
        DrawRadarSection(x - 2, y);
        DrawRadarSection(x - 2, y + 1);
        DrawRadarSection(x - 2, y + 2);
        DrawRadarSection(x + 2, y - 2);
        DrawRadarSection(x + 2, y - 1);
        DrawRadarSection(x + 2, y);
        DrawRadarSection(x + 2, y + 1);
        DrawRadarSection(x + 2, y + 2);
        DrawRadarSection(x - 1, y - 2);
        DrawRadarSection(x, y - 2);
        DrawRadarSection(x + 1, y - 2);
        DrawRadarSection(x - 1, y + 2);
        DrawRadarSection(x, y + 2);
        DrawRadarSection(x + 1, y + 2);
        CRadar::DrawRadarGangOverlay(false);
    }

    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, reinterpret_cast<void*>(FALSE));
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, reinterpret_cast<void*>(FALSE));

    // GPS path
    if (radar_gps_alpha_mask_fxc) {
        RwCameraEndUpdate(m_pCamera);
        m_pCamera->frameBuffer = m_pFrameBuffer2;

        RwCameraClear(m_pCamera, (RwRGBA*)&col, rwCAMERACLEARIMAGE);
        RwCameraBeginUpdate(m_pCamera);
        CGPS::DrawPathLine();

        RwCameraEndUpdate(m_pCamera);
        RwRasterPushContext(m_pFrameBuffer3);
        RwRasterRenderFast(m_pFrameBuffer2, 0, 0);
        RwRasterPopContext();
        RwCameraBeginUpdate(m_pCamera);

        float w = static_cast<float>(m_pFrameBuffer2->width);
        float h = static_cast<float>(m_pFrameBuffer2->height);

        rect = { 0.0f, 0.0f, w, h };
        col = { 255, 255, 255, 255 };

        CSprite2d::SetVertices(rect, col, col, col, col, 0.0f, 0.0f, 1.0f, 0.0f, Anim.u3, 1.0f, Anim.u4, 1.0f);
        CSprite2d::maVertices[0].rhw = Anim.rhw[0];
        CSprite2d::maVertices[1].rhw = Anim.rhw[1];
        CSprite2d::maVertices[2].rhw = Anim.rhw[2];
        CSprite2d::maVertices[3].rhw = Anim.rhw[3];

        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, m_pFrameBuffer3);
        _rwD3D9RWSetRasterStage(m_pFrameBuffer3, 0);
        RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, CSprite2d::maVertices, 4);
    }
    else {
        CGPS::DrawPathLine();
    }

    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, reinterpret_cast<void*>(TRUE));
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, reinterpret_cast<void*>(TRUE));

    RwCameraEndUpdate(m_pCamera);

    float _u3, _u4;
    float _rhw[2];

    if (Is3dRadar()) {
        _u3 = 0.3f;
        _u4 = 0.7f;
        _rhw[0] = 0.41f;
        _rhw[1] = 1.0f;
    }
    else {
        _u3 = 0.0f;
        _u4 = 1.0f;

        _rhw[0] = CSprite2d::RecipNearClip;
        _rhw[1] = CSprite2d::RecipNearClip;
    }

    Anim.u3 = _u3;
    Anim.u4 = _u4;
    Anim.rhw[0] = _rhw[0];
    Anim.rhw[1] = _rhw[0];
    Anim.rhw[2] = _rhw[1];
    Anim.rhw[3] = _rhw[1];

    RwRasterPushContext(m_pFrameBuffer3);
    RwRasterRenderFast(m_pFrameBuffer1, 0, 0);
    RwRasterPopContext();
    m_pCamera->frameBuffer = m_pFrameBuffer1;
    RwCameraBeginUpdate(m_pCamera);

    rect = { 0.0f, 0.0f, static_cast<float>(m_pFrameBuffer1->width), static_cast<float>(m_pFrameBuffer1->height) };
    col = { 255, 255, 255, 255 };
    CSprite2d::SetVertices(rect, col, col, col, col, 0.0f, 0.0f, 1.0f, 0.0f, Anim.u3, 1.0f, Anim.u4, 1.0f);
    CSprite2d::maVertices[0].rhw = Anim.rhw[0];
    CSprite2d::maVertices[1].rhw = Anim.rhw[1];
    CSprite2d::maVertices[2].rhw = Anim.rhw[2];
    CSprite2d::maVertices[3].rhw = Anim.rhw[3];

    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, m_pFrameBuffer3);
    _rwD3D9RWSetRasterStage(m_pFrameBuffer3, 0);
    RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, CSprite2d::maVertices, 4);

    RwCameraEndUpdate(m_pCamera);

    RwCameraBeginUpdate(Scene.m_pRwCamera);

    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, reinterpret_cast<void*>(TRUE));

    // Mask radar map
    col = { 255, 255, 255, 255 };
    rect.bottom = HUD_BOTTOM(GET_SETTING(HUD_RADAR_MASK).h);
    rect.right = HUD_X(GET_SETTING(HUD_RADAR_MASK).w);
    rect.top = HUD_BOTTOM(GET_SETTING(HUD_RADAR_MASK).y);
    rect.left = HUD_X(GET_SETTING(HUD_RADAR_MASK).x);

    unsigned int savedFilter;
    RwRenderStateGet(rwRENDERSTATETEXTUREFILTER, &savedFilter);
    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERLINEARMIPNEAREST);
    CSprite2d::SetVertices(rect, col, col, col, col, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    _rwD3D9RWSetRasterStage(RwTextureGetRaster(m_RadarSprites[RADAR_MASK]->m_pTexture), 1);
    _rwSetPixelShader(multi_alpha_mask_fxc);

    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, m_pFrameBuffer1);
    _rwD3D9RWSetRasterStage(m_pFrameBuffer1, 0);
    RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, CSprite2d::maVertices, 4);
    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)savedFilter);

    _rwSetPixelShader(NULL);

    if (CGPS::bShowGPS) {
        rect.bottom = HUD_BOTTOM(GET_SETTING(HUD_RADAR).y - GET_SETTING(HUD_RADAR).h);
        rect.right = HUD_X(GET_SETTING(HUD_RADAR).w + GET_SETTING(HUD_RADAR).x);
        rect.top = HUD_BOTTOM(GET_SETTING(HUD_RADAR).h + GET_SETTING(HUD_RADAR).y);
        rect.left = HUD_X(GET_SETTING(HUD_RADAR).x - GET_SETTING(HUD_RADAR).w);

        col = { 255, 255, 255, 255 };

        CSprite2d::SetVertices(rect, col, col, col, col, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

        _rwSetPixelShader(radar_gps_alpha_mask_fxc);

        CRGBA rgb;
        rgb.Set(CGPS::Dest.pathColor);

        float color[4];
        color[0] = rgb.r / 255.0f;
        color[1] = rgb.g / 255.0f;
        color[2] = rgb.b / 255.0f;
        color[3] = 3.0f;

        GetD3DDevice()->SetPixelShaderConstantF(0, color, 1);

        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, m_pFrameBuffer2);
        _rwD3D9RWSetRasterStage(m_pFrameBuffer2, 0);
        RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, CSprite2d::maVertices, 4);
        _rwSetPixelShader(NULL);
    }

    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);

    // Radar damage
    int radarDamageTime = 1000;
    int dmgAlpha = 0;
    static int alpha = 0;
    if (playa && CWorld::Players[0].m_nLastTimeEnergyLost + radarDamageTime > CTimer::m_snTimeInMilliseconds)
        dmgAlpha = GET_SETTING(HUD_RADAR_DAMAGE).col.a;

    alpha = interpF(alpha, dmgAlpha, 0.2f * CTimer::ms_fTimeStep);

    if (alpha > 0) {
        rect.bottom = HUD_BOTTOM(GET_SETTING(HUD_RADAR_DAMAGE).h);
        rect.right = HUD_X(GET_SETTING(HUD_RADAR_DAMAGE).w);
        rect.top = HUD_BOTTOM(GET_SETTING(HUD_RADAR_DAMAGE).y);
        rect.left = HUD_X(GET_SETTING(HUD_RADAR_DAMAGE).x);

        col = GET_SETTING(HUD_RADAR_DAMAGE).col;
        col.a = alpha;
        m_RadarSprites[RADAR_DAMAGE]->Draw(rect, col);
    }

    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, reinterpret_cast<void*>(FALSE));
}

void CRadarNew::ShowRadarTraceWithHeight(float x, float y, unsigned int size, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha, unsigned char type) {
    float w = ((float)size * 0.5f) + (GET_SETTING(HUD_RADAR_BLIPS_LEVEL_SIZE).w);
    float h = ((float)size * 0.5f) + (GET_SETTING(HUD_RADAR_BLIPS_LEVEL_SIZE).h);

    if (MenuNew.bDrawMenuMap) {
        if (MenuNew.MapCrosshairCheckHover(x - SCREEN_COORD(w), x + SCREEN_COORD(w), y - SCREEN_COORD(h), y + SCREEN_COORD(h)) && MenuNew.nCurrentInputType == MENUINPUT_TAB) {
            w *= RADAR_MENU_BLIP_HOVER_MULT;
            h *= RADAR_MENU_BLIP_HOVER_MULT;
        }
        else {
            w *= RADAR_MENU_BLIP_MULT;
            h *= RADAR_MENU_BLIP_MULT;
        }
    }

    int id = RADAR_SPRITE_LEVEL;
    int on = 800;
    int off = 400;
    if (FLASH_ITEM(on, off)) {
        switch (type) {
        case 0:
            id = RADAR_SPRITE_HIGHER;
            break;
        case 1:
            id = RADAR_SPRITE_LOWER;
            break;
        }
    }

    m_BlipsSprites[id]->Draw(CRect(x - SCREEN_COORD(w), y - SCREEN_COORD(h), x + SCREEN_COORD(w), y + SCREEN_COORD(h)), CRGBA(red, green, blue, alpha));   
    AddBlipToLegendList(true, id);
}

void CRadarNew::AddBlipToLegendList(bool trace, int id) {
    unsigned int c = TRACE_COLOUR_WHITE;
    bool friendly = true;
    CVector pos = {};

    if (trace && m_nActualTraceIdToPass != -1) {
        tRadarTrace t = CRadar::ms_RadarTrace[m_nActualTraceIdToPass];

        switch (id) {
        case RADAR_SPRITE_LEVEL:
        case RADAR_SPRITE_LOWER:
        case RADAR_SPRITE_HIGHER:
            switch (t.m_nBlipType) {
            case BLIP_CAR:
                id = RADAR_LEVEL_CAR;
                break;
            case BLIP_CHAR:
                if (t.m_bFriendly)
                    id = RADAR_LEVEL_CHAR_FRIENDLY;
                else
                    id = RADAR_LEVEL_CHAR_ENEMY;
                break;
            case BLIP_OBJECT:
                id = RADAR_LEVEL_OBJECT;
                break;
            default:
                id = RADAR_LEVEL_DESTINATION;
                break;
            }

            c = t.m_dwColour;
            friendly = t.m_bFriendly;
            pos = t.m_vPosition;
            break;
        }

        m_nActualTraceIdToPass = -1;
    }

    for (int i = 0; i < m_nMapLegendBlipCount; i++) {
        if (m_MapLegendBlipList[i].id == id)
            return;
    }

    m_MapLegendBlipList[m_nMapLegendBlipCount].friendly = friendly;
    m_MapLegendBlipList[m_nMapLegendBlipCount].col = c;
    m_MapLegendBlipList[m_nMapLegendBlipCount].id = id;
    m_MapLegendBlipList[m_nMapLegendBlipCount].pos = pos;

    ++m_nMapLegendBlipCount;
}

int CRadarNew::ClipRadarPoly(CVector2D* poly, CVector2D const* rect) {
    memcpy(poly, rect, 64);
    return 4;
}

void CRadarNew::StreamRadarSection(int x, int y) {
    for (int i = 0; i < 12; ++i) {
        for (int j = 0; j < 12; ++j) {
            if ((i >= x - 2 && i <= x + 2) && (j >= y - 2 && j <= y + 2))
                CStreaming::RequestModel(gRadarTextures[i + 12 * j] + 20000, 10);
            else
                CStreaming::RemoveModel(gRadarTextures[i + 12 * j] + 20000);
        };
    };
}

bool CRadarNew::IsPlayerInVehicle() {
    return FindPlayerPed(-1) && FindPlayerPed(-1)->m_nPedFlags.bInVehicle && FindPlayerPed(-1)->m_pVehicle; 
}

bool CRadarNew::Is3dRadar() {
    return m_b3dRadar;
}
