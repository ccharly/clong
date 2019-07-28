## Copyright Charly Chevalier
## ----------------------------------------------------------------------------

## CMake
## ----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.1)
include(ExternalProject)

## Setup project
## ----------------------------------------------------------------------------
project(clong)

## For convenience
set(CLONG_ROOT ${CMAKE_SOURCE_DIR})

## Directory containing third party packages
set(THIRD_PARTY_DIR ${CLONG_ROOT}/third_party)

## Temporary install dir for compiled third-party packages
set(BUILD_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/_install)

## Detect compilers
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
  set(IS_CLANG ON)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(IS_GCC ON)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set(IS_MSVC ON)
endif()

## Determine LLVM prefix (root) dir
if (NOT DEFINED LLVM_ROOT)
  if (WIN32)
    ## For travis
    if (IS_TRAVIS)
      set(LLVM_ROOT ${CLONG_ROOT}/llvm-clang-msvc)
    endif()
  elseif (APPLE)
    ## Assume LLVM has been installed using homebrew with standard location
    set(LLVM_ROOT /usr/local/opt/llvm)
  elseif (UNIX AND NOT APPLE)
    ## Use llvm-config to query LLVM's root
    execute_process(COMMAND llvm-config --prefix OUTPUT_VARIABLE LLVM_ROOT_RAW)
    ## Make sure to strip the \n at the end
    string(REGEX REPLACE "\n$" "" LLVM_ROOT "${LLVM_ROOT_RAW}")
  endif()
endif()

## Enable compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

## Logs
## ----------------------------------------------------------------------------
message(STATUS "LLVM_ROOT: ${LLVM_ROOT}")
message(STATUS "CLONG_ROOT: ${CLONG_ROOT}")
message(STATUS "THIRD_PARTY_DIR: ${THIRD_PARTY_DIR}")
message(STATUS "BUILD_INSTALL_DIR: ${BUILD_INSTALL_DIR}")

## Packages
## ----------------------------------------------------------------------------

## Look for a header-only package
function(find_third_party_header_only_package PREFIX HPP PKG)
  message(STATUS "Looking for header-only package: ${PKG}")
  set(PKG_DIR ${THIRD_PARTY_DIR}/${PKG})
  find_path(${PREFIX}_INCLUDE_DIR ${HPP} PATHS ${PKG_DIR} ${PKG_DIR}/include)
  if ((NOT ${PREFIX}_INCLUDE_DIR) OR (NOT EXISTS ${PKG_DIR}))
    ## Init submodule
    execute_process(COMMAND git submodule update --init -- ${PKG_DIR})
    set(${PREFIX}_INCLUDE_DIR ${PKG_DIR}
      CACHE PATH "${PKG} include directory")
  endif()
endfunction()

## Look for a package, build it and install it locally
function(find_third_party_package PREFIX DEP PKG)
  message(STATUS "Looking for package: ${PKG}")
  set(PKG_DIR ${THIRD_PARTY_DIR}/${PKG})
  set(PKG_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/_third_party/${PKG})
  find_path(${PREFIX}_FOUND ${HPP} PATHS ${BUILD_INSTALL_DIR})
  if ((NOT ${PREFIX}_FOUND) OR (NOT EXISTS ${PKG_DIR}))
    ## Init submodule and prepare for build
    execute_process(COMMAND git submodule update --init -- ${PKG_DIR})
    execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${PKG_BUILD_DIR})
    ## Forward platform on Windows (might wanna use something more specialized here than _IS_MSVC..)
    if (IS_MSVC)
      set(CMAKE_ARGS -A ${CMAKE_GENERATOR_PLATFORM})
    endif()
    ## Tell where to install and make install messages silent
    execute_process(COMMAND
      ${CMAKE_COMMAND} ${PKG_DIR}
        ${ARGN}
        -DCMAKE_INSTALL_PREFIX=${BUILD_INSTALL_DIR}
        -DCMAKE_INSTALL_MESSAGE=NEVER
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        ${CMAKE_ARGS}
      WORKING_DIRECTORY ${PKG_BUILD_DIR})
    ## Build and install
    execute_process(COMMAND
      ${CMAKE_COMMAND} --build ${PKG_BUILD_DIR} --config ${CMAKE_BUILD_TYPE})
    execute_process(COMMAND
      ${CMAKE_COMMAND} --build ${PKG_BUILD_DIR} --config ${CMAKE_BUILD_TYPE} --target install)
    set(${PREFIX}_FOUND TRUE
      CACHE PATH "${PKG} installed")
  endif()
