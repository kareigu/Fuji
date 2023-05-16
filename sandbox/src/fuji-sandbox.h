#pragma once


#ifdef _WIN32
  #define FUJI_SANDBOX_EXPORT __declspec(dllexport)
#else
  #define FUJI_SANDBOX_EXPORT
#endif

FUJI_SANDBOX_EXPORT void fuji_sandbox();
