project(tlow_prog)

file(COPY sprites.png DESTINATION ".")
file(COPY cursor.png DESTINATION ".")

# All sources that also need to be tested in unit tests go into a static library
add_library(tlow_lib STATIC game.cpp)
target_include_directories(tlow_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(tlow_lib PUBLIC cxx_std_17)
target_link_libraries(tlow_lib PRIVATE pge_lib)

# The main program
add_executable(tlow main.cpp)
target_link_libraries(tlow PRIVATE
        #tbb
        pge_lib
        tlow_lib)
