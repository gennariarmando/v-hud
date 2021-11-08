#define DEFAULT_WIDTH 1920.0f
#define DEFAULT_HEIGHT 1080.0f
#define SAFE_ZONE_HEIGHT_MULT (DEFAULT_HEIGHT / DEFAULT_WIDTH)

#define HUD_X(x) ScaleHudX(x)
#define HUD_Y(y) ScaleHudY(y)
#define HUD_RIGHT(x) ScaleHudRight(x)
#define HUD_BOTTOM(y) ScaleHudBottom(y)

#define MENU_X(x) ScaleMenuX(x)
#define MENU_Y(y) ScaleMenuY(y)
#define MENU_RIGHT(x) ScaleMenuRight(x)
#define MENU_BOTTOM(y) ScaleMenuBottom(y)

#define SCREEN_HALF_WIDTH SCREEN_WIDTH * 0.5f
#define SCREEN_HALF_HEIGHT SCREEN_HEIGHT * 0.5f

#define ABS(a)  (((a) < 0) ? (-(a)) : (a))
#define isNearlyEqualF(a, b, t) (fabs(a - b) <= t)
#define interpF(a, b, f) a = a + (f) * (b - a)
#define norm(value, min, max) (((value) < (min)) ? 0 : (((value) > (max)) ? 1 : (((value) - (min)) / ((max) - (min)))))
#define lerp(no, mi, ma) ( (no) * ((ma) - (mi)) + (mi) )

#define clamp(v, low, high) ((v)<(low) ? (low) : (v)>(high) ? (high) : (v))
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

#define FLASH_ITEM(on, off) (CTimer::m_snTimeInMillisecondsPauseMode % on + off < on)

extern bool hudSafeZones;
extern void SetHUDSafeZone(bool on);
extern float ScaleHudX(float x);
extern float ScaleHudY(float y);
extern float ScaleHudRight(float x);
extern float ScaleHudBottom(float y);
       
extern float ScaleMenuX(float x);
extern float ScaleMenuY(float y);
extern float ScaleMenuRight(float x);
extern float ScaleMenuBottom(float y);

extern void RotateVertices(CVector2D *rect, float x, float y, float angle);
extern void DrawProgressBar(float x, float y, float width, float height, float progress, CRGBA const& col, CRGBA const& colBack = NULL);
extern void DrawProgressBarWithProgressDifference(float x, float y, float width, float height, float progress, CRGBA const& col, float diff, CRGBA const& diffCol);
extern void DrawProgressBarWithSprite(class CSprite2d* sprite, float x, float y, float w, float h, float progress, CRGBA const& col);
extern bool FileCheck(const char* name);
extern bool faststrcmp(const char* str1, const char* str2, int offset = 0);
extern void _rwD3D9RWSetRasterStage(RwRaster* r, int arg);
extern void DrawSpriteWithBorder(class CSprite2d* sprite, float x, float y, float w, float h, float outline, CRGBA const& color, CRGBA const& borderColor = CRGBA(0, 0, 0, 255));
extern float ConstrainAngle(float x);
extern void* CreatePixelShaderFromResource(int id);
extern HMONITOR GetPrimaryMonitorHandle();
extern void TakeScreenShot(const char* path, const char* name);
