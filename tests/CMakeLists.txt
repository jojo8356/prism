add_executable(test_prism test_prism.cpp)
target_link_libraries(test_prism PRIVATE prism::prism)
target_compile_options(test_prism PRIVATE -Wall -Wextra -Wpedantic -Werror)
add_test(NAME test_prism COMMAND test_prism)
