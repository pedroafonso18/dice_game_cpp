cmake_minimum_required(VERSION 3.11)
project(Dinossaurineo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 4.5.0
)
FetchContent_MakeAvailable(raylib)

add_executable(Dinossaurineo src/main.cpp)
target_link_libraries(Dinossaurineo PRIVATE raylib) 