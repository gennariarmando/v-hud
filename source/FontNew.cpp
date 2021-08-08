#include "plugin.h"
#include "FontNew.h"
#include "TextureMgr.h"
#include "Utility.h"
#include "HudNew.h"

#include "CSprite2d.h"
#include "CFont.h"

using namespace plugin;

CFontNew FontNew;

CSprite2d* CFontNew::Sprite[NUM_FONTS];
CFontDetailsNew CFontNew::Details;
char CFontNew::Size[NUM_FONTS][160];
bool CFontNew::bNewLine;
int CFontNew::NumLines;
int CFontNew::PS2Symbol;
CSprite2d* CFontNew::ButtonSprite[NUM_BUTTONS];

char* ButtonFileName[] = {
    "none",
    "up",
    "down",
    "left",
    "right",
    "cross",
    "circle",
    "square",
    "triangle",
    "l1",
    "l2",
    "l3",
    "r1",
    "r2",
    "r3",
    "start",
    "select",
    "dud",
    "dlr",
    "thumbl",
    "thumblx",
    "thumblxl",
    "thumblxr",
    "thumbly",
    "thumblyu",
    "thumblyd",
    "thumbr",
    "thumbrx",
    "thumbrxl",
    "thumbrxr",
    "thumbry",
    "thumbryu",
    "thumbryd",
    "pc_up",
    "pc_down",
    "pc_left",
    "pc_right",
};

CFontNew::CFontNew() {

}

void CFontNew::Init() {
    Sprite[0] = new CSprite2d();
    Sprite[1] = new CSprite2d();
    Sprite[2] = new CSprite2d();
    Sprite[3] = new CSprite2d();
    Sprite[0]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\fonts"), "font1");
    Sprite[1]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\fonts"), "font2");
    Sprite[2]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\fonts"), "font3");
    Sprite[3]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH("VHud\\fonts"), "font4");

    char* path = "VHud\\buttons\\xbox";
    for (int i = 0; i < NUM_BUTTONS; i++) {
        ButtonSprite[i] = new CSprite2d();

        if (i > BUTTON_THUMBRYD)
            path = "VHud\\buttons\\pc";

        ButtonSprite[i]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH(path), ButtonFileName[i]);
    }
    ReadValuesFromFile();

    Clear();
}

void CFontNew::ReadValuesFromFile() {
    std::ifstream file(PLUGIN_PATH("VHud\\data\\fonts.dat"));

    int fontId = 0;
    int index = 0;
    if (file.is_open()) {
        for (std::string line; getline(file, line);) {
            if (!line[0] || line[0] == '#')
                continue;

            char name[64];
            sscanf(line.c_str(), "%s", &name);

            if (strcmp(name, "[FONT_ID]") == 0) {
                index = 0;
                getline(file, line);
                sscanf(line.c_str(), "%d", &fontId);
            }
            else if (strcmp(name, "[PROP]") == 0) {
                for (line; getline(file, line);) {
                    sscanf(line.c_str(), "%s", &name);

                    if (strcmp(name, "[\\PROP]") == 0) {
                        index = 0;
                        break;
                    }

                    int value[16];
                    sscanf(line.c_str(), "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
                        &value[0], &value[1], &value[2], &value[3], &value[4], &value[5], &value[6], &value[7],
                        &value[8], &value[9], &value[10], &value[11], &value[12], &value[13], &value[14], &value[15]);

                    for (int i = 0; i < 16; i++) {
                        Size[fontId][index] = value[i];
                        index++;
                    }
                    if (index > 160)
                        exit(0);
                }
            }
            else if (strcmp(name, "[\\PROP]") == 0) {
                continue;
            }
        }

        file.close();
    }
}

void CFontNew::Clear() {
    SetBackground(false);
    SetGradBackground(false);
    SetBackgroundColor(CRGBA(0, 0, 0, 255));
    SetBackgroundBorder(CRect(0, 0, 0, 0));
    SetClipX(SCREEN_WIDTH);
    SetWrapX(SCREEN_WIDTH);
    SetAlignment(ALIGN_LEFT);
    SetFontStyle(FONT_1);
    SetColor(CRGBA(255, 255, 255, 255));
    SetDropShadow(0.0f);
    SetDropColor(CRGBA(0, 0, 0, 255));
    SetScale(1.0f, 1.0f);
    NumLines = 0;
    bNewLine = false;
    PS2Symbol = BUTTON_NONE;
}

void CFontNew::Shutdown() {
    Sprite[0]->Delete();
    delete Sprite[0];

    Sprite[1]->Delete();
    delete Sprite[1];

    Sprite[2]->Delete();
    delete Sprite[2];

    Sprite[3]->Delete();
    delete Sprite[3];

    for (int i = 0; i < NUM_BUTTONS; i++) {
        ButtonSprite[i]->Delete();
        delete ButtonSprite[i];
    }
}

