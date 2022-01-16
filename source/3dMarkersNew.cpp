#include "VHud.h"
#include "3dMarkersNew.h"
#include "TextureMgr.h"
#include "HudColoursNew.h"
#include "Utility.h"
#include "FontNew.h"
#include "PadNew.h"

#include "CDraw.h"
#include "CSprite.h"
#include "C3dMarker.h"
#include "C3dMarkers.h"
#include "CCamera.h"
#include "CWorld.h"
#include "CTimer.h"

using namespace plugin;

C3dMarkersNew MarkersNew;

static LateStaticInit InstallHooks([]() {
    auto placeMarker = [](unsigned int id, unsigned short type, CVector& posn, float size, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha, unsigned short pulsePeriod, float pulseFraction, short rotateRate, float nrm_x, float nrm_y, float nrm_z, bool zCheck) {
        CRGBA col = { red, green, blue, alpha };
        if (type == MARKER3D_CYLINDER) {
            col = HudColourNew.GetRGB(HUD_COLOUR_YELLOW, alpha);
            size = 0.8f;
        }
        C3dMarkers::PlaceMarker(id, type, posn, size, col.r, col.g, col.b, col.a, pulsePeriod, pulseFraction, rotateRate, nrm_x, nrm_y, nrm_z, zCheck);
    };
    patch::RedirectCall(0x725BF0, (void(__cdecl*)(unsigned int, unsigned short, CVector&, float, unsigned char red, unsigned char, unsigned char, unsigned char, unsigned short, float, short, float, float, float, bool))placeMarker);
    patch::Nop(0x70CDBC, 5);
    patch::Nop(0x70CD6C, 5);
    patch::Nop(0x725657, 6);

    auto placeEntryExitMarker = [](unsigned int id, CVector& posn, float size, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha, int unused1, unsigned short pulsePeriod, float pulseFraction, float unused2, bool unused3) {
        MarkersNew.Markers[MarkersNew.MarkersCount].used = true;
        MarkersNew.Markers[MarkersNew.MarkersCount].type = 1;
        MarkersNew.Markers[MarkersNew.MarkersCount].pos = posn;
        MarkersNew.Markers[MarkersNew.MarkersCount].col = HudColourNew.GetRGB(HUD_COLOUR_YELLOW, alpha);
        MarkersNew.MarkersCount++;
    };
    patch::RedirectCall(0x440F4E, (void(__cdecl*)(unsigned int, CVector&, float, unsigned char, unsigned char, unsigned char, unsigned char, int, unsigned short, float, float, bool))placeEntryExitMarker);
});

void C3dMarkersNew::Init() {
    if (bInitialised)
        return;

    Sprite = new CSprite2d();
    Sprite->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\markers\\"), "arrow");

    SpriteAlpha = new CSprite2d();
    SpriteAlpha->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\markers\\"), "arrow", "arrowa");
    bInitialised = true;
}

void C3dMarkersNew::DrawArrows() {
    for (int i = 0; i < 32; i++) {
        C3dMarker* marker = &C3dMarkers::m_aMarkerArray[i];

        if (marker && marker->m_bIsUsed && marker->m_nType == MARKER3D_CONE) {
            marker->m_fSize = 0.0f;
            Markers[MarkersCount].used = marker->m_bIsUsed;
            Markers[MarkersCount].type = 0;
            Markers[MarkersCount].pos = marker->m_mat.pos;
            Markers[MarkersCount].col = marker->m_colour;
            MarkersCount++;
        }
    }

    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)rwRENDERSTATENARENDERSTATE);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);

    for (int i = 0; i < MarkersCount; i++) {
        const int type = Markers[i].type;
        CVector vin = { Markers[i].pos.x, Markers[i].pos.y, Markers[i].pos.z };
        
        if (type == 1)
            vin.z = CWorld::FindGroundZFor3DCoord(vin.x, vin.y, vin.z, NULL, NULL) + 1.0f;
        else
            vin.z -= 1.0f;

        const int pulse = 512;
        float s = sinf(M_PI * ((pulse - 1) & CTimer::m_snTimeInMilliseconds) / pulse);
        vin.z += 0.25f * s;

        RwV3d in = { vin.x, vin.y, vin.z };
        RwV3d out;
        CRGBA col = Markers[i].col;
        bool used = Markers[i].used;
        float w, h;

        if (used && CSprite::CalcScreenCoors(in, &out, &w, &h, true, true)) {
            const float recip = 1.0f / out.z;
            w = 1.0f * (recip * ((4.0f / 3.0f) / CDraw::ms_fAspectRatio)) * SCREEN_WIDTH;
            h = 1.0f * recip * SCREEN_HEIGHT;

            RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
            RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
            RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(SpriteAlpha->m_pTexture));
            CSprite::RenderOneXLUSprite(out.x, out.y, out.z, w * 0.18f, h * 0.2f, 0, 0, 0, 0, recip, 150, 0, 0);

            RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDONE);
            RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDONE);
            RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(Sprite->m_pTexture));
            CSprite::RenderOneXLUSprite(out.x, out.y, out.z, w * 0.18f, h * 0.2f, col.r, col.g, col.b, col.a, recip, 255, 0, 0);
        }
        Markers[i].Clear();
    }

    MarkersCount = 0;

    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, FALSE);
}

void C3dMarkersNew::Shutdown() {
    if (!bInitialised)
        return;

    if (Sprite) {
        Sprite->Delete();
        delete Sprite;
    }

    if (SpriteAlpha) {
        SpriteAlpha->Delete();
        delete SpriteAlpha;
    }

    bInitialised = false;
}