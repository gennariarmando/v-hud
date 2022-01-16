#include "VHud.h"
#include "FontNew.h"
#include "TextureMgr.h"
#include "Utility.h"
#include "HudNew.h"
#include "PadNew.h"
#include "HudColoursNew.h"

#include "CSprite2d.h"
#include "CFont.h"

#include "pugixml.hpp"

using namespace plugin;
using namespace pugi;

CFontNew FontNew;

bool CFontNew::bInitialised;
CFontDetailsNew CFontNew::Details;
bool CFontNew::bNewLine;
CtrlSprite CFontNew::PS2Symbol;
CSprite2d* CFontNew::ButtonSprite[NUM_BUTTONS];
CD3DSprite* CFontNew::m_pSprite;
ID3DXFont* CFontNew::m_pFont[NUM_FONTS];
int CFontNew::TrueTypeFontCount;
CFontTT CFontNew::TTF[NUM_FONTS];

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
    "SPACEBAR",
    "ESC",
    "F1",
    "F2",
    "F3",
    "F4",
    "F5",
    "F6",
    "F7",
    "F8",
    "F9",
    "F10",
    "F11",
    "F12",
    "INS",
    "DEL",
    "HOME",
    "END",
    "PGUP",
    "PGDN",
    "UP",
    "DOWN",
    "LEFT",
    "RIGHT",
    "DIVIDE",
    "TIMES",
    "PLUS",
    "MINUS",
    "PADDEL",
    "PADEND",
    "PADDOWN",
    "PADPGDN",
    "PADLEFT",
    "PAD5",
    "NUMLOCK",
    "PADRIGHT",
    "PADHOME",
    "PADUP",
    "PADPGUP",
    "PADINS",
    "ENTER",
    "SCROLL",
    "PAUSE",
    "BACKSP",
    "TAB",
    "CAPSLK",
    "ENTER",
    "LSHIFT",
    "RSHIFT",
    "SHIFT",
    "LCTRL",
    "RCTRL",
    "LALT",
    "RALT",
    "LWIN",
    "RWIN",
    "APPS",
    "NULL",
    "LMB",
    "MMB",
    "RMB",
    "MWHU",
    "MWHD",
    "0",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "A",
    "B",
    "C",
    "D",
    "E",
    "F",
    "G",
    "H",
    "I",
    "J",
    "K",
    "L",
    "M",
    "N",
    "O",
    "P",
    "Q",
    "R",
    "S",
    "T",
    "U",
    "V",
    "W",
    "X",
    "Y",
    "Z",
};

// Custom GInput action strings
const char* CustomGInputActions[] = {
#if 0
                "VEHICLE_FIREWEAPON",
                "VEHICLE_TURRET_LEFT_RIGHT",
                "VEHICLE_TURRET_UP_DOWN",
                "GO_LEFTRIGHT",
                "GO_UPDOWN",
                "MELEE_ATTACK",
                "BLOW_UP_RC_BUGGY",
#else
                "PED_MOVE",
                "BMX_HANDBRAKE",
                "BMX_BUNNYHOP",
                "CAMERA_LEFT_RIGHT",
                "CAMERA_UP_DOWN",
                "VEHICLE_CHANGE_RADIO_STATION",
                "GO_LEFTRIGHT",
                "GO_UPDOWN",
                "SNATCH_PACKAGE",
                "HYDRA_TARGET",
#endif
};

static LateStaticInit InstallHooks([]() {

});

void CFontNew::Init() {
    if (bInitialised)
        return;

    ReadValuesFromFile();

    for (int i = 0; i < TrueTypeFontCount; i++) {
        char path[1024];
        sprintf(path, "VHud\\fonts\\%s", TTF[i].fileName);

        if (AddFontResourceEx(PLUGIN_PATH(path), FR_PRIVATE, NULL)) {
            HRESULT h = AddFont(TTF[i], &m_pFont[i]);

            if (h != S_OK) {
                printf("[CFontNew] Error initializing font");
            }
        }
    }

    m_pSprite = new CD3DSprite();

    char* path = "VHud\\buttons\\xbox";
    for (int i = 0; i < NUM_BUTTONS; i++) {
        ButtonSprite[i] = new CSprite2d();

        if (i > BUTTON_THUMBRYD)
            path = "VHud\\buttons\\pc";

        ButtonSprite[i]->m_pTexture = CTextureMgr::LoadPNGTextureCB(PLUGIN_PATH(path), ButtonFileName[i]);
    }

    Clear();

    bInitialised = true;
}