float CFontNew::GetCharacterSize(char c) {
    return Size[Details.style][c] * Details.scale.x;
}

float CFontNew::GetStringWidth(char* s, bool spaces) {
    float w;

    w = 0.0f;
    for (; (*s != ' ' || spaces) && *s != '\0'; s++) {
        if (*s == '~') {
            s++;
            while (*s != '~') s++;
            s++;
            if (*s == ' ' && !spaces)
                break;
        }
        w += GetCharacterSize(*s - ' ');
    }
    return w;
}

char* CFontNew::GetNextSpace(char* s) {
    for (; *s != ' ' && *s != '\0'; s++)
        if (*s == '~') {
            s++;
            while (*s != '~') s++;
        }

    return s;
}

void CFontNew::PrintString(float x, float y, char* s) {
    if (*s != '*') {
        if (Details.background) {
            CRect rect;
            x += Details.backgroundBorder.left;
            y += Details.backgroundBorder.top;
            GetTextRect(&rect, x, y, s);
            if (Details.grad)
                CHudNew::DrawSimpleRectGrad(rect, Details.backgroundColor);
            else
                CSprite2d::DrawRect(rect, Details.backgroundColor);
        }
        GetNumberLines(true, x, y, s);
    }
}

int CFontNew::GetNumberLines(bool print, float xstart, float ystart, char* s) {
    float x = xstart;
    float y = ystart;
    int n = 1;

    if (Details.alignment == ALIGN_CENTER) {
        bool first = true;
        char* start = s;
        char* t = s;
        float length = 0.0f;
        int space = 0;

        if (Details.alignment == ALIGN_CENTER || Details.alignment == ALIGN_RIGHT)
            x = 0.0f;

        while (s) {
            while (s) {
                if (*s == '\0')
                    return n;

                float w = Details.alignment == ALIGN_CENTER ? Details.wrapX : Details.alignment == ALIGN_RIGHT ? xstart - Details.wrapX : xstart + Details.wrapX;

                if (x + GetStringWidth(s) > w && !first && !bNewLine) {
                    float sw = Details.alignment == ALIGN_LEFT || Details.alignment == ALIGN_CENTER ? 0.0f : (Details.wrapX - length) / space;
                    float cx = Details.alignment == ALIGN_CENTER ? xstart - x / 2 : Details.alignment == ALIGN_RIGHT ? xstart - x : xstart;
                    PrintString(print, cx, y, start, s, sw);

                    if (Details.alignment == ALIGN_CENTER || Details.alignment == ALIGN_RIGHT)
                        x = 0.0f;
                    else
                        x = xstart;

                    y += GetHeightScale(Details.scale.y);
                    start = s;

                    length = 0.0f;
                    space = 0;
                    first = true;
                    n++;
                }

                if (bNewLine && !first) {
                    y += GetHeightScale(Details.scale.y);
                    start = s;

                    length = 0.0f;
                    space = 0;
                    first = true;
                    bNewLine = false;
                }

                t = GetNextSpace(s);
                if (t[0] == '\0' || t[0] == ' ' && t[1] == '\0') {
                    break;
                }

                if (!first)
                    space++;

                first = false;
                x += GetStringWidth(s) + GetCharacterSize(*t - ' ');

                length = x;
                s = t + 1;
            }

            if (t[0] == ' ' && t[1] == '\0') {
                t[0] = '\0';
            }

            x += GetStringWidth(s);
            s = t;
            float cx = Details.alignment == ALIGN_CENTER ? xstart - x / 2 : Details.alignment == ALIGN_RIGHT ? xstart - x : xstart;
            PrintString(print, cx, y, start, s, 0.0f);
        }

        bNewLine = false;
        return n;
    }
    else if (Details.alignment == ALIGN_RIGHT) {
        x -= GetStringWidth(s, true);
    }

    for (s; *s != '\0'; s++) {
        if (*s == '~')
            s = ParseToken(s, Details.color);

        char c;
        c = *s - ' ';

        float f = Details.alignment == ALIGN_CENTER ? Details.wrapX : xstart + Details.wrapX;
        if (x + GetStringWidth(s) > f || bNewLine) {
            x = xstart;
            y += 32.0f * Details.scale.y * 0.5f + 2.0f * Details.scale.y;
            bNewLine = false;
            n++;
        }

        if (print)
            PrintChar(x, y, c);
        x += GetCharacterSize(c);
    }

    return n;
}

