#pragma once

#ifdef _WIN32
  #define giggle_EXPORT __declspec(dllexport)
#else
  #define giggle_EXPORT
#endif

giggle_EXPORT void giggle();
