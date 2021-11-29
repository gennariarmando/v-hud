#include "VHud.h"
#include "PadNew.h"
#include "Utility.h"

#include "pugixml.hpp"

#include "CMenuManager.h"
#include "CTimer.h"

using namespace plugin;
using namespace pugi;

CPadNew Pad;

bool bHasPadInHands = false;

char* ControlsFileName = "VHud\\ufiles\\controls.xml";

CControls Controls[NUM_CONTROL_ACTIONS] = {
    { "PED_FIREWEAPON", 2001 },
    { "PED_FIREWEAPON_ALT", -1 },
    { "PED_CYCLE_WEAPON_RIGHT", 2005 },
    { "PED_CYCLE_WEAPON_LEFT", 2004 },
    { "GO_FORWARD", 'W' },
    { "GO_BACK", 'S' },
    { "GO_LEFT", 'A' },
    { "GO_RIGHT", 'D' },
    { "PED_SNIPER_ZOOM_IN", 2004 },
    { "PED_SNIPER_ZOOM_OUT", 2005 },
    { "VEHICLE_ENTER_EXIT", 'F' },
    { "CAMERA_CHANGE_VIEW_ALL_SITUATIONS", 'V' },
    { "PED_JUMPING", 32 },
    { "PED_SPRINT", 1046 },
    { "PED_LOOKBEHIND", 'C' },
    { "PED_DUCK", 1049 },
    { "PED_ANSWER_PHONE", 1043 },
    { "SNEAK_ABOUT", 1049 },
    { "VEHICLE_FIREWEAPON", 2001 },
    { "VEHICLE_FIREWEAPON_ALT", 1049 },
    { "VEHICLE_STEERLEFT", 'A' },
    { "VEHICLE_STEERRIGHT", 'D' },
    { "VEHICLE_STEERUP", 1017 },
    { "VEHICLE_STEERDOWN", 1018 },
    { "VEHICLE_ACCELERATE", 'W' },
    { "VEHICLE_BRAKE", 'S' },
    { "VEHICLE_RADIO_STATION_UP", 2004 },
    { "VEHICLE_RADIO_STATION_DOWN", 2005 },
    { "UNKNOWN_1", 0 },
    { "VEHICLE_HORN", 1046 },
    { "TOGGLE_SUBMISSIONS", 1049 },
    { "VEHICLE_HANDBRAKE", 32 },
    { "PED_1RST_PERSON_LOOK_LEFT", 'Q' },
    { "PED_1RST_PERSON_LOOK_RIGHT", 'E'},
    { "VEHICLE_LOOKLEFT", 'Q' },
    { "VEHICLE_LOOKRIGHT", 'E'},
    { "VEHICLE_LOOKBEHIND", 'C'},
    { "VEHICLE_MOUSELOOK", -1 },
    { "VEHICLE_TURRETLEFT", -1 },
    { "VEHICLE_TURRETRIGHT", -1 },
    { "VEHICLE_TURRETUP", -1 },
    { "VEHICLE_TURRETDOWN", -1 },
    { "PED_CYCLE_TARGET_LEFT", 'Q'},
    { "PED_CYCLE_TARGET_RIGHT", 'E' },
    { "PED_CENTER_CAMERA_BEHIND_PLAYER", -1 },
    { "PED_LOCK_TARGET", 2003 },
    { "NETWORK_TALK", -1 },
    { "CONVERSATION_YES", 'Y' },
    { "CONVERSATION_NO", 'N' },
    { "GROUP_CONTROL_FWD", 'Y' },
    { "GROUP_CONTROL_BWD", 'N' },
    { "PED_1RST_PERSON_LOOK_UP", -1 },
    { "PED_1RST_PERSON_LOOK_DOWN", -1 },
    { "UNKNOWN_2", -1 },
    { "TOGGLE_DPAD", -1 },
    { "SWITCH_DEBUG_CAM_ON", -1 },
    { "TAKE_SCREEN_SHOT", -1 },
    { "SHOW_MOUSE_POINTER_TOGGLE", -1 },
    { "SHOW_WEAPON_WHEEL", 1043 },
    { "EXTEND_RADAR_RANGE", 'Z' },
    { "SHOW_PLAYER_STATS", 1051 },
    { "PHONE_SHOW", 2002 },
    { "PHONE_HIDE", 2003 },
    { "PHONE_UP", 2004 },
    { "PHONE_DOWN", 2005 },
    { "PHONE_ENTER", 2001 },
};

