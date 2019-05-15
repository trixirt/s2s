cmake_minimum_required(VERSION 3.4.3)

cmake_policy(SET CMP0048 NEW)
project(s2s)

find_package(LLVM CONFIG REQUIRED)
find_package(Clang CONFIG REQUIRED)
find_package(Lua51 REQUIRED)
find_package(Boost COMPONENTS system filesystem REQUIRED)

list(APPEND CMAKE_MODULE_PATH ${LLVM_DIR})
include(AddLLVM)
include(HandleLLVMOptions)

message(STATUS "S2S: configuring for LLVM ${LLVM_VERSION}...")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_definitions(${LLVM_DEFINITIONS})
include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${CLANG_INCLUDE_DIRS}
  ${LUA_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
  )

set(LLVM_LINK_COMPONENTS
  Option
  Support
  X86AsmParser
  X86Desc
  X86Info
  )

add_llvm_executable(s2s
  Configuration.cpp
  Process.cpp
  S2S.cpp
  Scripting.cpp
  Thread.cpp
  TempFile.cpp
  )

target_link_libraries(s2s
  PRIVATE
  clangBasic
  clangLex
  clangAST
  clangSema
  clangFrontend
  clangDriver
  clangTooling
  ${LUA_LIBRARIES}
  ${Boost_LIBRARIES}
  )

# Install programs
install(TARGETS s2s RUNTIME DESTINATION bin)
install(PROGRAMS DESTINATION bin)