void CFontNew::PrintString(bool print, float x, float y, char* start, char* end, float spwidth) {
    char* s, c;

    float xstart = x;
    for (s = start; s < end; s++) {
        int n = NumLines;

        if (*s == '~')
            s = ParseToken(s, Details.color);

        if (n != NumLines) {

            x = xstart;
            y += GetHeightScale(Details.scale.y);
        }

        c = *s - ' ';
        float sp = GetCharacterSize(c);
        if (print) {
            DrawButton(x, y, PS2Symbol);
            x += PS2Symbol ? Details.scale.y * 17.0f : 0.0f;

            PrintChar(x, y, c);
        }
        x += sp;

        if (c == 0)
            x += spwidth;

        PS2Symbol = BUTTON_NONE;
    }
}

char* CFontNew::ParseToken(char* s, CRGBA& color) {
    char* c = s + 1;

    switch (*c) {
    case 'N':
    case 'n':
        bNewLine = true;
        NumLines++;
        break;
    case 'X': 
    case 'x': 
        PS2Symbol = BUTTON_CROSS;
        break;
    case 'O':
    case 'o':
        PS2Symbol = BUTTON_CIRCLE;
        break;
    case 'Q': 
    case 'q': 
        PS2Symbol = BUTTON_SQUARE;
        break;
    case 'T': 
    case 't': 
        PS2Symbol = BUTTON_TRIANGLE;
        break;
    case 'K': 
    case 'k':
        PS2Symbol = BUTTON_L1;
        break;
    case 'M': 
    case 'm': 
        PS2Symbol = BUTTON_L2;
        break;
    case 'A': 
    case 'a': 
        PS2Symbol = BUTTON_L3;
        break;
    case 'J': 
    case 'j': 
        PS2Symbol = BUTTON_R1;
        break;
    case 'V': 
    case 'v': 
        PS2Symbol = BUTTON_R2;
        break;
    case 'C': 
    case 'c': 
        PS2Symbol = BUTTON_R3;
        break;
    case 'u':
    case 'U':
        if (c[1] == 'd' || c[1] == 'D') {
            c++;
            PS2Symbol = BUTTON_UPDOWN;
        }
        else
            PS2Symbol = BUTTON_UP;
        break;
    case 'd':
    case 'D':
        PS2Symbol = BUTTON_DOWN;
        break;
    case '<':
        if (c[1] == '>') {
            c++;
            PS2Symbol = BUTTON_LEFTRIGHT;
        }
        else
            PS2Symbol = BUTTON_LEFT;
        break;
    case '>':
        PS2Symbol = BUTTON_RIGHT;
        break;
    case '[':
        switch (c[1]) {
        case '~':
            PS2Symbol = BUTTON_THUMBL;
            break;
        case 'x':
            PS2Symbol = BUTTON_THUMBLX;
            break;
        case 'y':
            PS2Symbol = BUTTON_THUMBLY;
            break;
        case '<':
            PS2Symbol = BUTTON_THUMBLXL;
            break;
        case '>':
            PS2Symbol = BUTTON_THUMBLXR;
            break;
        case 'u':
            PS2Symbol = BUTTON_THUMBLYU;
            break;
        case 'd':
            PS2Symbol = BUTTON_THUMBLYD;
            break;
        }
        c++;
        break;
    case ']':
        switch (*(++c)) {
        case '~':
            PS2Symbol = BUTTON_THUMBR;
            break;
        case 'x':
            PS2Symbol = BUTTON_THUMBRX;
            break;
        case 'y':
            PS2Symbol = BUTTON_THUMBRY;
            break;
        case '<':
            PS2Symbol = BUTTON_THUMBRXL;
            break;
        case '>':
            PS2Symbol = BUTTON_THUMBRXR;
            break;
        case 'u':
            PS2Symbol = BUTTON_THUMBRYU;
            break;
        case 'd':
            PS2Symbol = BUTTON_THUMBRYD;
            break;
        }
        break;
        //default:
        //   c = CFont::ParseToken(s, color, true, false);
        //   PS2Symbol = CFont::m_nExtraFontSymbolId;
        //   return c;
    }
    while (*c != '~') c++;
    return c + 1;
}

void CFontNew::DrawButton(float x, float y, int id) {
    if (!id)
        return;

    CRect rect;
    rect.left = x;
    rect.top = Details.scale.y + Details.scale.y + y;
    rect.right = Details.scale.y * 17.0f + x;
    rect.bottom = Details.scale.y * 19.0f + y;

    int savedAlpha;
    RwRenderStateGet(rwRENDERSTATEVERTEXALPHAENABLE, &savedAlpha);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
    ButtonSprite[id]->Draw(rect, CRGBA(255, 255, 255, Details.color.a));
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)savedAlpha);
}

