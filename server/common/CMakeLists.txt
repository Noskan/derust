add_library(w2pp_common STATIC
  Basedef.cpp
  BaseItem.cpp
  BaseCombat.cpp
  BaseLoaders.cpp
  CPSock.cpp
  EventLoop.cpp
  Platform.cpp
  System.cpp
  ProtocolLayoutCheck.cpp
  RateLimit.cpp
  Metrics.cpp
  Basedef.h
  BasedefInternal.h
  CPSock.h
  EventLoop.h
  ItemEffect.h
  Platform.h
  RateLimit.h
  Metrics.h
  System.h
)

target_include_directories(w2pp_common
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_compile_features(w2pp_common PUBLIC cxx_std_17)

target_link_libraries(w2pp_common
  PUBLIC
    w2_shared
)

target_compile_definitions(w2pp_common
  PUBLIC
    $<$<CONFIG:Debug>:_DEBUG;_PACKET_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

target_compile_options(w2pp_common
  PUBLIC
    # O código veio do MSVC: conversões que o GCC só aceita com -fpermissive. No x86_64 o ponto
    # flutuante já é SSE2, como no MSVC Win32 de onde vêm os resultados de referência;
    # -ffp-contract=off impede o GCC de fundir multiplicação e soma, o que mudaria arredondamentos.
    $<$<CXX_COMPILER_ID:GNU>:-fpermissive -Wno-write-strings -Wno-narrowing -Wno-multichar -ffp-contract=off>
)
