#ifndef CLONG_CONFIG_HPP
#define CLONG_CONFIG_HPP

// For assert
#include <cassert>

/// Current version
#define CLONG_VERSION "0.0.0"

/// True if compiling using MSVC
#define CLONG_IS_MSVC _MSC_VER && !__INTEL_COMPILER

/// True if compiling using Clang
#define CLONG_IS_CLANG defined(__clang__)

/// True if compiling using GCC
#define CLONG_IS_GCC defined(__GNUC__) || defined(__GNUG__)

/// Required to export symbols
#ifdef CLONG_IS_MSVC
  #define CLONG_DLLEXPORT __declspec(dllexport)
  #define CLONG_DLLIMPORT __declspec(dllimport)
#else
  #define CLONG_DLLEXPORT
  #define CLONG_DLLIMPORT
#endif

#endif