void CFontNew::ReadValuesFromFile() {
    TrueTypeFontCount = 0;

    pugi::xml_document doc;
    xml_parse_result file = doc.load_file(PLUGIN_PATH("VHud\\data\\fonts.xml"));

    if (file) {
        auto fonts = doc.child("Fonts");

        for (int i = 0; i < NUM_FONTS; i++) {
            char buff[16];
            sprintf(buff, "font%d", i + 1);

            if (auto fontId = fonts.child(buff)) {
                strcpy(TTF[i].fontName, fontId.child("FontName").text().as_string());
                strcpy(TTF[i].fileName, fontId.child("FileName").text().as_string());
                TTF[i].charSet = fontId.child("CharSet").text().as_int();
                TTF[i].width = fontId.child("FontWidth").text().as_int();
                TTF[i].height = fontId.child("FontHeight").text().as_int();
                TTF[i].spaceWidth = fontId.child("SpaceWidth").text().as_int();
                TTF[i].quality = fontId.child("Quality").text().as_int();
            }

            TrueTypeFontCount = i;
        }
    }
}

void CFontNew::Clear() {
    SetBackground(false);
    SetGradBackground(false);
    SetBackgroundColor(CRGBA(0, 0, 0, 255));
    SetBackgroundBorder(CRect(0, 0, 0, 0));
    SetClipX(SCREEN_WIDTH);
    SetClipX(-1);
    SetWrapX(SCREEN_WIDTH);
    SetAlignment(ALIGN_LEFT);
    SetFontStyle(FONT_1);
    SetColor(CRGBA(255, 255, 255, 255));
    SetDropShadow(0.0f);
    SetDropColor(CRGBA(0, 0, 0, 255));
    SetScale(1.0f, 1.0f);
    bNewLine = false;
    PS2Symbol.Symbol = NULL;
    SetTokenToIgnore(NULL, NULL);
    SetIgnoreGamePadSymbols(false);
}

long CFontNew::AddFont(CFontTT t, LPD3DXFONT* font) {
    return D3DXCreateFontA(GetD3DDevice(), t.height, t.width, FW_NORMAL, 0, FALSE, t.charSet, OUT_DEFAULT_PRECIS, t.quality, DEFAULT_PITCH | FF_DONTCARE, t.fontName, font);
}

void CFontNew::Shutdown() {
    if (!bInitialised)
        return;

    for (int i = 0; i < TrueTypeFontCount; i++) {
        char path[512];
        sprintf(path, "VHud\\fonts\\%s", TTF[i].fileName);
        RemoveFontResourceEx(PLUGIN_PATH(path), FR_PRIVATE, NULL);

        if (m_pFont[i]) {
            m_pFont[i]->Release();
            m_pFont[i] = NULL;
        }
    }

    delete m_pSprite;

    for (int i = 0; i < NUM_BUTTONS; i++) {
        if (ButtonSprite[i]) {
            ButtonSprite[i]->Delete();
            delete ButtonSprite[i];
        }
    }

    bInitialised = false;
}

void CFontNew::Reset() {
    for (int i = 0; i < TrueTypeFontCount; i++) {
        if (m_pFont[i]) {
            m_pFont[i]->OnResetDevice();
        }
    }

    if (m_pSprite)
        m_pSprite->OnResetDevice();
}

void CFontNew::Lost() {
    for (int i = 0; i < TrueTypeFontCount; i++) {
        if (m_pFont[i]) {
            m_pFont[i]->OnLostDevice();
        }
    }

    if (m_pSprite)
        m_pSprite->OnLostDevice();
}

