#ifndef __GINPUTAPI
#define __GINPUTAPI

// GInput 1.11 API file

// With this little API prepared for C++, you can take advantage of some GInput features
// For functions list, scroll down to the interface declaration


// As this API file is compatible with GInputIII. GInputVC and GInputSA, you need to declare
// whether to compile III version, VC version, SA version, or a cross-compatible version

// The API doesn't target a specific game by default - you need one of those four defines in order for API to work correctly:

// To target GTA III, define GINPUT_COMPILE_III_VERSION in your project settings, code headers or just
// uncomment the line below
//#define GINPUT_COMPILE_III_VERSION

// To target GTA VC, define GINPUT_COMPILE_VC_VERSION in your project settings, code headers or just
// uncomment the line below
//#define GINPUT_COMPILE_VC_VERSION

// To target GTA SA, define GINPUT_COMPILE_SA_VERSION in your project settings, code headers or just
// uncomment the line below
//#define GINPUT_COMPILE_SA_VERSION

// You can also target all threee games at once by defining GINPUT_COMPILE_CROSSCOMPATIBLE_VERSION - define it
// in your project settings, code headers or just uncomment the line below
//#define GINPUT_COMPILE_CROSSCOMPATIBLE_VERSION

enum eGInputEvent
{
	GINPUT_NO_EVENT						= -1,	// DON'T USE, internal use only

	GINPUT_EVENT_CHANGE_RADIO_STATION	= 0,	// Returns NULL all the time, pParam specifies whether to set
												// next or previous radio station (0 for next station, 1 for previous)
												// Callable from SendEvent: YES
												// Callable from SendConstEvent: NO

	GINPUT_EVENT_CHANGE_USER_TRACK		= 1,	// Returns NULL all the time, pParam specifies whether to set
												// next or previous MP3 (0 for next MP3, 1 for previous)
												// Callable from SendEvent: YES
												// Callable from SendConstEvent: NO

	GINPUT_EVENT_REGISTER_SETTINGS_RELOAD_CALLBACK	= 2, // Registers a callback function which gets called after GInput*.ini gets reloaded
												// after WM_SETFOCUS gets called. Returns NULL. pParam should be a callback function
												// pointer (see GInputOnSettingsReloadCallback)
												// Cannot be recursive - the callbacks registered from within a callback will be ignored
												// Callable from SendEvent: YES
												// Callable from SendConstEvent: NO

	GINPUT_EVENT_FETCH_SETTINGS			= 3,	// DEPRECATED EVENT, DO NOT USE

	GINPUT_EVENT_FETCH_CHEAT_STRING		= 4,	// Copies the current cheat string to a char array sent via pParam.
												// pParam should point to an array of at least GINPUT_CHEAT_STRING_LENGTH characters
												// or a buffer overflow will occur.
												// Returns a value of type BOOL - when set to TRUE, cheat string has been changed since
												// the last frame - useful for determining when to re-check for a custom cheat combo
												// Letters from a cheat string are assigned to buttons as follows:
												// U - DPad Up, D - DPad Down, L - DPad Left, R - DPad Right
												// T - Y/Triangle, C - B/Circle, X - A/Cross, S - X/Square
												// 1 - L1/LB, 2 - L2/LT, 3 - R1/RB, 4 - R2/RT
												// Callable from SendEvent: YES
												// Callable from SendConstEvent: YES

	GINPUT_EVENT_RELOAD_WEAPON			= 5,	// Reloads the weapon player is holding
												// Supported only in SA
												// Callable from SendEvent: YES
												// Callable from SendConstEvent: NO

	GINPUT_EVENT_FETCH_GENERAL_SETTINGS	= 6,	// Fetches a structure filled with GInput general settings
												// pParam should point to a GINPUT_GENERAL_SETTINGS structure
												// Structure's cbSize field should equal sizeof(GINPUT_GENERAL_SETTINGS)
												// Callable from SendEvent: YES
												// Callable from SendConstEvent: YES

	GINPUT_EVENT_FETCH_PAD_SETTINGS		= 7,	// Fetches a structure filled with GInput ped-specific settings
												// pParam should point to a GINPUT_PAD_SETTINGS structure
												// Structure's cbSize field should equal sizeof(GINPUT_PAD_SETTINGS)
												// Callable from SendEvent: YES
												// Callable from SendConstEvent: YES

	GINPUT_EVENT_FETCH_SIXAXIS_INPUT	= 8,	// Fetches a structure filled with last frame's Sixaxis input
												// pParam should point to a SIXAXIS_INPUT structure
												// If Sixaxis is not supported, this struct will always contain zeroes
												// Callable from SendEvent: YES
												// Callable from SendConstEvent: YES

