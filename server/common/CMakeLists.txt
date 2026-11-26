add_library(w2pp_common STATIC
  Basedef.cpp
  BaseItem.cpp
  BaseCombat.cpp
  BaseLoaders.cpp
  CPSock.cpp
  ProtocolLayoutCheck.cpp
  Basedef.h
  BasedefInternal.h
  CPSock.h
  ItemEffect.h
)

target_include_directories(w2pp_common
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_compile_features(w2pp_common PUBLIC cxx_std_17)

target_compile_definitions(w2pp_common
  PUBLIC
    WIN32
    _WINDOWS
    _MBCS
    _CRT_SECURE_NO_WARNINGS
    _WINSOCK_DEPRECATED_NO_WARNINGS
    $<$<CONFIG:Debug>:_DEBUG;_PACKET_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

target_compile_options(w2pp_common
  PUBLIC
    $<$<CXX_COMPILER_ID:MSVC>:/W3 /source-charset:utf-8>
)

target_link_libraries(w2pp_common
  PUBLIC
    w2_shared
    ws2_32
    winmm
)
