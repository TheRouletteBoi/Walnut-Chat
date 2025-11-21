-- premake5.lua
workspace "Walnut-Chat-Headless"
   location ("build/%{_ACTION}")
   configurations { "Debug", "Release", "Dist" }
   startproject "App-Server"

   -- Workspace-wide defines
   defines
   {
       "WL_HEADLESS"
   }

   -- Workspace-wide build options for MSVC
   filter "system:windows"
      architecture "x64"
      buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }
    
   filter "system:linux"
      architecture "x64"

   filter "system:macosx"
      architecture "ARM64"

-- Directories
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
WalnutNetworkingBinDir = "Walnut/Walnut-Modules/Walnut-Networking/vendor/GameNetworkingSockets/bin/%{cfg.system}/%{cfg.buildcfg}/"

include "Walnut/Build-Walnut-Headless-External.lua"

group "App"
    include "App-Client/Build-App-Client-Headless.lua"
    include "App-Common/Build-App-Common-Headless.lua"
    include "App-Server/Build-App-Server-Headless.lua"
group ""