CPadNew::CPadNew() {
    patch::RedirectJump(0x748995, (void*)0x7489D4);
}

void CPadNew::SaveSettings() {
    pugi::xml_document doc;

    auto declarationNode = doc.append_child(pugi::node_declaration);
    declarationNode.append_attribute("version") = "1.0";
    declarationNode.append_attribute("encoding") = "UTF-8";

    auto controls = doc.append_child("Controls");
    controls.append_child("NUM_CONTROL_ACTIONS").append_attribute("value").set_value(NUM_CONTROL_ACTIONS);

    for (int i = 0; i < NUM_CONTROL_ACTIONS; i++) {
        controls.append_child(Controls[i].action).append_attribute("value").set_value(Controls[i].key);
    }

    bool file = doc.save_file(PLUGIN_PATH(ControlsFileName));
    if (!file) {
        printf("XML: Controls file can't be saved.");
    }

    PassControlsToCurrentGame(Controls);
}

void CPadNew::LoadSettings() {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 59; j++) {
            SetControllerKeyAssociatedWithAction((e_ControllerAction)j, -1, (eControllerType)i);
        }
    }

    pugi::xml_document doc;
    xml_parse_result file = doc.load_file(PLUGIN_PATH(ControlsFileName));

    if (file) {
        auto controls = doc.child("Controls");
        auto actions = controls.child("ActionsCount").attribute("value");

        if (!actions || actions.as_int() != NUM_CONTROL_ACTIONS) {
            printf("XML: Controls file is not compatible.");
        }
        else {
            for (int i = 0; i < NUM_CONTROL_ACTIONS; i++) {
                Controls[i].key = controls.child(Controls[i].action).attribute("value").as_int();
            }
        }
    }

    PassControlsToCurrentGame(Controls);
}

void CPadNew::PassControlsToCurrentGame(const CControls* c) {
    for (int i = 0; i < 59; i++) {
        int k = c[i].key;
        int t = 0;

        if (k >= 2000) {
            t = CONTROLLER_MOUSE;
            k -= 2000;
        }
        else
            t = CONTROLLER_KEYBOARD1;

        SetControllerKeyAssociatedWithAction((e_ControllerAction)i, k, (eControllerType)t);
    }
}

void CPadNew::SetControllerKeyAssociatedWithAction(e_ControllerAction action, int key, eControllerType type) {
    if (key == -1)
        key = 1056;

    ControlsManager.ResetSettingOrder(action);

    ControlsManager.m_actions[action].keys[type].keyCode = key;
    ControlsManager.m_actions[action].keys[type].priority = GetNumOfSettingsForAction(action) + 1;
}

int CPadNew::GetNumOfSettingsForAction(e_ControllerAction action) {
    int n = 0;

    if (ControlsManager.m_actions[action].keys[0].keyCode != 1056)
        n++;

    if (ControlsManager.m_actions[action].keys[1].keyCode != 1056)
        n++;

    if (ControlsManager.m_actions[action].keys[2].keyCode != 0)
        n++;

    if (ControlsManager.m_actions[action].keys[3].keyCode != 0)
        n++;

    return n;
}

