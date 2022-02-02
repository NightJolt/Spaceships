cmake_minimum_required(VERSION 3.22.0)

set(PROJECT_NAME Spaceships)
set(CMAKE_CXX_STANDARD 20)

if (UNIX)

    # NEEDS UPDATE!

    # project(${PROJECT_NAME} LANGUAGES CXX CUDA)

    # #find_package(Lua REQUIRED)
    # find_package(ImGui-SFML REQUIRED)

    # #set(CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES "/opt/cuda/bin/nvcc")
    # set(GCC_COVERAGE_COMPILE_FLAGS "-lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lsfml-network -lm -lGL -L/usr/X11R6/lib -lGLU -lGLEW -lXi -lXmu")
    # # -Ofast -pthread
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")

    # #include_directories(header)
    # #include_directories(source)

    # file (
    #         GLOB_RECURSE SOURCE_FILES

    #         main.cpp

    #         #header/*.h
    #         #source/*.cpp
    # )

    # add_executable(${PROJECT_NAME} ${SOURCE_FILES})

    # add_subdirectory(fun_engine_2d)

    # #target_link_libraries(${PROJECT_NAME} ${LUA_LIBRARIES})
    # target_link_libraries(${PROJECT_NAME} ImGui-SFML::ImGui-SFML)

elseif (WIN32)

    project(${PROJECT_NAME} LANGUAGES CXX)

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

    include_directories(header)
    include_directories(source)

    file (
        GLOB_RECURSE SOURCE_FILES

        main.cpp

        source/*.cpp
    )

    add_executable(${PROJECT_NAME} ${SOURCE_FILES})

    add_subdirectory(FunEngine2D)

elseif (APPLE)

    # ewww

endif()
