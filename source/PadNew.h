#pragma once
#include "CPad.h"

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

class CPadNew : public CPad {
public:
    bool DisablePlayerAim;

public:
    CPadNew();
    void Init();
    void Update();
    void Shutdown();
    static CPadNew* GetPad(int padNumber);

    bool GetTarget();
    bool GetTargetJustDown();

    bool GetOpenCloseMenuJustDown();

    bool GetMenuUpJustDown();
    bool GetMenuDownJustDown();
    bool GetMenuLeftJustDown();
    bool GetMenuRightJustDown();

    bool GetMenuBackJustDown();

    bool GetMenuEnterJustDown();

    bool GetMenuSpaceJustDown();

    bool GetLeftMouseJustDown();

    bool GetRightMouseJustDown();

    bool GetShowWeaponWheel();
    bool GetShowWeaponWheelJustUp();
    bool GetShowWeaponWheel(int time);

    bool GetShowPlayerInfo();
    bool GetShowPlayerInfoJustUp();
    bool GetShowPlayerInfo(int time);

    bool GetExtendRadarRange();

    static CVector2D GetMouseInput(float mult = 1.0f);
    bool CheckForControllerInput();
    bool CheckForMouseInput();
};

extern bool bHasPadInHands;
