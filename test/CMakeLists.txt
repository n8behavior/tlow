project(tlow_tests)

# The test program
add_executable(tests testmain.cpp)
target_link_libraries(tests PRIVATE tlow_lib catch)
target_compile_features(tlow_lib PUBLIC cxx_std_17)
target_compile_definitions(tests PRIVATE CATCH_CONFIG_CONSOLE_WIDTH=60)
