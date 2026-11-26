# Cabeçalhos com a mesma definição no servidor e no cliente (shared/protocol). Só cabeçalhos: cada
# lado os inclui pelo próprio common/Basedef.h.
add_library(w2_shared INTERFACE)

target_include_directories(w2_shared
  INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(w2_shared
  INTERFACE
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
    FILES protocol/GameStructs.h
)
