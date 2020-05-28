project(tlow_prog)

# All sources that also need to be tested in unit tests go into a static library
add_library(tlow_lib STATIC actor.cpp actor.hpp)
target_include_directories(tlow_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(tlow_lib PUBLIC cxx_std_17)

# The main program
add_executable(tlow main.cpp)
target_link_libraries(tlow PRIVATE tlow_lib)
