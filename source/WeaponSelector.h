#pragma once
class CHudSetting;
class CSprite2d;

enum eWheelSprites {
    WHEEL_PART_ACTIVE,
    WHEEL_PART_GRAD,
    WHEEL_PART_INACTIVE,
    WHEEL_PART_SOLID,
    WHEEL_ARROW_LEFT,
    WHEEL_ARROW_RIGHT,
    WHEEL_EXTRA,
    NUM_WHEEL_SPRITES
};

enum eExtraWepSprites {
    WEXTRA_PARA,
    NUM_WEXTRA_SPRITES,
};

class CWeaponWheel {
public:
    int id;
    char name[32];
    char tex[32];
    CSprite2d* sprite;

    CWeaponWheel();

    ~CWeaponWheel();
};

class CWeaponStat {
public:
    int id;
    char name[64];
    float skill1;
    float skill2;
    float skill3;

    CWeaponStat();
};

class CWeaponCrosshair {
public:
    char name[32];

    CWeaponCrosshair();
};

class CWeaponSelector {
public:
    static bool bInitialised;
    static CSprite2d* WheelSprite[NUM_WHEEL_SPRITES];
    static CSprite2d* ExtraSprite[NUM_WEXTRA_SPRITES];

    static CVector2D vMousePos;
    static bool bShowWeaponWheel;
    static bool bWeaponWheelJustClosed;
    static int nWeaponWheelOpenTime;
    static float fPreviousTimeScale;
    static int nSelectedSlot;
    static int nSelectedWeapon[8];
    static int nActiveSlot;
    static int nActiveWeapon[8];
    static CWeaponWheel* WeaponWheel[8][128];
    static bool bSlowCycle;
    static int nTimeSinceClosed;
    static float nPrevCamHor;
    static float nPrevCamVer;
    static int nNumSelectedWeaponAvailableInSlot[8];
    static int nNumWeaponsAvailableInSlot[8];
    static int nArrayOfAvailableWeapons[8][128];
    static CWeaponStat* WeaponStat;
    static float fStatMaxRate;
    static float fStatsProgress[4];
    static float fStatsDiff[4];
    static int nWeaponExtraFadeAlpha;
    static CWeaponCrosshair nCrosshairs[1024];

public:
    CWeaponSelector();

    static CWeaponWheel* GetActiveWeapon();

    static void Init();
    static void Clear();
    static void ReInit();
    static void Shutdown();
    static void ReadSlotFromFile();
    static void ReadWeaponRatesFromFile();
    static void RegisterSounds();

    static int GetFirstSlotAvailable();
    static int GetLastSlotAvailable();
    static bool IsAbleToSwitchWeapon();
    static void ProcessWeaponSelector();
    static void CenterCursor();
    static void UpdateCursorPos();
    static void ResetCameraMovement();
    static void DisableCameraMovement();
    static void PopulateSlot(int slot);
    static void SwitchWeaponFromSlot(const char* dir);
    static int GetFirstWeaponAvailable(int slot);
    static int GetLastWeaponAvailable(int slot);
    static int GetPreviousWeaponInSlot(int slot);
    static int GetNextWeaponInSlot(int slot);
    static void OpenWeaponWheelQuickSwitch(const char* dir);
    static void OpenWeaponWheel(bool slow);
    static void CloseWeaponWheel(bool switchon);
    static void ClearWheel();
    static void DrawWheelPart(char id, float x, float y, int n, CRGBA const& col);
    static void DrawWheel();
    static int FadeIn(int a);

    static float GetShiftOffsetForStatsBox();
    static void DrawStats(int selected_id, int active_id);
    static CVector2D LimitMousePosition(CVector2D& pos);
};
