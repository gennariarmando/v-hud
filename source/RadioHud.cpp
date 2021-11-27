#include "VHud.h"
#include "FontNew.h"
#include "HudColoursNew.h"
#include "MenuNew.h"
#include "RadioHud.h"
#include "TextureMgr.h"
#include "Utility.h"
#include "TextNew.h"

#include "CAERadioTrackManager.h"
#include "CTimer.h"

CRadioHud radioHud;

CSprite2d* CRadioHud::m_RadioIcons[NUM_RADIO_SPRITES];
bool CRadioHud::bInitialised = false;
int CRadioHud::nTimeToDisplay;

CRadioHud::CRadioHud() {
    //4EA670
}

void CRadioHud::Init() {
    if (bInitialised)
        return;

    for (int i = 1; i < NUM_RADIO_SPRITES; i++) {
        char name[32];
        sprintf(name, i == 14 ? "radio_current" : "radio_%d", i);

        m_RadioIcons[i] = new CSprite2d();
        m_RadioIcons[i]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\radio"), name);
    }
    bInitialised = true;

    nTimeToDisplay = 0;
}

void CRadioHud::Draw() {
    float x = 0.0f;
    float y = 173.0f;
    float w = 112.0f;
    float h = 112.0f;
    int i = (AERadioTrackManager.m_nStationsListed + AERadioTrackManager.m_Settings.m_nCurrentRadioStation);

    if (i) {
        if (i > 0) {
            if (i >= 14)
                i = i - 13;
        }
        else {
            i = i + 13;
        }
        static int previousRadioStation = i;
        if (previousRadioStation != i) {
            nTimeToDisplay = CTimer::m_snTimeInMilliseconds + 2000;
            previousRadioStation = i;
        }

        if (nTimeToDisplay > CTimer::m_snTimeInMilliseconds) {
            m_RadioIcons[i]->Draw(SCREEN_COORD_CENTER_LEFT(x + (w / 2)), HUD_Y(y), SCREEN_COORD(w), SCREEN_COORD(h), CRGBA(255, 255, 255, 255));
            m_RadioIcons[14]->Draw(SCREEN_COORD_CENTER_LEFT(x + (w / 2)), HUD_Y(y), SCREEN_COORD(w), SCREEN_COORD(h), HudColourNew.GetRGB(MenuNew.Settings.uiMainColor, 255));

            CFontNew::SetBackground(false);
            CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
            CFontNew::SetAlignment(CFontNew::ALIGN_CENTER);
            CFontNew::SetWrapX(SCREEN_COORD(640.0f));
            CFontNew::SetFontStyle(CFontNew::FONT_1);
            CFontNew::SetDropShadow(0.0f);
            CFontNew::SetOutline(SCREEN_COORD(2.0f));
            CFontNew::SetDropColor(CRGBA(0, 0, 0, 255));
            CFontNew::SetColor(HudColourNew.GetRGB(HUD_COLOUR_WHITE, 255));
            CFontNew::SetScale(SCREEN_MULTIPLIER(0.72f), SCREEN_MULTIPLIER(1.80f));

            char* str = NULL;
            if (bool radioOff = (i != RADIO_NONE)) {
                sprintf(gString, "RADIO%d", i);
                str = CTextNew::GetText(gString).text;
            }
            else {
                str = CTextNew::GetText("RADOFF").text;
            }
            CFontNew::PrintString(SCREEN_COORD_CENTER_LEFT(0.0f), HUD_Y(y + h + 18.0f), str);
        }
    }
}

void CRadioHud::Shutdown() { 
    if (!bInitialised)
        return;

    for (int i = 1; i < NUM_RADIO_SPRITES; i++) {
        m_RadioIcons[i]->Delete();
        delete m_RadioIcons[i];
    }

    bInitialised = false;
}
