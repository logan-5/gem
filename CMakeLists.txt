cmake_minimum_required(VERSION 3.12)

project(gem)

find_package(PythonInterp 2.7 REQUIRED)

# TODO platform independent yada yada
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Weffc++ -Werror")

set(GENERATORS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/generators)
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(TOOLS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tools)
add_custom_target(generate_opcodes 
    COMMAND ${PYTHON_EXECUTABLE} ${TOOLS_DIR}/gen_opcodes.py ${GENERATORS_DIR}/opcode.py ${GENERATED_DIR}/opcodes.cpp ${GENERATED_DIR}/opcodes.cache)
set(SRC ${SRC} ${GENERATED_DIR}/opcodes.cpp)

add_custom_target(generate_bootstrap 
    COMMAND ${PYTHON_EXECUTABLE} ${TOOLS_DIR}/gen_bootstrap.py ${GENERATORS_DIR}/DMG_ROM.bin ${GENERATED_DIR}/bootstrap.cpp ${GENERATED_DIR}/bootstrap.cache)
set(SRC ${SRC} ${GENERATED_DIR}/bootstrap.cpp)

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)

set(SRC ${SRC} ${SRC_DIR}/cpu.cpp)
set(SRC ${SRC} ${SRC_DIR}/mem.cpp)
set(SRC ${SRC} ${SRC_DIR}/main.cpp)
set(SRC ${SRC} ${SRC_DIR}/opcode.cpp)

set(INCLUDE_DIRS ${INCLUDE_DIRS} ${SRC_DIR})
include_directories(${PROJECT_NAME} ${INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${SRC})
add_dependencies(${PROJECT_NAME} generate_opcodes)
add_dependencies(${PROJECT_NAME} generate_bootstrap)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
