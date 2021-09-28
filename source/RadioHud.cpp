#include "VHud.h"
#include "RadioHud.h"
#include "TextureMgr.h"

CRadioHud radioHud;

CSprite2d* CRadioHud::m_RadioIcons[NUM_RADIO_SPRITES];
bool CRadioHud::bInitialised = false;

CRadioHud::CRadioHud() {

}

void CRadioHud::Init() {
    if (bInitialised)
        return;

    for (int i = 0; i < NUM_RADIO_SPRITES; i++) {
        char name[32];
        sprintf(name, i == 13 ? "radio_current" : "radio_%d", i + 1);

        m_RadioIcons[i] = new CSprite2d();
        m_RadioIcons[i]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\radio"), name);
    }
    bInitialised = true;
}

void CRadioHud::Shutdown() {
    if (!bInitialised)
        return;

    for (int i = 0; i < NUM_RADIO_SPRITES; i++) {
        m_RadioIcons[i]->Delete();
        delete m_RadioIcons[i];
    }

    bInitialised = false;
}
