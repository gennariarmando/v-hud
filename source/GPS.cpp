#include "VHud.h"
#include "GPS.h"
#include "TextureMgr.h"
#include "RadarNew.h"
#include "HudNew.h"
#include "HudColoursNew.h"
#include "Utility.h"
#include "VHud.h"
#include "FontNew.h"
#include "MenuNew.h"

#include "CStreaming.h"
#include "CRadar.h"
#include "CMenuManager.h"
#include "CWorld.h"
#include "CGeneral.h"
#include "CTheScripts.h"

#include <d3d9.h>

using namespace plugin;

CLocalization CGPS::Dest;
bool CGPS::bShowGPS;
CSprite2d CGPS::pathDirSprite;

int pathNodes[MAX_PATH_NODES] = { };

static LateStaticInit InstallHooks([]() {
    for (int i = 0; i < MAX_PATH_NODES - 1; i++) {
        pathNodes[i] = -1;
    }
    
#ifdef GTASA
    patch::SetFloat(0x450ACD + 1, MAX_SEARCH_RADIUS);
    patch::SetPointer(0x451782, &pathNodes);
    patch::SetPointer(0x451904, &pathNodes);
    patch::SetPointer(0x451AC3, &pathNodes);
    patch::SetPointer(0x451B33, &pathNodes);
    patch::SetUInt(0x4518F8, MAX_PATH_NODES);
    patch::SetUInt(0x4519B0, MAX_PATH_NODES - 5);
#endif
});

void CGPS::Init() {
    if (!pathDirSprite.m_pTexture)
        pathDirSprite.m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\gps"), "path_dir");
}

void CGPS::Shutdown() {
    if (pathDirSprite.m_pTexture)
        pathDirSprite.Delete();
}

void CGPS::DrawDistanceFromWaypoint() {
    if (!MenuNew.Settings.gpsRoute)
        return;

    if (bShowGPS) {
        float x, y, w, h;

        x = HUD_X(GET_SETTING(HUD_RADAR_GPS_RECT).x);
        y = HUD_BOTTOM(GET_SETTING(HUD_RADAR_GPS_RECT).y);
        w = HUD_X(GET_SETTING(HUD_RADAR_GPS_RECT).w);
        h = HUD_BOTTOM(GET_SETTING(HUD_RADAR_GPS_RECT).h);

        CSprite2d::DrawRect(CRect(x, y, w, h), GET_SETTING(HUD_RADAR_GPS_RECT).col);

        CFontNew::SetClipX(SCREEN_WIDTH);
        CFontNew::SetWrapX(SCREEN_WIDTH);
        CFontNew::SetBackground(false);
        CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
        CFontNew::SetFontStyle(CFontNew::FONT_4);
        CFontNew::SetAlignment(CFontNew::ALIGN_LEFT);
        CFontNew::SetColor(GET_SETTING(HUD_RADAR_GPS_DIST_TEXT).col);
        CFontNew::SetOutline(0.0f);
        CFontNew::SetDropColor(CRGBA(0, 0, 0, 255));
        CFontNew::SetDropShadow(SCREEN_MULTIPLIER(1.0f));
        w = GET_SETTING(HUD_RADAR_GPS_DIST_TEXT).w;
        h = GET_SETTING(HUD_RADAR_GPS_DIST_TEXT).h;
        CFontNew::SetScale(SCREEN_MULTIPLIER(w), SCREEN_MULTIPLIER(h));

        char text[16];

        if (MenuNew.Settings.measurementSys == 0) {
            if (Dest.fGPSDistance > 1000.0f)
                sprintf(text, "%.2fkm", Dest.fGPSDistance / 1000.0f);
            else
                sprintf(text, "%dm", (int)(Dest.fGPSDistance));
        }
        else if (MenuNew.Settings.measurementSys == 1) {
            int feet, totalInches, remainderInches;

            totalInches = (int)(Dest.fGPSDistance * 39.3701);
            feet = (int)(totalInches / 12);
            remainderInches = totalInches - (feet * 12);

            if (Dest.fGPSDistance > 1000.0f)
                sprintf(text, "%dft", feet);
            else
                sprintf(text, "%in", remainderInches);
        }

        x = GET_SETTING(HUD_RADAR_GPS_DIST_TEXT).x;
        y = GET_SETTING(HUD_RADAR_GPS_DIST_TEXT).y;

        CFontNew::PrintString(HUD_X(x), HUD_BOTTOM(y), text);

        if (Dest.nPathDirection != 8) {
            x = HUD_X(GET_SETTING(HUD_RADAR_GPS_DIST_ARROW).x);
            y = HUD_BOTTOM(GET_SETTING(HUD_RADAR_GPS_DIST_ARROW).y);
            w = SCREEN_COORD(GET_SETTING(HUD_RADAR_GPS_DIST_ARROW).w);
            h = SCREEN_COORD(GET_SETTING(HUD_RADAR_GPS_DIST_ARROW).h);

            float a = (Dest.nPathDirection * 90.0f) / 57.2957795f;
            CRadarNew::DrawRotatingRadarSprite(&pathDirSprite, x, y, -a, w, h, GET_SETTING(HUD_RADAR_GPS_DIST_ARROW).col);
        }
    }
}

