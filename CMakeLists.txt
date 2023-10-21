cmake_minimum_required(VERSION 3.26)
project(PhysicsSim C)

set(CMAKE_C_STANDARD 23)

set(CMAKE_PREFIX_PATH  ${CMAKE_CURRENT_SOURCE_DIR}/FreeGLUT;${CMAKE_CURRENT_SOURCE_DIR}/glew)

find_package(FreeGLUT)
find_package(glew)

include_directories(include)

add_executable(PhysicsSim
        src/main.c
        src/physics.c
        src/graphics.c
)

target_link_libraries(PhysicsSim PRIVATE GLEW::glew)
target_link_libraries(PhysicsSim PRIVATE FreeGLUT::freeglut_static)
