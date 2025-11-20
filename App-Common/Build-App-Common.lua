project "App-Common"
   kind "StaticLib"
   language "C++"
   cppdialect "C++23"
   targetdir "build/bin/%{cfg.buildcfg}"
   staticruntime "off"

   files { "Source/**.h", "Source/**.cpp" }

   includedirs
   {
      "../Walnut/vendor/imgui",
      "../Walnut/vendor/glfw/include",
      "../Walnut/vendor/glm",

      "../Walnut/Walnut/Source",
      "../Walnut-Networking/Source",

      "%{IncludeDir.VulkanSDK}",
      "../Walnut/vendor/spdlog/include",

      "../Walnut-Networking/vendor/GameNetworkingSockets/include"
   }

   links
   {
       "Walnut",
       "Walnut-Networking",
   }

   targetdir ("../build/bin/" .. outputdir .. "/%{prj.name}")
   objdir ("../build/bin-int/" .. outputdir .. "/%{prj.name}")

   filter "system:windows"
      systemversion "latest"
      defines { "WL_PLATFORM_WINDOWS" }

   filter "system:macosx"
      defines { "WL_PLATFORM_MACOS" }

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
      defines { "WL_DIST" }
      runtime "Release"
      optimize "On"
      symbols "Off"