void CGPS::DrawLine(CVector2D const&a, CVector2D const&b, float width, CRGBA color) {
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);

    CVector2D point[4], shift[2], dir;
    width *= 0.5f;
    dir.x = b.x - a.x;
    dir.y = b.y - a.y;
    float angle = atan2f(dir.y, dir.x);
    if (MenuNew.bDrawMenuMap) {
        float mp = max(2.0f, MenuNew.fMapZoom * 0.5f);
        shift[0].x = cosf(angle - 1.5707963f) * SCREEN_COORD(1.25f * mp);
        shift[0].y = sinf(angle - 1.5707963f) * SCREEN_COORD(1.25f * mp);
        shift[1].x = cosf(angle + 1.5707963f) * SCREEN_COORD(1.25f * mp);
        shift[1].y = sinf(angle + 1.5707963f) * SCREEN_COORD(1.25f * mp);
    }
    else {
        shift[0].x = cosf(angle - 1.5707963f) * width * (CRadarNew::m_vRadarMapQuality.x * 0.00390625f);
        shift[0].y = sinf(angle - 1.5707963f) * width * (CRadarNew::m_vRadarMapQuality.y * 0.00390625f);
        shift[1].x = cosf(angle + 1.5707963f) * width * (CRadarNew::m_vRadarMapQuality.x * 0.00390625f);
        shift[1].y = sinf(angle + 1.5707963f) * width * (CRadarNew::m_vRadarMapQuality.y * 0.00390625f);
    }
    point[0].x = a.x + shift[1].x;
    point[0].y = a.y + shift[1].y;
    point[1].x = b.x + shift[1].x;
    point[1].y = b.y + shift[1].y;
    point[2].x = a.x + shift[0].x;
    point[2].y = a.y + shift[0].y;
    point[3].x = b.x + shift[0].x;
    point[3].y = b.y + shift[0].y;
    CSprite2d::SetVertices(point[0].x, point[0].y, point[1].x, point[1].y, point[2].x, point[2].y, point[3].x, point[3].y, color, color, color, color);
    RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, CSprite2d::maVertices, 4);
}

void CGPS::ProcessPath(CLocalization& l) {
#ifdef GTASA
    if (!FindPlayerPed(PLAYER_ID))
        return;

    CVector start = FindPlayerCentreOfWorld(0);
    CVector end = l.vecDest;

#ifdef GTASA
    ThePaths.DoPathSearch(0, start, CNodeAddress(), end, l.resultNodes, &l.nNodesCount, MAX_NODE_POINTS, &l.fGPSDistance,
        999999.0f, NULL, 999999.0f, false, CNodeAddress(), false, false);
#else
    CVehicle* veh = FindPlayerPed(PLAYER_ID)->m_pVehicle;
    if (!veh)
        return;
    ThePaths.DoPathSearch(0, start, -1, end, l.resultNodes, &l.nNodesCount, MAX_NODE_POINTS, veh, NULL, 999999.0f, -1);
#endif
    if (l.nNodesCount > 0) {
        for (short i = 0; i < l.nNodesCount; i++) {
            CVector nodePosn = ThePaths.GetPathNode(l.resultNodes[i])->GetNodeCoors();
            CVector2D tmpPoint;
            CRadarNew::TransformRealWorldPointToRadarSpace(tmpPoint, CVector2D(nodePosn.x, nodePosn.y));
            if (MenuNew.bDrawMenuMap) {
                CRadarNew::TransformRadarPointToScreenSpace(l.nodePoints[i], tmpPoint);
            }
            else {
                CRadarNew::TransformRadarPoint(l.nodePoints[i], tmpPoint);
            }
        }

        CRGBA col;
        if (radar_gps_alpha_mask_fxc && !MenuNew.bDrawMenuMap)
            col = CRGBA(255, 255, 255, 255);
        else
            col.Set(l.pathColor);

        float w = (GET_SETTING("HUD_RADAR_GPS_LINE").w * 100.0f) / CRadar::m_radarRange;

        for (short i = 0; i < (l.nNodesCount - 1); i++) {
            DrawLine(l.nodePoints[i], l.nodePoints[i + 1], w, col);
        }
    }
#endif
}

