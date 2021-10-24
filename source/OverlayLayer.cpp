#include "VHud.h"
#include "CSprite2d.h"
#include "CScene.h"

#include "OverlayLayer.h"
#include "Utility.h"
#include "TextureMgr.h"
#include "WeaponSelector.h"
#include "HudNew.h"
#include "HudColoursNew.h"
#include "MenuNew.h"

#include "resource.h"

using namespace plugin;

COverlayLayer overlayLayer;

eOverlayEffect COverlayLayer::CurrentEffect;
float COverlayLayer::fShaderConstant[4];
bool COverlayLayer::bInitialised = false;

void* overlay_color_fxc;
void* blur_fxc;
void* crosshair_lens_fxc;
void* crosshair_blur_fxc;
void* black_n_white_fxc;
void* vignette_fxc;

RwRaster* frameBuffer;

void* im2dPixelShader = NULL;
void _rwSetPixelShader(void* ps) { im2dPixelShader = ps; }

COverlayLayer::COverlayLayer() {
    CdeclEvent<AddressList<0x7FB830, H_CALL>, PRIORITY_BEFORE, ArgPickNone, void(int, int)> OnSetPixelShader;

    OnSetPixelShader += [] {
        if (im2dPixelShader)
            _rwD3D9SetPixelShader(im2dPixelShader);
    };

    CdeclEvent<AddressList<0x53EAD3, H_CALL>, PRIORITY_AFTER, ArgPickNone, void()> OnRenderEffects;

    OnRenderEffects += [] {
        RenderEffects();
        COverlayLayer::SetEffect(EFFECT_NONE);
    };
}

void COverlayLayer::Init() {
    if (bInitialised)
        return;

    overlay_color_fxc = CreatePixelShaderFromResource(IDR_OVERLAY_COLOR);
    blur_fxc = CreatePixelShaderFromResource(IDR_BLUR);
    crosshair_lens_fxc = CreatePixelShaderFromResource(IDR_CROSSHAIR_LENS);
    crosshair_blur_fxc = CreatePixelShaderFromResource(IDR_CROSSHAIR_BLUR);
    black_n_white_fxc = CreatePixelShaderFromResource(IDR_BLACK_N_WHITE);
    vignette_fxc = CreatePixelShaderFromResource(IDR_VIGNETTE);

    for (int i = 0; i < 4; i++)
        fShaderConstant[i] = 1.0f;

    bInitialised = true;
}

void COverlayLayer::Shutdown() {
    if (!bInitialised)
        return;

    bInitialised = false;
}

void COverlayLayer::UpdateFrameBuffer() {
    if (frameBuffer) {
        if (frameBuffer->width != Scene.m_pRwCamera->frameBuffer->width ||
            frameBuffer->height != Scene.m_pRwCamera->frameBuffer->height ||
            frameBuffer->depth != Scene.m_pRwCamera->frameBuffer->depth) {
            RwRasterDestroy(frameBuffer);
            frameBuffer = NULL;
        }
    }

    if (!frameBuffer)
        frameBuffer = RwRasterCreate(Scene.m_pRwCamera->frameBuffer->width, Scene.m_pRwCamera->frameBuffer->height, Scene.m_pRwCamera->frameBuffer->depth, rwRASTERTYPECAMERATEXTURE);
}

#define SetVerticesHelper(r, ps, t) \
    do { \
        UpdateFrameBuffer(); \
        RwRasterPushContext(frameBuffer); \
        RwRasterRenderFast(Scene.m_pRwCamera->frameBuffer, 0, 0); \
        RwRasterPopContext(); \
        CSprite2d::SetVertices(r, c, c, c, c, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f); \
        _rwD3D9RWSetRasterStage(t, 1); \
        _rwSetPixelShader(ps); \
        GetD3DDevice()->SetPixelShaderConstantF(0, fShaderConstant, 1); \
        RwRenderStateSet(rwRENDERSTATETEXTURERASTER, frameBuffer); \
        _rwD3D9RWSetRasterStage(frameBuffer, 0); \
        RwIm2DRenderPrimitive(rwPRIMTYPETRIFAN, CSprite2d::maVertices, 4); \
        _rwSetPixelShader(NULL); \
    } while (0); \



void COverlayLayer::RenderEffects() {
    if (CurrentEffect == EFFECT_NONE)
        return;

    RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*)rwTEXTUREADDRESSWRAP);
    RwRenderStateSet(rwRENDERSTATETEXTUREPERSPECTIVE, (void*)TRUE);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
    RwRenderStateSet(rwRENDERSTATESHADEMODE, (void*)rwSHADEMODEGOURAUD);
    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERLINEARMIPLINEAR);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
    RwRenderStateSet(rwRENDERSTATEBORDERCOLOR, (void*)RWRGBALONG(0, 0, 0, 255));
    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATEFOGTYPE, (void*)rwFOGTYPELINEAR);
    RwRenderStateSet(rwRENDERSTATECULLMODE, (void*)rwCULLMODECULLNONE);
    RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTION, (void*)rwALPHATESTFUNCTIONGREATER);
    RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTIONREF, (void*)2);

    CRect rect = { SCREEN_COORD(-5.0f), SCREEN_COORD(-5.0f), SCREEN_COORD_RIGHT(-5.0f), SCREEN_COORD_BOTTOM(-5.0f) };
    CRGBA c = { 255, 255, 255, 255 };
    CRGBA fc = HudColourNew.GetRGB(MenuNew.Settings.uiMainColor, 255);

    switch (CurrentEffect) {
    case EFFECT_BLUR_COLOR:
        CSprite2d::DrawRect(rect, CRGBA(fc.r * 0.5f, fc.g * 0.5f, fc.b * 0.5f, fc.a * 0.25f));
        SetVerticesHelper(rect, blur_fxc, NULL);
        SetVerticesHelper(rect, vignette_fxc, NULL);
        break;
    case EFFECT_BLUR:
        SetVerticesHelper(rect, blur_fxc, NULL);
        break;
    case EFFECT_BLACK_N_WHITE:
        SetVerticesHelper(rect, black_n_white_fxc, NULL);
        SetVerticesHelper(rect, vignette_fxc, NULL);
        break;
    case EFFECT_LENS_DISTORTION:
        SetVerticesHelper(rect, crosshair_lens_fxc, NULL);
        SetVerticesHelper(rect, crosshair_blur_fxc, NULL);
        break;
    }

    RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
}
