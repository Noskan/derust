set(DBSRV_SOURCES
  core/CReadFiles.cpp
  core/CReadFiles.h
  core/Server.cpp
  core/Server.h
  core/Server_Admin.cpp
  core/Server_Config.cpp
  core/Server_Logs.cpp
  core/stdafx.h
  net/CUser.cpp
  net/CUser.h
  persistence/CFileDB.cpp
  persistence/CFileDB.h
  persistence/CFileDB_AccountPackets.cpp
  persistence/CFileDB_WorldPackets.cpp
  persistence/CRanking.cpp
  persistence/CRanking.h
  resources/DBSrv.rc
  resources/resource.h
)

add_executable(DBSrv WIN32 ${DBSRV_SOURCES})

# Fora do Windows, o main de server/compat chama o WinMain das fontes.
if(NOT WIN32)
  target_sources(DBSrv PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../compat/w2_main.cpp)
endif()

target_include_directories(DBSrv
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(DBSrv PRIVATE w2pp_common)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${DBSRV_SOURCES})
