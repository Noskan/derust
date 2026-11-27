cmake_minimum_required(VERSION 3.24)

# Os servidores usam POSIX (sockets, poll, glob, /proc/self/exe) e rodam só no Linux.
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
  message(FATAL_ERROR "DBSrv e TMSrv compilam só no Linux (DERUST_BUILD_SERVER=OFF nos outros sistemas).")
endif()

add_subdirectory(common)
add_subdirectory(dbsrv)
add_subdirectory(tmsrv)
