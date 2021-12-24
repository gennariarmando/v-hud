#include "VHud.h"
#include "PadNew.h"
#include "Utility.h"

#include "pugixml.hpp"

#include "CMenuManager.h"
#include "CTimer.h"

using namespace plugin;
using namespace pugi;

CPadNew Pad;
IGInputPad* GInputPad[2] = { NULL, NULL };
GINPUT_PAD_SETTINGS GInputPadSettings[2];
bool GINPUT = false;

char* ControlsFileName = "VHud\\ufiles\\controls.xml";
 
#define MOUSE_CUSTOM_OFFSET (2000)
#define MOUSE(x) x + MOUSE_CUSTOM_OFFSET
#define KEY(x) x
CControls Controls[NUM_CONTROL_ACTIONS] = {
    { "PED_FIREWEAPON", MOUSE(rsMOUSELEFTBUTTON), GAMEPAD_CIRCLE },
    { "PED_FIREWEAPON_ALT", KEY(rsNULL), GAMEPAD_LEFTSHOULDER1 },
    { "PED_CYCLE_WEAPON_RIGHT", MOUSE(rsMOUSEWHEELDOWNBUTTON), GAMEPAD_RIGHTSHOULDER2 },
    { "PED_CYCLE_WEAPON_LEFT", MOUSE(rsMOUSEWHEELUPBUTTON), GAMEPAD_LEFTSHOULDER2 },
    { "GO_FORWARD", KEY('W'), GAMEPAD_THUMBLYU },
    { "GO_BACK", KEY('S'), GAMEPAD_THUMBLYD },
    { "GO_LEFT", KEY('A'), GAMEPAD_THUMBLXL },
    { "GO_RIGHT", KEY('D'), GAMEPAD_THUMBLXR },
    { "PED_SNIPER_ZOOM_IN", MOUSE(rsMOUSEWHEELUPBUTTON), GAMEPAD_LEFTSHOULDER2 },
    { "PED_SNIPER_ZOOM_OUT", MOUSE(rsMOUSEWHEELDOWNBUTTON), GAMEPAD_RIGHTSHOULDER2 },
    { "VEHICLE_ENTER_EXIT", KEY('F'), GAMEPAD_TRIANGLE },
    { "CAMERA_CHANGE_VIEW_ALL_SITUATIONS", KEY('V'), GAMEPAD_SELECT },
    { "PED_JUMPING", rsSPACE, GAMEPAD_SQUARE },
    { "PED_SPRINT", KEY(rsLSHIFT), GAMEPAD_CROSS },
    { "PED_LOOKBEHIND", KEY('C'), GAMEPAD_THUMBR },
    { "PED_DUCK", KEY(rsLCTRL), GAMEPAD_THUMBL },
    { "PED_ANSWER_PHONE", KEY(rsTAB), GAMEPAD_LEFTSHOULDER1 },
    { "SNEAK_ABOUT", KEY(rsLCTRL), GAMEPAD_NONE },
    { "VEHICLE_FIREWEAPON", MOUSE(rsMOUSELEFTBUTTON), GAMEPAD_CIRCLE },
    { "VEHICLE_FIREWEAPON_ALT", KEY(rsLCTRL), GAMEPAD_LEFTSHOULDER1 },
    { "VEHICLE_STEERLEFT", KEY('A'), GAMEPAD_DPADLEFT },
    { "VEHICLE_STEERRIGHT", KEY('D'), GAMEPAD_DPADRIGHT },
    { "VEHICLE_STEERUP", KEY(rsPGUP), GAMEPAD_DPADUP },
    { "VEHICLE_STEERDOWN", KEY(rsPGDN), GAMEPAD_DPADDOWN },
    { "VEHICLE_ACCELERATE", KEY('W'), GAMEPAD_CROSS },
    { "VEHICLE_BRAKE", KEY('S'), GAMEPAD_SQUARE },
    { "VEHICLE_RADIO_STATION_UP", MOUSE(rsMOUSEWHEELUPBUTTON), GAMEPAD_NONE },
    { "VEHICLE_RADIO_STATION_DOWN", MOUSE(rsMOUSEWHEELDOWNBUTTON), GAMEPAD_NONE },
    { "UNKNOWN_1", KEY(rsNULL), GAMEPAD_NONE },
    { "VEHICLE_HORN", KEY(rsLSHIFT), GAMEPAD_THUMBL },
    { "TOGGLE_SUBMISSIONS", KEY(rsLCTRL), GAMEPAD_THUMBR },
    { "VEHICLE_HANDBRAKE", KEY(rsSPACE), GAMEPAD_RIGHTSHOULDER1 },
    { "PED_1RST_PERSON_LOOK_LEFT", KEY('Q'), GAMEPAD_NONE },
    { "PED_1RST_PERSON_LOOK_RIGHT", KEY('E'), GAMEPAD_NONE },
    { "VEHICLE_LOOKLEFT", KEY('Q'), GAMEPAD_LEFTSHOULDER2 },
    { "VEHICLE_LOOKRIGHT", KEY('E'), GAMEPAD_RIGHTSHOULDER2 },
    { "VEHICLE_LOOKBEHIND", KEY('C'), GAMEPAD_THUMBR },
    { "VEHICLE_MOUSELOOK",  KEY(rsNULL), GAMEPAD_NONE },
    { "VEHICLE_TURRETLEFT",  KEY(rsPADLEFT), GAMEPAD_THUMBRXL },
    { "VEHICLE_TURRETRIGHT",  KEY(rsPAD5), GAMEPAD_THUMBRXR },
    { "VEHICLE_TURRETUP", KEY(rsPADPGUP), GAMEPAD_THUMBRYU },
    { "VEHICLE_TURRETDOWN", KEY(rsPADRIGHT), GAMEPAD_THUMBRYD },
    { "PED_CYCLE_TARGET_LEFT", KEY('Q'), GAMEPAD_LEFTSHOULDER2 },
    { "PED_CYCLE_TARGET_RIGHT", KEY('E'), GAMEPAD_RIGHTSHOULDER2 },
    { "PED_CENTER_CAMERA_BEHIND_PLAYER", KEY(rsNULL), GAMEPAD_NONE },
    { "PED_LOCK_TARGET", MOUSE(rsMOUSERIGHTBUTTON), GAMEPAD_NONE },
    { "NETWORK_TALK", KEY(rsNULL), GAMEPAD_NONE },
    { "CONVERSATION_YES", KEY('Y'), GAMEPAD_DPADRIGHT },
    { "CONVERSATION_NO", KEY('N'), GAMEPAD_DPADLEFT },
    { "GROUP_CONTROL_FWD", KEY('Y'), GAMEPAD_DPADUP },
    { "GROUP_CONTROL_BWD", KEY('N'), GAMEPAD_DPADDOWN },
    { "PED_1RST_PERSON_LOOK_UP", KEY(rsNULL), GAMEPAD_NONE },
    { "PED_1RST_PERSON_LOOK_DOWN", KEY(rsNULL), GAMEPAD_NONE },
    { "UNKNOWN_2", KEY(rsNULL), GAMEPAD_NONE },
    { "TOGGLE_DPAD", KEY(rsNULL), GAMEPAD_NONE },
    { "SWITCH_DEBUG_CAM_ON", KEY(rsNULL), GAMEPAD_NONE },
    { "TAKE_SCREEN_SHOT", KEY(rsNULL), GAMEPAD_NONE },
    { "SHOW_MOUSE_POINTER_TOGGLE", KEY(rsNULL), GAMEPAD_NONE },
    { "SHOW_WEAPON_WHEEL", KEY(rsTAB), GAMEPAD_LEFTSHOULDER1 },
    { "EXTEND_RADAR_RANGE", KEY('Z'), GAMEPAD_DPADDOWN },
    { "SHOW_PLAYER_STATS", KEY(rsLALT), GAMEPAD_DPADDOWN },
    { "PHONE_SHOW", MOUSE(rsMOUSEMIDDLEBUTTON), GAMEPAD_DPADUP },
    { "PHONE_HIDE", MOUSE(rsMOUSERIGHTBUTTON), GAMEPAD_CIRCLE },
    { "PHONE_UP", KEY(rsUP), GAMEPAD_DPADUP },
    { "PHONE_DOWN", KEY(rsDOWN), GAMEPAD_DPADDOWN },
    { "PHONE_LEFT", MOUSE(rsMOUSEWHEELUPBUTTON), GAMEPAD_DPADLEFT },
    { "PHONE_RIGHT", MOUSE(rsMOUSEWHEELDOWNBUTTON), GAMEPAD_DPADRIGHT },
    { "PHONE_ENTER", MOUSE(rsMOUSELEFTBUTTON), GAMEPAD_CROSS },
    { "MENU_SHOW_HIDE_LEGEND", KEY('L'), GAMEPAD_SQUARE },
    { "MENU_PLACE_WAYPOINT", KEY(rsENTER), GAMEPAD_CROSS },
    { "MENU_DELETE_SAVE", KEY(' '), GAMEPAD_SQUARE },
    { "MENU_APPLY_CHANGES", KEY(' '), GAMEPAD_SQUARE },
    { "MENU_BACK", KEY(rsESC), GAMEPAD_CIRCLE },
    { "MENU_SELECT", KEY(rsENTER), GAMEPAD_CROSS }
};