void CFontNew::PrintChar(float x, float y, char c) {
    float _x = (c % 16);
    float _y = (c / 16);

    float u1 = _x / 16.0f;
    float v1 = _y / 12.8f;
    float u2 = (_x + 1.0f) / 16.0f - 0.003f;
    float v2 = _y / 12.8f;
    float u3 = _x / 16.0f;
    float v3 = (_y + 1.0f) / 12.8f - 0.003f;
    float u4 = (_x + 1.0f) / 16.0f - 0.003f;
    float v4 = (_y + 1.0f) / 12.8f - 0.003f;
    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERLINEARMIPLINEAR);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);

    // Text shadow
    if (Details.shadow > 0.0f) {
        Sprite[Details.style]->Draw(CRect((x + Details.shadow), (y + Details.shadow), (x + Details.shadow) + (32.0f * Details.scale.x * 1.0f), (y + Details.shadow) + (40.0f * Details.scale.y * 0.5f)), CRGBA(Details.dropColor),
            u1, v1, u2, v2,
            u3, v3, u4, v4);
    }
    else if (Details.outline > 0.0f) { // Text outline
        float outline_x[] = {
            Details.outline,
            -Details.outline,
            0.0f,
            0.0f,
            Details.outline,
            -Details.outline,
            Details.outline,
            -Details.outline,
        };

        float outline_y[] = {
            0.0f,
            0.0f,
            Details.outline,
            -Details.outline,
            Details.outline,
            Details.outline,
            -Details.outline,
            -Details.outline,
        };

        for (int i = 0; i < 8; i++) {
            Sprite[Details.style]->Draw(CRect((x + (outline_x[i])), (y + (outline_y[i])), (x + (outline_x[i])) + (32.0f * Details.scale.x * 1.0f), (y + (outline_y[i])) + (40.0f * Details.scale.y * 0.5f)), CRGBA(Details.dropColor),
                u1, v1, u2, v2,
                u3, v3, u4, v4);
        }
    }

    Sprite[Details.style]->Draw(CRect(x, y, x + (32.0f * Details.scale.x * 1.0f), y + (40.0f * Details.scale.y * 0.5f)), CRGBA(Details.color),
        u1, v1, u2, v2,
        u3, v3, u4, v4);
}

void CFontNew::PrintStringFromBottom(float x, float y, char* s) {
    y -= GetHeightScale(Details.scale.y) * GetNumberLines(false, x, y, s);

    PrintString(x, y, s);
}

void PrintCharMap() {
    char* charMap[] = {
        "!\"#$%&'()*+,-./",
        "0123456789:;<=>?",
        "@ABCDEFGHIJKLMNO",
        "PQRSTUVWXYZ[\]^_",
        "`abcdefghijklmno",
        "pqrstuvwxyz",
        "¿¡¬√ƒ«»… ÀÃÕŒœ—“”",
        "‘÷Ÿ⁄€‹ﬂ‡·‚„‰ÁËÈÍ",
        "ÎÏÌÓÔÚÛÙıˆ˘˙˚¸ÒÒø"
    };

    CFontNew::SetWrapX(SCREEN_WIDTH);
    CFontNew::SetClipX(SCREEN_WIDTH);
    CFontNew::SetFontStyle(CFontNew::FONT_4);
    CFontNew::SetAlignment(CFontNew::ALIGN_LEFT);
    CFontNew::SetDropColor(CRGBA(0, 0, 0, 255));
    CFontNew::SetColor(CRGBA(255, 255, 255, 255));
    CFontNew::SetDropShadow(0.0f);
    CFontNew::SetOutline(SCREEN_COORD(2.0f));
    CFontNew::SetScale(SCREEN_MULTIPLIER(1.0f), SCREEN_MULTIPLIER(2.0f));

    float spacing = 0.0f;
    for (int i = 0; i < 9; i++) {
        CFontNew::PrintString(UI_X(0.0f), SCREEN_COORD(spacing), charMap[i]);
        spacing += 48.0f;
    }
}

float CFontNew::GetHeightScale(float h) {
    return 32.0f * h * 0.5f + 2.0f * h;
}

void CFontNew::GetTextRect(CRect* rect, float xstart, float ystart, char* s) {
    if (Details.alignment == ALIGN_CENTER)
        xstart -= GetStringWidth(s, true) / 2;

    if (Details.alignment == ALIGN_RIGHT)
        xstart -= GetStringWidth(s, true);

    rect->left = xstart - (Details.backgroundBorder.left);
    rect->right = xstart + Details.wrapX + (Details.backgroundBorder.right);

    rect->top = ystart - Details.backgroundBorder.top;
    rect->bottom = ystart + (Details.backgroundBorder.bottom) + ((32.0f * Details.scale.y * 0.5f + 2.0f * Details.scale.y) * GetNumberLines(false, xstart, ystart, s));
}
