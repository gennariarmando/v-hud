#include "VHud.h"
#include "MenuPanels.h"
#include "Utility.h"
#include "FontNew.h"
#include "HudColoursNew.h"
#include "HudNew.h"
#include "TextNew.h"
#include "MenuNew.h"
#include "TextureMgr.h"

#include "CMenuSystem.h"
#include "CText.h"
#include "CMessages.h"
#include "CEntryExit.h"

using namespace plugin;

CMenuPanels MenuPanels;

CPanel CMenuPanels::Panel[32];
CSprite2d CMenuPanels::ShopUiSprites[32];
int CMenuPanels::NumPanels;

CMenuPanels::CMenuPanels() {
    patch::RedirectJump(0x580E00, Draw);
}

void CMenuPanels::Init() {
    ReadValuesFromFile();

    for (int i = 0; i < NumPanels; i++) {
        ShopUiSprites[i].m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\shopui"), Panel[i].textureName);
    }
}

void CMenuPanels::Shutdown() {
    for (int i = 0; i < NumPanels; i++) {
        ShopUiSprites[i].Delete();
    }
}

void CMenuPanels::Process(unsigned char panelId) {
    CMenuSystem::Process(panelId);
}

void CMenuPanels::ReadValuesFromFile() {
    std::ifstream file(PLUGIN_PATH("VHud\\data\\shopui.dat"));

    if (file.is_open()) {
        NumPanels = 0;
        for (std::string line; getline(file, line);) {
            char title[16];
            char interior[16];
            char iconType[16];
            char textureName[128];

            if (!line[0] || line[0] == '\t' || line[0] == ' ' || line[0] == '#' || line[0] == '[')
                continue;

            if (line[0] == ';') {
                NumPanels++;
                continue;
            }

            sscanf(line.c_str(), "%s %s %s %s", &title, &interior, &iconType, &textureName);

            for (int i = 0; i < 16; i++) {
                if (interior[i] == '_')
                    interior[i] = ' ';
            }

            strcpy(Panel[NumPanels].title, title);
            strcpy(Panel[NumPanels].interiorName, interior);
            strcpy(Panel[NumPanels].iconType, iconType);
            strcpy(Panel[NumPanels].textureName, textureName);
            NumPanels++;
        }
        file.close();
    }
}