const char* controlKeysStrings[62] = {
    "ESC",
    "F1",
    "F2",
    "F3",
    "F4",
    "F5",
    "F6",
    "F7",
    "F8",
    "F9",
    "F10",
    "F11",
    "F12",
    "INS",
    "DEL",
    "HOME",
    "END",
    "PGUP",
    "PGDN",
    "UP",
    "DOWN",
    "LEFT",
    "RIGHT",
    "DIVIDE",
    "TIMES",
    "PLUS",
    "MINUS",
    "PADDEL",
    "PADEND",
    "PADDOWN",
    "PADPGDN",
    "PADLEFT",
    "PAD5",
    "NUMLOCK",
    "PADRIGHT",
    "PADHOME",
    "PADUP",
    "PADPGUP",
    "PADINS",
    "PADENTER",
    "SCROLL",
    "PAUSE",
    "BACKSP",
    "TAB",
    "CAPSLK",
    "ENTER",
    "LSHIFT",
    "RSHIFT",
    "SHIFT",
    "LCTRL",
    "RCTRL",
    "LALT",
    "RALT",
    "LWIN",
    "RWIN",
    "APPS",
    "NULL",
    "LMB",
    "MMB",
    "RMB",
    "MWHU",
    "MWHD",
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

    for (int i = 0; i < NUM_CONTROL_ACTIONS; i++) {
        controls.append_child(Controls[i].action).append_attribute("value").set_value(KeyToString(Controls[i].key));
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

        for (int i = 0; i < NUM_CONTROL_ACTIONS; i++) {
            char tmp[16];
            sprintf(tmp, "%s", controls.child(Controls[i].action).attribute("value").as_string());
            Controls[i].key = StringToKey(tmp);
        }
    }

    PassControlsToCurrentGame(Controls);
}

