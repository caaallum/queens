cmake_minimum_required(VERSION 3.28)
project(Queens LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)

file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})

file(GLOB SRC src/*.h src/*.c)

add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3 SDL3_image::SDL3_image)

find_library(MATH_LIBRARY m)
if(MATH_LIBRARY)
    target_link_libraries(${PROJECT_NAME} PUBLIC ${MATH_LIBRARY})
    target_link_libraries(ref PUBLIC ${MATH_LIBRARY})
endif()