void CFontNew::PrepareSymbolScale() {
    CSprite2d* sprite = PS2Symbol.Symbol;
    if (sprite && sprite->m_pTexture) {
        PS2Symbol.PS2SymbolScale.x = clamp(sprite->m_pTexture->raster->width, 0, 128);
        PS2Symbol.PS2SymbolScale.y = clamp(sprite->m_pTexture->raster->height, 0, 128);
    }
}

float CFontNew::GetCharacterSize(char c) {
    float n = 0.0f;

    if (PS2Symbol.NoPrint) {
        PS2Symbol.NoPrint = false;
        return Details.scale.y * (PS2Symbol.PS2SymbolScale.x / 3);
    }

    switch (c) {
    case '~':
        return n;
    }

    return DrawChar(false, true, 0.0f, 0.0f, c, Details.style, NULL);
}

float CFontNew::GetStringWidth(const char* s, bool spaces) {
    float w;

    w = 0.0f;
    for (; (*s != ' ' || spaces) && *s != '\0'; s++) {
        if (*s == '~')
            s = ParseToken(false, s);

        w += GetCharacterSize(*s);
    }
    return w;
}

const char* CFontNew::GetNextSpace(const char* s) {
    for (; *s != ' ' && *s != '\0'; s++)
        if (*s == '~') {
            s++;
            while (*s != '~') s++;
        }

    return s;
}

void CFontNew::SetTokenToIgnore(char t1, char t2) {
    Details.ignoreTokens[0] = t1;
    Details.ignoreTokens[1] = t2;
}

int CFontNew::PrintString(float x, float y, const char* s) {
    int n = 0;
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
        n = GetNumberLines(true, x, y, s);
    }

    return n;
}

int CFontNew::GetNumberLines(bool print, float xstart, float ystart, const char* s) {
    float x = xstart;
    float y = ystart;
    int n = 1;
    int letterCount = 0;

    if (Details.alignment == ALIGN_CENTER) {
        bool first = true;
        char const* start = s;
        char* t = (char*)s;
        float length = 0.0f;
        int space = 0;

        x = 0.0f;

        while (s) {
            while (s) {
                if (*s == '\0')
                    return n;

                float w = Details.wrapX;

                if ((x + GetStringWidth(s) > w || bNewLine) && !first) {
                    float cx = xstart - x / 2;
                    PrintString(print, cx, y, start, s, 0.0f);

                    x = 0.0f;
                    y += GetHeightScale(Details.scale.y);
                    start = s;

                    length = 0.0f;
                    space = 0;
                    first = true;
                    n++;
                }

                t = (char*)GetNextSpace(s);
                if (t[0] == '\0' || t[0] == ' ' && t[1] == '\0') {
                    break;
                }

                if (!first)
                    space++;

                first = false;
                x += GetStringWidth(s) + GetCharacterSize(*t);

                length = x;
                s = t + 1;
            }

            if (t[0] == ' ' && t[1] == '\0') {
                t[0] = '\0';
            }

            x += GetStringWidth(s);
            s = t;
            float cx = xstart - x / 2;
            PrintString(print, cx, y, start, s, 0.0f);
        }

        bNewLine = false;
        return n;
    }
    else if (Details.alignment == ALIGN_RIGHT) {
        x -= GetStringWidth(s, true);
    }

    for (s; *s != '\0'; s++) {
        float f = xstart + Details.wrapX;

        if ((Details.clipXCount == -1 && (x + GetCharacterSize(*s) > xstart + Details.clipX)))
            break;
        else if (Details.clipXCount != -1 && letterCount >= Details.clipXCount)
            break;

        if (x + GetStringWidth(s) > f || bNewLine) {
            x = xstart;
            y += 32.0f * Details.scale.y * 0.5f + 2.0f * Details.scale.y;
            bNewLine = false;
            n++;
        }

        if (*s == '~')
            s = ParseToken(print, s);

        if (print)
            PrintChar(x, y, *s);

        letterCount++;
        x += GetCharacterSize(*s);
        PS2Symbol.Symbol = NULL;
        PS2Symbol.NoPrint = false;
    }

    return n;
}