bool CPadNew::GetKeyDown(int key) {
    if (key < 255) {
        if (NewKeyState.standardKeys[key])
            return true;
    }
    else if (key > 2000) {
        key -= 2000;

        switch (key) {
        case rsMOUSELEFTBUTTON:
            if (NewMouseControllerState.lmb)
                return true;
            break;
        case rsMOUSMIDDLEBUTTON:
            if (NewMouseControllerState.mmb)
                return true;
            break;
        case rsMOUSERIGHTBUTTON:
            if (NewMouseControllerState.rmb)
                return true;
            break;
        case rsMOUSEWHEELUPBUTTON:
            if (NewMouseControllerState.wheelUp)
                return true;
            break;
        case rsMOUSEWHEELDOWNBUTTON:
            if (NewMouseControllerState.wheelDown)
                return true;
            break;
        case rsMOUSEX1BUTTON:
            if (NewMouseControllerState.bmx1)
                return true;
            break;
        case rsMOUSEX2BUTTON:
            if (NewMouseControllerState.bmx2)
                return true;
            break;
        default: 
            break;
        }
    }

    for (int i = 0; i < 12; i++) {
        if (i + rsF1 == key) {
            if (NewKeyState.FKeys[i])
                return true;
        }
    }

    switch (key) {
    case rsESC:
        if (NewKeyState.esc)
            return true;
        break;
    case rsINS:
        if (NewKeyState.insert)
            return true;
        break;
    case rsDEL:
        if (NewKeyState.del)
            return true;
        break;
    case rsHOME:
        if (NewKeyState.home)
            return true;
        break;
    case rsEND:
        if (NewKeyState.end)
            return true;
        break;
    case rsPGUP:
        if (NewKeyState.pgup)
            return true;
        break;
    case rsPGDN:
        if (NewKeyState.pgdn)
            return true;
        break;
    case rsUP:
        if (NewKeyState.up)
            return true;
        break;
    case rsDOWN:
        if (NewKeyState.down)
            return true;
        break;
    case rsLEFT:
        if (NewKeyState.left)
            return true;
        break;
    case rsRIGHT:
        if (NewKeyState.right)
            return true;
        break;
    case rsSCROLL:
        if (NewKeyState.scroll)
            return true;
        break;
    case rsPAUSE:
        if (NewKeyState.pause)
            return true;
        break;
    case rsNUMLOCK:
        if (NewKeyState.numlock)
            return true;
        break;
    case rsDIVIDE:
        if (NewKeyState.div)
            return true;
        break;
    case rsTIMES:
        if (NewKeyState.mul)
            return true;
        break;
    case rsMINUS:
        if (NewKeyState.sub)
            return true;
        break;
    case rsPLUS:
        if (NewKeyState.add)
            return true;
        break;
    case rsPADENTER:
        if (NewKeyState.enter)
            return true;
        break;
    case rsPADDEL:
        if (NewKeyState.decimal)
            return true;
        break;
    case rsPADEND:
        if (NewKeyState.num1)
            return true;
        break;
    case rsPADDOWN:
        if (NewKeyState.num2)
            return true;
        break;
    case rsPADPGDN:
        if (NewKeyState.num3)
            return true;
        break;
    case rsPADLEFT:
        if (NewKeyState.num4)
            return true;
        break;
    case rsPAD5:
        if (NewKeyState.num5)
            return true;
        break;
    case rsPADRIGHT:
        if (NewKeyState.num6)
            return true;
        break;
    case rsPADHOME:
        if (NewKeyState.num7)
            return true;
        break;
    case rsPADUP:
        if (NewKeyState.num8)
            return true;
        break;
    case rsPADPGUP:
        if (NewKeyState.num9)
            return true;
        break;
    case rsPADINS:
        if (NewKeyState.num0)
            return true;
        break;
    case rsBACKSP:
        if (NewKeyState.back)
            return true;
        break;
    case rsTAB:
        if (NewKeyState.tab)
            return true;
        break;
    case rsCAPSLK:
        if (NewKeyState.capslock)
            return true;
        break;
    case rsENTER:
        if (NewKeyState.extenter)
            return true;
        break;
    case rsLSHIFT:
        if (NewKeyState.lshift)
            return true;
        break;
    case rsSHIFT:
        if (NewKeyState.shift)
            return true;
        break;
    case rsRSHIFT:
        if (NewKeyState.rshift)
            return true;
        break;
    case rsLCTRL:
        if (NewKeyState.lctrl)
            return true;
        break;
    case rsRCTRL:
        if (NewKeyState.rctrl)
            return true;
        break;
    case rsLALT:
        if (NewKeyState.lmenu)
            return true;
        break;
    case rsRALT:
        if (NewKeyState.rmenu)
            return true;
        break;
    case rsLWIN:
        if (NewKeyState.lwin)
            return true;
        break;
    case rsRWIN:
        if (NewKeyState.rwin)
            return true;
        break;
    case rsAPPS:
        if (NewKeyState.apps)
            return true;
        break;
    default: 
        break;
    }

    return false;
}

