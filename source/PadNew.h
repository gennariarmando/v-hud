#pragma once
#include "CPad.h"

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

    bool GetShowWeaponWheel();
    bool GetShowWeaponWheelJustUp();
    bool GetShowWeaponWheel(int time);

    bool GetShowPlayerInfo();
    bool GetShowPlayerInfoJustUp();
    bool GetShowPlayerInfo(int time);

    bool GetExtendRadarRange();

    static CVector2D GetMouseInput(float mult = 1.0f);
};