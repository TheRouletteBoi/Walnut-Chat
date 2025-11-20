-- premake5.lua
workspace "Walnut-Chat"
   location ("build/%{_ACTION}")
   cppdialect "C++23"
   configurations { "Debug", "Release", "Dist" }
   startproject "WalnutApp"

   -- Workspace-wide build options for MSVC
   filter "system:windows"
      architecture "x64"
      buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }

-- Directories
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
WalnutNetworkingBinDir = "Walnut/Walnut-Modules/Walnut-Networking/vendor/GameNetworkingSockets/bin/%{cfg.system}/%{cfg.buildcfg}/"

include "Walnut/Build-Walnut-External.lua"

group "App"
    include "App-Common/Build-App-Common.lua"
    include "App-Client/Build-App-Client.lua"
    include "App-Server/Build-App-Server.lua"
group ""