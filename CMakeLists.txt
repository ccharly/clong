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

## Determine LLVM prefix (root) dir
if (NOT DEFINED LLVM_ROOT)
  if (WIN32)
    ## Assume LLVM has been installed using chocolatey with standard location
    ## TODO
  elseif (APPLE)
    ## Assume LLVM has been installed using homebrew with standard location
    set(LLVM_ROOT /usr/local/opt/llvm)
  elseif (UNIX AND NOT APPLE)
    ## Use llvm-config to query LLVM's root
    execute_process(COMMAND llvm-config --prefix OUTPUT_VARIABLE LLVM_ROOT)
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
function(find_third_party_package PREFIX DEP PKG OPTS)
  message(STATUS "Looking for package: ${PKG}")
  set(PKG_DIR ${THIRD_PARTY_DIR}/${PKG})
  set(PKG_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/_third_party/${PKG})
  find_path(${PREFIX}_FOUND ${HPP} PATHS ${BUILD_INSTALL_DIR})
  if ((NOT ${PREFIX}_FOUND) OR (NOT EXISTS ${PKG_DIR}))
    ## Init submodule and prepare for build
    execute_process(COMMAND git submodule update --init -- ${PKG_DIR})
    execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${PKG_BUILD_DIR})
    ## Tell where to install and make install messages silent
    execute_process(COMMAND
      ${CMAKE_COMMAND} ${PKG_DIR}
        ${OPTS}
        -DCMAKE_INSTALL_PREFIX=${BUILD_INSTALL_DIR}
        -DCMAKE_INSTALL_MESSAGE=NEVER
      WORKING_DIRECTORY ${PKG_BUILD_DIR})
    ## Build and install
    execute_process(COMMAND ${CMAKE_COMMAND} --build ${PKG_BUILD_DIR})
    execute_process(COMMAND ${CMAKE_COMMAND} --build ${PKG_BUILD_DIR} --target install)
    set(${PREFIX}_FOUND TRUE
      CACHE PATH "${PKG} installed")
  endif()
endfunction()

## Packages
find_third_party_header_only_package(FILESYSTEM ghc/filesystem.hpp filesystem)
find_third_party_header_only_package(FMT fmt/format.h fmt)
find_third_party_package(GOOGLETEST lib/libgtest.h googletest -DBUILD_GMOCK=OFF)

## Setup library
## ----------------------------------------------------------------------------

## C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")


## Includes
include_directories(
  ${CLONG_ROOT}/include
  ${LLVM_ROOT}/include
  ${FILESYSTEM_INCLUDE_DIR}
  ${FMT_INCLUDE_DIR}
  ${BUILD_INSTALL_DIR}/include
)

