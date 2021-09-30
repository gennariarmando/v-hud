#include "VHud.h"
#include "CMenuManager.h"
#include "CTimer.h"

#include "PadNew.h"
#include "Utility.h"


using namespace plugin;

CPadNew Pad;

bool bHasPadInHands = false;

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

bool CPadNew::GetOpenCloseMenuJustDown() {
    return
        (NewKeyState.esc && !OldKeyState.esc);
}

bool CPadNew::GetMenuUpJustDown() {
    return
        (NewKeyState.up && !OldKeyState.up)
        || (NewMouseControllerState.wheelUp && !OldMouseControllerState.wheelUp);
}

bool CPadNew::GetMenuDownJustDown() {
    return
        (NewKeyState.down && !OldKeyState.down)
        || (NewMouseControllerState.wheelDown && !OldMouseControllerState.wheelDown);
}

bool CPadNew::GetMenuLeftJustDown() {
    return
        (NewKeyState.left && !OldKeyState.left);
}

bool CPadNew::GetMenuRightJustDown() {
    return
        (NewKeyState.right && !OldKeyState.right);
}

bool CPadNew::GetMenuBackJustDown() {
    return
        (NewKeyState.esc && !OldKeyState.esc);
}

bool CPadNew::GetMenuEnterJustDown() {
    return
        (NewKeyState.enter && !OldKeyState.enter)
        || (NewKeyState.extenter && !OldKeyState.extenter);
}

bool CPadNew::GetMenuSpaceJustDown() {
    return
        (NewKeyState.standardKeys[32] && !OldKeyState.standardKeys[32]);
}

bool CPadNew::GetLeftMouseJustDown() {
    return !!(NewMouseControllerState.lmb && !OldMouseControllerState.lmb);
}

bool CPadNew::GetRightMouseJustDown() {
    return !!(NewMouseControllerState.rmb && !OldMouseControllerState.rmb);
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
    float x = GetPad(0)->NewMouseControllerState.X / (200.0f);
    float y = GetPad(0)->NewMouseControllerState.Y / (200.0f);

    // Little hack
    x = clamp(x, -1.0f, 1.0f);
    y = clamp(y, -1.0f, 1.0f);

    int sx = 1;
    int sy = 1;

    if (FrontEndMenuManager.bInvertMouseX)
        sx = -1;

    if (FrontEndMenuManager.bInvertMouseY)
        sy = -1;

    x *= mult;
    y *= mult;

    return CVector2D(x * sx, y * sy);
}

bool CPadNew::CheckForControllerInput() {
    return !!NewState.RightStickX || !!NewState.RightStickY || !!NewState.LeftStickX || !!NewState.LeftStickY
        || !!NewState.DPadUp || !!NewState.DPadDown || !!NewState.DPadLeft || !!NewState.DPadRight
        || !!NewState.ButtonTriangle || !!NewState.ButtonCross || !!NewState.ButtonCircle || !!NewState.ButtonSquare
        || !!NewState.Start || !!NewState.Select
        || !!NewState.LeftShoulder1 || !!NewState.LeftShoulder2 || !!NewState.RightShoulder1 || !!NewState.RightShoulder2
        || !!NewState.ShockButtonL || !!NewState.ShockButtonR;
}

bool CPadNew::CheckForMouseInput() {
    return !!NewMouseControllerState.X
        || !!NewMouseControllerState.lmb
        || !!NewMouseControllerState.rmb
        || !!NewMouseControllerState.mmb
        || !!NewMouseControllerState.wheelUp
        || !!NewMouseControllerState.wheelDown
        || !!NewMouseControllerState.bmx1
        || !!NewMouseControllerState.bmx2
        || !!NewMouseControllerState.Z
        || !!NewMouseControllerState.X
        || !!NewMouseControllerState.Y;
}