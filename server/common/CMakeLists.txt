add_library(w2pp_common STATIC
  Basedef.cpp
  BaseItem.cpp
  BaseCombat.cpp
  BaseLoaders.cpp
  CPSock.cpp
  Platform.cpp
  System.cpp
  ProtocolLayoutCheck.cpp
  Basedef.h
  BasedefInternal.h
  CPSock.h
  ItemEffect.h
  Platform.h
  System.h
)

target_include_directories(w2pp_common
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_compile_features(w2pp_common PUBLIC cxx_std_17)

if(WIN32)
  target_compile_definitions(w2pp_common
    PUBLIC
      WIN32
      _WINDOWS
      _MBCS
      _CRT_SECURE_NO_WARNINGS
      _WINSOCK_DEPRECATED_NO_WARNINGS
  )
  target_link_libraries(w2pp_common
    PUBLIC
      w2_shared
      ws2_32
      winmm
  )
else()
  # Linux: o subconjunto Win32 que os servidores usam, emulado sobre POSIX (server/compat). O
  # diretório posix/ tem os Windows.h, io.h, winsock2.h... que as fontes incluem.
  target_sources(w2pp_common PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../compat/w2_win32.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../compat/w2_win32.h
  )
  target_include_directories(w2pp_common BEFORE
    PUBLIC
      ${CMAKE_CURRENT_SOURCE_DIR}/../compat/posix
  )
  target_link_libraries(w2pp_common
    PUBLIC
      w2_shared
  )
endif()

target_compile_definitions(w2pp_common
  PUBLIC
    $<$<CONFIG:Debug>:_DEBUG;_PACKET_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

target_compile_options(w2pp_common
  PUBLIC
    $<$<CXX_COMPILER_ID:MSVC>:/W3 /source-charset:utf-8>
    # O código vem do MSVC: conversões que o GCC só aceita com -fpermissive (como na checagem
    # mingw, tools/mingw_check.py). No x86_64 o ponto flutuante já é SSE2, como no MSVC Win32;
    # -ffp-contract=off impede o GCC de fundir multiplicação e soma, o que mudaria arredondamentos.
    $<$<CXX_COMPILER_ID:GNU>:-fpermissive -Wno-write-strings -Wno-narrowing -Wno-multichar -ffp-contract=off>
)
