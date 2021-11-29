#pragma once
#include "CPad.h"
#include "CControllerConfigManager.h"

// GInput stuff
enum eGInputActions {
#if 0
    // Custom III/VC actions
    ACTION_VEHICLE_FIREWEAPON,
    ACTION_VEHICLE_TURRET_LEFT_RIGHT,
    ACTION_VEHICLE_TURRET_UP_DOWN,
    ACTION_GO_LEFTRIGHT,
    ACTION_GO_UPDOWN,
    ACTION_MELEE_ATTACK,
    ACTION_BLOW_UP_RC_BUGGY,
#else
    // Custom SA actions
    ACTION_PED_MOVE,
    ACTION_BMX_HANDBRAKE,
    ACTION_BMX_BUNNYHOP,
    ACTION_CAMERA_LEFT_RIGHT,
    ACTION_CAMERA_UP_DOWN,
    ACTION_VEHICLE_CHANGE_RADIO_STATION,
    ACTION_GO_LEFTRIGHT,
    ACTION_GO_UPDOWN,
    ACTION_SNATCH_PACKAGE,
    ACTION_HYDRA_TARGET,
#endif

    NUM_CUSTOM_ACTIONS
};

enum eControlsActions {
    ACTION_PLACEHOLDER = 57,
    SHOW_WEAPON_WHEEL,
    EXTEND_RADAR_RANGE,
    SHOW_PLAYER_STATS,
    PHONE_SHOW,
    PHONE_HIDE,
    PHONE_UP,
    PHONE_DOWN,
    PHONE_ENTER,
    NUM_CONTROL_ACTIONS
};

class CControls {
public:
    char action[128];
    int key;
};

class CPadNew : public CPad {
public:
    bool DisablePlayerAim;

public:
    CPadNew();
    static void SaveSettings();
    static void LoadSettings();
    static void PassControlsToCurrentGame(const CControls* c);
    static void SetControllerKeyAssociatedWithAction(e_ControllerAction action, int key, eControllerType type);
    static int GetNumOfSettingsForAction(e_ControllerAction action);
    static CPadNew* GetPad(int padNumber);

    bool GetKeyDown(int key);
    bool GetKeyJustDown(int key);
    bool GetKeyUp(int key);

    bool GetOpenCloseMenuJustDown();

    bool GetMenuMapZoomIn();
    bool GetMenuMapZoomOut();

    bool GetMenuMapZoomInJustDown();
    bool GetMenuMapZoomOutJustDown();

    bool GetMenuUp();
    bool GetMenuDown();
    bool GetMenuLeft();
    bool GetMenuRight();

    bool GetMenuUpJustDown();
    bool GetMenuDownJustDown();
    bool GetMenuLeftJustDown();
    bool GetMenuRightJustDown();

    bool GetMenuBackJustDown();

    bool GetMenuEnterJustDown();

    bool GetMenuSpaceJustDown();

    bool GetLeftMouseDown();
    bool GetLeftMouseDoubleClickJustDown();

    bool GetMiddleMouseDown();

    bool GetMiddleMouseJustDown();

    bool GetMenuShowHideLegendJustDown();

    bool GetPhoneShowJustDown();
    bool GetPhoneHideJustDown();

    bool GetPhoneUpJustDown();

    bool GetPhoneDownJustDown();

    bool GetPhoneEnterJustDown();

    bool GetLeftMouseJustDown();

    bool GetLeftMouseJustUp();

    bool GetRightMouseJustDown();

    bool GetShowWeaponWheel();
    bool GetShowWeaponWheelJustUp();
    bool GetShowWeaponWheel(int time);

    bool GetShowPlayerInfo();
    bool GetShowPlayerInfo(int time);

    bool GetExtendRadarRange();

    bool CycleRadioStationLeftJustDown();

    bool CycleRadioStationRightJustDown();

    static CVector2D GetMouseInput(float mult = 1.0f);
    bool CheckForControllerInput();
    bool CheckForMouseInput();
};

extern bool bHasPadInHands;
extern CControls Controls[NUM_CONTROL_ACTIONS];

