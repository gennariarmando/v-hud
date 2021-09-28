#define DEFAULT_WIDTH 1920.0f
#define DEFAULT_HEIGHT 1080.0f
#define HUD_X(x) ((SCREEN_WIDTH / SCREEN_HEIGHT <= DEFAULT_WIDTH / DEFAULT_HEIGHT) ? SCREEN_COORD(x) : (SCREEN_WIDTH * 0.5f) + (SCREEN_COORD(x - (DEFAULT_WIDTH * 0.5f))))
#define HUD_Y(y) SCREEN_COORD(y)
#define HUD_RIGHT(x) SCREEN_WIDTH - HUD_X(x)

#define MENU_X(x) ((SCREEN_WIDTH / SCREEN_HEIGHT == DEFAULT_WIDTH / DEFAULT_HEIGHT) ? SCREEN_COORD(x) : (SCREEN_WIDTH * 0.5f) + (SCREEN_COORD(x - (DEFAULT_WIDTH * 0.5f))))
#define MENU_Y(y) SCREEN_COORD(y)
#define MENU_RIGHT(x) SCREEN_WIDTH - MENU_X(x)
#define MENU_BOTTOM(x) SCREEN_HEIGHT - MENU_Y(x)

#define ABS(a)  (((a) < 0) ? (-(a)) : (a))
#define isNearlyEqualF(a, b, t) fabs(a - b) <= ( (fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * t)
#define interpF(a, b, f) a = a + (f) * (b - a)
#define norm(value, min, max) (((value) < (min)) ? 0 : (((value) > (max)) ? 1 : (((value) - (min)) / ((max) - (min)))))
#define lerp(no, mi, ma) ( (no) * ((ma) - (mi)) + (mi) )

#define clamp(v, low, high) ((v)<(low) ? (low) : (v)>(high) ? (high) : (v))
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

extern void RotateVertices(CVector2D *rect, float x, float y, float angle);
extern void DrawProgressBar(float x, float y, float width, float height, float progress, CRGBA const& col, CRGBA const& colBack = NULL);
extern void DrawProgressBarWithProgressDifference(float x, float y, float width, float height, float progress, CRGBA const& col, float diff, CRGBA const& diffCol);
extern void DrawProgressBarWithSprite(class CSprite2d* sprite, float x, float y, float w, float h, float progress, CRGBA const& col);
extern bool FileCheck(const char* name);
extern bool faststrcmp(const char* str1, const char* str2, int offset = 0);
extern void _rwD3D9RWSetRasterStage(RwRaster* r, int arg);
extern RwChar** _psGetVideoModeList();
extern void _psSetVideoMode(int index);
extern void DrawSpriteWithBorder(class CSprite2d* sprite, float x, float y, float w, float h, float outline, CRGBA const& color, CRGBA const& borderColor = CRGBA(0, 0, 0, 255));
extern float ConstrainAngle(float x);
extern void* CreatePixelShaderFromResource(int id);
extern HMONITOR GetPrimaryMonitorHandle();