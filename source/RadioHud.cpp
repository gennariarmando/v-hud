#include "VHud.h"
#include "FontNew.h"
#include "HudColoursNew.h"
#include "MenuNew.h"
#include "RadioHud.h"
#include "TextureMgr.h"
#include "Utility.h"
#include "TextNew.h"
#include "HudNew.h"
#include "CellPhone.h"
#include "Audio.h"
#include "CReplay.h"
#include "CCamera.h"

#include "CAERadioTrackManager.h"
#include "CTimer.h"
#include "CAudioEngine.h"

using namespace plugin;

CRadioHud radioHud;

CSprite2d* CRadioHud::m_RadioIcons[NUM_RADIO_SPRITES];
bool CRadioHud::m_bInitialised = false;
int CRadioHud::m_nTimeToDisplay = 0;
int CRadioHud::m_nCurrentRadioId = 1;
int CRadioHud::m_nPreviousRadioId = 1;
bool CRadioHud::m_bChangeRadioStation = false;
bool CRadioHud::m_bJustEnteredAVehicle = false;

static LateStaticInit InstallHooks([]() {
    if (VHud::bUG)
        return;

    patch::RedirectJump(0x4EB660, CRadioHud::Process); // CAERadioTrackManager::CheckForStationRetune()
    patch::RedirectJump(0x507030, CRadioHud::Draw); // CAudioEngine::DisplayRadioStationName
});

void CRadioHud::Init() {
    if (VHud::bUG)
        return;

    if (m_bInitialised)
        return;

    for (int i = 1; i < NUM_RADIO_SPRITES; i++) {
        char name[32];
        sprintf(name, i == 14 ? "radio_current" : "radio_%d", i);

        m_RadioIcons[i] = new CSprite2d();
        m_RadioIcons[i]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\radio"), name);
    }

    m_bInitialised = true;
}

void CRadioHud::Clear() {
    if (VHud::bUG)
        return;

    m_nTimeToDisplay = 0;
    m_bJustEnteredAVehicle = false;
}

bool CRadioHud::CanRetuneRadioStation() {
    bool result = true;

    if (FindPlayerVehicle(-1, 0) && FindPlayerVehicle(-1, 0)->m_vehicleAudio.m_settings.m_nRadioType != RADIO_CIVILIAN)
        result = false;

    return result;
}

void CRadioHud::Process() {
    if (VHud::bUG)
        return;

    if (!MenuNew.Settings.showHUD)
        return;

    if (FindPlayerVehicle(-1, false) && AERadioTrackManager.IsVehicleRadioActive()) {
        if (FindPlayerVehicle(-1, false)) {
            if (!m_bJustEnteredAVehicle) {
                m_nCurrentRadioId = AudioEngine.GetCurrentRadioStationID();
                m_bJustEnteredAVehicle = true;
            }
        }
        else {
            m_bJustEnteredAVehicle = false;
        }

        if (!CHud::bDrawingVitalStats && !CellPhone.bActive) {
            if (CanRetuneRadioStation()) {
                if (CPadNew::GetPad(0)->CycleRadioStationLeftJustDown()) {
                    m_nPreviousRadioId = m_nCurrentRadioId;
                    m_nCurrentRadioId--;

                    if (m_nCurrentRadioId < 1)
                        m_nCurrentRadioId = 13;

                    m_bChangeRadioStation = true;
                }
                else if (CPadNew::GetPad(0)->CycleRadioStationRightJustDown()) {
                    m_nPreviousRadioId = m_nCurrentRadioId;
                    m_nCurrentRadioId++;

                    if (m_nCurrentRadioId > 13)
                        m_nCurrentRadioId = 1;

                    m_bChangeRadioStation = true;
                }
            }
        }

        if (m_bChangeRadioStation) {
            MenuNew.RetuneRadio(m_nCurrentRadioId);
            AERadioTrackManager.StopRadio(NULL, 0);
            AERadioTrackManager.StartRadio(m_nCurrentRadioId, AERadioTrackManager.m_Settings.m_nBassSet, LOWORD(AERadioTrackManager.m_Settings.m_fBassGain), 0);
            m_nTimeToDisplay = CTimer::m_snTimeInMilliseconds + 2000;
            m_bChangeRadioStation = false;

            Audio.PlayChunk(CHUNK_WHEEL_MOVE, 1.0f);
        }
    }
}

void CRadioHud::Draw() {
    if (VHud::bUG)
        return;

    if (CTimer::m_UserPause || CTimer::m_CodePause || MenuNew.bMenuActive)
        return;

    if (!MenuNew.Settings.showHUD)
        return;

    if (CReplay::Mode != 1 
        && !CWeapon::ms_bTakePhoto 
        && !TheCamera.m_bWideScreenOn 
        && TheCamera.GetScreenFadeStatus() != 2) {
        float x = 0.0f;
        float y = 173.0f;
        float w = 112.0f;
        float h = 112.0f;
        int i = m_nCurrentRadioId;

        if (!CanRetuneRadioStation() || i < 1)
            return;

        if (m_nTimeToDisplay > CTimer::m_snTimeInMilliseconds) {
            m_RadioIcons[i]->Draw(SCREEN_COORD_CENTER_LEFT(x + (w / 2)), HUD_Y(y), SCREEN_COORD(w), SCREEN_COORD(h), CRGBA(255, 255, 255, 255));
            m_RadioIcons[14]->Draw(SCREEN_COORD_CENTER_LEFT(x + (w / 2)), HUD_Y(y), SCREEN_COORD(w), SCREEN_COORD(h), HudColourNew.GetRGB(VHud::Settings.UIMainColor, 255));

            CFontNew::SetBackground(false);
            CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
            CFontNew::SetAlignment(CFontNew::ALIGN_CENTER);
            CFontNew::SetWrapX(SCREEN_COORD(640.0f));
            CFontNew::SetFontStyle(CFontNew::FONT_1);
            CFontNew::SetDropShadow(0.0f);
            CFontNew::SetOutline(SCREEN_MULTIPLIER(2.0f));
            CFontNew::SetDropColor(CRGBA(0, 0, 0, 255));
            CFontNew::SetColor(HudColourNew.GetRGB(HUD_COLOUR_WHITE, 255));
            CFontNew::SetScale(SCREEN_MULTIPLIER(0.72f), SCREEN_MULTIPLIER(1.80f));

            char* str = NULL;
            if (bool radioOff = (i != RADIO_NONE)) {
                sprintf(gString, "RADIO%d", i);
                str = TextNew.GetText(gString).text;
            }
            else {
                str = TextNew.GetText("RADOFF").text;
            }

            CFontNew::PrintString(SCREEN_COORD_CENTER_LEFT(0.0f), HUD_Y(y + h + 18.0f), str);
        }
    }
}

void CRadioHud::Shutdown() { 
    if (VHud::bUG)
        return;

    if (!m_bInitialised)
        return;

    for (int i = 1; i < NUM_RADIO_SPRITES; i++) {
        m_RadioIcons[i]->Delete();
        delete m_RadioIcons[i];
    }

    m_bInitialised = false;
}
