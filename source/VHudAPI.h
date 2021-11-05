#include "ModuleList.hpp"

namespace VHudAPI {
	static void* GetFunctionByName(const char* name) {
		const HMODULE h = ModuleList().Get(L"VHud");
		if (h) {
			auto a = (void* (*)())GetProcAddress(h, name);

			if (a) {
				return a;
			}
		}
		return NULL;
	}

	template <typename... Args>
	static void Call(const char* name, Args... args) {
		void* f = GetFunctionByName(name);
		if (f)
			reinterpret_cast<void(__cdecl*)(Args...)>(f)(args...);
	}

	template <typename Ret, typename... Args>
	static Ret CallAndReturn(const char* name, Args... args) {
		void* f = GetFunctionByName(name);

		if (f)
			return reinterpret_cast<Ret(__cdecl*)(Args...)>(f)(args...);

		return NULL;
	}

	struct CFontNew {
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

		static int PrintString(float x, float y, char* s) {
			return CallAndReturn<int, float, float, char*>("?PrintString@CFontNew@@SAHMMPAD@Z", x, y, s);
		}

		static void PrintStringFromBottom(float x, float y, char* s) {
			Call<float, float, char*>("?PrintStringFromBottom@CFontNew@@SAHMMPAD@Z", x, y, s);
		}

		static void SetScale(float w, float h) {
			Call<float, float>("?SetScale@CFontNew@@SAXMM@Z", w, h);
		}

		static void SetAlignment(int align) {
			Call<int>("?SetAlignment@CFontNew@@SAXH@Z", align);
		}

		static void SetColor(CRGBA const& col) {
			Call<CRGBA const&>("?SetColor@CFontNew@@SAXABVCRGBA@@@Z", col);
		}

		static void SetDropColor(CRGBA const& col) {
			Call<CRGBA const&>("?SetDropColor@CFontNew@@SAXABVCRGBA@@@Z", col);
		}

		static void SetFontStyle(int style) {
			Call<int>("?SetFontStyle@CFontNew@@SAXH@Z", style);
		}

		static void SetOutline(float f) {
			Call<float>("?SetOutline@CFontNew@@SAXM@Z", f);
		}

		static void SetDropShadow(float shadow) {
			Call<float>("?SetDropShadow@CFontNew@@SAXM@Z", shadow);
		}

		static float GetStringWidth(char* s, bool spaces = false) {
			Call<char*, bool>("?GetStringWidth@CFontNew@@SAMPAD_N@Z", s, spaces);
		}

		static void SetTokenToIgnore(char t1, char t2) {
			Call<char, char>("?SetTokenToIgnore@CFontNew@@SAXDD@Z", t1, t2);
		}

		static void SetBackground(bool on) {
			Call<bool>("?SetBackground@CFontNew@@SAX_N@Z", on);
		}

		static void SetGradBackground(bool on) {
			Call<bool>("?SetBackground@CFontNew@@SAX_N@Z", on);
		}

		static void SetBackgroundColor(CRGBA const& col) {
			Call<CRGBA const&>("?SetBackgroundColor@CFontNew@@SAXABVCRGBA@@@Z", col);
		}

		static void SetBackgroundBorder(CRect const& rect) {
			Call<CRect const&>("?SetBackgroundBorder@CFontNew@@SAXABVCRect@@@Z", rect);
		}

		static void SetClipX(float x) {
			Call<float>("?SetClipX@CFontNew@@SAXH@Z", x);
		}

		static void SetWrapX(float x) {
			Call<float>("?SetWrapX@CFontNew@@SAXM@Z", x);
		}
	};
}
