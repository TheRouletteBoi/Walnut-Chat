project "App-Client"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++23"
   targetdir "build/bin/%{cfg.buildcfg}"
   staticruntime "off"

   files { "src/**.h", "src/**.cpp" }

   includedirs
   {
      "../App-Common/Source",

      "../Walnut/vendor/imgui",
      "../Walnut/vendor/glfw/include",
      "../Walnut/vendor/glm",

      "../Walnut/Walnut/Source",
      "../Walnut/Walnut/Platform/GUI",

      "%{IncludeDir.VulkanSDK}",
      "../Walnut/vendor/spdlog/include",
      "../Walnut/vendor/yaml-cpp/include",
      
      -- Walnut-Networking
      "../Walnut/Walnut-Modules/Walnut-Networking/Source",
      "../Walnut/Walnut-Modules/Walnut-Networking/vendor/GameNetworkingSockets/include"
   }

   links
   {
       "App-Common",
       "Walnut",
       "Walnut-Networking",
       "ImGui",
       "GLFW",
       "vulkan",
       "yaml-cpp",
   }

   	defines
	{
		"YAML_CPP_STATIC_DEFINE"
	}

   targetdir ("../build/bin/" .. outputdir .. "/%{prj.name}")
   objdir ("../build/bin-int/" .. outputdir .. "/%{prj.name}")

   filter "system:windows"
      systemversion "latest"
      defines { "WL_PLATFORM_WINDOWS" }

      postbuildcommands 
	  {
	    '{COPY} "../%{WalnutNetworkingBinDir}/GameNetworkingSockets.dll" "%{cfg.targetdir}"',
	    '{COPY} "../%{WalnutNetworkingBinDir}/libcrypto-3-x64.dll" "%{cfg.targetdir}"',
	    '{COPY} "../%{WalnutNetworkingBinDir}/libprotobufd.dll" "%{cfg.targetdir}"',
	  }

   filter "configurations:Debug"
      defines { "WL_DEBUG" }
      runtime "Debug"
      symbols "On"

   filter "configurations:Release"
      defines { "WL_RELEASE" }
      runtime "Release"
      optimize "On"
      symbols "On"

   filter "configurations:Dist"
      kind "WindowedApp"
      defines { "WL_DIST" }
      runtime "Release"
      optimize "On"
      symbols "Off"

   filter "system:macosx"
     defines { "WL_PLATFORM_MACOS" }

     linkoptions { 
        "-rpath @executable_path/.",
      }
      libdirs { "../Walnut/Walnut-Modules/Walnut-Networking/vendor/GameNetworkingSockets/bin/Macos" }

      links 
      {
            "Cocoa.framework",
            "IOKit.framework",
            "CoreFoundation.framework",
            "GameNetworkingSockets",
      }

      postbuildcommands
      {
          "{COPY} ../../Walnut/Walnut-Modules/Walnut-Networking/vendor/GameNetworkingSockets/bin/Macos/libGameNetworkingSockets.dylib %{cfg.targetdir}",
          "{COPY} ../../Walnut/vendor/MoltenVK/libMoltenVK.dylib %{cfg.targetdir}/libvulkan.1.dylib"
      }
   filter {}