void CFontNew::PrintString(bool print, float x, float y, const char* start, const char* end, float spwidth) {
    char const* s;
    char c;

    float xstart = x;
    for (s = start; s < end; s++) {
        if (*s == '~')
            s = ParseToken(print, s);

        c = *s;

        if (print)
            PrintChar(x, y, c);

        x += GetCharacterSize(c);

        if (c == 0)
            x += spwidth;

        PS2Symbol.Symbol = NULL;
        PS2Symbol.NoPrint = false;
    }
}

const char* CFontNew::ParseToken(bool print, const char* s) {
    char const* c = s + 1;
    int a = Details.color.a;

    if (Details.ignoreTokens[0] != *c && Details.ignoreTokens[1] != *c) {
        switch (*c) {
        case '<':
            PS2Symbol.Symbol = ButtonSprite[BUTTON_LEFT];
            break;
        case '>':
            PS2Symbol.Symbol = ButtonSprite[BUTTON_RIGHT];
            break;
        case 'A':
        case 'a':
            PS2Symbol.Symbol = ButtonSprite[BUTTON_L3];
            break;
        case 'B':
        case 'b':
            SetColor(HudColourNew.GetRGB(HUD_COLOUR_BLUE, a));
            break;
        case 'C':
        case 'c':
            PS2Symbol.Symbol = ButtonSprite[BUTTON_R3];
            break;
        case 'D':
        case 'd':
            PS2Symbol.Symbol = ButtonSprite[BUTTON_DOWN];
            break;
        case 'G':
        case 'g':
            SetColor(HudColourNew.GetRGB(HUD_COLOUR_GREEN, a));
            break;
        case 'H':
        case 'h':
            SetColor(HudColourNew.GetRGB(HUD_COLOUR_WHITE, a));
            break;
        case 'J':
        case 'j':
            PS2Symbol.Symbol = ButtonSprite[BUTTON_R1];
            break;
        case 'K':
        case 'k':
            c += 3;
            PS2Symbol.Symbol = GetActionSprite(ParseCustomActions(c));
            break;
        case 'M':
        case 'm':
            PS2Symbol.Symbol = ButtonSprite[BUTTON_L2];
            break;
        case 'N':
        case 'n':
            bNewLine = true;
            break;
        case 'O':
        case 'o':
            PS2Symbol.Symbol = ButtonSprite[BUTTON_CIRCLE];
            break;
        case 'P':
        case 'p':
            SetColor(HudColourNew.GetRGB(HUD_COLOUR_PURPLE, a));
            break;
        case 'Q':
        case 'q':
            PS2Symbol.Symbol = ButtonSprite[BUTTON_SQUARE];
            break;
        case 'R':
        case 'r':
            SetColor(HudColourNew.GetRGB(HUD_COLOUR_RED, a));
            break;
        case 'S':
        case 's':
            SetColor(HudColourNew.GetRGB(HUD_COLOUR_WHITE, a));
            break;
        case 'T':
        case 't':
            PS2Symbol.Symbol = ButtonSprite[BUTTON_TRIANGLE];
            break;
        case 'U':
        case 'u':
            if (c[1] == 'd' || c[1] == 'D') {
                PS2Symbol.Symbol = ButtonSprite[BUTTON_UPDOWN];
                c++;
            }
            else
                PS2Symbol.Symbol = ButtonSprite[BUTTON_UP];
            break;
        case 'V':
        case 'v':
            PS2Symbol.Symbol = ButtonSprite[BUTTON_R2];
            break;
        case 'W':
        case 'w':
            SetColor(HudColourNew.GetRGB(HUD_COLOUR_WHITE, a));
            break;
        case 'X':
        case 'x':
            PS2Symbol.Symbol = ButtonSprite[BUTTON_CROSS];
            break;
        case 'Y':
        case 'y':
            SetColor(HudColourNew.GetRGB(HUD_COLOUR_YELLOW, a));
            break;
        case 'l':
            SetColor(HudColourNew.GetRGB(HUD_COLOUR_BLACK, a));
            break;
        case '[':
            switch (c[1]) {
            case '~':
                PS2Symbol.Symbol = ButtonSprite[BUTTON_THUMBL];
                break;
            case 'x':
                PS2Symbol.Symbol = ButtonSprite[BUTTON_THUMBLX];
                break;
            case 'y':
                PS2Symbol.Symbol = ButtonSprite[BUTTON_THUMBLY];
                break;
            case '<':
                PS2Symbol.Symbol = ButtonSprite[BUTTON_THUMBLXL];
                break;
            case '>':
                PS2Symbol.Symbol = ButtonSprite[BUTTON_THUMBLXR];
                break;
            case 'u':
                PS2Symbol.Symbol = ButtonSprite[BUTTON_THUMBLYU];
                break;
            case 'd':
                PS2Symbol.Symbol = ButtonSprite[BUTTON_THUMBLYD];
                break;
            }
            c++;
            break;
        case ']':
            switch (*(++c)) {
            case '~':
                PS2Symbol.Symbol = ButtonSprite[BUTTON_THUMBR];
                break;
            case 'x':
                PS2Symbol.Symbol = ButtonSprite[BUTTON_THUMBRX];
                break;
            case 'y':
                PS2Symbol.Symbol = ButtonSprite[BUTTON_THUMBRY];
                break;
            case '<':
                PS2Symbol.Symbol = ButtonSprite[BUTTON_THUMBRXL];
                break;
            case '>':
                PS2Symbol.Symbol = ButtonSprite[BUTTON_THUMBRXR];
                break;
            case 'u':
                PS2Symbol.Symbol = ButtonSprite[BUTTON_THUMBRYU];
                break;
            case 'd':
                PS2Symbol.Symbol = ButtonSprite[BUTTON_THUMBRYD];
                break;
            }
            break;
        case '@':
            if (HAS_PAD_IN_HANDS(0))
                ParseGInputActions(++c);
            else
                PS2Symbol.Symbol = GetActionSprite(CPadNew::StringToKey(++c));
            c++;
            break;
        }
    }

    PrepareSymbolScale();

    if (!print) {
        if (PS2Symbol.Symbol) {
            PS2Symbol.NoPrint = true;
            PS2Symbol.Symbol = NULL;
        }
        bNewLine = false;
    }

    while (*c != '~') c++;

    if (*c == '~' && *c + 1 == '~')
        return c + 1;

    return c;
}

