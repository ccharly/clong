cmake_minimum_required(VERSION 3.1)

## Setup project
## ----------------------------------------------------------------------------
project(clong)
set(CLONG_ROOT ${CMAKE_SOURCE_DIR})
set(LLVM_ROOT /usr/local/opt/llvm)

## Setup library
## ----------------------------------------------------------------------------

## C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")

## Includes
include_directories(${CLONG_ROOT}/include ${LLVM_ROOT}/include)

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
link_directories(${LLVM_ROOT}/lib)

## Bin
add_executable(clong ${clong.src})
target_link_libraries(clong ${CLANG_LIBS} ${LLVM_LIBS} )

## Install
## ----------------------------------------------------------------------------
install(TARGETS clong RUNTIME DESTINATION bin)
