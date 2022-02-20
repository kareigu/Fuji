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
   cppdialect "C++20"
   targetdir "build/bin/%{cfg.buildcfg}"
   objdir "build/obj/%{cfg.buildcfg}"

   files { source_files }

   includedirs {
     "vendor/fmt/include"
   }

   libdirs { "vendor" }

   links {
      "fmt"
   }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "RELEASE" }
      optimize "On"