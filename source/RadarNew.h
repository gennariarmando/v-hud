#define RADAR_MENU_BLIP_MULT (1.3f)
#define RADAR_MENU_BLIP_HOVER_MULT (1.8f)

#define RADAR_START (-CRadarNew::m_nRadarMapSize / 2)
#define RADAR_END (CRadarNew::m_nRadarMapSize / 2)
#define RADAR_SIZE (RADAR_END - RADAR_START)

#define RADAR_TILE_SIZE (500)
#define RADAR_NUM_TILES (RADAR_SIZE / RADAR_TILE_SIZE)

#include "CSprite2d.h"

extern void* radar_gps_alpha_mask_fxc;
extern void* multi_alpha_mask_fxc;

enum eRadarNewSprites {
    RADAR_RECT,
    RADAR_RECT_2P,
    RADAR_PLANE_MASK,
    RADAR_PLANE,
    RADAR_DAMAGE,
    RADAR_MASK,
    NUM_RADAR_SPRITES,
};

enum eBlipsNewSprites {
    RADAR_SPRITE_COP = 64,
    RADAR_SPRITE_COP_HELI,
    RADAR_SPRITE_VCONE,
    RADAR_SPRITE_LEVEL,
    RADAR_SPRITE_LOWER,
    RADAR_SPRITE_HIGHER,
    NUM_BLIPS_SPRITES = 256,
};

enum eRadarLevelExtra {
    RADAR_LEVEL_CAR = -1,
    RADAR_LEVEL_CHAR_FRIENDLY = -2,
    RADAR_LEVEL_CHAR_ENEMY = -3,
    RADAR_LEVEL_OBJECT = -4,
    RADAR_LEVEL_DESTINATION = -5,
};

enum ePickupsBlipsSprites {
    PICKUP_UNKNOWN,
    PICKUP_WEAPON_ARMOUR,
    PICKUP_WEAPON_ASSAULT_RIFLE,
    PICKUP_WEAPON_BAT,
    PICKUP_WEAPON_DOWN,
    PICKUP_WEAPON_GRENADELAUNCHER,
    PICKUP_WEAPON_GRENADES,
    PICKUP_WEAPON_HEALTH,
    PICKUP_WEAPON_KNIFE,
    PICKUP_WEAPON_MOLOTOV,
    PICKUP_WEAPON_PIPEBOMB,
    PICKUP_WEAPON_PISTOL,
    PICKUP_WEAPON_POOLCUE,
    PICKUP_WEAPON_ROCKET,
    PICKUP_WEAPON_SHOTGUN,
    PICKUP_WEAPON_SMG,
    PICKUP_WEAPON_SNIPER,
    PICKUP_WEAPON_STICKYBOMB,
    PICKUP_WEAPON_UP,
    NUM_PICKUPS_BLIPS_SPRITES,
};

enum eRadarTraceColour {
    TRACE_COLOUR_RED,
    TRACE_COLOUR_GREEN,
    TRACE_COLOUR_BLUE,
    TRACE_COLOUR_WHITE,
    TRACE_COLOUR_YELLOW,
    TRACE_COLOUR_PURPLE,
    TRACE_COLOUR_CYAN,
    TRACE_COLOUR_THREAT,     
    TRACE_COLOUR_DESTINATION
};

class CBlip {
public:
    char texName[64];
    CRGBA color;
};

class CRadarAnim {
public:
    float u3, u4;
    float rhw[4];
};

class CHudSetting;

struct CRadarLegend {
    int id;
    unsigned int col;
    bool friendly;
    int sprite;
    CVector pos;
};

