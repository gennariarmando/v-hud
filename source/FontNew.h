#pragma once

class CSprite2d;

struct CFontDetailsNew {
	CRGBA color;
	CRGBA dropColor;
	CVector2D scale;
	int alignment;
	float wrapX;
	float clipX;
	int style;
	float shadow;
	float outline;
	bool background;
	bool grad;
	CRGBA backgroundColor;
	CRect backgroundBorder;
	bool isBlip;

	CFontDetailsNew() {}
};

class CFontNew {
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

		BUTTON_PC_UP,
		BUTTON_PC_DOWN,
		BUTTON_PC_LEFT,
		BUTTON_PC_RIGHT,

		NUM_BUTTONS,
	};

    static CSprite2d* Sprite[NUM_FONTS];
	static CFontDetailsNew Details;
	static char Size[NUM_FONTS][160];
	static bool bNewLine;
	static int NumLines;
	static int PS2Symbol;
	static CSprite2d* ButtonSprite[NUM_BUTTONS];

public:
	CFontNew();
	static void Init();
	static void ReadValuesFromFile();
	static void Clear();
	static void Shutdown();
	static float GetCharacterSize(char c);
	static float GetStringWidth(char* s, bool spaces = false);
	static char* GetNextSpace(char* s);
	static int GetNumberLines(bool print, float x, float y, char* s);
	static char* ParseToken(char *s, CRGBA& color);
	static void DrawButton(float x, float y, int id);
	static void PrintString(float x, float y, char* s);
	static void PrintString(bool print, float x, float y, char* start, char* end, float spwidth);
	static void PrintChar(float x, float y, char c);
	static void PrintStringFromBottom(float x, float y, char* s);
	static float GetHeightScale(float h);
	static void GetTextRect(CRect* rect, float xstart, float ystart, char* s);

public:
	static void SetBackground(bool on) { Details.background = on; }
	static void SetGradBackground(bool on) { Details.grad = on; }
	static void SetBackgroundColor(CRGBA const& col) { Details.backgroundColor = col; }
	static void SetBackgroundBorder(CRect const& rect) { Details.backgroundBorder = rect; }
	static void SetClipX(float x) { Details.clipX = x; }
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