endfunction()

## CI cases
if (IS_TRAVIS AND IS_MSVC)
  ## Use .dll for MSVC as we're using LLVM dynamically linked version!
  set(GTEST_CMAKE_ARGS -DBUILD_SHARED_LIBS=ON)
endif()

## Packages
find_third_party_header_only_package(FILESYSTEM ghc/filesystem.hpp filesystem)
find_third_party_header_only_package(FMT fmt/format.h fmt)
find_third_party_header_only_package(SPDLOG spdlog/spdlog.h spdlog)
find_third_party_package(GTEST lib/libgtest.h googletest -DBUILD_GMOCK=OFF ${GTEST_CMAKE_ARGS})

## Setup library
## ----------------------------------------------------------------------------

## C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
## Seems that those flags are required for GNU-like compilers
if (IS_CLANG OR IS_GCC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

## Includes
include_directories(
  ${CLONG_ROOT}/include
  ${LLVM_ROOT}/include
  ${FILESYSTEM_INCLUDE_DIR}
  ${FMT_INCLUDE_DIR}
  ${SPDLOG_INCLUDE_DIR}
  ${BUILD_INSTALL_DIR}/include
)

## Sources
set(clong.src ${CLONG_ROOT}/src/clong.cpp)
set(clong_test.src ${CLONG_ROOT}/tests/lib/clong_test.cpp)

## Link
set(CLANG_LIBS
  clangTooling
  clangDriver
  clangFrontend
  clangParse
  clangSerialization
  clangSema
  clangAnalysis
  clangEdit
  clangLex
  clangAST
  clangBasic
)
set(LLVM_LIBS
  LLVMOption
  LLVMProfileData
  LLVMMCParser
  LLVMMC
  LLVMBitReader
  LLVMCore
  LLVMBinaryFormat
  LLVMSupport
  LLVMDemangle
)
if (IS_MSVC)
  ## Required for those functions: GetFileVersionInfoSize and VerQueryValueW
  set(LLVM_DEP_LIBS
    version
  )
else()
  ## Those ones are only available on UNIX
  set(LLVM_DEP_LIBS
    curses
    xml2
    m
    z
    pthread ## Required for linux
  )
endif()
link_directories(
  ${LLVM_ROOT}/lib
  ${BUILD_INSTALL_DIR}/lib
)
## On Windows, .dll are located under the bin/ dir, so add it as LIBPATH:
if (IS_MSVC)
link_directories(
  ${BUILD_INSTALL_DIR}/bin
)
endif()

## Bin
add_executable(clong ${clong.src})
target_link_libraries(clong ${CLANG_LIBS} ${LLVM_LIBS} ${LLVM_DEP_LIBS})

## Log compilation flags
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")

## Tests
## ----------------------------------------------------------------------------
enable_testing()

## Create an executable as a test
function(add_test_executable target src)
  add_executable(${target} ${src})
  ## We don't want test to be part of the main target
  set_target_properties(${target} PROPERTIES EXCLUDE_FROM_ALL TRUE)
  ## Required by gtest
  if (NOT IS_MSVC)
    set(GTEST_DEP_LIBS pthread)
  endif()
  ## Link to gtest
  target_link_libraries(${target} clong_test gtest gtest_main ${GTEST_DEP_LIBS})
  ## Add it to the global `tests` target
  add_dependencies(tests ${target})
  ## Run the test when invoking the target
  add_test(NAME ${target} COMMAND $<TARGET_FILE:${target}>)
endfunction()

## Test lib
add_library(clong_test SHARED ${clong_test.src})
set_target_properties(clong_test PROPERTIES EXCLUDE_FROM_ALL TRUE)
target_link_libraries(clong_test ${CLANG_LIBS} ${LLVM_LIBS} ${LLVM_DEP_LIBS})
## Global test target
add_custom_target(tests)
## All tests
add_test_executable(tests.empty tests/empty.cpp)
add_test_executable(tests.multiline tests/multiline.cpp)
add_test_executable(tests.auto_register_parent tests/auto_register_parent.cpp)

## Install
## ----------------------------------------------------------------------------
install(TARGETS clong RUNTIME DESTINATION bin)
