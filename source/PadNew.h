#pragma once
#define MOUSE_CUSTOM_OFFSET (2000)
#define HAS_PAD_IN_HANDS(id) CPadNew::HasPadInHands(id)

#include "CPad.h"
#include "CControllerConfigManager.h"

#define GINPUT_COMPILE_SA_VERSION
#include "GInputAPI.h"

enum eGamePadButtons {
    GAMEPAD_NONE = -1,
    GAMEPAD_DPADUP,
    GAMEPAD_DPADDOWN,
    GAMEPAD_DPADLEFT,
    GAMEPAD_DPADRIGHT,
    
    GAMEPAD_CROSS,
    GAMEPAD_CIRCLE,
    GAMEPAD_SQUARE,
    GAMEPAD_TRIANGLE,
    
    GAMEPAD_LEFTSHOULDER1,
    GAMEPAD_LEFTSHOULDER2,
    GAMEPAD_LEFTSHOCK,
    
    GAMEPAD_RIGHTSHOULDER1,
    GAMEPAD_RIGHTSHOULDER2,
    GAMEPAD_RIGHTSHOCK,
    
    GAMEPAD_START,
    GAMEPAD_SELECT,
    
    GAMEPAD_BUTTON_UPDOWN,
    GAMEPAD_BUTTON_LEFTRIGHT,
    
    GAMEPAD_THUMBL,
    GAMEPAD_THUMBLX,
    GAMEPAD_THUMBLXL,
    GAMEPAD_THUMBLXR,
    GAMEPAD_THUMBLY,
    GAMEPAD_THUMBLYU,
    GAMEPAD_THUMBLYD,
    
    GAMEPAD_THUMBR,
    GAMEPAD_THUMBRX,
    GAMEPAD_THUMBRXL,
    GAMEPAD_THUMBRXR,
    GAMEPAD_THUMBRY,
    GAMEPAD_THUMBRYU,
    GAMEPAD_THUMBRYD,
};

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
    PHONE_LEFT,
    PHONE_RIGHT,
    PHONE_ENTER,
    MENU_SHOW_HIDE_LEGEND,
    MENU_PLACE_WAYPOINT,
    MENU_DELETE_SAVE,
    MENU_APPLY_CHANGES,
    MENU_BACK,
    MENU_SELECT,
    MENU_UNSETKEY,
    MENU_ML_PREVPAGE,
    MENU_ML_NEXTPAGE,
    NUM_CONTROL_ACTIONS
};

class CControls {
public:
    char action[128];
    int key;
    int button;
    int button2;
};

class CPadNew : public CPad {
public:
    int Id;
    bool DisablePlayerAim;

public:
    static bool bInitialised;

public:
    CPadNew();
    static void Init();
    static void Shutdown();
    static void SaveSettings();
    static void LoadSettings();
    static void Copy(CControls* dst, const CControls* src);
    static int StringToKey(const char* str);
    static const char* KeyToString(int key);
    static void PassControlsToCurrentGame(const CControls* c);
    static void SetControllerKeyAssociatedWithAction(e_ControllerAction action, int key, eControllerType type);
    static int GetNumOfSettingsForAction(e_ControllerAction action);
    static CPadNew* GetPad(int padNumber);
    static void GInputUpdate();
    static void GInputRelease();
    static bool HasPadInHands(int id);

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
    bool GetPhoneLeftJustDown();
    bool GetPhoneRightJustDown();

    bool GetPhoneEnterJustDown();

    bool GetLeftMouseJustDown();

    bool GetLeftMouseJustUp();

    bool GetRightMouseJustDown();

    bool GetShowWeaponWheel();
    bool GetShowWeaponWheelJustUp();
    bool GetShowWeaponWheel(int time);

    bool GetWeaponWheelCycleLeft();
    bool GetWeaponWheelCycleRight();

    bool GetShowPlayerInfo();
    bool GetShowPlayerInfo(int time);

    bool GetExtendRadarRange();

    bool CycleRadioStationLeftJustDown();

    bool CycleRadioStationRightJustDown();

    static CVector2D GetMouseInput(float mult = 1.0f);
    bool CheckForKeyboardInput();
    bool CheckForControllerInput();
    bool CheckForMouseInput();
};

extern CControls DefaultControls[NUM_CONTROL_ACTIONS];
extern CControls Controls[NUM_CONTROL_ACTIONS];

extern const char* controlKeysStrings[62];

extern IGInputPad* GInputPad[2];
extern GINPUT_PAD_SETTINGS GInputPadSettings[2];
extern bool GINPUT;
