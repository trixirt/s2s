cmake_minimum_required(VERSION 3.4.3)

cmake_policy(SET CMP0048 NEW)
project(s2s)

find_package(LLVM CONFIG REQUIRED)
find_package(Clang CONFIG REQUIRED)
find_package(Lua51 REQUIRED)
find_package(Boost COMPONENTS REQUIRED)

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

if( MSVC )
  # LLVM is built static by default
  # Logic copied from cmake wiki do a dynamic swap
  # https://gitlab.kitware.com/cmake/community/wikis/ \
  #   FAQ#how-can-i-build-my-msvc-application-with-a-static-runtime
  if (NOT DEFINED S2S_MSVC_STATIC OR S2S_MSVC_STATIC)
    foreach(flag_var
            CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
      if(${flag_var} MATCHES "/MD")
        string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
      endif(${flag_var} MATCHES "/MD")
    endforeach(flag_var)
  endif()

endif()

# Install programs
install(TARGETS s2s RUNTIME DESTINATION bin)
install(PROGRAMS DESTINATION bin)

