#include "VHud.h"

#include "GameLogicNew.h"
#include "HudNew.h"
#include "MenuNew.h"

using namespace plugin;

CGameLogicNew GameLogicNew;

static LateStaticInit InstallHooks([]() {
    CdeclEvent<AddressList<0x442128, H_CALL>, PRIORITY_BEFORE, ArgPickNone, void()> OnResurrection;

    OnResurrection += [] {
        CHudNew::ReInit();
        MenuNew.Clear();
    };
});