	GINPUT_EVENT_REGISTER_SIXAXIS_FETCH_CALLBACK = 9, // Registers a callback function which gets called after Sixaxis sends input
												// pParam should be a callback function pointer (see GInputOnSixaxisFetchCallback)
												// Cannot be recursive - the callbacks registered from within a callback will be ignored
												// Callable from SendEvent: YES
												// Callable from SendConstEvent: NO

	GINPUT_EVENT_IS_USING_SCP_DRIVER_CAPABILITIES = 10, // Checks if the game uses the pressure sensitive buttons feature, exclusive to
												// SCP Driver Package.
												// Returns BOOL - if TRUE, both SCP's xinput1_3.dll and a controller with pressure sensitive
												// face buttons (ie. DualShock 3) is used, FALSE otherwise.
												// Can be also used to determine whether Sixaxis is available or not
												// Callable from SendEvent: YES
												// Callable from SendConstEvent: YES

	GINPUT_EVENT_OVERRIDE_SIXAXIS_SETTINGS = 11, // Overrides all Sixaxis toggles.
												// if pParam is set to TRUE, the override gets enabled. If pParam is set to FALSE,
												// the override gets disabled.
												// Returns NULL all the time
												// Callable from SendEvent: YES
												// Callable from SendConstEvent: NO

	NUM_GINPUT_EVENTS
};

struct SIXAXIS_INPUT
{
	short	ACCEL_X;
	short	ACCEL_Y;
	short	ACCEL_Z;

	short	GYRO;
};

// Those are options from [GInput] section
// The options not present in the particular game are always 0
struct GINPUT_GENERAL_SETTINGS
{
	DWORD		cbSize;		// Fill with sizeof(GINPUT_GENERAL_SETTINGS)

	bool		DisableOnFocusLost : 1;
	bool		Vibration : 1;
	bool		CheatsFromPad : 1;
	bool		GuideLaunchesOverlay : 1;
	bool		ApplyMissionSpecificFixes : 1;
	bool		ApplyGXTFixes : 1;
	bool		PlayStationButtons : 1;
	bool		MapPadOneToPadTwo : 1;
	bool		FreeAim : 1;

};

// Those are options from [PadX] section
// The options not present in the particular game are always 0
struct GINPUT_PAD_SETTINGS
{
	DWORD		cbSize;		// Fill with sizeof(GINPUT_PAD_SETTINGS)

	BYTE		ControlsSet	: 4;
	bool		Southpaw : 1;
	bool		SAStyleSniperZoom : 1;
	bool		SwapSticksDuringAiming : 1;
	bool		DrivebyWithAnalog : 1;
	bool		HotkeyToDriveby : 1;
	bool		InvertLook : 1;

	bool		InvertLeftXAxis : 1;
	bool		InvertLeftYAxis : 1;
	bool		SwapLeftAxes : 1;
	float		LeftStickDeadzone;
	float		LeftStickSensitivity;

	bool		InvertRightXAxis : 1;
	bool		InvertRightYAxis : 1;
	bool		SwapRightAxes : 1;
	float		RightStickDeadzone;
	float		RightStickSensitivity;

	float		FaceButtonsSensitivity;
	float		SixaxisSensitivity;
	bool		SixaxisReloading : 1;
	bool		SixaxisCarSteering : 1;
	bool		SixaxisBikeSteering : 1;
	bool		SixaxisBoatSteering : 1;
	bool		SixaxisHeliSteering : 1;
	bool		SixaxisPlaneSteering : 1;
	bool		SixaxisHydraulics : 1;
};

// The size of a char array returned from GINPUT_FETCH_CHEAT_STRING
#define GINPUT_CHEAT_STRING_LENGTH		12

// Callback called on INI file reload
// Note - it is NOT called the first time INIs are loaded
typedef void (*GInputOnSettingsReloadCallback)();

// Callback called when Sixaxis data gets fetched and processed by GInput
// Gets called ONLY on input change
typedef void (CALLBACK *GInputOnSixaxisFetchCallback)(const SIXAXIS_INPUT&);



// Internal declarations
#ifndef GINPUT_COMPILE_CROSSCOMPATIBLE_VERSION
#if defined GINPUT_COMPILE_III_VERSION
#define GINPUT_FILENAME "GInputIII.asi"
#elif defined GINPUT_COMPILE_VC_VERSION
#define GINPUT_FILENAME "GInputVC.asi"
#elif defined GINPUT_COMPILE_SA_VERSION
#define GINPUT_FILENAME "GInputSA.asi"
#endif
#endif

#define GINPUT_MODVERSION 0x00010B00	// 1.11

