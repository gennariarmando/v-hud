#include "VHud.h"
#include "Utility.h"
#include "CSprite2d.h"

void RotateVertices(CVector2D *rect, float x, float y, float angle) {
    float xold, yold;
    //angle /= 57.2957795;
    float _cos = cosf(angle);
    float _sin = sinf(angle);
    for (unsigned int i = 0; i < 4; i++) {
        xold = rect[i].x;
        yold = rect[i].y;
        rect[i].x = x + (xold - x) * _cos + (yold - y) * _sin;
        rect[i].y = y - (xold - x) * _sin + (yold - y) * _cos;
    }
}

void DrawProgressBar(float x, float y, float w, float h, float progress, CRGBA const& col, CRGBA const& colBack) {
    // progress value is 0.0f - 1.0f
    if (progress <= 0.0f)
        progress = 0.0f;
    else if (progress >= 1.0f)
        progress = 1.0f;

    CRGBA back = CRGBA(col.r / 2, col.g / 2, col.b / 2, col.a);
    if (colBack != NULL)
        back = colBack;

    CSprite2d::DrawRect(CRect(x, y, x + w, y + h), back);
    CSprite2d::DrawRect(CRect(x, y, x + (w * progress), y + h), col);
}

void DrawProgressBarWithSprite(CSprite2d* sprite, float x, float y, float w, float h, float progress, CRGBA const& col) {
    // progress value is 0.0f - 1.0f
    if (progress <= 0.0f)
        progress = 0.0f;
    else if (progress >= 1.0f)
        progress = 1.0f;

    sprite->Draw(CRect(x, y, x + w, y + h), CRGBA(col.r / 2, col.g / 2, col.b / 2, col.a));
    sprite->Draw(CRect(x, y, x + (w * progress), y + h), col, 0.0f, 0.0f, progress, 0.0f, 0.0f, 0.0f, progress, 0.0f);
}

void DrawProgressBarWithProgressDifference(float x, float y, float w, float h, float progress, 
                                           CRGBA const& col, float diff, CRGBA const& diffCol) {
    // progress value is 0.0f - 1.0f
    if (progress <= 0.0f)
        progress = 0.0f;
    else if (progress >= 1.0f)
        progress = 1.0f;

    CSprite2d::DrawRect(CRect(x, y, x + w, y + h), CRGBA(col.r, col.g, col.b, col.a / 2));
    CSprite2d::DrawRect(CRect(x, y, x + (w * progress), y + h), col);

    while ((progress + diff) < 0.0f) diff += 0.01f;
    while ((progress + diff) > 1.0f) diff -= 0.01f;

    CSprite2d::DrawRect(CRect(x + (w * progress), y, (x + (w * progress)) + (w * diff), y + h), diffCol);
}

bool FileCheck(const char* name) {
    struct stat buffer;

    return (stat(name, &buffer) == 0);
}

bool faststrcmp(const char* str1, const char* str2, int offset) {
    str1 += offset;
    str2 += offset;
    for (; *str1; str1++, str2++) {
        if (*str1 != *str2)
            return true;
    }
    return *str2 != '\0';
}

void _rwD3D9RWSetRasterStage(RwRaster* r, int arg) {
    plugin::Call<0x7FDCD0, RwRaster*, int>(r, arg);
}

RwChar** _psGetVideoModeList() {
    return plugin::CallAndReturn<RwChar**, 0x745AF0>();
}

void _psSetVideoMode(int index) {
    plugin::Call<0x745C70, int>(index);
}

void DrawSpriteWithBorder(CSprite2d* sprite, float x, float y, float w, float h, float outline, CRGBA const& color, CRGBA const& borderColor) {
    float outline_x[] = {
        outline,
        -outline,
        0.0f,
        0.0f,
        outline,
        -outline,
        outline,
        -outline,
    };

    float outline_y[] = {
        0.0f,
        0.0f,
        outline,
        -outline,
        outline,
        outline,
        -outline,
        -outline,
    };

    for (int i = 0; i < 8; i++) {
        sprite->Draw(CRect((x + (outline_x[i])), (y + (outline_y[i])), (x + (outline_x[i])) + (w), (y + (outline_y[i])) + (h)), borderColor);
    }

    sprite->Draw(CRect(x, y, x + w, y + h), color);
}

float ConstrainAngle(float x) {
    x = fmod(x, 360);
    if (x < 0)
        x += 360;
    return x;
}

static HMODULE thisModule = NULL;
void* CreatePixelShaderFromResource(int id) {
    if (thisModule == NULL)
        GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)CreatePixelShaderFromResource, &thisModule);

    void* ps = NULL;
    HRSRC resource = FindResource(thisModule, MAKEINTRESOURCE(id), RT_RCDATA);
    if (resource != NULL) {
        RwUInt32* shader = (RwUInt32*)(LoadResource(thisModule, resource));

        if (shader) {
            RwD3D9CreatePixelShader(shader, &ps);
            FreeResource(shader);
        }
    }
    
    return ps;
}

HMONITOR GetPrimaryMonitorHandle() {
    const POINT ptZero = { 0, 0 };
    return MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);
}
