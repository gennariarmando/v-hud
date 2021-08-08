workspace "v-hud"
	configurations { "Release", "Debug" }
	location "project_files"
   
project "v-hud"
	files {
		"source/**.*",
	}
	
	includedirs { 
		"source/**",
	}
	
	includedirs {
		"$(PLUGIN_SDK_DIR)/shared/",
		"$(PLUGIN_SDK_DIR)/shared/game/",
		"$(PLUGIN_SDK_DIR)/plugin_sa/",
		"$(PLUGIN_SDK_DIR)/plugin_sa/game_sa/",
		"$(DXSDK_DIR)/Include/"
	}
	
	libdirs { 
		"$(PLUGIN_SDK_DIR)/output/lib/",
		"$(DXSDK_DIR)/Lib/x86"
	}
	
	
	kind "SharedLib"
	language "C++"
	targetdir "output/asi/"
	objdir ("output/obj")
	targetextension ".asi"
	characterset ("MBCS")
	linkoptions "/SAFESEH:NO"
	buildoptions { "-std:c++latest", "/permissive" }
	defines { "_CRT_SECURE_NO_WARNINGS", "_CRT_NON_CONFORMING_SWPRINTFS", "_USE_MATH_DEFINES" }
	disablewarnings { "4244", "4800", "4305", "4073", "4838", "4996", "4221", "4430", "26812", "26495", "6031" }
	defines { "GTASA", "PLUGIN_SGV_10US", "_DX9_SDK_INSTALLED" }

	filter "configurations:Debug"		
		links { "plugin_d" }
		targetname "VHud"
		defines { "DEBUG" }
		symbols "on"
		staticruntime "on"
		debugdir "$(GTA_SA_DIR)"
		debugcommand "$(GTA_SA_DIR)/gta-sa.exe"
		postbuildcommands "copy /y \"$(TargetPath)\" \"$(GTA_SA_DIR)\\scripts\\VHud.asi\""

	filter "configurations:Release"
		links { "plugin" }
		targetname "VHud"
		defines { "NDEBUG" }
		symbols "off"
		optimize "On"
		staticruntime "on"
		debugdir "$(GTA_SA_DIR)"
		debugcommand "$(GTA_SA_DIR)/gta-sa.exe"
		postbuildcommands "copy /y \"$(TargetPath)\" \"$(GTA_SA_DIR)\\scripts\\VHud.asi\""