// The pad interface
// The interface is safe to use without validating a pointer - in case of GInput loading failure,
// these functions are set to return false all the time
// NOTE: Do not use any of these functions before GInput_Load is called on your interface pointer!
class IGInputPad
{
protected:
	virtual					~IGInputPad() { };

public:
	// Returns true when XInput compatible pad is connected
	virtual bool			IsPadConnected() const =0;

	// Returns true when last input came from a pad, false otherwise
	virtual bool			HasPadInHands() const =0;

	// Returns installed GInput version (see GINPUT_MODVERSION), -1 on failure
	virtual int				GetVersion() const =0;

	// Sends an event to GInput, allowing the script to take advantage of GInput features not available
	// through CPad or other original GTA functions
	// See eGInputEvent enum for supported events and their params/return values (if any)
	virtual void*			SendEvent(eGInputEvent eEvent, void* pParam)=0;
	virtual void*			SendConstEvent(eGInputEvent eEvent, void* pParam) const =0;
};

// GInput management functions

// Use one of those functions ONCE to initialise GInput API
// DO NOT use both functions in the same plugin!
// DO NOT CALL THOSE IN DLLMAIN, SINCE GINPUT MIGHT NOT BE INITIALIZED YET AT THE TIME YOUR PLUGIN LOADS
// Takes a pointer to pad interface pointer as an argument, returns true if succeed and false otherwise
// (GInput not installed or any other error occured)
bool GInput_Load(IGInputPad** pInterfacePtr);
// Takes a pointer to an array of TWO interface pointers as an argument, returns true if succeed and false otherwise
// (GInput not installed or any other error occured)
// In SA, it fetches both pads. In III/VC, the second fetched pad will ALWAYS be a dummy pad
bool GInput_Load_TwoPads(IGInputPad** pInterfacePtr);

// Releases GInput API
// Call it when your program terminates
void GInput_Release();


// Management functions definitions - internal use only, do not change anything here
#include <windows.h>

// GInput ASI handle
inline HMODULE* _GInput_HandlePtr()
{
	// Making it a static variable outside of the function would duplicate it for each .cpp file which uses any API funcs (bad)
	static HMODULE		hGInputHandle = NULL;
	return &hGInputHandle;
}

// Although these functions may not be inlined, we need to declare them as so
inline IGInputPad* _GInput_SafeMode()
{
	static class CDummyPad : public IGInputPad
	{
	public:
		virtual bool			IsPadConnected() const { return false; };
		virtual bool			HasPadInHands() const { return false; };
		virtual int				GetVersion() const { return -1; };
		virtual void*			SendEvent(eGInputEvent eEvent, void* pParam) { UNREFERENCED_PARAMETER(eEvent); UNREFERENCED_PARAMETER(pParam); return NULL; };
		virtual void*			SendConstEvent(eGInputEvent eEvent, void* pParam) const { UNREFERENCED_PARAMETER(eEvent); UNREFERENCED_PARAMETER(pParam); return NULL; };
	} DummyClass;
	return &DummyClass;
}

inline bool GInput_Load(IGInputPad** pInterfacePtr)
{
	static IGInputPad*	pCopiedPtr = NULL;		// We keep a backup of the interface pointer in case user calls GInput_Load multiple times
	static bool			bLoadingResult = false;	// Loading result is also cached so GInput_Load always returns the same value when called multiple times

	// Have we attempted to load GInput already? If so, just return a valid interface pointer and return
	// The pointer can be either a GInput interface or a dummy, 'safe-mode' interface which got initialised
	// due to GInput*.asi loading failure
	if ( pCopiedPtr != NULL )
	{
		*pInterfacePtr = pCopiedPtr;
		return bLoadingResult;
	}
	HMODULE			hHandle;

	// Cross compatible version?
#ifdef GINPUT_COMPILE_CROSSCOMPATIBLE_VERSION
	if ( GetModuleHandleEx(0, "GInputIII.asi", &hHandle) == 0 )
	{
		if ( GetModuleHandleEx(0, "GInputVC.asi", &hHandle) == 0 )
		{
			if ( GetModuleHandleEx(0, "GInputSA.asi", &hHandle) == 0 )
			{
				*pInterfacePtr = pCopiedPtr = _GInput_SafeMode();
				bLoadingResult = false;
				return false;
			}
		}
	}
#else
	// If not, compile non-cross compatible code
	if ( GetModuleHandleEx(0, GINPUT_FILENAME, &hHandle) == 0 )
	{
		// Failed? The mod is probably not installed (or Init is called from DllMain), so let's jump into 'safe-mode' and initialise
		// a dummy interface
		*pInterfacePtr = pCopiedPtr = _GInput_SafeMode();
		bLoadingResult = false;
		return false;
	}
#endif
	// Let's call a GInput export to get the proper interface
	IGInputPad*		(*ExportFunc)() = (IGInputPad*(*)())GetProcAddress(hHandle, (LPCSTR)1);

	// Just to make sure function pointer is valid (will not be valid if GInput 1.0 or 1.01 is used)
	if ( ExportFunc == NULL )
	{
		// Probably too old GInput version, no API support yet (applies only to GInputVC, though)
		*pInterfacePtr = pCopiedPtr = _GInput_SafeMode();
		bLoadingResult = false;
		return false;
	}

	*pInterfacePtr = pCopiedPtr = ExportFunc();
	if ( pCopiedPtr != NULL )
	{
		*_GInput_HandlePtr() = hHandle;
		bLoadingResult = true;
		return true;
	}
	else
	{
		// GInput loaded, but for some reason there's no valid interface pointer - let's do the same safe-mode trick
		*pInterfacePtr = pCopiedPtr = _GInput_SafeMode();
		bLoadingResult = false;
		return false;
	}
}

