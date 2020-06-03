project(tlow_tests)

find_package(Boost REQUIRED COMPONENTS serialization)

# The test program
add_executable(tests testmain.cpp)
target_link_libraries(tests PRIVATE
        catch
        tlow_lib
        pge_lib
        ${Boost_LIBRARIES}
        )
target_compile_features(tlow_lib PUBLIC cxx_std_17)
target_compile_definitions(tests PRIVATE CATCH_CONFIG_CONSOLE_WIDTH=60)
