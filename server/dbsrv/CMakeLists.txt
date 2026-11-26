set(DBSRV_SOURCES
  core/CReadFiles.cpp
  core/CReadFiles.h
  core/Server.cpp
  core/Server.h
  core/stdafx.h
  net/CUser.cpp
  net/CUser.h
  persistence/CFileDB.cpp
  persistence/CFileDB.h
  persistence/CRanking.cpp
  persistence/CRanking.h
  resources/DBSrv.rc
  resources/resource.h
)

add_executable(DBSrv WIN32 ${DBSRV_SOURCES})

target_include_directories(DBSrv
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(DBSrv PRIVATE w2pp_common)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${DBSRV_SOURCES})