void CGPS::DrawPathLine() {
#ifdef GTASA
    if (!MenuNew.Settings.gpsRoute)
        return;

    bShowGPS = false;
    CPed* playa = FindPlayerPed(0);

    Dest.Clear();

    if (playa) {
        if (FrontEndMenuManager.m_nTargetBlipIndex
            && CRadarNew::GetRadarTrace()[LOWORD(FrontEndMenuManager.m_nTargetBlipIndex)].m_nCounter == HIWORD(FrontEndMenuManager.m_nTargetBlipIndex)
            && CRadarNew::GetRadarTrace()[LOWORD(FrontEndMenuManager.m_nTargetBlipIndex)].m_nBlipDisplayFlag
            && DistanceBetweenPoints(CVector2D(FindPlayerCentreOfWorld_NoInteriorShift(0)), CVector2D(CRadarNew::GetRadarTrace()[LOWORD(FrontEndMenuManager.m_nTargetBlipIndex)].m_vPosition)) < MAX_TARGET_DISTANCE) {
            CRadar::ClearBlip(FrontEndMenuManager.m_nTargetBlipIndex);
            FrontEndMenuManager.m_nTargetBlipIndex = 0;
        }

        if ((CRadarNew::IsPlayerInVehicle()
            && playa->m_pVehicle->m_nVehicleSubClass != VEHICLE_PLANE
            && playa->m_pVehicle->m_nVehicleSubClass != VEHICLE_HELI
            && playa->m_pVehicle->m_nVehicleSubClass != VEHICLE_BMX) || MenuNew.bDrawMenuMap) {
            if (FrontEndMenuManager.m_nTargetBlipIndex
                && CRadarNew::GetRadarTrace()[LOWORD(FrontEndMenuManager.m_nTargetBlipIndex)].m_nCounter == HIWORD(FrontEndMenuManager.m_nTargetBlipIndex)
                && CRadarNew::GetRadarTrace()[LOWORD(FrontEndMenuManager.m_nTargetBlipIndex)].m_nBlipDisplayFlag) {
                CVector destPosn = CRadarNew::GetRadarTrace()[LOWORD(FrontEndMenuManager.m_nTargetBlipIndex)].m_vPosition;
                destPosn.z = CWorld::FindGroundZForCoord(destPosn.x, destPosn.y);

                Dest.pathColor = CRadarNew::GetBlipColor(RADAR_SPRITE_WAYPOINT).ToInt();
                Dest.vecDest = destPosn;
                Dest.bDestFound = true;
            }
            else {
                for (int i = 0; i < CRadarNew::m_nMapLegendBlipCount; i++) {
                    CRadarLegend l = CRadarNew::m_MapLegendBlipList[i];

                    if (CTheScripts::IsPlayerOnAMission()) {
                        if (l.id == RADAR_LEVEL_DESTINATION) {
                            Dest.pathColor = CRadarNew::GetRadarTraceColour(l.col, false, l.friendly);
                            Dest.vecDest = l.pos;
                            Dest.bDestFound = true;
                        }
                    }
                }
            }

            Dest.nPathDirection = DIR_NONE;
            if (Dest.bDestFound) {
                ProcessPath(Dest);

                if (Dest.nNodesCount > 0) {
                    CPathNode* node = ThePaths.GetPathNode(Dest.resultNodes[0]);

                    if (!node)
                        return;

                    Dest.fGPSDistance += DistanceBetweenPoints(FindPlayerCentreOfWorld_NoInteriorShift(0), node->GetNodeCoors());
                    Dest.nPathDirection = GetPathDirection(ThePaths.GetPathNode(Dest.resultNodes[Dest.nNodesCount - 1])->GetNodeCoors(), FindPlayerCentreOfWorld_NoInteriorShift(0), ThePaths.GetPathNode(Dest.resultNodes[0])->GetNodeCoors());
                    bShowGPS = true;
                }
            }
        }
    }
#endif
}

