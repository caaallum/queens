cmake_minimum_required(VERSION 3.28)
project(Queens LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.2.28
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM
)
FetchContent_MakeAvailable(SDL3)

add_executable(${PROJECT_NAME} main.c grid.c grid.h level.c level.h vector.c vector.h vector2.h intset.h intset.c)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)

find_library(MATH_LIBRARY m)
if(MATH_LIBRARY)
    target_link_libraries(${PROJECT_NAME} PUBLIC ${MATH_LIBRARY})
    target_link_libraries(ref PUBLIC ${MATH_LIBRARY})
endif()

if(WIN32)
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:SDL3::SDL3>" "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    VERBATIM
)
endif()