int CPadNew::StringToKey(const char* str) {
    int key = str[0];

    if (str[0] && str[1] != '\0') {
        if (!faststrcmp(str, "SPACEBAR"))
            return rsSPACE;

        for (int i = 0; i < ARRAY_SIZE(controlKeysStrings); i++) {
            if (!strncmp(controlKeysStrings[i], str, sizeof(controlKeysStrings[i]))) {
                if (i > (rsNULL - rsESC))
                    key = (i - (rsNULL - rsESC)) + MOUSE_CUSTOM_OFFSET;
                else
                    key = i + rsESC;
            }
        }
    }

    return key;
}

const char* CPadNew::KeyToString(int key) {
    if (key >= rsESC && key < MOUSE_CUSTOM_OFFSET) {
        return controlKeysStrings[key - rsESC];
    }
    else if (key > MOUSE_CUSTOM_OFFSET) {
        return controlKeysStrings[(key + rsNULL) - (MOUSE_CUSTOM_OFFSET + rsESC)];
    }
    else {
        char c = (char)key;

        if (c == ' ')
            return "SPACEBAR";

        char* buff = new char[2]{};
        sprintf(buff, "%c", c);
        return buff;
    }
}

void CPadNew::PassControlsToCurrentGame(const CControls* c) {
    for (int i = 0; i < 59; i++) {
        int k = c[i].key;
        int t = 0;

        if (k >= MOUSE_CUSTOM_OFFSET) {
            t = CONTROLLER_MOUSE;
            k -= MOUSE_CUSTOM_OFFSET;
        }
        else
            t = CONTROLLER_KEYBOARD1;

        SetControllerKeyAssociatedWithAction((e_ControllerAction)i, k, (eControllerType)t);
    }
}

void CPadNew::SetControllerKeyAssociatedWithAction(e_ControllerAction action, int key, eControllerType type) {
    if (key == -1)
        key = rsNULL;

    ControlsManager.ResetSettingOrder(action);

    ControlsManager.m_actions[action].keys[type].keyCode = key;
    ControlsManager.m_actions[action].keys[type].priority = GetNumOfSettingsForAction(action) + 1;
}

