project (pge)

set(THREADS_PREFER_PTHREAD_FLAD ON)
find_package(Threads REQUIRED)

find_package(OpenGL REQUIRED)

if (UNIX)
    find_package(X11 REQUIRED)
    find_package(PNG REQUIRED)
    include_directories(${PNG_INCLUDE_DIR})
endif (UNIX)

if (WIN32)
    include_directories(${WinSDK})
endif (WIN32)

# Header only library, therefore INTERFACE
add_library(pge_lib INTERFACE)

# INTERFACE targets only have INTERFACE properties
target_include_directories(pge_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(pge_lib INTERFACE cxx_std_17)

target_link_libraries(pge_lib INTERFACE ${Executable}
    Threads::Threads
    OpenGL::OpenGL
    OpenGL::GL
    OpenGL::GLX
    )

if (UNIX)
    target_link_libraries(pge_lib INTERFACE ${Executable}
        ${X11_LIBRARIES}
        PNG::PNG
        )
endif (UNIX)
if (WIN32)
    target_link_libraries(pge_lib INTERFACE ${Executable}
        ${WinSDK}
        )
endif (WIN32)
