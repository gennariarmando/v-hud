#include "plugin.h"
#include "PadNew.h"
#include "CTimer.h"

using namespace plugin;

CPadNew Pad;

void CPadNew::Init() {


}

void CPadNew::Update () {

}

void CPadNew::Shutdown() {

}


CPadNew::CPadNew() {

}

CPadNew* CPadNew::GetPad(int padNumber) {
    return (CPadNew*)CPad::GetPad(padNumber);
}

bool CPadNew::GetTarget() {
    return false;

    if (DisablePlayerControls || DisablePlayerAim)
        return false;

    if (Mode <= 2)
        return NewState.RightShoulder1;
    if (Mode == 3)
        return NewState.LeftShoulder1;
}

bool CPadNew::GetTargetJustDown() {
    return false;

    if (DisablePlayerControls || DisablePlayerAim)
        return false;

    if (Mode <= 2)
        return NewState.RightShoulder1 && !OldState.RightShoulder1;
    if (Mode == 3)
        return NewState.LeftShoulder1 && !OldState.LeftShoulder1;
}

bool CPadNew::GetShowWeaponWheel() {
    if (DisablePlayerControls || bDisablePlayerCycleWeapon)
        return false;

    return (NewKeyState.tab);
}

bool CPadNew::GetShowWeaponWheelJustUp() {
    if (DisablePlayerControls || bDisablePlayerCycleWeapon)
        return false;

    return !!(!NewKeyState.tab && OldKeyState.tab);
}

int weaponWheelTime = 0;
bool CPadNew::GetShowWeaponWheel(int time) {
    if (GetShowWeaponWheel())
        weaponWheelTime += (int)(50 * CTimer::ms_fTimeStep);
    else
        weaponWheelTime = 0;

    return (weaponWheelTime + CTimer::m_snTimeInMilliseconds > CTimer::m_snTimeInMilliseconds + time) ? GetShowWeaponWheel() : false;
}

bool CPadNew::GetShowPlayerInfo() {
    if (DisablePlayerControls || bDisablePlayerDisplayVitalStats)
        return false;

    return NewKeyState.lmenu;
}

bool CPadNew::GetShowPlayerInfoJustUp() {
    if (DisablePlayerControls || bDisablePlayerDisplayVitalStats)
        return false;

    return !!(!NewKeyState.lmenu && OldKeyState.lmenu);
}

int showPlayerInfoTime = 0;
bool CPadNew::GetShowPlayerInfo(int time) {
    if (GetShowPlayerInfo())
        showPlayerInfoTime += (int)(50 * CTimer::ms_fTimeStep);
    else
        showPlayerInfoTime = 0;

    return (showPlayerInfoTime + CTimer::m_snTimeInMilliseconds > CTimer::m_snTimeInMilliseconds + time) ? GetShowPlayerInfo() : false;
}

bool CPadNew::GetExtendRadarRange() {
    if (DisablePlayerControls)
        return false;

    return NewKeyState.standardKeys[90]; // Z
}

CVector2D CPadNew::GetMouseInput(float mult) {
    float x = GetPad(0)->NewMouseControllerState.X * mult;
    float y = -GetPad(0)->NewMouseControllerState.Y * mult;

    return CVector2D(x, y);
}
