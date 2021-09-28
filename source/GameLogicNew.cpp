#include "VHud.h"

#include "GameLogicNew.h"
#include "HudNew.h"

using namespace plugin;

CGameLogicNew GameLogicNew;

CGameLogicNew::CGameLogicNew() {
    CdeclEvent<AddressList<0x442128, H_CALL>, PRIORITY_BEFORE, ArgPickNone, void()> OnResurrection;

    OnResurrection += [] {
        CHudNew::ReInit();
    };
}
