#pragma once

class CSprite2d;

class CtrlSprite {
public:
	bool NoPrint;
	CSprite2d* Symbol;
	CVector PS2SymbolScale;
};

struct CFontDetailsNew {
	CRGBA color;
	CRGBA dropColor;
	CVector2D scale;
	int alignment;
	float wrapX;
	float clipX;
	int clipXCount;
	int style;
	float shadow;
	float outline;
	bool background;
	bool grad;
	CRGBA backgroundColor;
	CRect backgroundBorder;
	bool isBlip;
	char ignoreTokens[2];

	CFontDetailsNew() {}
};

class VHUD_API CFontNew {
public:
	enum {
		ALIGN_LEFT,
		ALIGN_CENTER,
		ALIGN_RIGHT,
	};
	enum {
		FONT_1,
		FONT_2,
		FONT_3,
		FONT_4,
		NUM_FONTS
	};

	enum eButtonSprites {
		BUTTON_NONE = 0,

		BUTTON_UP,
		BUTTON_DOWN,
		BUTTON_LEFT,
		BUTTON_RIGHT,

		BUTTON_CROSS,
		BUTTON_CIRCLE,
		BUTTON_SQUARE,
		BUTTON_TRIANGLE,

		BUTTON_L1,
		BUTTON_L2,
		BUTTON_L3,
		BUTTON_R1,
		BUTTON_R2,
		BUTTON_R3,

		BUTTON_START,
		BUTTON_SELECT,

		BUTTON_UPDOWN,
		BUTTON_LEFTRIGHT,

		BUTTON_THUMBL,
		BUTTON_THUMBLX,
		BUTTON_THUMBLXL,
		BUTTON_THUMBLXR,
		BUTTON_THUMBLY,
		BUTTON_THUMBLYU,
		BUTTON_THUMBLYD,

		BUTTON_THUMBR,
		BUTTON_THUMBRX,
		BUTTON_THUMBRXL,
		BUTTON_THUMBRXR,
		BUTTON_THUMBRY,
		BUTTON_THUMBRYU,
		BUTTON_THUMBRYD,

		BUTTON_PC_SPACEBAR,
		BUTTON_PC_ESC,
		BUTTON_PC_F1,
		BUTTON_PC_F2,
		BUTTON_PC_F3,
		BUTTON_PC_F4,
		BUTTON_PC_F5,
		BUTTON_PC_F6,
		BUTTON_PC_F7,
		BUTTON_PC_F8,
		BUTTON_PC_F9,
		BUTTON_PC_F10,
		BUTTON_PC_F11,
		BUTTON_PC_F12,
		BUTTON_PC_INS,
		BUTTON_PC_DEL,
		BUTTON_PC_HOME,
		BUTTON_PC_END,
		BUTTON_PC_PGUP,
		BUTTON_PC_PGDN,
		BUTTON_PC_UP,
		BUTTON_PC_DOWN,
		BUTTON_PC_LEFT,
		BUTTON_PC_RIGHT,
		BUTTON_PC_DIVIDE,
		BUTTON_PC_TIMES,
		BUTTON_PC_PLUS,
		BUTTON_PC_MINUS,
		BUTTON_PC_PADDEL,
		BUTTON_PC_PADEND,
		BUTTON_PC_PADDOWN,
		BUTTON_PC_PADPGDN,
		BUTTON_PC_PADLEFT,
		BUTTON_PC_PAD5,
		BUTTON_PC_NUMLOCK,
		BUTTON_PC_PADRIGHT,
		BUTTON_PC_PADHOME,
		BUTTON_PC_PADUP,
		BUTTON_PC_PADPGUP,
		BUTTON_PC_PADINS,
		BUTTON_PC_PADENTER,
		BUTTON_PC_SCROLL,
		BUTTON_PC_PAUSE,
		BUTTON_PC_BACKSP,
		BUTTON_PC_TAB,
		BUTTON_PC_CAPSLK,
		BUTTON_PC_ENTER,
		BUTTON_PC_LSHIFT,
		BUTTON_PC_RSHIFT,
		BUTTON_PC_SHIFT,
		BUTTON_PC_LCTRL,
		BUTTON_PC_RCTRL,
		BUTTON_PC_LALT,
		BUTTON_PC_RALT,
		BUTTON_PC_LWIN,
		BUTTON_PC_RWIN,
		BUTTON_PC_APPS,
		BUTTON_PC_NULL,
		BUTTON_PC_LMB,
		BUTTON_PC_MMB,
		BUTTON_PC_RMB,
		BUTTON_PC_MWHU,
		BUTTON_PC_MWHD,
		BUTTON_PC_0,
		BUTTON_PC_1,
		BUTTON_PC_2,
		BUTTON_PC_3,
		BUTTON_PC_4,
		BUTTON_PC_5,
		BUTTON_PC_6,
		BUTTON_PC_7,
		BUTTON_PC_8,
		BUTTON_PC_9,
		BUTTON_PC_A,
		BUTTON_PC_B,
		BUTTON_PC_C,
		BUTTON_PC_D,
		BUTTON_PC_E,
		BUTTON_PC_F,
		BUTTON_PC_G,
		BUTTON_PC_H,
		BUTTON_PC_I,
		BUTTON_PC_J,
		BUTTON_PC_K,
		BUTTON_PC_L,
		BUTTON_PC_M,
		BUTTON_PC_N,
		BUTTON_PC_O,
		BUTTON_PC_P,
		BUTTON_PC_Q,
		BUTTON_PC_R,
		BUTTON_PC_S,
		BUTTON_PC_T,
		BUTTON_PC_U,
		BUTTON_PC_V,
		BUTTON_PC_W,
		BUTTON_PC_X,
		BUTTON_PC_Y,
		BUTTON_PC_Z,
		NUM_BUTTONS,
	};

