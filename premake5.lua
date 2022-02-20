workspace "Application"
   configurations { "Debug", "Release" }
   architecture "x64"
   startproject "Application"
   source_files = {
	   "src/**.h", 
	   "src/**.hpp",
	   "src/**.cpp",
   }
   
   include "vendor/fmt/premake.lua"

project "Application"
   kind "ConsoleApp"
   language "C++"
   targetdir "build/bin/%{cfg.buildcfg}"
   objdir "build/obj/%{cfg.buildcfg}"

   files { source_files }

   includedirs {
	"src/",
     "vendor/fmt/include",
   }

   libdirs { "vendor" }

   links {
      "fmt"
   }
   
   filter "system:windows"
      cppdialect "C++20"

   filter "system:linux"
      cppdialect "gnu++2a"


   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "RELEASE" }
      optimize "On"
