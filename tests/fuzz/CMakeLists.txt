# Alvos de fuzzing (libFuzzer). Só com clang, num build próprio com cobertura e sanitizers:
#   cmake -S . -B build/fuzz -DCMAKE_CXX_COMPILER=clang++ -DDERUST_BUILD_FUZZERS=ON \
#     -DCMAKE_CXX_FLAGS="-fsanitize=fuzzer-no-link,address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer" \
#     -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined"
# Quem roda é tools/fuzz_tmsrv.py, que monta a árvore sintética que o fuzz_client_message lê.
if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  message(FATAL_ERROR "DERUST_BUILD_FUZZERS precisa do clang (libFuzzer).")
endif()

add_executable(fuzz_sock_read fuzz_sock_read.cpp)
target_link_libraries(fuzz_sock_read PRIVATE w2pp_common)
target_link_options(fuzz_sock_read PRIVATE -fsanitize=fuzzer)

add_executable(fuzz_client_message fuzz_client_message.cpp)
target_link_libraries(fuzz_client_message PRIVATE tmsrv_core)
target_link_options(fuzz_client_message PRIVATE -fsanitize=fuzzer)