char CGPS::GetPathDirection(CVector start, CVector plr, CVector end) {
    float v3; // ST2C_4
    double v4; // st6
    int v5; // ST1C_4
    float v6; // ST30_4
    double v7; // st5
    int v8; // edx
    float v9; // ST2C_4
    double v10; // st4
    float v11; // ST2C_4
    float v12; // ST20_4
    float v13; // ST24_4
    float v14; // ST18_4
    double v15; // st6
    double v16; // st5
    double v17; // st7
    double v18; // rt2
    double v19; // st5
    double v20; // st6
    double v21; // rtt
    double v22; // st5
    double v23; // rt1
    double v24; // st5
    double v25; // st6
    double v26; // st6
    double v27; // st5
    double v28; // st5
    double v29; // st6
    double v30; // st4
    double v31; // rt0
    double v32; // st5
    double v33; // rt2
    double v34; // st4
    double v35; // st5
    float v36; // ST30_4
    double v37; // st7
    float v38; // ST30_4
    int v40; // [esp+8h] [ebp-2Ch]
    float v41; // [esp+Ch] [ebp-28h]
    int v42; // [esp+10h] [ebp-24h]
    float v43; // [esp+14h] [ebp-20h]
    float v44; // [esp+2Ch] [ebp-8h]
    float v45; // [esp+2Ch] [ebp-8h]
    float v46; // [esp+2Ch] [ebp-8h]
    float v47; // [esp+30h] [ebp-4h]

    v40 = plr.x;
    v42 = plr.z;
    v3 = end.x - plr.x;
    v41 = plr.y;
    v4 = v3;
    v5 = end.z;
    v6 = v3;
    v7 = end.y;
    v8 = start.z;
    v9 = v7 - v41;
    v10 = v9;
    v11 = v9 * v9 + v6 * v6;
    if (v11 >= 400.0f)
        return 1;
    v12 = start.x - end.x;
    v13 = start.y - v7;
    v43 = v4;
    v46 = CGeneral::GetATanOfXY(v12, v13);
    v47 = v46 * 57.2957763671875 - 90.0f;
    v15 = v47;
    v16 = 360.0;
    if (v47 < 0.0f) {
        while (1) {
            v18 = v16;
            v19 = v15 + v16;
            v20 = v18;
            v47 = v19;
            if (v47 >= 0.0f)
                break;
            v16 = v20;
            v15 = v47;
        }
        v21 = v20;
        v15 = v47;
        v17 = v21;
    }
    else {
        v17 = 360.0f;
    }
    v22 = 360.0f;
    if (v15 > 360.0f) {
        while (1) {
            v23 = v22;
            v24 = v15;
            v25 = v23;
            v47 = v24 - v17;
            if ( v47 <= v23 )
                break;
            v22 = v25;
            v15 = v47;
        }
    }
    v14 = v10;
    v45 = CGeneral::GetATanOfXY(v43, v14);
    v44 = v45 * 57.2957763671875 - 90.0f;
    v26 = v44;
    v27 = 360.0f;
    if (v44 < 0.0f) {
        while (1) {
            v31 = v27;
            v32 = v26 + v27;
            v29 = v31;
            v45 = v32;
            v28 = v45;
            if (v45 >= 0.0f)
                break;
            v27 = v29;
            v26 = v45;
        }
    }
    else {
        v28 = v44;
        v29 = 360.0f;
    }
    v30 = 360.0f;
    if (v28 > 360.0f) {
        while (1) {
            v33 = v30;
            v34 = v28;
            v35 = v33;
            v46 = v34 - v29;
            if (v46 <= v33)
                break;
            v30 = v35;
            v28 = v46;
        }
        v28 = v46;
    }
    v36 = v47 - v28;
    v37 = v36;
    if (v36 < 0.0f) {
        v38 = v37 + v29;
        v37 = v38;
    }
    if (v37 >= 30.0f && v37 < 180.0f)
        return 2;
    if (v37 > 330.0f || v37 <= 180.0f)
        return 1;
    return 0;
}

void CLocalization::Clear() {
    bDestFound = false;
    vecDest = { 0.0f, 0.0f, 0.0f };
    fGPSDistance = 0.0f;
    nPathDirection = DIR_NONE;
    nNodesCount = 0;
    pathColor = 0;
}
