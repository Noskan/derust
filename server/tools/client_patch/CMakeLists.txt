add_library(ClientPatch_v7662 SHARED
  main.cpp
)

target_compile_definitions(ClientPatch_v7662
  PRIVATE
    WIN32
    _WINDOWS
    _MBCS
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

target_compile_options(ClientPatch_v7662
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W3>
)
