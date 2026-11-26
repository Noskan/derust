cmake_minimum_required(VERSION 3.24)

add_subdirectory(common)
add_subdirectory(dbsrv)
add_subdirectory(tmsrv)

if(W2PP_BUILD_CLIENT_PATCH)
  add_subdirectory(tools/client_patch)
endif()
