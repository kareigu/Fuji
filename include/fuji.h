#pragma once


#ifdef _WIN32
  #define FUJI_EXPORT __declspec(dllexport)
#else
  #define FUJI_EXPORT
#endif

FUJI_EXPORT int fuji();
