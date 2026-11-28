cmake_minimum_required(VERSION 3.24)

# Testes das regras do servidor (w2pp_common).
if(DERUST_BUILD_SERVER)
  add_executable(w2pp_common_tests
    w2pp_common_tests.cpp
  )

  target_link_libraries(w2pp_common_tests PRIVATE w2pp_common)

  add_test(NAME w2pp_common_tests COMMAND w2pp_common_tests)

  # Testes de caracterização das funções BASE_* (hashes de referência da tag pre-modularization).
  add_executable(w2pp_characterization_tests
    w2pp_characterization_tests.cpp
  )

  target_link_libraries(w2pp_characterization_tests PRIVATE w2pp_common)

  add_test(NAME w2pp_characterization_tests COMMAND w2pp_characterization_tests)

  # Layout do arquivo de conta, referência do codec do w2db (services/w2db/src/legacy.rs).
  add_executable(account_layout_dump
    account_layout_dump.cpp
  )

  target_link_libraries(account_layout_dump PRIVATE w2pp_common)

  add_test(NAME account_layout COMMAND account_layout_dump ${CMAKE_SOURCE_DIR}/shared/protocol/account_layout.txt)

  # Conversão arquivo de conta <-> proto do DBSrv contra o codec do w2db (os .pb das contas de teste,
  # que o teste golden_protos do Rust gera).
  if(TARGET w2db_cpp)
    add_executable(w2db_convert_tests
      w2db_convert_tests.cpp
    )

    target_link_libraries(w2db_convert_tests PRIVATE w2db_cpp)

    add_test(NAME w2db_convert COMMAND w2db_convert_tests ${CMAKE_SOURCE_DIR}/services/w2db/tests/data/account/D
      ${CMAKE_SOURCE_DIR}/services/w2db/tests/data/golden)
  endif()
endif()

# Testes de caracterização das funções BASE_* do cliente (TMProject). Compila só o que esses
# fontes usam (client/src/common/Base*.cpp e as variáveis globais de TMGlobal/TMLog), sem o
# restante do cliente.
if(DERUST_BUILD_CLIENT)
  set(_tm_src "${CMAKE_SOURCE_DIR}/client/src")
  add_executable(tmproject_characterization_tests
    tmproject_characterization_tests.cpp
    ${_tm_src}/common/Basedef.cpp
    ${_tm_src}/common/BaseItem.cpp
    ${_tm_src}/common/BaseCombat.cpp
    ${_tm_src}/common/BaseWorld.cpp
    ${_tm_src}/core/TMGlobal.cpp
    ${_tm_src}/core/TMLog.cpp
  )
  # Mesmo DirectX SDK legado e mesmas definições do TMProject (client/CMakeLists.txt).
  set_target_properties(tmproject_characterization_tests PROPERTIES
    VS_USER_PROPS "${CMAKE_BINARY_DIR}/client/tm_directx.props"
  )
  target_include_directories(tmproject_characterization_tests PRIVATE ${_tm_src})
  target_compile_features(tmproject_characterization_tests PRIVATE cxx_std_17)
  target_compile_definitions(tmproject_characterization_tests PRIVATE
    WIN32 _WINDOWS _MBCS _CRT_SECURE_NO_WARNINGS _WINSOCK_DEPRECATED_NO_WARNINGS
  )
  target_compile_options(tmproject_characterization_tests PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W3 /permissive->
  )
  target_link_libraries(tmproject_characterization_tests PRIVATE w2_shared tm_directx wininet)
  add_test(NAME tmproject_characterization_tests COMMAND tmproject_characterization_tests)

  # Cliente sintético do login e do jogo (tools/login_client), com o CPSock e as estruturas do cliente. Não
  # é um teste do CTest: precisa do DBSrv e do TMSrv no ar, e quem o roda é
  # tools/server_boot_smoke.py.
  add_executable(LoginClient
    ${CMAKE_SOURCE_DIR}/tools/login_client/LoginClient.cpp
    ${_tm_src}/net/CPSock.cpp
    ${_tm_src}/common/Basedef.cpp
    ${_tm_src}/common/BaseItem.cpp
    ${_tm_src}/common/BaseCombat.cpp
    ${_tm_src}/common/BaseWorld.cpp
    ${_tm_src}/core/TMGlobal.cpp
    ${_tm_src}/core/TMLog.cpp
  )
  set_target_properties(LoginClient PROPERTIES
    VS_USER_PROPS "${CMAKE_BINARY_DIR}/client/tm_directx.props"
  )
  target_include_directories(LoginClient PRIVATE ${_tm_src})
  target_compile_features(LoginClient PRIVATE cxx_std_17)
  target_compile_definitions(LoginClient PRIVATE
    WIN32 _WINDOWS _MBCS _CRT_SECURE_NO_WARNINGS _WINSOCK_DEPRECATED_NO_WARNINGS
  )
  target_compile_options(LoginClient PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W3 /permissive->
  )
  target_link_libraries(LoginClient PRIVATE w2_shared tm_directx wininet ws2_32 iphlpapi winmm)
endif()
