#define DEFAULT_WIDTH 1920.0f
#define DEFAULT_HEIGHT 1080.0f
#define UI_X(x) CenterX(x)
#define UI_Y(y) SCREEN_COORD(y)
#define UI_RIGHT(x) UI_X(DEFAULT_WIDTH - (x))

#define ABS(a)  (((a) < 0) ? (-(a)) : (a))
#define isNearlyEqualF(a, b, t) fabs(a - b) <= ( (fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * t)
#define interpF(a, b, f) a = a + (f) * (b - a)
#define norm(value, min, max) (((value) < (min)) ? 0 : (((value) > (max)) ? 1 : (((value) - (min)) / ((max) - (min)))))
#define lerp(norm, min, max) ( (norm) * ((max) - (min)) + (min) )

#define clamp(v, low, high) ((v)<(low) ? (low) : (v)>(high) ? (high) : (v))
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

extern void RotateVertices(CVector2D *rect, float x, float y, float angle);
extern void DrawProgressBar(float x, float y, float width, float height, float progress, CRGBA const& col);
extern void DrawProgressBarWithProgressDifference(float x, float y, float width, float height, float progress, CRGBA const& col, float diff, CRGBA const& diffCol);
extern void DrawProgressBarWithSprite(class CSprite2d* sprite, float x, float y, float w, float h, float progress, CRGBA const& col);
extern bool FileCheck(const char* name);
extern float CenterX(float x);
extern bool faststrcmp(const char* str1, const char* str2, int offset = 0);
extern void _rwD3D9RWSetRasterStage(RwRaster* r, int arg);
extern void DrawSpriteWithBorder(class CSprite2d* sprite, float x, float y, float w, float h, float outline, CRGBA const& color, CRGBA const& borderColor = CRGBA(0, 0, 0, 255));
extern void StringReplace(std::string& str, std::string substring, std::string replacement);
extern float ConstrainAngle(float x);
extern void* CreatePixelShaderFromResource(int id);