class CRadarNew {
public:
    static CSprite2d* m_RadarSprites[NUM_RADAR_SPRITES];
    static CSprite2d* m_BlipsSprites[NUM_BLIPS_SPRITES];
    static CSprite2d** m_MiniMapSprites;
    static CSprite2d* m_PickupsSprites[NUM_PICKUPS_BLIPS_SPRITES];
    static CRadarAnim Anim;
    static CVector2D m_vRadarMapQuality;
    static CBlip m_BlipsList[NUM_BLIPS_SPRITES];
    static int m_BlipsCount;
    static bool m_bInitialised;
    static RwCamera* m_pCamera;
    static RwFrame* m_pFrame;
    static RwRaster* m_pFrameBuffer1;
    static RwRaster* m_pFrameBuffer2;
    static RwRaster* m_pFrameBuffer3;
    static RwRaster* m_pFrameBuffer4;
    static int m_nMapLegendBlipCount;
    static CRadarLegend m_MapLegendBlipList[175];
    static unsigned int m_nActualTraceIdToPass;
    static bool m_bCopPursuit;
    static bool m_b3dRadar;
    static int m_nRadarRangeExtendTime;
    static bool m_bRemoveBlipsLimit;
    static int m_nRadarMapSize;
    static char m_NamePrefix[16];
    static char m_FileFormat[4];

public:
    static void InitBeforeGame();
    static void Init();
    static void Shutdown();
    static void Clear();
    static void ReadBlipsFromFile();
    static void ReadRadarInfoFromFile();
    static void CreateCamera();
    static void DestroyCamera();
    static int CalculateBlipAlpha(float dist);
    static void DrawRadarCop();
    static void DrawBlips();
    static void DrawEntityBlip(int id, bool sprite);
    static void DrawCoordBlip(int id, bool sprite);
    static void DrawPickupBlips();
    static void TransformRadarPointToRealWorldSpace(CVector2D& out, CVector2D& in);
    static void TransformRealWorldPointToRadarSpace(CVector2D& out, CVector2D& in);
    static void TransformRadarPointToScreenSpace(CVector2D &out, CVector2D &in);
    static void TransformRadarPoint(CVector2D& out, CVector2D& in);
    static bool IsPointTouchingRect(CVector2D& point, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);
    static void LimitPoint(float& x1, float& y1, float x2, float y2, float x3, float y3, float x4, float y4);
    static float LimitRadarPoint(CVector2D& point);
    static void DrawRadarSprite(unsigned short id, float x, float y, unsigned char alpha);
    static bool DisplayThisBlip(int spriteId, char priority);
    static void LimitToMap(float* x, float* y);
    static void AddAnyBlipNoLegend(CSprite2d* sprite, CVector posn, float width, float height, float angle, bool vcone, CRGBA const& col, bool limit);
    static void AddAnyBlip(unsigned short id, CEntity e, float width, float height, float angle, bool vcone, CRGBA const& col, bool limit);
    static void AddAnyBlip(unsigned short id, CVector posn, float width, float height, float angle, bool vcone, CRGBA const& col, bool limit);
    static void DrawRadarRectangle();
    static void ScanCopPursuit();
    static void GetTextureCorners(int x, int y, CVector2D* out);
    static void DrawAreaOnRadar(CRect const& rect, CRGBA const& col, bool force);
    static void DrawGangOverlay(bool force);
    static void DrawRotatingRadarSprite(CSprite2d* sprite, float x, float y, float angle, float width, float height, CRGBA color);
    static void CalculateCachedSinCos();
    static void DrawMap();
    static void DrawRadarSectionMap(int x, int y, CRect const& rect, CRGBA const& col);
    static void TransformRealWorldToTexCoordSpace(CVector2D& out, CVector2D& in, int x, int y);
    static void DrawRadarSection(int x, int y);
    static void DrawRadarMap(int x, int y);
    static void ShowRadarTraceWithHeight(float x, float y, unsigned int size, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha, unsigned char type);
    static void AddBlipToLegendList(bool trace, int id);
    static int ClipRadarPoly(CVector2D* out, CVector2D const* in);
    static void StreamRadarSection(int x, int y);
    static bool IsPlayerInVehicle();
    static bool Is3dRadar();
    static void SetBlipSprite(int i, unsigned short icon);
    static unsigned int GetRadarTraceColour(unsigned int c, bool bright, bool friendly);
};

