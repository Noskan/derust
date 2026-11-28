cmake_minimum_required(VERSION 3.24)

project(derust LANGUAGES CXX)

# Os servidores rodam só no Linux; o cliente (DirectX 9) só existe no Windows.
if(WIN32)
  set(_derust_server_default OFF)
  set(_derust_client_default ON)
else()
  set(_derust_server_default ON)
  set(_derust_client_default OFF)
endif()
option(DERUST_BUILD_SERVER "Build W2PP server targets (DBSrv, TMSrv; Linux only)" ${_derust_server_default})
option(DERUST_BUILD_CLIENT "Build TMProject client target" ${_derust_client_default})
option(W2PP_BUILD_CLIENT_PATCH "Build legacy ClientPatch_v7662 DLL" OFF)
option(DERUST_BUILD_TESTS "Build unit tests (CTest)" ON)
option(DERUST_BUILD_FUZZERS "Build libFuzzer targets in tests/fuzz (clang, Linux servers)" OFF)

if(MSVC AND NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
  message(WARNING "derust targets Win32 (x86) for W2PP and TMProject DirectX 9 libs; current pointer size is ${CMAKE_SIZEOF_VOID_P} bytes.")
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Cabeçalhos comuns aos dois lados (shared/protocol), usados pelo servidor e pelo cliente.
add_subdirectory(shared)

if(DERUST_BUILD_SERVER)
  add_subdirectory(server)
endif()

if(DERUST_BUILD_CLIENT)
  add_subdirectory(client)
endif()

# DLL do cliente (patch do v7662): não depende dos servidores.
if(W2PP_BUILD_CLIENT_PATCH)
  add_subdirectory(server/tools/client_patch)
endif()

if(DERUST_BUILD_TESTS AND (DERUST_BUILD_SERVER OR DERUST_BUILD_CLIENT))
  enable_testing()
  add_subdirectory(tests)
endif()

if(DERUST_BUILD_FUZZERS AND DERUST_BUILD_SERVER)
  add_subdirectory(tests/fuzz)
endif()
