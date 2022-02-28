#include "VHud.h"

#include "GameLogicNew.h"
#include "HudNew.h"
#include "MenuNew.h"

using namespace plugin;

CGameLogicNew GameLogicNew;

static LateStaticInit InstallHooks([]() {
#ifdef GTASA
    CdeclEvent<AddressList<0x442128, H_CALL>, PRIORITY_BEFORE, ArgPickNone, void()> onResurrection;
#elif GTAVC
    CdeclEvent<AddressList<0x42B9CA, H_CALL>, PRIORITY_BEFORE, ArgPickNone, void()> onResurrection;
#elif GTA3
    CdeclEvent<AddressList<0X421BDE, H_CALL>, PRIORITY_BEFORE, ArgPickNone, void()> onResurrection;
#endif

    onResurrection += [] {
        CHudNew::ReInit();
        MenuNew.Clear();
    };
});
