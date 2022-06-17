cmake_minimum_required(VERSION "3.19.2")

project("program")

# All files in source directory
file(GLOB_RECURSE SRC_DIR RELATIVE ${CMAKE_SOURCE_DIR} src/*.c)

# All files in include directory
file(GLOB_RECURSE INC_DIR RELATIVE ${CMAKE_SOURCE_DIR} include/*.h)

# Set output directories
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Set required packages
find_package(CURL REQUIRED)
find_package(cJSON REQUIRED)

# Specify executable
add_executable(${PROJECT_NAME} ${SRC_DIR} ${INC_DIR})

# Add flags to compilation
target_compile_options(${PROJECT_NAME} PRIVATE -g)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall)
target_compile_options(${PROJECT_NAME} PRIVATE -Werror)
target_compile_options(${PROJECT_NAME} PRIVATE -DLOG_USE_COLOR)

# Set include direcotires and required libraries
include_directories(${CMAKE_SOURCE_DIR}/include ${CURL_INCLUDE_DIR}
        ${CJSON_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${CURL_LIBRARIES} ${CJSON_LIBRARIES})
