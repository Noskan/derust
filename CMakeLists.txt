cmake_minimum_required(VERSION 3.24)

project(derust LANGUAGES CXX)

option(DERUST_BUILD_SERVER "Build W2PP server targets (DBSrv, TMSrv)" ON)
option(DERUST_BUILD_CLIENT "Build TMProject client target" ON)
option(W2PP_BUILD_CLIENT_PATCH "Build legacy ClientPatch_v7662 DLL" OFF)
option(DERUST_BUILD_TESTS "Build unit tests (CTest)" ON)

if(MSVC AND NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
  message(WARNING "derust targets Win32 (x86) for W2PP and TMProject DirectX 9 libs; current pointer size is ${CMAKE_SIZEOF_VOID_P} bytes.")
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(DERUST_BUILD_SERVER)
  add_subdirectory(server)
endif()

if(DERUST_BUILD_CLIENT)
  add_subdirectory(client)
endif()

if(DERUST_BUILD_TESTS AND DERUST_BUILD_SERVER)
  enable_testing()
  add_subdirectory(tests)
endif()
