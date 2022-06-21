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
target_compile_options(${PROJECT_NAME} PRIVATE -std=gnu11)
target_compile_options(${PROJECT_NAME} PRIVATE -g)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall)
target_compile_options(${PROJECT_NAME} PRIVATE -Werror)
target_compile_options(${PROJECT_NAME} PRIVATE -Wextra)
target_compile_options(${PROJECT_NAME} PRIVATE -Wstrict-prototypes)
target_compile_options(${PROJECT_NAME} PRIVATE -Wwrite-strings)
target_compile_options(${PROJECT_NAME} PRIVATE -Wshadow)
target_compile_options(${PROJECT_NAME} PRIVATE -Winit-self)
target_compile_options(${PROJECT_NAME} PRIVATE -Wcast-align)
target_compile_options(${PROJECT_NAME} PRIVATE -Wmissing-prototypes)
target_compile_options(${PROJECT_NAME} PRIVATE -Wstrict-overflow=2)
target_compile_options(${PROJECT_NAME} PRIVATE -Wcast-qual)
target_compile_options(${PROJECT_NAME} PRIVATE -Wundef)
target_compile_options(${PROJECT_NAME} PRIVATE -Wswitch-default)
target_compile_options(${PROJECT_NAME} PRIVATE -Wconversion)
target_compile_options(${PROJECT_NAME} PRIVATE -Wc++-compat)
target_compile_options(${PROJECT_NAME} PRIVATE -fstack-protector-strong)
target_compile_options(${PROJECT_NAME} PRIVATE -Wcomment)
target_compile_options(${PROJECT_NAME} PRIVATE -Wdouble-promotion)
target_compile_options(${PROJECT_NAME} PRIVATE -Wparentheses)
target_compile_options(${PROJECT_NAME} PRIVATE -Wunused-macros)
target_compile_options(${PROJECT_NAME} PRIVATE -Wmissing-declarations)
target_compile_options(${PROJECT_NAME} PRIVATE -Wswitch-enum)
target_compile_options(${PROJECT_NAME} PRIVATE -Wsign-conversion)

target_compile_options(${PROJECT_NAME} PRIVATE -DLOG_USE_COLOR)

# Set include direcotires and required libraries
include_directories(${CMAKE_SOURCE_DIR}/include ${CURL_INCLUDE_DIR}
        ${CJSON_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${CURL_LIBRARIES} ${CJSON_LIBRARIES})
