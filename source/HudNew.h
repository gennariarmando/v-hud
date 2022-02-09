#pragma once
#include "CHud.h"
#define GET_SETTING(a) CHudNew::GetSetting(a)

enum eWantedSprites {
    WANTED_STAR_1,
    WANTED_STAR_2,
    NUM_WANTED_SPRITES,
};

enum eCrosshairsSprites {
    CROSSHAIR_DOT,
    CROSSHAIR_CROSS,
    CROSSHAIR_ROCKET,
    CROSSHAIR_SNIPER,
    CROSSHAIR_SNIPERTARGET,
    CROSSHAIR_ASSAULT,
    CROSSHAIR_PISTOL,
    CROSSHAIR_SHOTGUN,
    CROSSHAIR_SMG,
    NUM_CROSSHAIRS_SPRITES
};

enum ePlayerStatsSprites {
    PLRSTAT_PLAYER1_ACTIVE,
    PLRSTAT_PLAYER2_ACTIVE,
    PLRSTAT_PLAYER3_ACTIVE,
    PLRSTAT_PLAYER4_ACTIVE,
    PLRSTAT_PROGRESS_BAR,
    PLRSTAT_WHEEL,

    NUM_PLRSTATS_SPRITES
};

enum eHudSettings {
    HUD_RADAR,
    HUD_RADAR_RECT,
    HUD_RADAR_MASK,
    HUD_RADAR_DAMAGE,
    HUD_RADAR_FOREGROUND,
    HUD_RADAR_BACKGROUND,
    HUD_RADAR_BACKGROUND_P2,
    HUD_RADAR_BLIPS_SIZE,
    HUD_RADAR_BLIPS_LEVEL_SIZE,
    HUD_RADAR_BLIPS_COP_SIZE,
    HUD_RADAR_BLIPS_COP_HELI_SIZE,
    HUD_RADAR_BLIPS_BORDER_SIZE,
    HUD_HEALTH_BAR,
    HUD_ARMOUR_BAR,
    HUD_ARMOUR_BAR_B,
    HUD_BREATH_BAR,
    HUD_NITRO_BAR,
    HUD_NITRO_TEXT,
    HUD_HEALTH_BAR_P2,
    HUD_ARMOUR_BAR_P2,
    HUD_ARMOUR_BAR_B_P2,
    HUD_BREATH_BAR_P2,
    HUD_RADAR_GPS_LINE,
    HUD_RADAR_GPS_RECT,
    HUD_RADAR_GPS_DIST_TEXT,
    HUD_RADAR_GPS_DIST_ARROW,
    HUD_SUBTITLES,
    HUD_WANTED_STARS,
    HUD_CASH,
    HUD_AMMO,
    HUD_WEAPON_WHEEL,
    HUD_WEAPON_WHEEL_MOUSE_LINE,
    HUD_WEAPON_STATS_BOX,
    HUD_WEAPON_STATS_BAR,
    HUD_VEHICLE_NAME,
    HUD_ZONE_NAME,
    HUD_LEVEL_NAME,
    HUD_HELP_BOX_TEXT,
    HUD_HELP_BOX_BORDER,
    HUD_HELP_BOX_SMALL_TEXT,
    HUD_HELP_BOX_SMALL_BORDER,
    HUD_CROSSHAIR_DOT,
    HUD_CROSSHAIR_CROSS,
    HUD_VITAL_STATS,
    HUD_PLAYER_WHEEL,
    HUD_WASTED_TEXT,
    HUD_BUSTED_TEXT,
    HUD_BIG_MESSAGE,
    HUD_MISSION_TITLE,
    HUD_MISSION_TIMERS,

    NUM_HUD_SETTINGS,
};

class CHudSetting {
public:
    char name[64];
    float x, y, w, h;
    CRGBA col;

    CHudSetting() {
        strcpy(name, "NONE");
        x = y = w = h = 0.0f;
        col = { 0, 0, 0, 0 };
    }
};

class CHudNew : CHud {
public:
    static CHudSetting m_HudList[256];

    static bool bInitialised;
    static bool bShowMoney;
    static bool bShowMoneyDifference;
    static int nMoneyFadeAlpha;
    static int nMoneyDifferenceFadeAlpha;
    static int nTimeToShowMoney;
    static int nTimeToShowMoneyDifference;

    static bool bShowAmmo;
    static int nAmmoFadeAlpha;
    static int nTimeToShowAmmoDifference;

    static int m_nPreviousMoney;
    static int m_nDiffMoney;

    static int nTargettedEntityDeathTime;

    static bool m_bShowMissionText;
    static char m_LastMissionName[128];

    static bool m_bShowWastedBusted;
    static bool m_bShowSuccessFailed;
    static int m_nBigMessageTime;
    static int m_nBigMessageTime2;
    static float m_fBigMessageOffset;
    static CRGBA m_BigMessageColor;
    static char m_SuccessFailedText[2][128];
    static int m_nMiddleTopMessageTime;
    static char m_MiddleTopMessage[16][128];
    static char m_MiddleTopSubMessage[128];
    static int m_nCurrentMiddleTopMessage;
    static bool m_bShowMiddleTopMessage;
    static int m_nMiddleTopMessageIdToSet;

    static char m_CurrentLevelName[128];
    static int m_nLevelNameState;

    static CSprite2d* WantedSprites[NUM_WANTED_SPRITES];
    static CSprite2d* CrosshairsSprites[NUM_CROSSHAIRS_SPRITES];
    static CSprite2d* StatsSprites[NUM_PLRSTATS_SPRITES];
    static CSprite2d* PlayerPortrait[4][2];
    static int previousModelIndex[4];

    static char* MissionTimersString[5][2];
    static int nTimersCount;

public:
    static void Init();
    static void ReInit();
    static void Shutdown();
    static void ReadSettingsFromFile();
    static CHudSetting GetSetting(int setting);
    static CHudSetting GetSetting(char* name);
    static void Draw();
    static bool IsAimingWeapon();
    static bool IsFirstPersonAiming();
    static void DrawCrosshairs();
    static void DrawPlayerInfo();
    static void DrawMoneyCounter();
    static void DrawAmmo();
    static void DrawSimpleRect(CRect const& rect, CRGBA const& col);
    static void DrawSimpleRectGrad(CRect const& rect, CRGBA const& col);
    static void DrawSimpleRectGradInverted(CRect const& rect, CRGBA const& col);
    static void DrawSimpleRectGradCentered(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, CRGBA col);
    static void DrawWanted();
    static void DrawVehicleName();
    static void DrawMissionTimers();
    static void DrawRadar();
    static void DrawTripSkip();
    static void DrawScriptText(char priority);
    static void DrawSubtitles();
    static void DrawHelpText();
    static void PrintSmallHelpText(int alpha);
    static void PrintBigHelpText(int alpha);
    static void DrawOddJobMessage();
    static void DrawSuccessFailedMessage();
    static void DrawZoneName();

    static void DrawLevelName();

    static void DrawAfterFade();
    static void DrawWastedBustedText();

    static void DrawMissionTitle();

    static bool IsMoneyCounterDisplaying();
    static bool IsMoneyDifferenceDisplaying();

    static bool IsAmmoCounterDisplaying();

    static void TakePhotograph();

    static void CheckPlayerPortrait(int id);
    static void DrawPlayerPortrait(int id, float x, float y, float w, float h);
    static void DrawStats();
    static float GetShiftOffsetForMoney();
    static float GetShiftOffsetForAmmo();
};