CSprite2d* CFontNew::GetActionSprite(int key) {
    CSprite2d* sprite = NULL;

    if (HAS_PAD_IN_HANDS(0) && !Details.ignoreGamePadSymbols) {
        if (key != GAMEPAD_NONE)
            sprite = ButtonSprite[key + 1];
    }
    else {
        if (key > MOUSE_CUSTOM_OFFSET) {
            sprite = ButtonSprite[(key - MOUSE_CUSTOM_OFFSET) + BUTTON_PC_NULL];
        }
        else {
            if ((key >= 'A' && key <= 'Z'))
                sprite = ButtonSprite[(key - 'A') + BUTTON_PC_A];

            if (key >= 'a' && key <= 'z')
                sprite = ButtonSprite[(key - 'a') + BUTTON_PC_A];

            if (key >= '0' && key <= '9')
                sprite = ButtonSprite[(key - '9') + BUTTON_PC_0];

            if (key == rsSPACE)
                sprite = ButtonSprite[BUTTON_PC_SPACEBAR];

            if (key >= rsESC && key < rsNULL)
                sprite = ButtonSprite[(key - rsESC) + BUTTON_PC_ESC];
        }
    }

    return sprite;
}

bool astrcmp(const char* s, const char* a) {
    while (*a) {
        if (*s != *a)
            return false;

        s++;
        a++;
    }
    return true;
}

int CFontNew::ParseCustomActions(const char* s) {
    for (int i = 0; i < NUM_CONTROL_ACTIONS; i++) {
        if (astrcmp(s, Controls[i].action)) {
            if (HAS_PAD_IN_HANDS(0) && !Details.ignoreGamePadSymbols)
                return Controls[i].button;
            else
                return Controls[i].key;
        }
    }

    return rsNULL;
}

