cmake_minimum_required(VERSION 3.24)

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