bool CPadNew::GetKeyJustDown(int key) {
    if (key < 255) {
        if (NewKeyState.standardKeys[key] && !OldKeyState.standardKeys[key])
            return true;
    }
    else if (key > 2000) {
        key -= 2000;

        switch (key) {
        case rsMOUSELEFTBUTTON:
            if (NewMouseControllerState.lmb && !OldMouseControllerState.lmb)
                return true;
            break;
        case rsMOUSMIDDLEBUTTON:
            if (NewMouseControllerState.mmb && !OldMouseControllerState.mmb)
                return true;
            break;
        case rsMOUSERIGHTBUTTON:
            if (NewMouseControllerState.rmb && !OldMouseControllerState.rmb)
                return true;
            break;
        case rsMOUSEWHEELUPBUTTON:
            if (NewMouseControllerState.wheelUp && !OldMouseControllerState.wheelUp)
                return true;
            break;
        case rsMOUSEWHEELDOWNBUTTON:
            if (NewMouseControllerState.wheelDown && !OldMouseControllerState.wheelDown)
                return true;
            break;
        case rsMOUSEX1BUTTON:
            if (NewMouseControllerState.bmx1 && !OldMouseControllerState.bmx1)
                return true;
            break;
        case rsMOUSEX2BUTTON:
            if (NewMouseControllerState.bmx2 && !OldMouseControllerState.bmx2)
                return true;
            break;
        default:
            break;
        }
    }

    for (int i = 0; i < 12; i++) {
        if (i + rsF1 == key) {
            if (NewKeyState.FKeys[i] && !OldKeyState.FKeys[i])
                return true;
        }
    }

    switch (key) {
    case rsESC:
        if (NewKeyState.esc && !OldKeyState.esc)
            return true;
        break;
    case rsINS:
        if (NewKeyState.insert && !OldKeyState.insert)
            return true;
        break;
    case rsDEL:
        if (NewKeyState.del && !OldKeyState.del)
            return true;
        break;
    case rsHOME:
        if (NewKeyState.home && !OldKeyState.home)
            return true;
        break;
    case rsEND:
        if (NewKeyState.end && !OldKeyState.end)
            return true;
        break;
    case rsPGUP:
        if (NewKeyState.pgup && !OldKeyState.pgup)
            return true;
        break;
    case rsPGDN:
        if (NewKeyState.pgdn && !OldKeyState.pgdn)
            return true;
        break;
    case rsUP:
        if (NewKeyState.up && !OldKeyState.up)
            return true;
        break;
    case rsDOWN:
        if (NewKeyState.down && !OldKeyState.down)
            return true;
        break;
    case rsLEFT:
        if (NewKeyState.left && !OldKeyState.left)
            return true;
        break;
    case rsRIGHT:
        if (NewKeyState.right && !OldKeyState.right)
            return true;
        break;
    case rsSCROLL:
        if (NewKeyState.scroll && !OldKeyState.scroll)
            return true;
        break;
    case rsPAUSE:
        if (NewKeyState.pause && !OldKeyState.pause)
            return true;
        break;
    case rsNUMLOCK:
        if (NewKeyState.numlock && !OldKeyState.numlock)
            return true;
        break;
    case rsDIVIDE:
        if (NewKeyState.div && !OldKeyState.div)
            return true;
        break;
    case rsTIMES:
        if (NewKeyState.mul && !OldKeyState.mul)
            return true;
        break;
    case rsMINUS:
        if (NewKeyState.sub && !OldKeyState.sub)
            return true;
        break;
    case rsPLUS:
        if (NewKeyState.add && !OldKeyState.add)
            return true;
        break;
    case rsPADENTER:
        if (NewKeyState.enter && !OldKeyState.enter)
            return true;
        break;
    case rsPADDEL:
        if (NewKeyState.decimal && !OldKeyState.decimal)
            return true;
        break;
    case rsPADEND:
        if (NewKeyState.num1 && !OldKeyState.num1)
            return true;
        break;
    case rsPADDOWN:
        if (NewKeyState.num2 && !OldKeyState.num2)
            return true;
        break;
    case rsPADPGDN:
        if (NewKeyState.num3 && !OldKeyState.num3)
            return true;
        break;
    case rsPADLEFT:
        if (NewKeyState.num4 && !OldKeyState.num4)
            return true;
        break;
    case rsPAD5:
        if (NewKeyState.num5 && !OldKeyState.num5)
            return true;
        break;
    case rsPADRIGHT:
        if (NewKeyState.num6 && !OldKeyState.num6)
            return true;
        break;
    case rsPADHOME:
        if (NewKeyState.num7 && !OldKeyState.num7)
            return true;
        break;
    case rsPADUP:
        if (NewKeyState.num8 && !OldKeyState.num8)
            return true;
        break;
    case rsPADPGUP:
        if (NewKeyState.num9 && !OldKeyState.num9)
            return true;
        break;
    case rsPADINS:
        if (NewKeyState.num0 && !OldKeyState.num0)
            return true;
        break;
    case rsBACKSP:
        if (NewKeyState.back && !OldKeyState.back)
            return true;
        break;
    case rsTAB:
        if (NewKeyState.tab && !OldKeyState.tab)
            return true;
        break;
    case rsCAPSLK:
        if (NewKeyState.capslock && !OldKeyState.capslock)
            return true;
        break;
    case rsENTER:
        if (NewKeyState.extenter && !OldKeyState.extenter)
            return true;
        break;
    case rsLSHIFT:
        if (NewKeyState.lshift && !OldKeyState.lshift)
            return true;
        break;
    case rsSHIFT:
        if (NewKeyState.shift && !OldKeyState.shift)
            return true;
        break;
    case rsRSHIFT:
        if (NewKeyState.rshift && !OldKeyState.rshift)
            return true;
        break;
    case rsLCTRL:
        if (NewKeyState.lctrl && !OldKeyState.lctrl)
            return true;
        break;
    case rsRCTRL:
        if (NewKeyState.rctrl && !OldKeyState.rctrl)
            return true;
        break;
    case rsLALT:
        if (NewKeyState.lmenu && !OldKeyState.lmenu)
            return true;
        break;
    case rsRALT:
        if (NewKeyState.rmenu && !OldKeyState.rmenu)
            return true;
        break;
    case rsLWIN:
        if (NewKeyState.lwin && !OldKeyState.lwin)
            return true;
        break;
    case rsRWIN:
        if (NewKeyState.rwin && !OldKeyState.rwin)
            return true;
        break;
    case rsAPPS:
        if (NewKeyState.apps && !OldKeyState.apps)
            return true;
        break;
    default: 
        break;
    }

    return false;
}

