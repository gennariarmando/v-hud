#include "plugin.h"
#include "MenuNew.h"
#include "TextureMgr.h"
#include "CMenuManager.h"
#include "PadNew.h"

using namespace plugin;

CMenuNew MenuNew;

void __fastcall test(CMenuManager* _this, int) {

}

CMenuNew::CMenuNew() {
    //patch::Nop(0x53E7A0, 10);
    //patch::Nop(0x53BF3F, 10);
    //patch::Set<BYTE>(0x53E797, 0xEB);
    //patch::RedirectJump(0x57B440, test);
}

void CMenuNew::Init() {
    MenuSprites[0] = new CSprite2d();
    MenuSprites[1] = new CSprite2d();
    MenuSprites[2] = new CSprite2d();
    MenuSprites[3] = new CSprite2d();
    MenuSprites[0]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\menus"), "mouse_arrow");
    MenuSprites[1]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\menus"), "mouse_fucku");
    MenuSprites[2]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\menus"), "mouse_grab");
    MenuSprites[3]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\menus"), "mouse_hand");

    mouseMult = 1.0f;
}

void CMenuNew::Update() {
    float x = CPadNew::GetMouseInput(mouseMult).x;
    float y = CPadNew::GetMouseInput(mouseMult).y;

    OldMousePos.x = MousePos.x;
    OldMousePos.y = MousePos.y;

    if (x < 0.01f || x > 0.01f) 
        MousePos.x += x;

    if (y < 0.01f || y > 0.01f) 
        MousePos.y += y;

    if (MousePos.x < 0)
        MousePos.x = 0;
    if (MousePos.x > SCREEN_WIDTH)
        MousePos.x = SCREEN_WIDTH;
    if (MousePos.y < 0)
        MousePos.y = 0;
    if (MousePos.y > SCREEN_HEIGHT)
        MousePos.y = SCREEN_HEIGHT;

    // MenuSprites[0]->Draw(MousePos.x, MousePos.y, SCREEN_COORD(30.0f), SCREEN_COORD(32.0f), CRGBA(255, 255, 255, 255));
}

void CMenuNew::SetMouseMult(float m) {
    mouseMult = m;
}

void CMenuNew::Shutdown() {
    MenuSprites[0]->Delete();
    MenuSprites[1]->Delete();
    MenuSprites[2]->Delete();
    MenuSprites[3]->Delete();
}

void CMenuNew::Process() {

}