bool CFontNew::ParseGInputActions(const char* s) {
    short mode = CPadNew::GetPad(0)->Mode;
    bool southPaw = GInputPadSettings[0].Southpaw;
    for (int i = 0; i < ARRAY_SIZE(CustomGInputActions); i++) {
        if (astrcmp(s, CustomGInputActions[i])) {
            switch (i) {
            case ACTION_PED_MOVE:
                PS2Symbol.Symbol = southPaw ? ButtonSprite[BUTTON_THUMBR] : ButtonSprite[BUTTON_THUMBL];
                break;
            case ACTION_BMX_HANDBRAKE:
                switch (mode) {
                case 0:
                    PS2Symbol.Symbol = ButtonSprite[BUTTON_R1];
                    break;
                case 1:
                    PS2Symbol.Symbol = ButtonSprite[BUTTON_R2];
                    break;
                }
                break;
            case ACTION_BMX_BUNNYHOP:
                switch (mode) {
                case 0:
                    PS2Symbol.Symbol = ButtonSprite[BUTTON_L1];
                    break;
                case 1:
                    PS2Symbol.Symbol = ButtonSprite[BUTTON_SQUARE];
                    break;
                }
                break;
            case ACTION_CAMERA_LEFT_RIGHT:
                PS2Symbol.Symbol = southPaw ? ButtonSprite[BUTTON_THUMBLX] : ButtonSprite[BUTTON_THUMBRX];
                break;
            case ACTION_CAMERA_UP_DOWN:
                PS2Symbol.Symbol = southPaw ? ButtonSprite[BUTTON_THUMBLY] : ButtonSprite[BUTTON_THUMBRY];
                break;
            case ACTION_VEHICLE_CHANGE_RADIO_STATION:
                switch (mode) {
                case 0:
                    PS2Symbol.Symbol = ButtonSprite[BUTTON_UPDOWN];
                    break;
                case 1:
                    PS2Symbol.Symbol = ButtonSprite[BUTTON_LEFTRIGHT];
                    break;
                }
                break;
            case ACTION_GO_LEFTRIGHT:
                PS2Symbol.Symbol = southPaw ? ButtonSprite[BUTTON_THUMBRX] : ButtonSprite[BUTTON_THUMBLX];
                break;
            case ACTION_GO_UPDOWN:
                PS2Symbol.Symbol = southPaw ? ButtonSprite[BUTTON_THUMBRY] : ButtonSprite[BUTTON_THUMBLY];
                break;
            case ACTION_SNATCH_PACKAGE:
                switch (mode) {
                case 0:
                    PS2Symbol.Symbol = ButtonSprite[BUTTON_L1];
                    break;
                case 1:
                    PS2Symbol.Symbol = ButtonSprite[BUTTON_CIRCLE];
                    break;
                }
                break;
            case ACTION_HYDRA_TARGET:
                switch (mode) {
                case 0:
                    PS2Symbol.Symbol = ButtonSprite[BUTTON_R1];
                    break;
                case 1:
                    PS2Symbol.Symbol = ButtonSprite[BUTTON_SQUARE];
                    break;
                }
                break;
            }

            return true;
        }
    }

    return false;
}

void CFontNew::DrawButton(float& x, float y, CSprite2d* sprite) {
    if (!sprite || sprite && !sprite->m_pTexture)
        return;

    CRect rect = { };

    float w = Details.scale.y * (PS2Symbol.PS2SymbolScale.x / 3);
    float h = Details.scale.y * (PS2Symbol.PS2SymbolScale.y / 3);

    rect.left = x;
    rect.top = y + SCREEN_COORD(1.0f);
    rect.right = rect.left + (w);
    rect.bottom = rect.top + (h);

    x += w;

    int savedAlpha;
    RwRenderStateGet(rwRENDERSTATEVERTEXALPHAENABLE, &savedAlpha);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
    sprite->Draw(rect, CRGBA(255, 255, 255, Details.color.a));
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)savedAlpha);
}

float CFontNew::PrintChar(float& x, float y, char c) {
    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERLINEARMIPLINEAR);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
    RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*)rwTEXTUREADDRESSWRAP);

    DrawButton(x, y, PS2Symbol.Symbol);

    // Text shadow
    if (Details.shadow > 0.0f) {
        DrawChar(true, false, x + (Details.shadow), y + (Details.shadow), c, Details.style, CRGBA(Details.dropColor));
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
            DrawChar(true, false, x + (outline_x[i]), y + (outline_y[i]), c, Details.style, CRGBA(Details.dropColor));
        }
    }

    return DrawChar(true, true, x, y, c, Details.style, CRGBA(Details.color));
}

