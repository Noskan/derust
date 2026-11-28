set(DBSRV_SOURCES
  core/CReadFiles.cpp
  core/CReadFiles.h
  core/Server.cpp
  core/Server.h
  core/Server_Admin.cpp
  core/Server_Config.cpp
  core/Server_Logs.cpp
  core/ServerMetrics.cpp
  core/ServerMetrics.h
  core/stdafx.h
  net/CUser.cpp
  net/CUser.h
  persistence/CFileDB.cpp
  persistence/CFileDB.h
  persistence/CFileDB_AccountPackets.cpp
  persistence/CFileDB_WorldPackets.cpp
  persistence/CRanking.cpp
  persistence/CRanking.h
  persistence/W2DbClient.h
)

add_executable(DBSrv ${DBSRV_SOURCES})

target_include_directories(DBSrv
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(DBSrv PRIVATE w2pp_common)

# Cliente do w2db (docs/W2DB.md): contas no Postgres por um serviço em Rust, via gRPC. Sem o gRPC
# de C++ (libgrpc++-dev, protobuf-compiler-grpc), o DBSrv compila só com os arquivos, e recusa
# subir se houver um w2db configurado.
option(DERUST_W2DB "DBSrv com o cliente do w2db (gRPC)" ON)
set(_w2db_proto "${CMAKE_SOURCE_DIR}/services/w2db/proto/w2db/v1/w2db.proto")
if(DERUST_W2DB)
  find_package(Protobuf QUIET)
  find_package(gRPC CONFIG QUIET)
  find_program(DERUST_GRPC_CPP_PLUGIN grpc_cpp_plugin)
  if(NOT (Protobuf_FOUND AND gRPC_FOUND AND DERUST_GRPC_CPP_PLUGIN AND Protobuf_PROTOC_EXECUTABLE))
    message(WARNING "gRPC de C++ não encontrado: DBSrv sem o w2db (DERUST_W2DB=OFF para calar o aviso).")
    set(DERUST_W2DB OFF)
  endif()
endif()

if(DERUST_W2DB)
  set(_w2db_gen "${CMAKE_CURRENT_BINARY_DIR}/w2db_gen")
  set(_w2db_out
    ${_w2db_gen}/w2db/v1/w2db.pb.cc
    ${_w2db_gen}/w2db/v1/w2db.pb.h
    ${_w2db_gen}/w2db/v1/w2db.grpc.pb.cc
    ${_w2db_gen}/w2db/v1/w2db.grpc.pb.h
  )
  add_custom_command(
    OUTPUT ${_w2db_out}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${_w2db_gen}
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
      --proto_path=${CMAKE_SOURCE_DIR}/services/w2db/proto
      --cpp_out=${_w2db_gen}
      --grpc_out=${_w2db_gen}
      --plugin=protoc-gen-grpc=${DERUST_GRPC_CPP_PLUGIN}
      ${_w2db_proto}
    DEPENDS ${_w2db_proto}
    COMMENT "Gerando o C++ do contrato do w2db"
    VERBATIM
  )

  # O código que o protoc gera não é nosso: os avisos dele (falsos positivos do gcc com ASan) ficam
  # de fora.
  set_source_files_properties(${_w2db_out} PROPERTIES COMPILE_OPTIONS "-w")

  # Código gerado e a conversão STRUCT_ACCOUNTFILE <-> proto (também usada pelos testes).
  add_library(w2db_cpp STATIC
    ${_w2db_out}
    persistence/W2DbConvert.cpp
    persistence/W2DbConvert.h
  )
  target_include_directories(w2db_cpp
    PUBLIC
      ${_w2db_gen}
      ${CMAKE_CURRENT_SOURCE_DIR}
  )
  target_link_libraries(w2db_cpp PUBLIC w2pp_common gRPC::grpc++ protobuf::libprotobuf)

  target_sources(DBSrv PRIVATE persistence/W2DbClient.cpp)
  target_link_libraries(DBSrv PRIVATE w2db_cpp)
else()
  target_sources(DBSrv PRIVATE persistence/W2DbClient_none.cpp)
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${DBSRV_SOURCES})