void CMenuPanels::Draw(unsigned char panelId) {
    bool active = CMenuSystem::MenuInUse[panelId];

    CFontNew::SetBackground(false);
    CFontNew::SetBackgroundColor(CRGBA(0, 0, 0, 0));
    CFontNew::SetAlignment(CFontNew::ALIGN_LEFT);
    CFontNew::SetWrapX(SCREEN_COORD(640.0f));
    CFontNew::SetFontStyle(CFontNew::FONT_1);
    CFontNew::SetDropShadow(0.0f);
    CFontNew::SetOutline(0.0f);
    CFontNew::SetDropColor(CRGBA(0, 0, 0, 0));
    CFontNew::SetColor(HudColourNew.GetRGB(HUD_COLOUR_WHITE, 255));
    CFontNew::SetScale(SCREEN_MULTIPLIER(0.64f), SCREEN_MULTIPLIER(1.18f));

    if (active) {
        CHud::m_pHelpMessage[0] = NULL;

        CRect rect = { 0.0f, 0.0f, 0.0f, 0.0f };
        tMenuPanel* panel = MenuNumber[panelId];

        int panelIDToUse = -1;
        for (int i = 0; i < NumPanels; i++) {
            char* intName = CEntryExit::ms_spawnPoint->m_szName;
            if (intName && intName[0] != '\0')
                intName = TheText.Get(intName);
            else
                intName = "BLANK";
         
            bool gxt = !faststrcmp(Panel[i].title, panel->m_acTitle);
            bool interior = (Panel[i].interiorName[0] != '0') ? (!faststrcmp(Panel[i].interiorName, intName)) : true;
            if (gxt && interior) {
                panelIDToUse = i;
                break;
            }
        }

        rect.left = 96.0f;
        rect.top = 54.0f;
        rect.right = rect.left + 432.0f;
        rect.bottom = rect.top;

        int iconType = MENU_BOUGHT;
        if (panelIDToUse != -1) {
            rect.bottom += 108.0f;
            ShopUiSprites[panelIDToUse].Draw(CRect(HUD_X(rect.left), HUD_Y(rect.top), HUD_X(rect.right), HUD_Y(rect.bottom)), CRGBA(255, 255, 255, 255));

            if (!faststrcmp(Panel[panelIDToUse].iconType, "WEAP")) {
                iconType = MENU_WEAPONS;
            }
            else if (!faststrcmp(Panel[panelIDToUse].iconType, "CLOT")) {
                iconType = MENU_CLOTHES;
            }
            else if (!faststrcmp(Panel[panelIDToUse].iconType, "CAR")) {
                iconType = MENU_CAR;
            }
        }

        rect.top = rect.bottom;
        rect.bottom = rect.top + 38.0f;

        CSprite2d::DrawRect(CRect(HUD_X(rect.left), HUD_Y(rect.top), HUD_X(rect.right), HUD_Y(rect.bottom)), CRGBA(0, 0, 0, 255));

        char* str = TheText.Get(panel->m_acTitle);
        CTextNew::UpperCase(str);;
        CFontNew::PrintString(HUD_X(rect.left) + SCREEN_COORD(12.0f), HUD_Y(rect.top) + SCREEN_COORD(6.0f), str);

        rect.top = rect.bottom;
        rect.bottom = rect.top + (38.0f * panel->m_nNumRows);

        CSprite2d::DrawRect(CRect(HUD_X(rect.left), HUD_Y(rect.top), HUD_X(rect.right), HUD_Y(rect.bottom)), CRGBA(0, 0, 0, 150));

        float x = rect.left + 12.0f;
        float y = rect.top + 6.0f;
        CRGBA textCol;

        bool selectedRectDrawn = false;
        int iconTypeToDraw = iconType;
        for (int i = 0; i < panel->m_nNumColumns; i++) {
            for (int j = 0; j < panel->m_nNumRows; j++) {
                char* title = panel->m_aaacRowTitles[i][j];
                if (title[0]) {
                    int num1 = panel->m_aadwNumberInRowTitle[i][j];
                    int num2 = panel->m_aadw2ndNumberInRowTitle[i][j];

                    if (panel->m_abRowSelectable[j]) {
                        if (j == panel->m_nSelectedRow) {
                            textCol = HudColourNew.GetRGB(HUD_COLOUR_BLACK, 255);
                            iconTypeToDraw = iconTypeToDraw + 1;
                            if (!selectedRectDrawn) {
                                CSprite2d::DrawRect(CRect(HUD_X(rect.left), HUD_Y(y - 6.0f), HUD_X(rect.right), HUD_Y(y - 6.0f + 38.0f)), HudColourNew.GetRGB(HUD_COLOUR_WHITE, 255));
                                selectedRectDrawn = true;
                            }
                        }
                        else {
                            textCol = HudColourNew.GetRGB(HUD_COLOUR_WHITE, 255);
                        }
                    }
                    else {
                        textCol = HudColourNew.GetRGB(HUD_COLOUR_GREEN, 255);
                    }

                    CMessages::InsertNumberInString(TheText.Get(title), num1, num2, -1, -1, -1, -1, gString);
                    CMessages::InsertPlayerControlKeysInString(gString);
                    if (panel->m_anColumnAlignment[i] == ALIGN_RIGHT) {
                        CFontNew::SetAlignment(CFontNew::ALIGN_RIGHT);
                        x = rect.right - 12.0f;
                    }
                    else {
                        CFontNew::SetAlignment(CFontNew::ALIGN_LEFT);
                        x = rect.left + 12.0f;
                    }

                    if (panel->m_abRowAlreadyBought[j] || !panel->m_abRowSelectable[j]) {
                        MenuNew.MenuSprites[iconTypeToDraw]->Draw(HUD_X(rect.right - 36.0f), HUD_Y(y - 3.0f), SCREEN_COORD(32.0f), SCREEN_COORD(32.0f), CRGBA(255, 255, 255, 255));
                    }

                    CFontNew::SetColor(textCol);
                    CFontNew::PrintString(HUD_X(x), HUD_Y(y), gString);

                    y += 38.0f;
                    iconTypeToDraw = iconType;
                }
            }
            y = rect.top + 6.0f;
        }
    }
}
