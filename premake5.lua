workspace "Engine"
   configurations { "Debug", "Release" }
   architecture "x64"
   startproject "Engine"
   source_files = {
	   "src/**.h", 
	   "src/**.hpp",
	   "src/**.cpp",
   }
   
   include "vendor/fmt/premake.lua"
   include "vendor/glfw/premake5.lua"

project "Engine"
   kind "ConsoleApp"
   language "C++"
   targetdir "build/bin/%{cfg.buildcfg}"
   objdir "build/obj/%{cfg.buildcfg}"

   files { source_files }

   includedirs {
	"src/",
     "vendor/fmt/include",
	"vendor/glfw/include",
   }

   libdirs { "vendor" }

   links {
      "fmt",
	 "glfw"
   }
   
   filter "system:windows"
      cppdialect "C++17"

   filter "system:linux"
      cppdialect "gnu++17"


   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "RELEASE" }
      optimize "On"