inline bool GInput_Load_TwoPads(IGInputPad** pInterfacePtr)
{
	static IGInputPad*	pCopiedPtr[2];			// We keep a backup of the interface pointer in case user calls GInput_Load multiple times
	static bool			bLoadingResult = false;	// Loading result is also cached so GInput_Load always returns the same value when called multiple times

	// Have we attempted to load GInput already? If so, just return a valid interface pointer and return
	// The pointer can be either a GInput interface or a dummy, 'safe-mode' interface which got initialised
	// due to GInput*.asi loading failure
	if ( pCopiedPtr[0] != NULL && pCopiedPtr[1] != NULL )
	{
		pInterfacePtr[0] = pCopiedPtr[0];
		pInterfacePtr[1] = pCopiedPtr[1];
		return bLoadingResult;
	}

	// Cross compatible version?
#ifdef GINPUT_COMPILE_CROSSCOMPATIBLE_VERSION
	HMODULE			hHandle;

	if ( GetModuleHandleEx(0, "GInputIII.asi", &hHandle) == 0 )
	{
		if ( GetModuleHandleEx(0, "GInputVC.asi", &hHandle) == 0 )
		{
			if ( GetModuleHandleEx(0, "GInputSA.asi", &hHandle) == 0 )
			{
				pInterfacePtr[0] = pCopiedPtr[0] = _GInput_SafeMode();
				pInterfacePtr[1] = pCopiedPtr[1] = _GInput_SafeMode();
				bLoadingResult = false;
				return false;
			}
		}
	}
#elif defined GINPUT_COMPILE_SA_VERSION
	HMODULE			hHandle;

	// If not, compile non-cross compatible code
	if ( GetModuleHandleEx(0, GINPUT_FILENAME, &hHandle) == 0 )
	{
		// Failed? The mod is probably not installed (or Init is called from DllMain), so let's jump into 'safe-mode' and initialise
		// a dummy interface
		pInterfacePtr[0] = pCopiedPtr[0] = _GInput_SafeMode();
		pInterfacePtr[1] = pCopiedPtr[1] = _GInput_SafeMode();
		bLoadingResult = false;
		return false;
	}
	// Let's call a GInput export to get the proper interface
	IGInputPad**	(*ExportFunc)() = (IGInputPad**(*)())GetProcAddress(hHandle, (LPCSTR)2);

	// Just to make sure function pointer is valid (will not be valid if GInput 1.0 or 1.01 is used)
	if ( ExportFunc == NULL )
	{
		pInterfacePtr[0] = pCopiedPtr[0] = _GInput_SafeMode();
		pInterfacePtr[1] = pCopiedPtr[1] = _GInput_SafeMode();
		bLoadingResult = false;
		return false;
	}

	IGInputPad**	pTempPtr = ExportFunc();
	pInterfacePtr[0] = pCopiedPtr[0] = pTempPtr[0];
	pInterfacePtr[1] = pCopiedPtr[1] = pTempPtr[1];
	if ( pCopiedPtr[0] != NULL && pCopiedPtr[1] != NULL )
	{
		*_GInput_HandlePtr() = hHandle;
		bLoadingResult = true;
		return true;
	}
	else
	{
		// GInput loaded, but for some reason there's no valid interface pointer - let's do the same safe-mode trick
		pInterfacePtr[0] = pCopiedPtr[0] = _GInput_SafeMode();
		pInterfacePtr[1] = pCopiedPtr[1] = _GInput_SafeMode();
		bLoadingResult = false;
		return false;
	}
#else
	bLoadingResult = false;
	return false;
#endif
}

inline void GInput_Release()
{
	// Just release the ASI
	HMODULE*	pHandle = _GInput_HandlePtr();
	if ( *pHandle )
		FreeLibrary(*pHandle);
}

#endif