float CFontNew::DrawChar(bool print, bool calc, float x, float y, char c, int style, CRGBA const& col) {
    float characterSize = 0.0f;
    static char s[2];
    s[0] = c;

    if (c != '~') {
        //x += SCREEN_WIDTH * 0.002f;
        y += SCREEN_HEIGHT * 0.002f;

        float w = Details.scale.x * FONT_WIDTH_MULT;
        float h = Details.scale.y * FONT_HEIGHT_MULT;
        RECT d3drect = { x / w, y / h, (x + SCREEN_WIDTH) / w, (y + SCREEN_HEIGHT) / h };
        D3DXMATRIX S, P;

        if (m_pFont[Details.style] && m_pSprite) {
            if (print) {
                DWORD savedColorWrite = NULL;
                IDirect3DVertexDeclaration9* savedVertexDecl = NULL;
                IDirect3DVertexShader9* savedVertexShader = NULL;
                IDirect3DPixelShader9* savedPixelShader = NULL;

                GetD3DDevice()->GetRenderState(D3DRS_COLORWRITEENABLE, &savedColorWrite);
                GetD3DDevice()->GetVertexDeclaration(&savedVertexDecl);
                GetD3DDevice()->GetVertexShader(&savedVertexShader);
                GetD3DDevice()->GetPixelShader(&savedPixelShader);

                D3DXMatrixScaling(&S, w, h, 1.0f);
                m_pSprite->GetTransform(&P);
                m_pSprite->SetTransform(&S);
                m_pSprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_TEXTURE | D3DXSPRITE_DO_NOT_ADDREF_TEXTURE);
                m_pFont[style]->DrawTextA(m_pSprite, s, -1, &d3drect, DT_LEFT | DT_TOP, D3DCOLOR_RGBA(col.r, col.g, col.b, col.a));
                m_pSprite->End();
                m_pSprite->SetTransform(&P);

                GetD3DDevice()->SetRenderState(D3DRS_COLORWRITEENABLE, savedColorWrite);
                GetD3DDevice()->SetVertexDeclaration(savedVertexDecl);
                GetD3DDevice()->SetVertexShader(savedVertexShader);
                GetD3DDevice()->SetPixelShader(savedPixelShader);
            }

            if (calc) {
                d3drect = { 0, 0, 0, 0 };
                m_pFont[style]->DrawTextA(m_pSprite, s, -1, &d3drect, DT_LEFT | DT_TOP | DT_NOCLIP | DT_SINGLELINE | DT_CALCRECT, NULL);
                characterSize = ((d3drect.right - d3drect.left) + (Details.outline)) * w;

                if (c == ' ')
                    characterSize += (TTF[Details.style].spaceWidth) * w;
            }
        }
    }

    return characterSize;
}

void CFontNew::PrintStringFromBottom(float x, float y, const char* s) {
    y -= GetHeightScale(Details.scale.y) * (GetNumberLines(false, x, y, s) - 1);

    PrintString(x, y, s);
}

float CFontNew::GetHeightScale(float h) {
    return TTF[Details.style].height * (h * FONT_HEIGHT_MULT);
}

void CFontNew::GetTextRect(CRect* rect, float xstart, float ystart, const char* s) {
    if (Details.alignment == ALIGN_CENTER)
        xstart -= GetStringWidth(s, true) / 2;

    if (Details.alignment == ALIGN_RIGHT)
        xstart -= GetStringWidth(s, true);

    rect->left = xstart - (Details.backgroundBorder.left);
    rect->right = xstart + Details.wrapX + (Details.backgroundBorder.right);

    rect->top = ystart - Details.backgroundBorder.top;
    rect->bottom = ystart + (Details.backgroundBorder.bottom) + ((32.0f * Details.scale.y * 0.5f + 2.0f * Details.scale.y) * GetNumberLines(true, xstart, ystart, s));
}