	static bool bInitialised;
    static CSprite2d* Sprite[NUM_FONTS];
	static CFontDetailsNew Details;
	static char Size[NUM_FONTS][160];
	static bool bNewLine;
	static CtrlSprite PS2Symbol;
	static CSprite2d* ButtonSprite[NUM_BUTTONS];

public:
	CFontNew();
	static void Init();
	static void ReadValuesFromFile();
	static void Clear();
	static void Shutdown();
	static void PrepareSymbolScale();
	static float GetCharacterSize(char c);
	static float GetStringWidth(const char* s, bool spaces = false);
	static const char* GetNextSpace(const char* s);
	static void SetTokenToIgnore(char t1, char t2);
	static int GetNumberLines(bool print, float x, float y, const char* s);
	static const char* ParseToken(bool print, char const* s);
	static CSprite2d* GetActionSprite(int key);
	static int ParseCustomActions(char const* s);
	static bool ParseGInputActions(const char* s);
	static void DrawButton(float& x, float y, CSprite2d* sprite);
	static int PrintString(float x, float y, const char* s);
	static void PrintString(bool print, float x, float y, const char* start, const char* end, float spwidth);
	static void PrintChar(float& x, float y, char c);
	static void PrintStringFromBottom(float x, float y, const char* s);
	static float GetHeightScale(float h);
	static void GetTextRect(CRect* rect, float xstart, float ystart, const char* s);

public:
	static void SetBackground(bool on) { Details.background = on; }
	static void SetGradBackground(bool on) { Details.grad = on; }
	static void SetBackgroundColor(CRGBA const& col) { Details.backgroundColor = col; }
	static void SetBackgroundBorder(CRect const& rect) { Details.backgroundBorder = rect; }

	static void SetClipX(float x) { 
		Details.clipX = x; 
		Details.clipXCount = -1;
	}

	static void SetClipX(int count) { Details.clipXCount = count; }
	static void SetWrapX(float x) { Details.wrapX = x; }
	static void SetAlignment(int align) { Details.alignment = align; }
	static void SetDropShadow(float shadow) { Details.shadow = shadow; }
	static void SetOutline(float f) { Details.outline = f; }
	static void SetFontStyle(int style) { 
		Details.style = style;
	}
	static void SetColor(CRGBA const& col) { Details.color = col; }
	static void SetDropColor(CRGBA const& col) { Details.dropColor = col; }
	static void SetScale(float w, float h) { Details.scale = CVector2D(w, h); }
};

extern void PrintCharMap();