int CPadNew::GetNumOfSettingsForAction(e_ControllerAction action) {
    int n = 0;

    if (ControlsManager.m_actions[action].keys[0].keyCode != rsNULL)
        n++;

    if (ControlsManager.m_actions[action].keys[1].keyCode != rsNULL)
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
    else if (key > MOUSE_CUSTOM_OFFSET) {
        key -= MOUSE_CUSTOM_OFFSET;

        switch (key) {
        case rsMOUSELEFTBUTTON:
            if (NewMouseControllerState.lmb)
                return true;
            break;
        case rsMOUSEMIDDLEBUTTON:
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
    else if (key > MOUSE_CUSTOM_OFFSET) {
        key -= MOUSE_CUSTOM_OFFSET;

        switch (key) {
        case rsMOUSELEFTBUTTON:
            if (NewMouseControllerState.lmb && !OldMouseControllerState.lmb)
                return true;
            break;
        case rsMOUSEMIDDLEBUTTON:
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

void CPadNew::GInputUpdate() {
    const HMODULE h = ModuleList().Get(GINPUT_FILENAMEW);
    if (h) {
        GInput_Load(GInputPad);

        for (int i = 0; i < 2; i++) {
            CPadNew* pad = CPadNew::GetPad(i);
            IGInputPad* gInput = GInputPad[i];
            GINPUT_PAD_SETTINGS* s = &GInputPadSettings[i];

            if (gInput) {
                pad->HasPadInHands = gInput->HasPadInHands();

                s->cbSize = sizeof(GINPUT_PAD_SETTINGS);
                gInput->SendEvent(GINPUT_EVENT_FETCH_PAD_SETTINGS, s);
            }
        }
        GINPUT = true;
    }
}

void CPadNew::GInputRelease() {
    GInput_Release();
}

bool CPadNew::GetOpenCloseMenuJustDown() {
    if (HasPadInHands)
        return (NewState.Start && !OldState.Start);

    return
        (NewKeyState.esc && !OldKeyState.esc);
}

bool CPadNew::GetMenuMapZoomIn() {
    if (HasPadInHands)
        return NewState.RightShoulder2;

    return (NewKeyState.pgup);
}

bool CPadNew::GetMenuMapZoomOut() {
    if (HasPadInHands)
        return NewState.LeftShoulder2;

    return (NewKeyState.pgdn);
}

bool CPadNew::GetMenuMapZoomInJustDown() {
    return (NewMouseControllerState.wheelUp && !OldMouseControllerState.wheelUp);
}

bool CPadNew::GetMenuMapZoomOutJustDown() {
    return (NewMouseControllerState.wheelDown && !OldMouseControllerState.wheelDown);
}

bool CPadNew::GetMenuUp() {
    if (HasPadInHands)
        return NewState.DPadUp || NewState.LeftStickY < 0;

    return (NewKeyState.up) || (GetKeyDown('W'));
}

bool CPadNew::GetMenuDown() {
    if (HasPadInHands)
        return NewState.DPadDown || NewState.LeftStickY > 0;

    return (NewKeyState.down) || (GetKeyDown('S'));
}

bool CPadNew::GetMenuLeft() {
    if (HasPadInHands)
        return NewState.DPadLeft || NewState.LeftStickX < 0;

    return (NewKeyState.left) || (GetKeyDown('A'));
}

bool CPadNew::GetMenuRight() {
    if (HasPadInHands)
        return NewState.DPadRight || NewState.LeftStickX > 0;

    return (NewKeyState.right) || (GetKeyDown('D'));
}

bool CPadNew::GetMenuUpJustDown() {
    if (HasPadInHands)
        return NewState.DPadUp && !OldState.DPadUp;

    return
        (NewKeyState.up && !OldKeyState.up)
        || (NewMouseControllerState.wheelUp && !OldMouseControllerState.wheelUp)
        || (GetKeyJustDown('W'));
}

bool CPadNew::GetMenuDownJustDown() {
    if (HasPadInHands)
        return NewState.DPadDown && !OldState.DPadDown;

    return
        (NewKeyState.down && !OldKeyState.down)
        || (NewMouseControllerState.wheelDown && !OldMouseControllerState.wheelDown)
        || (GetKeyJustDown('S'));
}

bool CPadNew::GetMenuLeftJustDown() {
    if (HasPadInHands)
        return NewState.DPadLeft && !OldState.DPadLeft;

    return
        (NewKeyState.left && !OldKeyState.left)
        || (GetKeyJustDown('A'));
}

bool CPadNew::GetMenuRightJustDown() {
    if (HasPadInHands)
        return NewState.DPadRight && !OldState.DPadRight;

    return
        (NewKeyState.right && !OldKeyState.right)
        || (GetKeyJustDown('D'));
}

bool CPadNew::GetMenuBackJustDown() {
    if (HasPadInHands)
        return NewState.ButtonCircle && !OldState.ButtonCircle;

    return
        (NewKeyState.esc && !OldKeyState.esc);
}

bool CPadNew::GetMenuEnterJustDown() {
    if (HasPadInHands)
        return NewState.ButtonCross && !OldState.ButtonCross;

    return
        (NewKeyState.enter && !OldKeyState.enter)
        || (NewKeyState.extenter && !OldKeyState.extenter);
}

bool CPadNew::GetMenuSpaceJustDown() {
    if (HasPadInHands)
        return NewState.ButtonSquare && !OldState.ButtonSquare;

    return
        (GetKeyJustDown(Controls[MENU_APPLY_CHANGES].key));
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
    if (HasPadInHands)
        return NewState.ButtonSquare && !OldState.ButtonSquare;

    return !!(NewKeyState.standardKeys[76] && !OldKeyState.standardKeys[76]);
}

bool CPadNew::GetPhoneShowJustDown() {
    if (HasPadInHands)
        return NewState.DPadUp && !OldState.DPadUp;

    return GetKeyJustDown(Controls[PHONE_SHOW].key);
}

bool CPadNew::GetPhoneHideJustDown() {
    if (HasPadInHands)
        return NewState.ButtonCircle && !OldState.ButtonCircle;

    return GetKeyJustDown(Controls[PHONE_HIDE].key);
}

bool CPadNew::GetPhoneUpJustDown() {
    if (HasPadInHands)
        return NewState.DPadUp && !OldState.DPadUp;

    return GetKeyJustDown(Controls[PHONE_UP].key);
}

bool CPadNew::GetPhoneDownJustDown() {
    if (HasPadInHands)
        return NewState.DPadDown && !OldState.DPadDown;

    return GetKeyJustDown(Controls[PHONE_DOWN].key);
}

bool CPadNew::GetPhoneLeftJustDown() {
    if (HasPadInHands)
        return NewState.DPadLeft && !OldState.DPadLeft;

    return GetKeyJustDown(Controls[PHONE_LEFT].key);
}

bool CPadNew::GetPhoneRightJustDown() {
    if (HasPadInHands)
        return NewState.DPadRight && !OldState.DPadRight;

    return GetKeyJustDown(Controls[PHONE_RIGHT].key);
}

bool CPadNew::GetPhoneEnterJustDown() {
    if (HasPadInHands)
        return NewState.ButtonCross && !OldState.ButtonCross;

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

    if (HasPadInHands)
        return NewState.LeftShoulder1;

    return GetKeyDown(Controls[SHOW_WEAPON_WHEEL].key);
}

bool CPadNew::GetShowWeaponWheelJustUp() {
    if (DisablePlayerControls || bDisablePlayerCycleWeapon)
        return false;

    if (HasPadInHands)
        return !NewState.LeftShoulder1 && OldState.LeftShoulder1;

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

bool CPadNew::GetWeaponWheelCycleLeft() {
    if (HasPadInHands) {
        switch (Mode) {
        case 0:
            return NewState.LeftShoulder2 && !OldState.LeftShoulder2;
        case 1:
            return NewState.DPadLeft && !OldState.DPadLeft;
        }
    }

    return GetKeyJustDown(Controls[CA_PED_CYCLE_WEAPON_LEFT].key);
}

bool CPadNew::GetWeaponWheelCycleRight() {
    if (HasPadInHands) {
        switch (Mode) {
        case 0:
            return NewState.RightShoulder2 && !OldState.RightShoulder2;
        case 1:
            return NewState.DPadRight && !OldState.DPadRight;
        }
    }

    return GetKeyJustDown(Controls[CA_PED_CYCLE_WEAPON_RIGHT].key);
}

bool CPadNew::GetShowPlayerInfo() {
    if (DisablePlayerControls || bDisablePlayerDisplayVitalStats)
        return false;

    if (HasPadInHands)
        return NewState.DPadDown;

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

    if (HasPadInHands)
        return NewState.DPadDown;

    return GetKeyDown(Controls[EXTEND_RADAR_RANGE].key);
}

bool CPadNew::CycleRadioStationLeftJustDown() {
    if (DisablePlayerControls)
        return false;

    return GetKeyJustDown(Controls[CA_VEHICLE_RADIO_STATION_UP].key);
}

bool CPadNew::CycleRadioStationRightJustDown() {
    if (DisablePlayerControls)
        return false;

    if (HasPadInHands)
        return NewState.DPadLeft && !OldState.DPadLeft;

    return GetKeyJustDown(Controls[CA_VEHICLE_RADIO_STATION_DOWN].key);
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
