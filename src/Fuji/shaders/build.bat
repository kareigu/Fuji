@echo off
set GLSLC_PATH="W:\Dev\VulkanSDK\Bin\glslc.exe"

setlocal enableDelayedExpansion

for /r %%f in (.\*.glsl) do (
  SETLOCAL
  set _SHADER_FILENAME=%%f
  set _OUTPUT=!_SHADER_FILENAME:.glsl=.spv!
  echo Compiling !_SHADER_FILENAME! to !_OUTPUT!
  %GLSLC_PATH% %%f -o !_OUTPUT!
)
PAUSE