## Sources
file(GLOB_RECURSE clong.src
  ${CLONG_ROOT}/src/*.cpp
)

## Link
set(CLANG_LIBS
  -lclangTooling
  -lclangDriver
  -lclangFrontend
  -lclangParse
  -lclangSerialization
  -lclangSema
  -lclangAnalysis
  -lclangEdit
  -lclangLex
  -lclangAST
  -lclangBasic
)
set(LLVM_LIBS
  -lLLVMLTO
  -lLLVMPasses
  -lLLVMObjCARCOpts
  -lLLVMMIRParser
  -lLLVMSymbolize
  -lLLVMDebugInfoPDB
  -lLLVMDebugInfoDWARF
  -lLLVMCoverage
  -lLLVMMCA
  -lLLVMTableGen
  -lLLVMDlltoolDriver
  -lLLVMXRay
  -lLLVMOrcJIT
  -lLLVMXCoreDisassembler
  -lLLVMXCoreCodeGen
  -lLLVMXCoreDesc
  -lLLVMXCoreInfo
  -lLLVMXCoreAsmPrinter
  -lLLVMWebAssemblyDisassembler
  -lLLVMWebAssemblyCodeGen
  -lLLVMWebAssemblyDesc
  -lLLVMWebAssemblyAsmPrinter
  -lLLVMWebAssemblyAsmParser
  -lLLVMWebAssemblyInfo
  -lLLVMSystemZDisassembler
  -lLLVMSystemZCodeGen
  -lLLVMSystemZAsmParser
  -lLLVMSystemZDesc
  -lLLVMSystemZInfo
  -lLLVMSystemZAsmPrinter
  -lLLVMSparcDisassembler
  -lLLVMSparcCodeGen
  -lLLVMSparcAsmParser
  -lLLVMSparcDesc
  -lLLVMSparcInfo
  -lLLVMSparcAsmPrinter
  -lLLVMPowerPCDisassembler
  -lLLVMPowerPCCodeGen
  -lLLVMPowerPCAsmParser
  -lLLVMPowerPCDesc
  -lLLVMPowerPCInfo
  -lLLVMPowerPCAsmPrinter
  -lLLVMNVPTXCodeGen
  -lLLVMNVPTXDesc
  -lLLVMNVPTXInfo
  -lLLVMNVPTXAsmPrinter
  -lLLVMMSP430Disassembler
  -lLLVMMSP430CodeGen
  -lLLVMMSP430AsmParser
  -lLLVMMSP430Desc
  -lLLVMMSP430Info
  -lLLVMMSP430AsmPrinter
  -lLLVMMipsDisassembler
  -lLLVMMipsCodeGen
  -lLLVMMipsAsmParser
  -lLLVMMipsDesc
  -lLLVMMipsInfo
  -lLLVMMipsAsmPrinter
  -lLLVMLanaiDisassembler
  -lLLVMLanaiCodeGen
  -lLLVMLanaiAsmParser
  -lLLVMLanaiDesc
  -lLLVMLanaiAsmPrinter
  -lLLVMLanaiInfo
  -lLLVMHexagonDisassembler
  -lLLVMHexagonCodeGen
  -lLLVMHexagonAsmParser
  -lLLVMHexagonDesc
  -lLLVMHexagonInfo
  -lLLVMBPFDisassembler
  -lLLVMBPFCodeGen
  -lLLVMBPFAsmParser
  -lLLVMBPFDesc
  -lLLVMBPFInfo
  -lLLVMBPFAsmPrinter
  -lLLVMARMDisassembler
  -lLLVMARMCodeGen
  -lLLVMARMAsmParser
  -lLLVMARMDesc
  -lLLVMARMInfo
  -lLLVMARMAsmPrinter
  -lLLVMARMUtils
  -lLLVMAMDGPUDisassembler
  -lLLVMAMDGPUCodeGen
  -lLLVMAMDGPUAsmParser
  -lLLVMAMDGPUDesc
  -lLLVMAMDGPUInfo
  -lLLVMAMDGPUAsmPrinter
  -lLLVMAMDGPUUtils
  -lLLVMAArch64Disassembler
  -lLLVMAArch64CodeGen
  -lLLVMAArch64AsmParser
  -lLLVMAArch64Desc
  -lLLVMAArch64Info
  -lLLVMAArch64AsmPrinter
  -lLLVMAArch64Utils
  -lLLVMObjectYAML
  -lLLVMLibDriver
  -lLLVMOption
  -lLLVMOptRemarks
  -lLLVMWindowsManifest
  -lLLVMTextAPI
  -lLLVMFuzzMutate
  -lLLVMX86Disassembler
  -lLLVMX86AsmParser
  -lLLVMX86CodeGen
  -lLLVMGlobalISel
  -lLLVMSelectionDAG
  -lLLVMAsmPrinter
  -lLLVMX86Desc
  -lLLVMMCDisassembler
  -lLLVMX86Info
  -lLLVMX86AsmPrinter
  -lLLVMX86Utils
  -lLLVMMCJIT
  -lLLVMLineEditor
  -lLLVMInterpreter
  -lLLVMExecutionEngine
  -lLLVMRuntimeDyld
  -lLLVMCodeGen
  -lLLVMTarget
  -lLLVMCoroutines
  -lLLVMipo
  -lLLVMInstrumentation
  -lLLVMVectorize
  -lLLVMScalarOpts
  -lLLVMLinker
  -lLLVMIRReader
  -lLLVMAsmParser
  -lLLVMInstCombine
  -lLLVMBitWriter
  -lLLVMAggressiveInstCombine
  -lLLVMTransformUtils
  -lLLVMAnalysis
  -lLLVMProfileData
  -lLLVMObject
  -lLLVMMCParser
  -lLLVMMC
  -lLLVMDebugInfoCodeView
  -lLLVMDebugInfoMSF
  -lLLVMBitReader
  -lLLVMCore
  -lLLVMBinaryFormat
  -lLLVMSupport
  -lLLVMDemangle
  -lcurses
  -lxml2
  -lm
  -lz
)
link_directories(
  ${LLVM_ROOT}/lib
  ${BUILD_INSTALL_DIR}/lib
)

## Bin
add_executable(clong ${clong.src})
target_link_libraries(clong ${CLANG_LIBS} ${LLVM_LIBS} )

## Tests
## ----------------------------------------------------------------------------
enable_testing()

## Create an executable as a test
function(add_test_executable target src)
  add_executable(${target} ${src})
  ## We don't want test to be part of the main target
  set_target_properties(${target} PROPERTIES EXCLUDE_FROM_ALL TRUE)
  ## Link to gtest
  target_link_libraries(${target} gtest gtest_main)
  ## Add it to the global `tests` target
  add_dependencies(tests ${target})
  ## Run the test when invoking the target
  add_test(NAME ${target} COMMAND $<TARGET_FILE:${target}>)
endfunction()

## Global test target
add_custom_target(tests)
## All tests
add_test_executable(tests.empty tests/empty.cpp)

## Install
## ----------------------------------------------------------------------------
install(TARGETS clong RUNTIME DESTINATION bin)