bool CPadNew::GetKeyUp(int key) {
    return !GetKeyDown(key) && !GetKeyJustDown(key);
}

CPadNew* CPadNew::GetPad(int padNumber) {
    return (CPadNew*)CPad::GetPad(padNumber);
}

bool CPadNew::GetOpenCloseMenuJustDown() {
    return
        (NewKeyState.esc && !OldKeyState.esc);
}

bool CPadNew::GetMenuMapZoomIn() {
    return (NewKeyState.pgup);
}

bool CPadNew::GetMenuMapZoomOut() {
    return (NewKeyState.pgdn);
}

bool CPadNew::GetMenuMapZoomInJustDown() {
    return (NewMouseControllerState.wheelUp && !OldMouseControllerState.wheelUp);
}

bool CPadNew::GetMenuMapZoomOutJustDown() {
    return (NewMouseControllerState.wheelDown && !OldMouseControllerState.wheelDown);
}

bool CPadNew::GetMenuUp() {
    return (NewKeyState.up);
}

bool CPadNew::GetMenuDown() {
    return (NewKeyState.down);
}

bool CPadNew::GetMenuLeft() {
    return (NewKeyState.left);
}

bool CPadNew::GetMenuRight() {
    return (NewKeyState.right);
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

bool CPadNew::GetLeftMouseDown() {
    return !!(NewMouseControllerState.lmb);
}

int leftMouseCount = 0;
int leftMouseCountTime = 0;
bool CPadNew::GetLeftMouseDoubleClickJustDown() {
    if (leftMouseCountTime < CTimer::m_snTimeInMillisecondsPauseMode) {
        leftMouseCountTime = 0;
        leftMouseCount = 0;
    }

    if (GetLeftMouseJustDown()) {
        leftMouseCount++;
        leftMouseCountTime = CTimer::m_snTimeInMillisecondsPauseMode + 250;
    }

    if (leftMouseCount >= 2) {
        leftMouseCount = 0;
        leftMouseCountTime = 0;
        return true;
    }

    return false;
}

bool CPadNew::GetMiddleMouseDown() {
    return !!(NewMouseControllerState.mmb);
}

bool CPadNew::GetMiddleMouseJustDown() {
    return !!(NewMouseControllerState.mmb && !OldMouseControllerState.mmb);
}

bool CPadNew::GetMenuShowHideLegendJustDown() {
    return !!(NewKeyState.standardKeys[76] && !OldKeyState.standardKeys[76]);
}

bool CPadNew::GetPhoneShowJustDown() {
    return GetKeyJustDown(Controls[PHONE_SHOW].key);
}

bool CPadNew::GetPhoneHideJustDown() {
    return GetKeyJustDown(Controls[PHONE_HIDE].key);
}

bool CPadNew::GetPhoneUpJustDown() {
    return GetKeyJustDown(Controls[PHONE_UP].key);
}

bool CPadNew::GetPhoneDownJustDown() {
    return GetKeyJustDown(Controls[PHONE_DOWN].key);
}

bool CPadNew::GetPhoneEnterJustDown() {
    return GetKeyJustDown(Controls[PHONE_ENTER].key);
}

bool CPadNew::GetLeftMouseJustDown() {
    return !!(NewMouseControllerState.lmb && !OldMouseControllerState.lmb);
}

bool CPadNew::GetLeftMouseJustUp() {
    return !!(!NewMouseControllerState.lmb && OldMouseControllerState.lmb);
}

bool CPadNew::GetRightMouseJustDown() {
    return !!(NewMouseControllerState.rmb && !OldMouseControllerState.rmb);
}

bool CPadNew::GetShowWeaponWheel() {
    if (DisablePlayerControls || bDisablePlayerCycleWeapon)
        return false;

    return GetKeyDown(Controls[SHOW_WEAPON_WHEEL].key);
}

bool CPadNew::GetShowWeaponWheelJustUp() {
    if (DisablePlayerControls || bDisablePlayerCycleWeapon)
        return false;

    return GetKeyUp(Controls[SHOW_WEAPON_WHEEL].key);
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

    return GetKeyDown(Controls[SHOW_PLAYER_STATS].key);
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

    return GetKeyDown(Controls[EXTEND_RADAR_RANGE].key);
}

bool CPadNew::CycleRadioStationLeftJustDown() {
    if (DisablePlayerControls)
        return false;

    return GetKeyJustDown(Controls[CA_VEHICLE_RADIO_STATION_DOWN].key);
}

bool CPadNew::CycleRadioStationRightJustDown() {
    if (DisablePlayerControls)
        return false;

    return GetKeyJustDown(Controls[CA_VEHICLE_RADIO_STATION_UP].key);
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